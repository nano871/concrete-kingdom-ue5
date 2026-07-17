#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKWeaponWheelWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKWeaponWheelWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void OpenWheel();
    UFUNCTION(BlueprintCallable) void CloseWheel();
    UFUNCTION(BlueprintCallable) void SelectSlot(int32 Slot); // 1-4
    UFUNCTION(BlueprintCallable) FString GetSelectedWeapon();
    UFUNCTION(BlueprintCallable) bool IsOpen() { return bIsOpen; }

    UPROPERTY(BlueprintReadOnly) int32 SelectedSlot;
    UPROPERTY(BlueprintReadOnly) TArray<FString> AvailableWeapons;
private:
    bool bIsOpen;
};
