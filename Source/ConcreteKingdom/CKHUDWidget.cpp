#include "CKHUDWidget.h"
#include "Components/TextBlock.h"
void UCKHUDWidget::UpdateHUD(int32 Money, int32 Wanted, int32 Ammo) {
    if (MoneyText) MoneyText->SetText(FText::FromString(FString::Printf(TEXT("$%d"), Money)));
    if (WantedText) WantedText->SetText(FText::FromString(FString::Printf(TEXT("WANTED: %d"), Wanted)));
    if (AmmoText) AmmoText->SetText(FText::FromString(FString::Printf(TEXT("AMMO: %d"), Ammo)));
}
