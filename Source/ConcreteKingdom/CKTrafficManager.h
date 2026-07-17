#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKTrafficManager.generated.h"

USTRUCT()
struct FRoadLane {
    GENERATED_BODY()
    TArray<FVector> Waypoints;
    FVector Direction;
    float SpeedLimit;
    void UpdateVehicles(float DeltaTime);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKTrafficManager : public UActorComponent {
    GENERATED_BODY()
public:
    UCKTrafficManager();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius;

    UFUNCTION(BlueprintCallable)
    void BuildRoadNetwork();

    UFUNCTION(BlueprintCallable)
    void SetDesiredDensity(float Density);

protected:
    void SpawnVehicle();
    TArray<FRoadLane> RoadLanes;
    TArray<class AActor*> ActiveVehicles;
    float SpawnTimer;
    float DesiredDensity;
    void UpdateVehicles(float DeltaTime);
};