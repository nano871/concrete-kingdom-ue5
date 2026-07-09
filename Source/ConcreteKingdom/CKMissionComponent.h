#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKMissionComponent.generated.h"

USTRUCT(BlueprintType)
struct FMissionObjective {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FString Description;
    UPROPERTY(EditAnywhere) FString Type;   // goto, interact, kill, timed, escape
    UPROPERTY(EditAnywhere) FVector TargetLocation;
    UPROPERTY(EditAnywhere) float Radius;
    UPROPERTY(EditAnywhere) int32 Count;
    UPROPERTY(BlueprintReadOnly) int32 Progress;
    UPROPERTY(BlueprintReadOnly) bool bCompleted;
};

USTRUCT(BlueprintType)
struct FMissionDef {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FName MissionID;
    UPROPERTY(EditAnywhere) FString Title;
    UPROPERTY(EditAnywhere) FString Description;
    UPROPERTY(EditAnywhere) FString Faction;
    UPROPERTY(EditAnywhere) TArray<FName> RequiredMissions;
    UPROPERTY(EditAnywhere) TArray<FMissionObjective> Objectives;
    UPROPERTY(EditAnywhere) int32 RewardMoney;
    UPROPERTY(EditAnywhere) bool bActive;
    UPROPERTY(EditAnywhere) bool bCompleted;
    UPROPERTY(EditAnywhere) bool bLocked;
    UPROPERTY(EditAnywhere) FVector MissionMarkerLocation;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionUpdated, FName, MissionID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKMissionComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKMissionComponent();

    UPROPERTY(BlueprintAssignable) FOnMissionUpdated OnMissionStarted;
    UPROPERTY(BlueprintAssignable) FOnMissionUpdated OnMissionCompleted;

    UFUNCTION(BlueprintCallable) void DefineMissions();
    UFUNCTION(BlueprintCallable) bool StartMission(FName MissionID);
    UFUNCTION(BlueprintCallable) void ReportAction(FString ActionType, FString TargetID);
    UFUNCTION(BlueprintCallable) FName GetActiveMissionID();
    UFUNCTION(BlueprintCallable) FString GetCurrentObjective();
    UFUNCTION(BlueprintCallable) FVector GetObjectiveLocation();
    UFUNCTION(BlueprintCallable) TArray<FName> GetCompletedMissions();

    UPROPERTY(BlueprintReadOnly) TMap<FName, FMissionDef> Missions;
    UPROPERTY(BlueprintReadOnly) FMissionDef* ActiveMission;

    void TickMission(float DeltaTime);

private:
    TArray<FName> CompletedMissions;
    float TimedObjectiveAccum;
};
