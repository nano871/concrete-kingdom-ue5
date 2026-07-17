#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKCoverIndicatorWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKCoverIndicatorWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void SetInCover(bool bCovered);
    UFUNCTION(BlueprintCallable) void SetBlindFire(bool bFiring);
    UPROPERTY(meta=(BindWidget)) class UImage* CoverIcon;
    UPROPERTY(BlueprintReadOnly) bool bInCover;
private:
    bool bBlindFiring;
};
