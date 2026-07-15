// UE5.8 - uses UNavigationSystemV1
#include "CKPoliceAIController.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ACKPoliceAIController::ACKPoliceAIController()
{
    PatrolRadius = 500.0f;
    ChaseSpeed = 1000.0f;
    WantedLevel = 0;
}

void ACKPoliceAIController::BeginPlay()
{
    Super::BeginPlay();
    if (GetPawn()) PatrolOrigin = GetPawn()->GetActorLocation();
    Patrol();
}

void ACKPoliceAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (WantedLevel > 0) {
        GetWorld()->GetTimerManager().PauseTimer(PatrolTimerHandle);
        Chase();
    } else {
        GetWorld()->GetTimerManager().UnPauseTimer(PatrolTimerHandle);
    }
}

void ACKPoliceAIController::SetWantedLevel(int32 Level)
{
    WantedLevel = Level;
    if (Level == 0) Patrol();
}

void ACKPoliceAIController::Patrol()
{
    if (!GetPawn()) return;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return;
    FVector NavLocation;
    if (NavSys->GetRandomReachablePointInRadius(PatrolOrigin, PatrolRadius, NavLocation))
        MoveToLocation(NavLocation);
    GetWorld()->GetTimerManager().SetTimer(PatrolTimerHandle, this, &ACKPoliceAIController::Patrol, 3.0f + FMath::FRand() * 3.0f);
}

void ACKPoliceAIController::Chase()
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Player) MoveToActor(Player, 200.0f);

    // Spawn helicopter at wanted 3+
    if (WantedLevel >= 3 && !bHelicopterSpawned)
    {
        bHelicopterSpawned = true;
        FVector HeliPos = Player ? Player->GetActorLocation() + FVector(0, 0, 2000) : FVector(0, 0, 2000);
        AActor* Heli = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), HeliPos, FRotator::ZeroRotator);
        if (Heli) Heli->SetActorLabel(TEXT("PoliceHelicopter"));
        UE_LOG(LogTemp, Warning, TEXT("Police helicopter deployed!"));
    }

    // Spawn roadblock at wanted 3+
    if (WantedLevel >= 3 && !bRoadblockSpawned)
    {
        bRoadblockSpawned = true;
        if (Player)
        {
            FVector BlockLoc = Player->GetActorLocation() + Player->GetActorForwardVector() * 1000;
            AActor* Block = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), BlockLoc, FRotator::ZeroRotator);
            if (Block) Block->SetActorLabel(TEXT("Roadblock"));
        }
        UE_LOG(LogTemp, Warning, TEXT("Roadblock deployed!"));
    }
}

void ACKPoliceAIController::ResetChase()
{
    bHelicopterSpawned = false;
    bRoadblockSpawned = false;
}
