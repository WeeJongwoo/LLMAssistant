#pragma once

#include "CoreMinimal.h"

// 비-스트리밍: 응답 완료 시 호출
DECLARE_DELEGATE_ThreeParams(FOnLLMResponseComplete, bool /*bSuccess*/, const FString& /*Response*/, const FString& /*Error*/);

// 스트리밍: 청크가 올 때마다 호출 (부분 텍스트)
DECLARE_DELEGATE_OneParam(FOnLLMStreamChunk, const FString& /*DeltaText*/);

// 스트리밍: 완료 시 호출
DECLARE_DELEGATE_TwoParams(FOnLLMStreamComplete, bool /*bSuccess*/, const FString& /*Error*/);

class ILLMProvider
{
public:
    virtual ~ILLMProvider() = default;

    /** 비-스트리밍 요청 */
    virtual void SendRequest(const FString& MessagesJson, FOnLLMResponseComplete OnComplete) = 0;

    /** 스트리밍 요청 — 청크 단위로 텍스트를 받음 */
    virtual void SendStreamingRequest(
        const FString& MessagesJson,
        FOnLLMStreamChunk OnChunk,
        FOnLLMStreamComplete OnComplete) = 0;

    virtual FString GetProviderName() const = 0;
};