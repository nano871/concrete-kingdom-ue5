#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKReputationComponent.generated.h"

USTRUCT(BlueprintType)
struct FActionRecord {
    GENERATED_BODY()
    UPROPERTY() FString ActionType; // robbery, assault, murder, help, rescue
    UPROPERTY() FVector Location;
    UPROPERTY() float Time;
    UPROPERTY() int32 Severity; // 1-10
};

USTRUCT(BlueprintType)
struct FDistrictReputation {
    GENERATED_BODY()
    UPROPERTY() FString DistrictName;
    UPROPERTY() int32 Reputation; // -100 (hostile) to +100 (allied)
    UPROPERTY() TArray<FActionRecord> RecentActions;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKReputationComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKReputationComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick, FActorComponentTickFunction*) override;

    UFUNCTION(BlueprintCallable) void RecordAction(FString ActionType, int32 Severity, FString District);
    UFUNCTION(BlueprintCallable) int32 GetDistrictReputation(FString District);
    UFUNCTION(BlueprintCallable) FString GetDistrictReaction(FString District); // "hostile", "nervous", "neutral", "friendly", "allied"
    UFUNCTION(BlueprintCallable) TArray<FActionRecord> GetRecentActions(float WithinSeconds);

    UPROPERTY(BlueprintReadOnly) TMap<FString, FDistrictReputation> DistrictReputations;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MemoryDuration; // seconds until actions fade
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ActionDecayRate;

private:
    float GameTime;
    void DecayActions();
};
