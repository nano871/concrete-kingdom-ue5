#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKFactionWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKFactionWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void ShowFactions();
    UFUNCTION(BlueprintCallable) void HideFactions();
    UFUNCTION(BlueprintCallable) bool IsVisible() { return bVisible; }
private:
    bool bVisible;
};
