#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "CKPedestrianAIController.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKPedestrianAIController : public AAIController {
    GENERATED_BODY()
public:
    ACKPedestrianAIController();
    virtual void Tick(float DeltaTime) override;
    UFUNCTION(BlueprintCallable) void SetPedestrianType(FString Type);
    UFUNCTION(BlueprintCallable) void ReactToPlayer(float PlayerSpeed, float Distance);
protected:
    virtual void BeginPlay() override;
    void Wander();
    FVector WanderOrigin;
    FTimerHandle WanderTimer;
    FString PedType;
    float BaseSpeed;
    bool bFleeing;
    FVector FleeDirection;
};
