#include "SLLMChatPanel.h"
#include "LLMService.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"

void SLLMChatPanel::Construct(const FArguments& InArgs)
{
    LLMService = MakeShared<FLLMService>();

    ChildSlot
        [
            SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .Padding(4.0f)
                [
                    SAssignNew(ChatScrollBox, SScrollBox)
                        + SScrollBox::Slot()
                        [
                            SNew(STextBlock)
                                .Text(FText::FromString(TEXT("LLM Assistant에 오신 걸 환영합니다. 메시지를 입력하세요.")))
                                .ColorAndOpacity(FSlateColor(FLinearColor::Gray))
                                .AutoWrapText(true)
                        ]
                ]

            + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f)
                [
                    SNew(SHorizontalBox)

                        + SHorizontalBox::Slot()
                        .FillWidth(1.0f)
                        .Padding(0.0f, 0.0f, 4.0f, 0.0f)
                        [
                            SNew(SBox)
                                .MinDesiredHeight(60.0f)
                                .MaxDesiredHeight(120.0f)
                                [
                                    SAssignNew(InputTextBox, SMultiLineEditableTextBox)
                                        .AutoWrapText(true)
                                ]
                        ]

                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Bottom)
                        [
                            SAssignNew(SendButton, SButton)
                                .Text(FText::FromString(TEXT("Send")))
                                .OnClicked(this, &SLLMChatPanel::OnSendClicked)
                        ]
                ]
        ];
}

FReply SLLMChatPanel::OnSendClicked()
{
    if (!InputTextBox.IsValid() || !LLMService.IsValid()) return FReply::Handled();
    if (LLMService->IsRequestInProgress()) return FReply::Handled();

    FString UserMessage = InputTextBox->GetText().ToString();
    UserMessage.TrimStartAndEndInline();
    if (UserMessage.IsEmpty()) return FReply::Handled();

    // 유저 메시지 표시
    AddChatBubble(UserMessage, true);
    InputTextBox->SetText(FText::GetEmpty());
    SetInputEnabled(false);

    // 스트리밍용 AI 버블을 미리 생성 (빈 상태)
    StreamingFullText.Empty();
    StreamingTextBlock.Reset();

    ChatScrollBox->AddSlot()
        .Padding(FMargin(4.0f, 4.0f, 40.0f, 4.0f))
        [
            SNew(SBorder)
                .BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
                .Padding(8.0f)
                [
                    SAssignNew(StreamingTextBlock, STextBlock)
                        .Text(FText::FromString(TEXT("[AI] ...")))
                        .AutoWrapText(true)
                        .ColorAndOpacity(FSlateColor(FLinearColor::White))
                ]
        ];
    ChatScrollBox->ScrollToEnd();

    // 스트리밍 요청
    LLMService->SendMessageStreaming(
        UserMessage,
        FOnChatStreamChunk::CreateSP(this, &SLLMChatPanel::OnStreamChunk),
        FOnChatStreamComplete::CreateSP(this, &SLLMChatPanel::OnStreamComplete));

    return FReply::Handled();
}

void SLLMChatPanel::OnStreamChunk(const FString& DeltaText)
{
    // 텍스트 조각이 올 때마다 누적하여 표시
    StreamingFullText += DeltaText;

    if (StreamingTextBlock.IsValid())
    {
        StreamingTextBlock->SetText(
            FText::FromString(TEXT("[AI] ") + StreamingFullText));
    }

    ChatScrollBox->ScrollToEnd();
}

void SLLMChatPanel::OnStreamComplete(bool bSuccess, const FString& Error)
{
    if (!bSuccess)
    {
        // 에러 시 스트리밍 버블 텍스트를 에러로 교체
        if (StreamingTextBlock.IsValid())
        {
            StreamingTextBlock->SetText(
                FText::FromString(FString::Printf(TEXT("[오류] %s"), *Error)));
        }
    }

    // 참조 해제
    StreamingTextBlock.Reset();
    StreamingFullText.Empty();

    SetInputEnabled(true);
}

void SLLMChatPanel::SetInputEnabled(bool bEnabled)
{
    if (InputTextBox.IsValid()) InputTextBox->SetEnabled(bEnabled);
    if (SendButton.IsValid()) SendButton->SetEnabled(bEnabled);
}

void SLLMChatPanel::AddChatBubble(const FString& Message, bool bIsUser)
{
    FLinearColor BubbleColor = bIsUser
        ? FLinearColor(0.2f, 0.4f, 0.8f, 1.0f)
        : FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

    FString Prefix = bIsUser ? TEXT("[You] ") : TEXT("[AI] ");

    ChatScrollBox->AddSlot()
        .Padding(FMargin(bIsUser ? 40.0f : 4.0f, 4.0f, bIsUser ? 4.0f : 40.0f, 4.0f))
        [
            SNew(SBorder)
                .BorderBackgroundColor(BubbleColor)
                .Padding(8.0f)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(Prefix + Message))
                        .AutoWrapText(true)
                        .ColorAndOpacity(FSlateColor(FLinearColor::White))
                ]
        ];

    ChatScrollBox->ScrollToEnd();
}