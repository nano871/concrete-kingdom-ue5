// UE5.8 - GTA V style 5-star wanted system
#include "CKWantedComponent.h"

UCKWantedComponent::UCKWantedComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    WantedLevel = 0;
    CurrentWantedLevel = 0;
    StarProgress = 0.0f;
    Heat = 0.0f;
    HeatDecayTimer = 0.0f;
    SpawnCaps = {0, 3, 5, 8, 12, 15};
    SearchRadii = {0, 20, 30, 40, 50, 60};
    DecayTimes = {0, 8, 12, 18, 25, 35};
}

void UCKWantedComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CurrentWantedLevel = WantedLevel;

    if (WantedLevel > 0 && StarProgress <= 0)
    {
        HeatDecayTimer += DeltaTime;
        if (HeatDecayTimer >= DecayTimes[FMath::Min(WantedLevel, DecayTimes.Num() - 1)])
        {
            WantedLevel = FMath::Max(0, WantedLevel - 1);
            HeatDecayTimer = 0.0f;
            UE_LOG(LogTemp, Warning, TEXT("[WANTED] Level dropped to %d"), WantedLevel);
        }
    }
}

void UCKWantedComponent::AddHeat(float Amount)
{
    Heat += Amount;
    StarProgress += Amount;
    if (StarProgress >= 1.0f && WantedLevel < 5)
    {
        StarProgress = 0.0f;
        WantedLevel++;
        HeatDecayTimer = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("[WANTED] Level increased to %d"), WantedLevel);
    }
}

void UCKWantedComponent::SetWantedLevel(int32 Level)
{
    WantedLevel = FMath::Clamp(Level, 0, 5);
    CurrentWantedLevel = WantedLevel;
}

int32 UCKWantedComponent::GetWantedLevel()
{
    return WantedLevel;
}

void UCKWantedComponent::ClearWanted()
{
    WantedLevel = 0;
    CurrentWantedLevel = 0;
    Heat = 0.0f;
    StarProgress = 0.0f;
    HeatDecayTimer = 0.0f;
    UE_LOG(LogTemp, Warning, TEXT("[WANTED] Cleared"));
}

int32 UCKWantedComponent::GetSpawnCap()
{
    return SpawnCaps.IsValidIndex(WantedLevel) ? SpawnCaps[WantedLevel] : 0;
}

float UCKWantedComponent::GetSearchRadius()
{
    return SearchRadii.IsValidIndex(WantedLevel) ? SearchRadii[WantedLevel] : 0;
}

bool UCKWantedComponent::ShouldSpawnHelicopter()
{
    return WantedLevel >= 3;
}

bool UCKWantedComponent::ShouldSpawnRoadblock()
{
    return WantedLevel >= 3;
}
