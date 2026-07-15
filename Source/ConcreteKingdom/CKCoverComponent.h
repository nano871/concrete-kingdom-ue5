#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKCoverComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKCoverComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKCoverComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick, FActorComponentTickFunction*) override;

    UFUNCTION(BlueprintCallable) void ToggleCover();
    UFUNCTION(BlueprintCallable) void BlindFire();
    UFUNCTION(BlueprintCallable) bool IsInCover() { return bInCover; }

private:
    bool bInCover;
    bool bBlindFiring;
    FVector CoverDirection;
    void FindCover();
};
