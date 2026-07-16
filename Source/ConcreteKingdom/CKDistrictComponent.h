#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKDistrictComponent.generated.h"

USTRUCT(BlueprintType)
struct FDistrictDef {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) FLinearColor Color;
    UPROPERTY(EditAnywhere) float NPC_Density;
    UPROPERTY(EditAnywhere) float PoliceResponse;
    UPROPERTY(EditAnywhere) float LootMultiplier;
    UPROPERTY(EditAnywhere) float WantedRisk;
    UPROPERTY(EditAnywhere) float BuildingHeight;

    // 24-hour spawn curve (24 values, one per hour, 0-1)
    UPROPERTY(EditAnywhere) TArray<float> HourlySpawnCurve;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKDistrictComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKDistrictComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick, FActorComponentTickFunction*) override;
    UFUNCTION(BlueprintCallable) void DefineDistricts();
    UFUNCTION(BlueprintCallable) FDistrictDef GetDistrictAtLocation(FVector WorldLocation);
    UFUNCTION(BlueprintCallable) FDistrictDef GetCurrentDistrict();
    UFUNCTION(BlueprintCallable) float GetCurrentSpawnRate(); // 0-1 based on time+district
    UFUNCTION(BlueprintCallable) bool ShouldSpawnHighLOD(FVector NPC_Location);

    UPROPERTY(BlueprintReadOnly) TMap<FString, FDistrictDef> Districts;
    UPROPERTY(BlueprintReadOnly) FString CurrentDistrictName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TimeOfDay; // 0-24

private:
    FString GetDistrictForGrid(int32 Col, int32 Row);
};
