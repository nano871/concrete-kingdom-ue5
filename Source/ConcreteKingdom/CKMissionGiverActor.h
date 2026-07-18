#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKMissionGiverActor.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKMissionGiverActor : public AActor {
    GENERATED_BODY()
public:
    ACKMissionGiverActor();
    UFUNCTION(BlueprintCallable) void OfferMission(FString MissionID);
    UFUNCTION(BlueprintCallable) void AcceptMission();
    UFUNCTION(BlueprintCallable) void DeclineMission();
    UPROPERTY() bool bMissionOffered;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString MissionID;
};
