// HUD implementation - money, wanted, ammo display
#include "CKHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UCKHUDWidget::UpdateMoney(int32 Amount)
{
    if (MoneyText)
        MoneyText->SetText(FText::Format(NSLOCTEXT("CK", "Money", "${0}"), FText::AsNumber(Amount)));
}

void UCKHUDWidget::UpdateWanted(int32 Level)
{
    if (WantedText)
    {
        FString Stars;
        for (int32 i = 0; i < Level; i++) Stars += TEXT("★");
        for (int32 i = Level; i < 5; i++) Stars += TEXT("☆");
        WantedText->SetText(FText::FromString(Stars));
        WantedText->SetColorAndOpacity(Level > 2 ? FLinearColor::Red : FLinearColor::White);
    }
}

void UCKHUDWidget::UpdateAmmo(int32 Current, int32 Max)
{
    if (AmmoText)
        AmmoText->SetText(FText::Format(NSLOCTEXT("CK", "Ammo", "{0}/{1}"), FText::AsNumber(Current), FText::AsNumber(Max)));
}
