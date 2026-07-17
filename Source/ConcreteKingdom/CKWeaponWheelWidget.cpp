#include "CKWeaponWheelWidget.h"
#include "Components/Image.h"

void UCKWeaponWheelWidget::OpenWheel() { bIsOpen = true; SetVisibility(ESlateVisibility::Visible); }
void UCKWeaponWheelWidget::CloseWheel() { bIsOpen = false; SetVisibility(ESlateVisibility::Hidden); }

void UCKWeaponWheelWidget::SelectSlot(int32 Slot)
{
    if (Slot < 1 || Slot > AvailableWeapons.Num()) return;
    SelectedSlot = Slot;
}

FString UCKWeaponWheelWidget::GetSelectedWeapon()
{
    return AvailableWeapons.IsValidIndex(SelectedSlot - 1) ? AvailableWeapons[SelectedSlot - 1] : TEXT("");
}
