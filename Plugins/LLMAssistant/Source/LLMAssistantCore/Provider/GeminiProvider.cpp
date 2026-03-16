#include "GeminiProvider.h"
#include "LLMAssistantSettings.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

// ── 비-스트리밍 (기존 코드 그대로) ──

void FGeminiProvider::SendRequest(const FString& MessagesJson, FOnLLMResponseComplete OnComplete)
{
    const ULLMAssistantSettings* Settings = ULLMAssistantSettings::Get();

    if (Settings->APIKey.IsEmpty())
    {
        OnComplete.ExecuteIfBound(false, TEXT(""), TEXT("API 키가 설정되지 않았습니다."));
        return;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Settings->EndpointURL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"),
        FString::Printf(TEXT("Bearer %s"), *Settings->APIKey));

    FString RequestBody = FString::Printf(
        TEXT("{\"model\":\"%s\",\"max_tokens\":%d,\"messages\":%s}"),
        *Settings->ModelName, Settings->MaxTokens, *MessagesJson);

    Request->SetContentAsString(RequestBody);

    Request->OnProcessRequestComplete().BindLambda(
        [OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConnected)
        {
            if (!bConnected || !Resp.IsValid())
            {
                OnComplete.ExecuteIfBound(false, TEXT(""), TEXT("네트워크 연결 실패"));
                return;
            }
            if (Resp->GetResponseCode() != 200)
            {
                OnComplete.ExecuteIfBound(false, TEXT(""),
                    FString::Printf(TEXT("API 오류 (HTTP %d): %s"),
                        Resp->GetResponseCode(), *Resp->GetContentAsString()));
                return;
            }

            TSharedPtr<FJsonObject> Json;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());
            if (FJsonSerializer::Deserialize(Reader, Json))
            {
                const TArray<TSharedPtr<FJsonValue>>* Choices;
                if (Json->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
                {
                    FString Content = (*Choices)[0]->AsObject()
                        ->GetObjectField(TEXT("message"))
                        ->GetStringField(TEXT("content"));
                    OnComplete.ExecuteIfBound(true, Content, TEXT(""));
                    return;
                }
            }
            OnComplete.ExecuteIfBound(false, TEXT(""), TEXT("응답 파싱 실패"));
        });

    Request->ProcessRequest();
}

// ── 스트리밍 ──

void FGeminiProvider::SendStreamingRequest(
    const FString& MessagesJson,
    FOnLLMStreamChunk OnChunk,
    FOnLLMStreamComplete OnComplete)
{
    const ULLMAssistantSettings* Settings = ULLMAssistantSettings::Get();

    if (Settings->APIKey.IsEmpty())
    {
        OnComplete.ExecuteIfBound(false, TEXT("API 키가 설정되지 않았습니다."));
        return;
    }

    // SSE 버퍼 초기화
    SSEBuffer.Empty();

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Settings->EndpointURL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"),
        FString::Printf(TEXT("Bearer %s"), *Settings->APIKey));

    // stream: true 추가
    FString RequestBody = FString::Printf(
        TEXT("{\"model\":\"%s\",\"max_tokens\":%d,\"stream\":true,\"messages\":%s}"),
        *Settings->ModelName, Settings->MaxTokens, *MessagesJson);

    Request->SetContentAsString(RequestBody);

    // ── 스트리밍 청크 수신 콜백 ──
    // OnRequestProgress는 데이터가 수신될 때마다 호출됨
    Request->OnRequestProgress().BindLambda(
        [this, OnChunk](FHttpRequestPtr Req, int32 BytesSent, int32 BytesReceived)
        {
            // 현재까지 수신된 전체 데이터
            FHttpResponsePtr Resp = Req->GetResponse();
            if (!Resp.IsValid()) return;

            FString FullContent = Resp->GetContentAsString();

            // 이전에 처리한 부분 이후의 새 데이터만 추출
            FString NewData = FullContent.Mid(SSEBuffer.Len());
            if (NewData.IsEmpty()) return;

            // SSE 버퍼에 추가
            SSEBuffer = FullContent;

            // SSE 포맷 파싱: "data: {...}\n\n" 단위로 처리
            TArray<FString> Lines;
            NewData.ParseIntoArray(Lines, TEXT("\n"), false);

            for (const FString& Line : Lines)
            {
                FString TrimmedLine = Line.TrimStartAndEnd();

                // 빈 줄이나 주석 무시
                if (TrimmedLine.IsEmpty() || TrimmedLine.StartsWith(TEXT(":")))
                {
                    continue;
                }

                // "data: [DONE]" = 스트리밍 종료 신호
                if (TrimmedLine == TEXT("data: [DONE]"))
                {
                    continue;
                }

                // "data: {...}" 에서 JSON 추출
                if (TrimmedLine.StartsWith(TEXT("data: ")))
                {
                    FString JsonStr = TrimmedLine.Mid(6); // "data: " 이후

                    TSharedPtr<FJsonObject> ChunkJson;
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);

                    if (FJsonSerializer::Deserialize(Reader, ChunkJson) && ChunkJson.IsValid())
                    {
                        // choices[0].delta.content 추출
                        const TArray<TSharedPtr<FJsonValue>>* Choices;
                        if (ChunkJson->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
                        {
                            TSharedPtr<FJsonObject> Delta =
                                (*Choices)[0]->AsObject()->GetObjectField(TEXT("delta"));

                            if (Delta.IsValid())
                            {
                                FString DeltaContent;
                                if (Delta->TryGetStringField(TEXT("content"), DeltaContent))
                                {
                                    // UI에 텍스트 조각 전달
                                    OnChunk.ExecuteIfBound(DeltaContent);
                                }
                            }
                        }
                    }
                }
            }
        });

    // ── 요청 완료 콜백 ──
    Request->OnProcessRequestComplete().BindLambda(
        [this, OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConnected)
        {
            SSEBuffer.Empty();

            if (!bConnected || !Resp.IsValid())
            {
                OnComplete.ExecuteIfBound(false, TEXT("네트워크 연결 실패"));
                return;
            }
            if (Resp->GetResponseCode() != 200)
            {
                OnComplete.ExecuteIfBound(false,
                    FString::Printf(TEXT("API 오류 (HTTP %d)"), Resp->GetResponseCode()));
                return;
            }

            OnComplete.ExecuteIfBound(true, TEXT(""));
        });

    Request->ProcessRequest();
}