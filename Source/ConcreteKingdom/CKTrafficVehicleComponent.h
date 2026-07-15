#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKTrafficVehicleComponent.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKTrafficVehicleComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKTrafficVehicleComponent();

    UPROPERTY() int32 CurrentLaneIndex;
    UPROPERTY() int32 CurrentWaypoint;
    UPROPERTY() float Speed;

    // IDM car-following parameters (from GTA V research)
    UPROPERTY() float DesiredSpeed;
    UPROPERTY() float TimeHeadway;      // seconds (GTA V: ~1.8s)
    UPROPERTY() float MaxAcceleration;  // cm/s²
    UPROPERTY() float ComfortableBraking;
    UPROPERTY() float MinimumGap;       // cm
    UPROPERTY() bool bInitialized;

    float IDMAcceleration(float CurrentSpeed, float DistanceToLead, float LeadSpeed);
};