#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKTrafficVehicleComponent.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKTrafficVehicleComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UPROPERTY() int32 CurrentLaneIndex;
    UPROPERTY() int32 CurrentWaypoint;
    UPROPERTY() float Speed;
};