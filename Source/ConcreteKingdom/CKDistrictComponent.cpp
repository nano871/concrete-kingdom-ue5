// District system with 24-hour NPC spawn curves and 3-tier AI LOD
// From Cyberpunk 2077 NPC population research
#include "CKDistrictComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UCKDistrictComponent::UCKDistrictComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    TimeOfDay = 12.0f;
}

void UCKDistrictComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // Time advances 1 game hour per ~10 real seconds
    TimeOfDay += DeltaTime * 0.1f;
    if (TimeOfDay > 24.0f) TimeOfDay -= 24.0f;
}

FString UCKDistrictComponent::GetDistrictForGrid(int32 Col, int32 Row)
{
    if (Col == 3 && Row == 3) return "central";
    if (Col >= 3 && Row >= 3) return "commercial";
    if (Col < 3 && Row >= 3) return "residential";
    if (Col >= 3 && Row < 3) return "industrial";
    return "entertainment";
}

void UCKDistrictComponent::DefineDistricts()
{
    Districts.Empty();

    // Commercial: busy during day, quiet at night
    FDistrictDef Commercial;
    Commercial.Name = "commercial";
    Commercial.NPC_Density = 0.9f;
    Commercial.PoliceResponse = 0.9f;
    Commercial.LootMultiplier = 1.5f;
    Commercial.WantedRisk = 1.3f;
    Commercial.BuildingHeight = 0.8f;
    Commercial.HourlySpawnCurve = {0.1f,0.1f,0.1f,0.1f,0.1f,0.2f,0.4f,0.7f,0.9f,1.0f,1.0f,1.0f,1.0f,0.9f,0.9f,0.8f,0.7f,0.5f,0.3f,0.2f,0.1f,0.1f,0.1f,0.1f};
    Districts.Add("commercial", Commercial);

    // Industrial: early morning spike, quiet otherwise
    FDistrictDef Industrial;
    Industrial.Name = "industrial";
    Industrial.NPC_Density = 0.3f;
    Industrial.PoliceResponse = 0.4f;
    Industrial.LootMultiplier = 1.0f;
    Industrial.WantedRisk = 0.6f;
    Industrial.BuildingHeight = 0.5f;
    Industrial.HourlySpawnCurve = {0.1f,0.1f,0.1f,0.1f,0.3f,0.8f,1.0f,0.9f,0.7f,0.5f,0.4f,0.4f,0.3f,0.3f,0.3f,0.3f,0.5f,0.7f,0.3f,0.1f,0.1f,0.1f,0.1f,0.1f};
    Districts.Add("industrial", Industrial);

    // Residential: active evening, quiet day
    FDistrictDef Residential;
    Residential.Name = "residential";
    Residential.NPC_Density = 0.5f;
    Residential.PoliceResponse = 0.6f;
    Residential.LootMultiplier = 0.5f;
    Residential.WantedRisk = 0.8f;
    Residential.BuildingHeight = 0.3f;
    Residential.HourlySpawnCurve = {0.8f,0.7f,0.5f,0.3f,0.2f,0.2f,0.3f,0.4f,0.3f,0.2f,0.2f,0.2f,0.2f,0.2f,0.2f,0.3f,0.5f,0.7f,0.9f,1.0f,0.9f,0.8f,0.8f,0.8f};
    Districts.Add("residential", Residential);

    // Entertainment: nightlife peak
    FDistrictDef Entertainment;
    Entertainment.Name = "entertainment";
    Entertainment.NPC_Density = 1.0f;
    Entertainment.PoliceResponse = 1.0f;
    Entertainment.LootMultiplier = 2.0f;
    Entertainment.WantedRisk = 1.5f;
    Entertainment.BuildingHeight = 0.9f;
    Entertainment.HourlySpawnCurve = {0.5f,0.4f,0.3f,0.2f,0.1f,0.1f,0.1f,0.1f,0.1f,0.2f,0.3f,0.4f,0.5f,0.5f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f,1.0f,0.9f,0.8f,0.7f};
    Districts.Add("entertainment", Entertainment);

    // Central: balanced, steady density
    FDistrictDef Central;
    Central.Name = "central";
    Central.NPC_Density = 0.7f;
    Central.PoliceResponse = 0.8f;
    Central.LootMultiplier = 1.2f;
    Central.WantedRisk = 1.0f;
    Central.BuildingHeight = 0.7f;
    Central.HourlySpawnCurve = {0.2f,0.2f,0.2f,0.2f,0.2f,0.3f,0.5f,0.7f,0.8f,0.9f,0.9f,0.9f,0.8f,0.8f,0.8f,0.8f,0.7f,0.7f,0.6f,0.5f,0.4f,0.3f,0.2f,0.2f};
    Districts.Add("central", Central);

    UE_LOG(LogTemp, Warning, TEXT("Defined %d districts with 24-hour spawn curves"), Districts.Num());
}

FDistrictDef UCKDistrictComponent::GetDistrictAtLocation(FVector WorldLocation)
{
    const float WorldSize = 16000.0f;
    const float BlockSize = (WorldSize - 400.0f * 5) / 6.0f;
    int32 Col = FMath::FloorToInt((WorldLocation.X + WorldSize / 2) / (BlockSize + 400.0f));
    int32 Row = FMath::FloorToInt((WorldLocation.Y + WorldSize / 2) / (BlockSize + 400.0f));
    FString Name = GetDistrictForGrid(Col, Row);
    FDistrictDef* Found = Districts.Find(Name);
    return Found ? *Found : FDistrictDef();
}

FDistrictDef UCKDistrictComponent::GetCurrentDistrict()
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player) return FDistrictDef();
    return GetDistrictAtLocation(Player->GetActorLocation());
}

float UCKDistrictComponent::GetCurrentSpawnRate()
{
    FDistrictDef District = GetCurrentDistrict();
    int32 Hour = FMath::FloorToInt(TimeOfDay) % 24;
    if (District.HourlySpawnCurve.IsValidIndex(Hour))
        return District.HourlySpawnCurve[Hour] * District.NPC_Density;
    return 0.5f;
}

bool UCKDistrictComponent::ShouldSpawnHighLOD(FVector NPC_Location)
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player) return false;
    float Dist = FVector::Dist(Player->GetActorLocation(), NPC_Location);
    // 3-tier LOD: high (<30m), medium (30-80m), low (>80m)
    return Dist < 3000.0f; // game units, ~30m in UE5 scale
}
