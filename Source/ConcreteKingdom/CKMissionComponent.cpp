#include "CKMissionComponent.h"
#include "Engine/World.h"

UCKMissionComponent::UCKMissionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    ActiveMissionID = FName("None");
    TimedObjectiveAccum = 0.0f;
}

void UCKMissionComponent::DefineMissions()
{
    Missions.Empty();
    auto Add = [&](FName ID, FString Title, FString Desc, FString Faction,
                   TArray<FName> Reqs, TArray<FMissionObjective> Objs, int32 Reward, FVector MarkerPos)
    {
        FMissionDef M;
        M.MissionID = ID; M.Title = Title; M.Description = Desc; M.Faction = Faction;
        M.RequiredMissions = Reqs; M.Objectives = Objs; M.RewardMoney = Reward;
        M.bActive = false; M.bCompleted = false; M.bLocked = Reqs.Num() > 0;
        M.MissionMarkerLocation = MarkerPos;
        Missions.Add(ID, M);
    };

    // Act 1
    TArray<FMissionObjective> Act1;
    FMissionObjective O1; O1.Description = "Go to the bar"; O1.Type = "goto";
    O1.TargetLocation = FVector(1000, -600, 0); O1.Radius = 300; Act1.Add(O1);
    FMissionObjective O2; O2.Description = "Take over the bar"; O2.Type = "interact"; Act1.Add(O2);
    Add("m01_first_score", "First Score", "Knock over the corner store.", "neutral",
        {}, Act1, 500, FVector(1000, -600, 0));

    // Act 2
    TArray<FMissionObjective> Act2;
    FMissionObjective W1; W1.Description = "Get to the warehouse"; W1.Type = "goto";
    W1.TargetLocation = FVector(-1800, -900, 0); W1.Radius = 300; Act2.Add(W1);
    FMissionObjective W2; W2.Description = "Breach the warehouse"; W2.Type = "interact"; Act2.Add(W2);
    FMissionObjective W3; W3.Description = "Survive the police (20s)"; W3.Type = "timed";
    W3.Count = 20; Act2.Add(W3);
    Add("m02_warehouse", "Warehouse Job", "The syndicate wants the warehouse.", "syndicate",
        {"m01_first_score"}, Act2, 1200, FVector(-1800, -900, 0));

    // Act 3
    TArray<FMissionObjective> Act3;
    FMissionObjective A1; A1.Description = "Reach the apartment"; A1.Type = "goto";
    A1.TargetLocation = FVector(-1400, 1600, 0); A1.Radius = 300; Act3.Add(A1);
    FMissionObjective A2; A2.Description = "Crack the safe"; A2.Type = "interact"; Act3.Add(A2);
    Add("m03_apartment", "Apartment Heist", "The Mafia has a safe inside.", "mafia",
        {"m01_first_score"}, Act3, 2000, FVector(-1400, 1600, 0));
}

bool UCKMissionComponent::StartMission(FName MissionID)
{
    FMissionDef* M = Missions.Find(MissionID);
    if (!M || M->bLocked || M->bCompleted || M->bActive) return false;
    M->bActive = true;
    ActiveMissionID = MissionID;
    TimedObjectiveAccum = 0.0f;
    OnMissionStarted.Broadcast(MissionID);
    return true;
}

void UCKMissionComponent::ReportAction(FString ActionType, FString TargetID)
{
    FMissionDef* M = Missions.Find(ActiveMissionID);
    if (!M || ActiveMissionID == FName("None")) return;
    for (FMissionObjective& Obj : M->Objectives)
    {
        if (Obj.bCompleted) continue;
        if (Obj.Type == ActionType) Obj.bCompleted = true;
    }
    bool bAllDone = true;
    for (FMissionObjective& Obj : M->Objectives)
        if (!Obj.bCompleted) { bAllDone = false; break; }
    if (bAllDone)
    {
        M->bCompleted = true;
        M->bActive = false;
        CompletedMissions.Add(ActiveMissionID);
        OnMissionCompleted.Broadcast(ActiveMissionID);
        ActiveMissionID = FName("None");
    }
}

void UCKMissionComponent::TickMission(float DeltaTime)
{
    FMissionDef* M = Missions.Find(ActiveMissionID);
    if (!M || ActiveMissionID == FName("None")) return;
    for (FMissionObjective& Obj : M->Objectives)
    {
        if (Obj.bCompleted || Obj.Type != "timed") continue;
        TimedObjectiveAccum += DeltaTime;
        Obj.Progress = FMath::FloorToInt(TimedObjectiveAccum);
        if (Obj.Progress >= Obj.Count) Obj.bCompleted = true;
    }
}

FName UCKMissionComponent::GetActiveMissionID() { return ActiveMissionID; }

FString UCKMissionComponent::GetCurrentObjective()
{
    FMissionDef* M = Missions.Find(ActiveMissionID);
    if (!M) return "";
    for (FMissionObjective& Obj : M->Objectives)
        if (!Obj.bCompleted) return Obj.Description;
    return "All complete";
}

FVector UCKMissionComponent::GetObjectiveLocation()
{
    FMissionDef* M = Missions.Find(ActiveMissionID);
    if (!M) return FVector::ZeroVector;
    for (FMissionObjective& Obj : M->Objectives)
        if (!Obj.bCompleted && Obj.Type == "goto") return Obj.TargetLocation;
    return FVector::ZeroVector;
}

TArray<FName> UCKMissionComponent::GetCompletedMissions() { return CompletedMissions; }
