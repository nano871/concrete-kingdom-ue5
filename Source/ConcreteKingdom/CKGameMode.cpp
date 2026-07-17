#include "CKGameMode.h"

ACKGameMode::ACKGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    WantedLevel = 0;
    Money = 0;
}

void ACKGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("Concrete Kingdom started! Money: $%d, Wanted: %d"), Money, WantedLevel);
}

void ACKGameMode::AddWantedLevel(int32 Amount)
{
    WantedLevel = FMath::Clamp(WantedLevel + Amount, 0, 5);
    UE_LOG(LogTemp, Warning, TEXT("Wanted level: %d"), WantedLevel);
}

void ACKGameMode::AddMoney(int32 Amount)
{
    Money += Amount;
    UE_LOG(LogTemp, Warning, TEXT("Money: $%d"), Money);

void ACKGameMode::OnPlayerSpawned(ACKCharacter* Player)
{
    if (!Player) return;

    // Attach core systems to the player on spawn
    WantedSystem = NewObject<UCKWantedComponent>(Player);
    WantedSystem->RegisterComponent();
    UE_LOG(LogTemp, Warning, TEXT("GameMode: Wanted system attached to player"));

    DayNight = FindComponentByClass<UCKDayNightComponent>();
    if (!DayNight)
    {
        DayNight = NewObject<UCKDayNightComponent>(this);
        DayNight->RegisterComponent();
        UE_LOG(LogTemp, Warning, TEXT("GameMode: Day/night cycle initialized"));
    }

    Districts = NewObject<UCKDistrictComponent>(Player);
    Districts->RegisterComponent();
    Districts->DefineDistricts();
    UE_LOG(LogTemp, Warning, TEXT("GameMode: District system attached"));

    Factions = NewObject<UCKFactionComponent>(Player);
    Factions->RegisterComponent();
    Factions->InitFactions();
    UE_LOG(LogTemp, Warning, TEXT("GameMode: Faction system attached"));

    Reputation = NewObject<UCKReputationComponent>(Player);
    Reputation->RegisterComponent();
    UE_LOG(LogTemp, Warning, TEXT("GameMode: Reputation system attached"));

    Weapons = FindComponentByClass<UCKWeaponComponent>();
    if (!Weapons)
    {
        Weapons = NewObject<UCKWeaponComponent>(Player);
        Weapons->RegisterComponent();
        Weapons->DefineWeapons();
        UE_LOG(LogTemp, Warning, TEXT("GameMode: Weapon system attached"));
    }
}

}
