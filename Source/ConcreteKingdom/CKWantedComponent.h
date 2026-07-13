#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKWantedComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKWantedComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKWantedComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly) int32 WantedLevel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float StarProgress;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeatDecayTimer;

    UFUNCTION(BlueprintCallable) void AddHeat(float Amount);
    UFUNCTION(BlueprintCallable) void SetWantedLevel(int32 Level);
    UFUNCTION(BlueprintCallable) int32 GetSpawnCap();
    UFUNCTION(BlueprintCallable) float GetSearchRadius();
    UFUNCTION(BlueprintCallable) bool ShouldSpawnHelicopter();
    UFUNCTION(BlueprintCallable) bool ShouldSpawnRoadblock();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> SpawnCaps;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> SearchRadii;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> DecayTimes;
};
