#include "CKFactionComponent.h"

UCKFactionComponent::UCKFactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCKFactionComponent::InitFactions()
{
    auto Add = [this](FString Name, int32 Rep, FString Status)
    {
        FFactionStanding F;
        F.FactionName = Name; F.Reputation = Rep; F.Status = Status;
        Factions.Add(Name, F);
    };
    Add("player", 0, "neutral");
    Add("police", 0, "neutral");
    Add("syndicate", 0, "neutral");
    Add("mafia", 0, "neutral");
    Add("neutral", 0, "neutral");
}

void UCKFactionComponent::ModifyReputation(FString FactionName, int32 Amount)
{
    FFactionStanding* F = Factions.Find(FactionName);
    if (!F) return;
    F->Reputation += Amount;
    if (F->Reputation < -50) F->Status = "hostile";
    else if (F->Reputation > 50) F->Status = "allied";
    else if (F->Reputation > 20) F->Status = "friendly";
    else F->Status = "neutral";
}

FFactionStanding UCKFactionComponent::GetFaction(FString FactionName)
{
    FFactionStanding F;
    FFactionStanding* Found = Factions.Find(FactionName);
    return Found ? *Found : F;
}

TArray<FString> UCKFactionComponent::GetHostileFactions()
{
    TArray<FString> Hostile;
    for (auto& Pair : Factions)
        if (Pair.Value.Status == "hostile") Hostile.Add(Pair.Key);
    return Hostile;
}
