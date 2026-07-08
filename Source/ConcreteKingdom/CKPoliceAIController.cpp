#include "CKPoliceAIController.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ACKPoliceAIController::ACKPoliceAIController() { PatrolRadius = 500.0f; ChaseSpeed = 1000.0f; WantedLevel = 0; }
void ACKPoliceAIController::BeginPlay() { Super::BeginPlay(); PatrolOrigin = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector; Patrol(); }
void ACKPoliceAIController::Tick(float DeltaTime) { Super::Tick(DeltaTime); if (WantedLevel > 0) Chase(); }
void ACKPoliceAIController::SetWantedLevel(int32 Level) { WantedLevel = Level; if (Level == 0) Patrol(); }

void ACKPoliceAIController::Patrol()
{
    if (!GetPawn()) return;
    FVector NavLocation;
    if (UNavigationSystemV1::GetCurrent(GetWorld())->GetRandomReachablePointInRadius(PatrolOrigin, PatrolRadius, NavLocation))
        MoveToLocation(NavLocation);
    GetWorld()->GetTimerManager().SetTimer(PatrolTimerHandle, this, &ACKPoliceAIController::Patrol, 3.0f + FMath::FRand() * 3.0f);
}

void ACKPoliceAIController::Chase()
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Player) MoveToActor(Player, 200.0f);
}
