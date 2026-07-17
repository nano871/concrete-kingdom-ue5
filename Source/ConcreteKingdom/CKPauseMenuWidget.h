#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKPauseMenuWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKPauseMenuWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void OpenPauseMenu();
    UFUNCTION(BlueprintCallable) void ClosePauseMenu();
    UFUNCTION(BlueprintCallable) void SaveGame();
    UFUNCTION(BlueprintCallable) void LoadGame();
    UFUNCTION(BlueprintCallable) void QuitToMainMenu();
    UFUNCTION(BlueprintCallable) bool IsPaused() { return bIsPaused; }

    UPROPERTY(meta=(BindWidget)) class UTextBlock* MoneyDisplay;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* TimeDisplay;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* WantedDisplay;
private:
    bool bIsPaused;
};
