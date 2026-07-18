#include "CKPhoneScreenWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"

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
}
void UCKPhoneScreenWidget::ShowMissionBrief(FString Sender, FString Message)
{
    UE_LOG(LogTemp, Warning, TEXT("[PHONE] MISSION BRIEF from %s: %s"), *Sender, *Message);
}
