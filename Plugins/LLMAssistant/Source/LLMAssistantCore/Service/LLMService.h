#pragma once

#include "CoreMinimal.h"
#include "LLMMessage.h"
#include "ILLMProvider.h"

DECLARE_DELEGATE_ThreeParams(FOnChatResponseReady, bool, const FString&, const FString&);

// 스트리밍용 콜백
DECLARE_DELEGATE_OneParam(FOnChatStreamChunk, const FString& /*DeltaText*/);
DECLARE_DELEGATE_TwoParams(FOnChatStreamComplete, bool /*bSuccess*/, const FString& /*Error*/);

class LLMASSISTANTCORE_API FLLMService
{
public:
    FLLMService();
    ~FLLMService();

    /** 비-스트리밍 전송 */
    void SendMessage(const FString& UserMessage, FOnChatResponseReady OnReady);

    /** 스트리밍 전송 */
    void SendMessageStreaming(
        const FString& UserMessage,
        FOnChatStreamChunk OnChunk,
        FOnChatStreamComplete OnComplete);

    void ClearHistory();
    bool IsRequestInProgress() const { return bIsRequesting; }

private:
    FString ConvertHistoryToJson() const;

    TArray<FLLMMessage> ConversationHistory;
    TUniquePtr<ILLMProvider> Provider;
    bool bIsRequesting = false;

    // 스트리밍 중 AI 응답을 누적하기 위한 버퍼
    FString StreamingResponseBuffer;
};