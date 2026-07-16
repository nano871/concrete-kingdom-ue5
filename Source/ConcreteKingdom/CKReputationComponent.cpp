// Per-district NPC reputation and memory system — from RDR2 research
#include "CKReputationComponent.h"
#include "Engine/World.h"

UCKReputationComponent::UCKReputationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    GameTime = 0.0f;
    MemoryDuration = 3600.0f; // 1 hour in game time
    ActionDecayRate = 0.1f;   // reputation decay per tick
}

void UCKReputationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!GetWorld()) return;
    GameTime += DeltaTime;
    DecayActions();
}

void UCKReputationComponent::RecordAction(FString ActionType, int32 Severity, FString District)
{
    FActionRecord Record;
    Record.ActionType = ActionType;
    Record.Location = FVector::ZeroVector;
    Record.Time = GameTime;
    Record.Severity = Severity;
    
    FDistrictReputation* Rep = DistrictReputations.Find(District);
    if (!Rep)
    {
        FDistrictReputation NewRep;
        NewRep.DistrictName = District;
        NewRep.Reputation = 0;
        NewRep.RecentActions.Add(Record);
        DistrictReputations.Add(District, NewRep);
    }
    else
    {
        Rep->RecentActions.Add(Record);
        
        // Calculate reputation change
        int32 Change = 0;
        if (ActionType == "robbery" || ActionType == "assault" || ActionType == "murder")
            Change = -Severity * 10;
        else if (ActionType == "help" || ActionType == "rescue")
            Change = Severity * 15;
        
        Rep->Reputation = FMath::Clamp(Rep->Reputation + Change, -100, 100);
        
        // RDR2-style: crimes in one district affect adjacent districts slightly
        for (auto& Pair : DistrictReputations)
        {
            if (Pair.Key != District)
            {
                Pair.Value.Reputation = FMath::Clamp(Pair.Value.Reputation + Change * 0.2f, -100, 100);
            }
        }
    }
}

int32 UCKReputationComponent::GetDistrictReputation(FString District)
{
    FDistrictReputation* Rep = DistrictReputations.Find(District);
    return Rep ? Rep->Reputation : 0;
}

FString UCKReputationComponent::GetDistrictReaction(FString District)
{
    int32 Rep = GetDistrictReputation(District);
    if (Rep <= -80) return TEXT("hostile");     // NPCs attack on sight
    if (Rep <= -40) return TEXT("nervous");     // NPCs flee, shops close
    if (Rep <= -10) return TEXT("cold");        // NPCs are curt, higher prices
    if (Rep < 10)   return TEXT("neutral");     // Default
    if (Rep < 40)   return TEXT("friendly");    // NPCs greet you
    if (Rep < 80)   return TEXT("welcoming");   // Discounts, favors
    return TEXT("allied");                       // Best prices, special access
}

TArray<FActionRecord> UCKReputationComponent::GetRecentActions(float WithinSeconds)
{
    TArray<FActionRecord> Recent;
    for (auto& Pair : DistrictReputations)
    {
        for (auto& Action : Pair.Value.RecentActions)
        {
            if (GameTime - Action.Time <= WithinSeconds)
                Recent.Add(Action);
        }
    }
    return Recent;
}

void UCKReputationComponent::DecayActions()
{
    for (auto& Pair : DistrictReputations)
    {
        // Remove expired actions
        Pair.Value.RecentActions.RemoveAll([this](const FActionRecord& A) {
            return GameTime - A.Time > MemoryDuration;
        });
        
        // Slowly decay reputation toward neutral
        if (Pair.Value.Reputation > 0)
            Pair.Value.Reputation = FMath::Max(0, Pair.Value.Reputation - FMath::FloorToInt(ActionDecayRate));
        else if (Pair.Value.Reputation < 0)
            Pair.Value.Reputation = FMath::Min(0, Pair.Value.Reputation + FMath::FloorToInt(ActionDecayRate));
    }
}
