#include "CKPhoneWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UCKPhoneWidget::OpenPhone() { bPhoneOpen = true; SetVisibility(ESlateVisibility::Visible); }
void UCKPhoneWidget::ClosePhone() { bPhoneOpen = false; SetVisibility(ESlateVisibility::Hidden); }

void UCKPhoneWidget::AddContact(FString Name, FString Status)
{
    Contacts.Add(Name);
    if (ContactList)
        ContactList->SetText(FText::FromString(FString::Join(Contacts, TEXT("\n"))));
}

void UCKPhoneWidget::ShowMissionBrief(FString Title, FString Desc)
{
    UE_LOG(LogTemp, Warning, TEXT("Mission Brief: %s - %s"), *Title, *Desc);
}
