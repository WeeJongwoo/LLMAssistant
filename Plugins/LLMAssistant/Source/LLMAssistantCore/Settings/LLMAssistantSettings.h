#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LLMAssistantSettings.generated.h"

/** 지원하는 LLM 프로바이더 목록 */
UENUM()
enum class ELLMProvider : uint8
{
    Groq_Llama33_70B        UMETA(DisplayName = "Groq - Llama 3.3 70B (무료, 추천)"),
    Groq_Llama31_8B         UMETA(DisplayName = "Groq - Llama 3.1 8B (무료, 빠름)"),
    Gemini_20_Flash         UMETA(DisplayName = "Gemini - 2.0 Flash (무료)"),
    Gemini_20_FlashLite     UMETA(DisplayName = "Gemini - 2.0 Flash-Lite (무료)"),
    OpenAI_GPT4o            UMETA(DisplayName = "OpenAI - GPT-4o (유료)"),
    OpenAI_GPT4oMini        UMETA(DisplayName = "OpenAI - GPT-4o Mini (유료)"),
    Custom                  UMETA(DisplayName = "직접 입력 (Custom)")
};

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "LLM Assistant"))
class LLMASSISTANTCORE_API ULLMAssistantSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    ULLMAssistantSettings();

    // ── 프로바이더 선택 (드롭다운) ──

    /** LLM 프로바이더 선택 */
    UPROPERTY(config, EditAnywhere, Category = "Provider",
        meta = (DisplayName = "LLM Provider"))
    ELLMProvider Provider;

    // ── API 설정 ──

    /** API 키 */
    UPROPERTY(config, EditAnywhere, Category = "API",
        meta = (DisplayName = "API Key", PasswordField = true))
    FString APIKey;

    /** API 엔드포인트 URL (Custom 선택 시에만 직접 입력) */
    UPROPERTY(config, EditAnywhere, Category = "API",
        meta = (DisplayName = "Endpoint URL", EditCondition = "Provider == ELLMProvider::Custom"))
    FString EndpointURL;

    /** 모델 이름 (Custom 선택 시에만 직접 입력) */
    UPROPERTY(config, EditAnywhere, Category = "API",
        meta = (DisplayName = "Model Name", EditCondition = "Provider == ELLMProvider::Custom"))
    FString ModelName;

    /** 응답 최대 토큰 수 */
    UPROPERTY(config, EditAnywhere, Category = "API",
        meta = (DisplayName = "Max Tokens", ClampMin = "1", ClampMax = "32768"))
    int32 MaxTokens;

    // ── 헬퍼 ──

    /** 프로바이더 enum에 맞는 실제 URL/모델명을 반환 */
    FString GetResolvedEndpointURL() const;
    FString GetResolvedModelName() const;

    static const ULLMAssistantSettings* Get()
    {
        return GetDefault<ULLMAssistantSettings>();
    }

    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
    virtual FName GetSectionName() const override { return TEXT("LLMAssistant"); }

    // 프로바이더 변경 시 URL/모델 자동 갱신
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};