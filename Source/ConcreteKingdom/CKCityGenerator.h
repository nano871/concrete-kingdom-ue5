#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKCityGenerator.generated.h"

USTRUCT(BlueprintType)
struct FBuildingDef {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FVector Position;
    UPROPERTY(EditAnywhere) FVector Size;
    UPROPERTY(EditAnywhere) float Height;
    UPROPERTY(EditAnywhere) FLinearColor Color;
    UPROPERTY(EditAnywhere) FString Name;
};

UCLASS()
class CONCRETEKINGDOM_API ACKCityGenerator : public AActor {
    GENERATED_BODY()
public:
    ACKCityGenerator();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City")
    int32 GridSizeX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City")
    int32 GridSizeY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City")
    float BlockSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City")
    float RoadWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City")
    TArray<FBuildingDef> CustomBuildings;

    UFUNCTION(BlueprintCallable, Category="City")
    void GenerateCity();

    UFUNCTION(BlueprintCallable, Category="City")
    void SpawnRoad(FVector Start, FVector End, float Width);

    UFUNCTION(BlueprintCallable, Category="City")
    void SpawnBuilding(FVector Position, FVector Size, float Height, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category="City")
    void ClearCity();

protected:
    UPROPERTY() TArray<class AActor*> SpawnedActors;
    TArray<FVector> RoadEndpoints;
};
