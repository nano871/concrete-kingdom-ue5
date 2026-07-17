#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKHelicopterActor.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKHelicopterActor : public AActor {
    GENERATED_BODY()
public:
    ACKHelicopterActor();
    virtual void Tick(float DeltaTime) override;
    UFUNCTION(BlueprintCallable) void Deploy(FVector TargetLocation);
    UPROPERTY() class USpotLightComponent* SearchLight;
    FVector Target;
    float Speed;
    bool bActive;
};
