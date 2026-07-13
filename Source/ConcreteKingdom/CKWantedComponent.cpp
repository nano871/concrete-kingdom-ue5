#include "CKWantedComponent.h"

UCKWantedComponent::UCKWantedComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    WantedLevel = 0;
    StarProgress = 0.0f;
    HeatDecayTimer = 0.0f;
    SpawnCaps = {0, 3, 5, 8, 12, 15};
    SearchRadii = {0, 20, 30, 40, 50, 60};
    DecayTimes = {0, 8, 12, 18, 25, 35};
}

void UCKWantedComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (WantedLevel > 0 && StarProgress <= 0) {
        HeatDecayTimer += DeltaTime;
        if (HeatDecayTimer >= DecayTimes[WantedLevel]) {
            int32 Old = WantedLevel;
            WantedLevel = FMath::Max(0, WantedLevel - 1);
            HeatDecayTimer = 0.0f;
            if (WantedLevel != Old) UE_LOG(LogTemp, Warning, TEXT("Wanted level dropped to %d"), WantedLevel);
        }
    }
}

void UCKWantedComponent::AddHeat(float Amount)
{
    StarProgress += Amount;
    if (StarProgress >= 1.0f && WantedLevel < 5) {
        StarProgress = 0.0f;
        WantedLevel++;
        HeatDecayTimer = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("Wanted level increased to %d"), WantedLevel);
    }
}

void UCKWantedComponent::SetWantedLevel(int32 Level) { WantedLevel = FMath::Clamp(Level, 0, 5); }
int32 UCKWantedComponent::GetSpawnCap() { return SpawnCaps.IsValidIndex(WantedLevel) ? SpawnCaps[WantedLevel] : 0; }
float UCKWantedComponent::GetSearchRadius() { return SearchRadii.IsValidIndex(WantedLevel) ? SearchRadii[WantedLevel] : 0; }
bool UCKWantedComponent::ShouldSpawnHelicopter() { return WantedLevel >= 3; }
bool UCKWantedComponent::ShouldSpawnRoadblock() { return WantedLevel >= 3; }
