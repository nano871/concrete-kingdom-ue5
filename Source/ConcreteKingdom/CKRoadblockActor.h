#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKRoadblockActor.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKRoadblockActor : public AActor {
    GENERATED_BODY()
public:
    ACKRoadblockActor();
    UFUNCTION(BlueprintCallable) void Deploy(FVector Location);
};
