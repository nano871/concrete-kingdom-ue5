#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKDistrictComponent.generated.h"

USTRUCT(BlueprintType)
struct FDistrictDef {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FString Name;        // commercial, industrial, residential, entertainment
    UPROPERTY(EditAnywhere) FLinearColor Color;
    UPROPERTY(EditAnywhere) float NPC_Density;    // 0-1: how many pedestrians
    UPROPERTY(EditAnywhere) float PoliceResponse; // 0-1: how fast police respond
    UPROPERTY(EditAnywhere) float LootMultiplier; // 0-1: robbery payouts
    UPROPERTY(EditAnywhere) float WantedRisk;     // 0-1: heat gain per crime
    UPROPERTY(EditAnywhere) float BuildingHeight; // 0-1: average building height
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKDistrictComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKDistrictComponent();
    UFUNCTION(BlueprintCallable) void DefineDistricts();
    UFUNCTION(BlueprintCallable) FDistrictDef GetDistrictAtLocation(FVector WorldLocation);
    UFUNCTION(BlueprintCallable) FDistrictDef GetCurrentDistrict();

    UPROPERTY(BlueprintReadOnly) TMap<FString, FDistrictDef> Districts;
    UPROPERTY(BlueprintReadOnly) FString CurrentDistrictName;
};
