#include "CKPhoneScreenWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/Border.h"

void UCKPhoneScreenWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    if (!PhoneBackground)
    {
        UCanvasPanel* Root = NewObject<UCanvasPanel>(this);
        WidgetTree->RootWidget = Root;
        PhoneBackground = NewObject<UImage>(this);
        if (PhoneBackground) Root->AddChild(PhoneBackground);
    }
    bIsOpen = false;
    SetVisibility(ESlateVisibility::Hidden);
}

void UCKPhoneScreenWidget::OpenPhone() { bIsOpen = true; SetVisibility(ESlateVisibility::Visible); }
void UCKPhoneScreenWidget::ClosePhone() { bIsOpen = false; SetVisibility(ESlateVisibility::Hidden); }

void UCKPhoneScreenWidget::SelectContact(int32 Index)
{
    TArray<FString> Contacts = {TEXT("Simeon"), TEXT("Gerald"), TEXT("Mallorie")};
    if (Contacts.IsValidIndex(Index))
        UE_LOG(LogTemp, Warning, TEXT("[PHONE] Selected contact: %s"), *Contacts[Index]);
}

void UCKPhoneScreenWidget::ShowMessage(FString Sender, FString Message)
{
    UE_LOG(LogTemp, Warning, TEXT("[PHONE] %s: %s"), *Sender, *Message);
    // Build a dynamic text block to display the message
    UCanvasPanel* Panel = Cast<UCanvasPanel>(GetRootWidget());
    if (!Panel) return;
    UTextBlock* MsgText = NewObject<UTextBlock>(this);
    if (MsgText)
    {
        MsgText->SetText(FText::FromString(Sender + TEXT(": ") + Message));
        MsgText->Font.Size = 14;
        MsgText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        Panel->AddChild(MsgText);
    }
}

void UCKPhoneScreenWidget::ShowMissionBrief(FString Sender, FString Message)
{
    UE_LOG(LogTemp, Warning, TEXT("[PHONE] MISSION BRIEF from %s: %s"), *Sender, *Message);
    // Add a highlighted border + text for mission briefs
    UCanvasPanel* Panel = Cast<UCanvasPanel>(GetRootWidget());
    if (!Panel) return;
    UBorder* BriefBorder = NewObject<UBorder>(this);
    UTextBlock* BriefText = NewObject<UTextBlock>(this);
    if (BriefBorder && BriefText)
    {
        BriefText->SetText(FText::FromString(TEXT("[MISSION] ") + Sender + TEXT(": ") + Message));
        BriefText->Font.Size = 16;
        BriefText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
        BriefBorder->SetContent(BriefText);
        BriefBorder->SetBrushColor(FLinearColor(0.1f, 0.05f, 0.0f, 0.8f));
        Panel->AddChild(BriefBorder);
    }
}
