#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKHUDWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKHUDWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="HUD") void UpdateMoney(int32 Amount);
    UFUNCTION(BlueprintCallable, Category="HUD") void UpdateWanted(int32 Level);
    UFUNCTION(BlueprintCallable, Category="HUD") void UpdateAmmo(int32 Current, int32 Max);

    UPROPERTY(meta=(BindWidget)) class UTextBlock* MoneyText;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* WantedText;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* AmmoText;
    UPROPERTY(meta=(BindWidget)) class UImage* WantedStars;
public:
    UPROPERTY(meta = (BindWidget)) class UTextBlock* MoneyText;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* WantedText;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* AmmoText;
    UFUNCTION(BlueprintCallable) void UpdateHUD(int32 Money, int32 Wanted, int32 Ammo);
};
