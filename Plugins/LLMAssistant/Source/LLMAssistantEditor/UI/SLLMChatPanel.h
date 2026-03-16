#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SScrollBox;
class SMultiLineEditableTextBox;
class SButton;
class STextBlock;
class FLLMService;

class SLLMChatPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLLMChatPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    FReply OnSendClicked();
    void AddChatBubble(const FString& Message, bool bIsUser);
    void SetInputEnabled(bool bEnabled);

    // 스트리밍 콜백
    void OnStreamChunk(const FString& DeltaText);
    void OnStreamComplete(bool bSuccess, const FString& Error);

    TSharedPtr<SScrollBox> ChatScrollBox;
    TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
    TSharedPtr<SButton> SendButton;
    TSharedPtr<FLLMService> LLMService;

    // 스트리밍 중 AI 응답 버블의 텍스트 블록 (실시간 갱신용)
    TSharedPtr<STextBlock> StreamingTextBlock;
    FString StreamingFullText;
};