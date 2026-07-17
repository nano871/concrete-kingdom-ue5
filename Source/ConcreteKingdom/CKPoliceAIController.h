#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "CKHelicopterActor.h"
#include "CKRoadblockActor.h"
#include "CKPoliceAIController.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKPoliceAIController : public AAIController {
    GENERATED_BODY()
public:
    ACKPoliceAIController();
    virtual void Tick(float DeltaTime) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PatrolRadius;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ChaseSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 WantedLevel;
    UFUNCTION(BlueprintCallable) void SetWantedLevel(int32 Level);
    UFUNCTION(BlueprintCallable) void ResetChase();
protected:
    virtual void BeginPlay() override;
    void Patrol();
    void Chase();
    FTimerHandle PatrolTimerHandle;
    FVector PatrolOrigin;
    bool bHelicopterSpawned;
    bool bRoadblockSpawned;
};
