// GameMode - central system wiring for all gameplay systems
#include "CKGameMode.h"
#include "CKCharacter.h"
#include "CKPlayerController.h"
#include "CKWantedComponent.h"
#include "CKDayNightComponent.h"
#include "CKDistrictComponent.h"
#include "CKFactionComponent.h"
#include "CKReputationComponent.h"
#include "CKWeaponComponent.h"
#include "CKTrafficManager.h"
#include "CKBusinessComponent.h"
#include "CKMissionComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ACKGameMode::ACKGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PlayerMoney = 500;
    WantedSystem = nullptr;
    DayNight = nullptr;
    Districts = nullptr;
    Factions = nullptr;
    Reputation = nullptr;
    Weapons = nullptr;
    Traffic = nullptr;
    Economy = nullptr;
    Missions = nullptr;
}

void ACKGameMode::BeginPlay()
{
    Super::BeginPlay();

    // ── Traffic Manager ──
    Traffic = NewObject<UCKTrafficManager>(this);
    Traffic->RegisterComponent();
    Traffic->BuildRoadNetwork();
    UE_LOG(LogTemp, Warning, TEXT("[GAME] Traffic system initialized"));

    // ── Day/Night Cycle ──
    DayNight = NewObject<UCKDayNightComponent>(this);
    DayNight->RegisterComponent();
    UE_LOG(LogTemp, Warning, TEXT("[GAME] Day/night cycle initialized"));

    // ── Mission System ──
    Missions = NewObject<UCKMissionComponent>(this);
    Missions->RegisterComponent();
    Missions->DefineMissions();
    UE_LOG(LogTemp, Warning, TEXT("[GAME] Mission system initialized"));

    // ── Economy ──
    Economy = NewObject<UCKBusinessComponent>(this);
    Economy->RegisterComponent();
    Economy->InitBusinesses();
    UE_LOG(LogTemp, Warning, TEXT("[GAME] Economy initialized, starting money: $%d"), PlayerMoney);

    // Wire player-spawned systems via OnPlayerSpawned
    // This is called from the Character's BeginPlay or from the controller when possessing
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ACKCharacter* Char = Cast<ACKCharacter>(PC->GetPawn()))
        {
            OnPlayerSpawned(Char);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[GAME] Concrete Kingdom GameMode ready"));
}

void ACKGameMode::OnPlayerSpawned(ACKCharacter* Player)
{
    if (!Player) return;
    UE_LOG(LogTemp, Warning, TEXT("[GAME] Player spawned - attaching systems"));

    // Wanted System
    if (!Player->FindComponentByClass<UCKWantedComponent>())
    {
        WantedSystem = NewObject<UCKWantedComponent>(Player);
        WantedSystem->RegisterComponent();
        UE_LOG(LogTemp, Warning, TEXT("[GAME] Wanted system attached to player"));
    }

    // Districts
    if (!Player->FindComponentByClass<UCKDistrictComponent>())
    {
        Districts = NewObject<UCKDistrictComponent>(Player);
        Districts->RegisterComponent();
        Districts->DefineDistricts();
        UE_LOG(LogTemp, Warning, TEXT("[GAME] Districts attached to player"));
    }

    // Factions
    if (!Player->FindComponentByClass<UCKFactionComponent>())
    {
        Factions = NewObject<UCKFactionComponent>(Player);
        Factions->RegisterComponent();
        Factions->InitFactions();
        UE_LOG(LogTemp, Warning, TEXT("[GAME] Factions attached to player"));
    }

    // Reputation
    if (!Player->FindComponentByClass<UCKReputationComponent>())
    {
        Reputation = NewObject<UCKReputationComponent>(Player);
        Reputation->RegisterComponent();
        UE_LOG(LogTemp, Warning, TEXT("[GAME] Reputation attached to player"));
    }

    // Weapons
    if (!Player->FindComponentByClass<UCKWeaponComponent>())
    {
        Weapons = NewObject<UCKWeaponComponent>(Player);
        Weapons->RegisterComponent();
        Weapons->DefineWeapons();
        Weapons->EquipWeapon(TEXT("pistol"));
        UE_LOG(LogTemp, Warning, TEXT("[GAME] Weapons attached to player, pistol equipped"));
    }
}

void ACKGameMode::AddMoney(int32 Amount)
{
    PlayerMoney += Amount;
    UE_LOG(LogTemp, Warning, TEXT("[GAME] Money: $%d"), PlayerMoney);
}

void ACKGameMode::AddWantedLevel(int32 Amount)
{
    if (WantedSystem)
    {
        WantedSystem->AddHeat(Amount * 10);
    }
}
