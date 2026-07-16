// District system with gameplay tradeoffs — from procedural generation research
#include "CKDistrictComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UCKDistrictComponent::UCKDistrictComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCKDistrictComponent::DefineDistricts()
{
    Districts.Empty();

    // Commercial: high density, fast police, good loot, high risk
    FDistrictDef Commercial;
    Commercial.Name = "commercial";
    Commercial.Color = FLinearColor(0.3f, 0.5f, 0.8f); // blue
    Commercial.NPC_Density = 0.9f;     // lots of witnesses
    Commercial.PoliceResponse = 0.9f;   // cops arrive fast
    Commercial.LootMultiplier = 1.5f;   // richer stores
    Commercial.WantedRisk = 1.3f;       // heat builds faster (cameras + witnesses)
    Commercial.BuildingHeight = 0.8f;   // tall buildings
    Districts.Add("commercial", Commercial);

    // Industrial: low density, slow police, medium loot, low risk
    FDistrictDef Industrial;
    Industrial.Name = "industrial";
    Industrial.Color = FLinearColor(0.5f, 0.4f, 0.3f); // brown
    Industrial.NPC_Density = 0.3f;      // few witnesses
    Industrial.PoliceResponse = 0.4f;    // cops take time
    Industrial.LootMultiplier = 1.0f;    // standard payouts
    Industrial.WantedRisk = 0.6f;        // easy to get away
    Industrial.BuildingHeight = 0.5f;    // warehouses
    Districts.Add("industrial", Industrial);

    // Residential: low density, medium police, low loot, medium risk
    FDistrictDef Residential;
    Residential.Name = "residential";
    Residential.Color = FLinearColor(0.4f, 0.7f, 0.4f); // green
    Residential.NPC_Density = 0.5f;
    Residential.PoliceResponse = 0.6f;
    Residential.LootMultiplier = 0.5f;   // less money in homes
    Residential.WantedRisk = 0.8f;
    Residential.BuildingHeight = 0.3f;   // low houses
    Districts.Add("residential", Residential);

    // Entertainment: very high density, fast police, very good loot, very high risk
    FDistrictDef Entertainment;
    Entertainment.Name = "entertainment";
    Entertainment.Color = FLinearColor(0.8f, 0.3f, 0.6f); // pink
    Entertainment.NPC_Density = 1.0f;
    Entertainment.PoliceResponse = 1.0f;
    Entertainment.LootMultiplier = 2.0f; // clubs, casinos
    Entertainment.WantedRisk = 1.5f;     // max risk
    Entertainment.BuildingHeight = 0.9f; // highrises
    Districts.Add("entertainment", Entertainment);

    // Central (city center, block 0,0): balanced high-end
    FDistrictDef Central;
    Central.Name = "central";
    Central.Color = FLinearColor(0.7f, 0.7f, 0.7f); // gray
    Central.NPC_Density = 0.7f;
    Central.PoliceResponse = 0.8f;
    Central.LootMultiplier = 1.2f;
    Central.WantedRisk = 1.0f;
    Central.BuildingHeight = 0.7f;
    Districts.Add("central", Central);

    UE_LOG(LogTemp, Warning, TEXT("Defined %d districts with gameplay tradeoffs"), Districts.Num());
}

FDistrictDef UCKDistrictComponent::GetDistrictAtLocation(FVector WorldLocation)
{
    // Determine district by position in the 6x6 grid
    const float WorldSize = 16000.0f;
    const float BlockSize = (WorldSize - 400.0f * 5) / 6.0f;
    
    int32 Col = FMath::FloorToInt((WorldLocation.X + WorldSize / 2) / (BlockSize + 400.0f));
    int32 Row = FMath::FloorToInt((WorldLocation.Y + WorldSize / 2) / (BlockSize + 400.0f));
    
    // Center block (3,3) is central district
    if (Col == 3 && Row == 3) return *Districts.Find("central");
    
    // Quadrants: NE=commercial, NW=residential, SE=industrial, SW=entertainment
    if (Col >= 3 && Row >= 3) return *Districts.Find("commercial");
    if (Col < 3 && Row >= 3) return *Districts.Find("residential");
    if (Col >= 3 && Row < 3) return *Districts.Find("industrial");
    return *Districts.Find("entertainment");
}

FDistrictDef UCKDistrictComponent::GetCurrentDistrict()
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player) return FDistrictDef();
    return GetDistrictAtLocation(Player->GetActorLocation());
}
