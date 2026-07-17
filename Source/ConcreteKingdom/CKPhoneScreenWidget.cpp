#include "CKPhoneScreenWidget.h"
void UCKPhoneScreenWidget::OpenPhone() { bIsOpen = true; SetVisibility(ESlateVisibility::Visible); }
void UCKPhoneScreenWidget::ClosePhone() { bIsOpen = false; SetVisibility(ESlateVisibility::Hidden); }
void UCKPhoneScreenWidget::ShowMissionBrief(FString Sender, FString Message)
{
    UE_LOG(LogTemp, Warning, TEXT("MESSAGE from %s: %s"), *Sender, *Message);
}
void UCKPhoneScreenWidget::SelectContact(int32 Index) { UE_LOG(LogTemp, Warning, TEXT("Selected contact %d"), Index); }
void UCKPhoneScreenWidget::ShowMessage(FString Sender, FString Message) { UE_LOG(LogTemp, Warning, TEXT("SMS from %s: %s"), *Sender, *Message); }
