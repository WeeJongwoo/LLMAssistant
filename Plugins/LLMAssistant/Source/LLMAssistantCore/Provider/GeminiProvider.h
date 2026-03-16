#pragma once

#include "CoreMinimal.h"
#include "ILLMProvider.h"

class FGeminiProvider : public ILLMProvider
{
public:
    virtual void SendRequest(const FString& MessagesJson, FOnLLMResponseComplete OnComplete) override;

    virtual void SendStreamingRequest(
        const FString& MessagesJson,
        FOnLLMStreamChunk OnChunk,
        FOnLLMStreamComplete OnComplete) override;

    virtual FString GetProviderName() const override { return TEXT("Gemini"); }

private:
    // SSE 파싱 시 이전 청크에서 잘린 데이터를 보관하는 버퍼
    FString SSEBuffer;
};