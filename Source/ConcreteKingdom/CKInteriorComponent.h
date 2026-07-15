#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKInteriorComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKInteriorComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKInteriorComponent();

    UFUNCTION(BlueprintCallable) void EnterInterior(FString InteriorType);
    UFUNCTION(BlueprintCallable) void ExitInterior();
    UFUNCTION(BlueprintCallable) bool IsInside() { return bInside; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector InteriorCameraLocation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector InteriorCameraLookAt;

private:
    bool bInside;
    FVector OutsideLocation;
    FRotator OutsideRotation;
};
