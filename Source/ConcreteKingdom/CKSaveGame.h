#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CKSaveGame.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKSaveGame : public USaveGame {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite) int32 Money;
    UPROPERTY(BlueprintReadWrite) TArray<FString> CompletedMissions;
    UPROPERTY(BlueprintReadWrite) TArray<FString> OwnedWeapons;
    UPROPERTY(BlueprintReadWrite) TArray<FString> OwnedBusinesses;
    UPROPERTY(BlueprintReadWrite) FVector PlayerPosition;
    UPROPERTY(BlueprintReadWrite) float TimeOfDay;
};
