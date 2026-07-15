#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKFactionComponent.generated.h"

USTRUCT(BlueprintType)
struct FFactionStanding {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FString FactionName;
    UPROPERTY(BlueprintReadWrite) int32 Reputation;
    UPROPERTY(BlueprintReadWrite) FString Status; // hostile, neutral, friendly, allied
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKFactionComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKFactionComponent();
    UFUNCTION(BlueprintCallable) void InitFactions();
    UFUNCTION(BlueprintCallable) void ModifyReputation(FString FactionName, int32 Amount);
    UFUNCTION(BlueprintCallable) FFactionStanding GetFaction(FString FactionName);
    UFUNCTION(BlueprintCallable) TArray<FString> GetHostileFactions();

    UPROPERTY(BlueprintReadOnly) TMap<FString, FFactionStanding> Factions;
};
