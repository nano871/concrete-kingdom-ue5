#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKMinimapWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKMinimapWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Minimap")
    void UpdateMinimap(const FVector& PlayerLocation, float PlayerYaw);

    UPROPERTY(meta=(BindWidget)) class UImage* MapImage;
    UPROPERTY(meta=(BindWidget)) class UImage* PlayerArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WorldSize;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinimapScale;
};
