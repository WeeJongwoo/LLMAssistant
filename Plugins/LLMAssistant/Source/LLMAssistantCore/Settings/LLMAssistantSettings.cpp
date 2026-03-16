
#include "LLMAssistantSettings.h"

ULLMAssistantSettings::ULLMAssistantSettings()
{
    Provider = ELLMProvider::Groq_Llama33_70B;
    EndpointURL = TEXT("https://api.groq.com/openai/v1/chat/completions");
    ModelName = TEXT("llama-3.3-70b-versatile");
    MaxTokens = 4096;

    // 환경변수 폴백
    FString EnvKey = FPlatformMisc::GetEnvironmentVariable(TEXT("LLM_API_KEY"));
    if (!EnvKey.IsEmpty())
    {
        APIKey = EnvKey;
    }
}

FString ULLMAssistantSettings::GetResolvedEndpointURL() const
{
    switch (Provider)
    {
    case ELLMProvider::Groq_Llama33_70B:
    case ELLMProvider::Groq_Llama31_8B:
        return TEXT("https://api.groq.com/openai/v1/chat/completions");

    case ELLMProvider::Gemini_20_Flash:
    case ELLMProvider::Gemini_20_FlashLite:
        return TEXT("https://generativelanguage.googleapis.com/v1beta/openai/chat/completions");

    case ELLMProvider::OpenAI_GPT4o:
    case ELLMProvider::OpenAI_GPT4oMini:
        return TEXT("https://api.openai.com/v1/chat/completions");

    case ELLMProvider::Custom:
    default:
        return EndpointURL;
    }
}

FString ULLMAssistantSettings::GetResolvedModelName() const
{
    switch (Provider)
    {
    case ELLMProvider::Groq_Llama33_70B:
        return TEXT("llama-3.3-70b-versatile");
    case ELLMProvider::Groq_Llama31_8B:
        return TEXT("llama-3.1-8b-instant");
    case ELLMProvider::Gemini_20_Flash:
        return TEXT("gemini-2.0-flash");
    case ELLMProvider::Gemini_20_FlashLite:
        return TEXT("gemini-2.0-flash-lite");
    case ELLMProvider::OpenAI_GPT4o:
        return TEXT("gpt-4o");
    case ELLMProvider::OpenAI_GPT4oMini:
        return TEXT("gpt-4o-mini");
    case ELLMProvider::Custom:
    default:
        return ModelName;
    }
}

void ULLMAssistantSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // 프로바이더 드롭다운 변경 시 URL/모델 필드를 자동 갱신
    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ULLMAssistantSettings, Provider))
    {
        if (Provider != ELLMProvider::Custom)
        {
            EndpointURL = GetResolvedEndpointURL();
            ModelName = GetResolvedModelName();
            SaveConfig();
        }
    }
}