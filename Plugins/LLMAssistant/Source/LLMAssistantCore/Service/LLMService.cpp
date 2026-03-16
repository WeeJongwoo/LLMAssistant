#include "LLMService.h"
#include "GeminiProvider.h"

FLLMService::FLLMService()
{
    Provider = MakeUnique<FGeminiProvider>();
}

FLLMService::~FLLMService() = default;

// ── 비-스트리밍 (기존 그대로) ──

void FLLMService::SendMessage(const FString& UserMessage, FOnChatResponseReady OnReady)
{
    if (bIsRequesting)
    {
        OnReady.ExecuteIfBound(false, TEXT(""), TEXT("이전 요청이 진행 중입니다."));
        return;
    }

    ConversationHistory.Add(FLLMMessage(TEXT("user"), UserMessage));
    FString MessagesJson = ConvertHistoryToJson();
    bIsRequesting = true;

    Provider->SendRequest(MessagesJson,
        FOnLLMResponseComplete::CreateLambda(
            [this, OnReady](bool bSuccess, const FString& Response, const FString& Error)
            {
                bIsRequesting = false;
                if (bSuccess)
                {
                    ConversationHistory.Add(FLLMMessage(TEXT("assistant"), Response));
                }
                OnReady.ExecuteIfBound(bSuccess, Response, Error);
            }));
}

// ── 스트리밍 ──

void FLLMService::SendMessageStreaming(
    const FString& UserMessage,
    FOnChatStreamChunk OnChunk,
    FOnChatStreamComplete OnComplete)
{
    if (bIsRequesting)
    {
        OnComplete.ExecuteIfBound(false, TEXT("이전 요청이 진행 중입니다."));
        return;
    }

    ConversationHistory.Add(FLLMMessage(TEXT("user"), UserMessage));
    FString MessagesJson = ConvertHistoryToJson();
    bIsRequesting = true;
    StreamingResponseBuffer.Empty();

    Provider->SendStreamingRequest(
        MessagesJson,

        // 청크 수신: UI에 전달 + 버퍼에 누적
        FOnLLMStreamChunk::CreateLambda(
            [this, OnChunk](const FString& DeltaText)
            {
                StreamingResponseBuffer += DeltaText;
                OnChunk.ExecuteIfBound(DeltaText);
            }),

        // 완료: 누적된 전체 응답을 히스토리에 추가
        FOnLLMStreamComplete::CreateLambda(
            [this, OnComplete](bool bSuccess, const FString& Error)
            {
                bIsRequesting = false;
                if (bSuccess && !StreamingResponseBuffer.IsEmpty())
                {
                    ConversationHistory.Add(
                        FLLMMessage(TEXT("assistant"), StreamingResponseBuffer));
                }
                StreamingResponseBuffer.Empty();
                OnComplete.ExecuteIfBound(bSuccess, Error);
            }));
}

void FLLMService::ClearHistory()
{
    ConversationHistory.Empty();
}

FString FLLMService::ConvertHistoryToJson() const
{
    FString Json = TEXT("[");
    for (int32 i = 0; i < ConversationHistory.Num(); ++i)
    {
        if (i > 0) Json += TEXT(",");
        FString EscapedContent = ConversationHistory[i].Content;
        EscapedContent.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
        EscapedContent.ReplaceInline(TEXT("\""), TEXT("\\\""));
        EscapedContent.ReplaceInline(TEXT("\n"), TEXT("\\n"));
        EscapedContent.ReplaceInline(TEXT("\r"), TEXT("\\r"));
        EscapedContent.ReplaceInline(TEXT("\t"), TEXT("\\t"));
        Json += FString::Printf(TEXT("{\"role\":\"%s\",\"content\":\"%s\"}"),
            *ConversationHistory[i].Role, *EscapedContent);
    }
    Json += TEXT("]");
    return Json;
}