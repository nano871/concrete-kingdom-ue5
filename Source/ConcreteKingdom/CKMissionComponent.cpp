#include "CKMissionComponent.h"
#include "Engine/World.h"

UCKMissionComponent::UCKMissionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    ActiveMission = nullptr;
    TimedObjectiveAccum = 0.0f;
}

void UCKMissionComponent::DefineMissions()
{
    Missions.Empty();

    auto Add = [&](FName ID, FString Title, FString Desc, FString Faction, TArray<FName> Reqs, TArray<FMissionObjective> Objs, int32 Reward, FVector MarkerPos)
    {
        FMissionDef M;
        M.MissionID = ID;
        M.Title = Title;
        M.Description = Desc;
        M.Faction = Faction;
        M.RequiredMissions = Reqs;
        M.Objectives = Objs;
        M.RewardMoney = Reward;
        M.bActive = false;
        M.bCompleted = false;
        M.bLocked = Reqs.Num() > 0;
        M.MissionMarkerLocation = MarkerPos;
        Missions.Add(ID, M);
    };

    // Act 1: Rise
    TArray<FMissionObjective> Objs1;
    FMissionObjective O1; O1.Description = "Go to the bar"; O1.Type = "goto"; O1.TargetLocation = FVector(1000, -600, 0); O1.Radius = 300; Objs1.Add(O1);
    FMissionObjective O2; O2.Description = "Take over the bar"; O2.Type = "interact"; Objs1.Add(O2);
    FMissionObjective O3; O3.Description = "Lose the police"; O3.Type = "escape"; Objs1.Add(O3);
    Add("m01_first_score", "First Score", "Knock over the corner store.", "neutral", {}, Objs1, 500, FVector(1000, -600, 0));

    // Act 2: Warehouse Job
    TArray<FMissionObjective> Objs2;
    FMissionObjective W1; W1.Description = "Get to the warehouse"; W1.Type = "goto"; W1.TargetLocation = FVector(-1800, -900, 0); W1.Radius = 300; Objs2.Add(W1);
    FMissionObjective W2; W2.Description = "Breach the warehouse"; W2.Type = "interact"; Objs2.Add(W2);
    FMissionObjective W3; W3.Description = "Survive the police response (20s)"; W3.Type = "timed"; W3.Count = 20; Objs2.Add(W3);
    FMissionObjective W4; W4.Description = "Grab the shipment"; W4.Type = "interact"; Objs2.Add(W4);
    FMissionObjective W5; W5.Description = "Lose the cops"; W5.Type = "escape"; Objs2.Add(W5);
    Add("m02_warehouse", "Warehouse Job", "The syndicate wants the warehouse.", "syndicate", {"m01_first_score"}, Objs2, 1200, FVector(-1800, -900, 0));

    // Act 3: Apartment Heist
    TArray<FMissionObjective> Objs3;
    FMissionObjective A1; A1.Description = "Reach the apartment"; A1.Type = "goto"; A1.TargetLocation = FVector(-1400, 1600, 0); A1.Radius = 300; Objs3.Add(A1);
    FMissionObjective A2; A2.Description = "Crack the safe"; A2.Type = "interact"; Objs3.Add(A2);
    FMissionObjective A3; A3.Description = "Escape the area"; A3.Type = "escape"; Objs3.Add(A3);
    Add("m03_apartment", "Apartment Heist", "The Mafia has a safe in the apartment.", "mafia", {"m01_first_score"}, Objs3, 2000, FVector(-1400, 1600, 0));

    UE_LOG(LogTemp, Warning, TEXT("Defined %d missions"), Missions.Num());
}

bool UCKMissionComponent::StartMission(FName MissionID)
{
    FMissionDef* M = Missions.Find(MissionID);
    if (!M || M->bLocked || M->bCompleted || M->bActive) return false;
    M->bActive = true;
    ActiveMission = M;
    TimedObjectiveAccum = 0.0f;
    OnMissionStarted.Broadcast(MissionID);
    UE_LOG(LogTemp, Warning, TEXT("Mission started: %s"), *MissionID.ToString());
    return true;
}

void UCKMissionComponent::ReportAction(FString ActionType, FString TargetID)
{
    if (!ActiveMission) return;
    for (FMissionObjective& Obj : ActiveMission->Objectives)
    {
        if (Obj.bCompleted) continue;
        if (Obj.Type == ActionType) Obj.bCompleted = true;
    }
    // Check completion
    bool bAllDone = true;
    for (FMissionObjective& Obj : ActiveMission->Objectives)
    {
        if (!Obj.bCompleted) { bAllDone = false; break; }
    }
    if (bAllDone)
    {
        ActiveMission->bCompleted = true;
        ActiveMission->bActive = false;
        CompletedMissions.Add(ActiveMission->MissionID);
        OnMissionCompleted.Broadcast(ActiveMission->MissionID);
        UE_LOG(LogTemp, Warning, TEXT("Mission completed: %s, reward $%d"), *ActiveMission->MissionID.ToString(), ActiveMission->RewardMoney);
        ActiveMission = nullptr;
    }
}

void UCKMissionComponent::TickMission(float DeltaTime)
{
    if (!ActiveMission) return;
    for (FMissionObjective& Obj : ActiveMission->Objectives)
    {
        if (Obj.bCompleted || Obj.Type != "timed") continue;
        TimedObjectiveAccum += DeltaTime;
        Obj.Progress = FMath::FloorToInt(TimedObjectiveAccum);
        if (Obj.Progress >= Obj.Count) Obj.bCompleted = true;
    }
}

FName UCKMissionComponent::GetActiveMissionID()
{
    return ActiveMission ? ActiveMission->MissionID : FName("None");
}

FString UCKMissionComponent::GetCurrentObjective()
{
    if (!ActiveMission) return "";
    for (FMissionObjective& Obj : ActiveMission->Objectives)
        if (!Obj.bCompleted) return Obj.Description;
    return "All objectives complete";
}

FVector UCKMissionComponent::GetObjectiveLocation()
{
    if (!ActiveMission) return FVector::ZeroVector;
    for (FMissionObjective& Obj : ActiveMission->Objectives)
        if (!Obj.bCompleted && Obj.Type == "goto") return Obj.TargetLocation;
    return FVector::ZeroVector;
}

TArray<FName> UCKMissionComponent::GetCompletedMissions() { return CompletedMissions; }
