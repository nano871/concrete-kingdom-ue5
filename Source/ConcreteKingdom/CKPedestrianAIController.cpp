// UE5.8 - pedestrian AI with RDR2 reactivity
#include "CKPedestrianAIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

ACKPedestrianAIController::ACKPedestrianAIController()
{
    BaseSpeed = 60.0f;
    bFleeing = false;
}

void ACKPedestrianAIController::BeginPlay()
{
    Super::BeginPlay();
    if (GetPawn()) {
        WanderOrigin = GetPawn()->GetActorLocation();
        GetWorld()->GetTimerManager().SetTimer(WanderTimer, this, &ACKPedestrianAIController::Wander, 3.0f, true);
    }
}

void ACKPedestrianAIController::SetPedestrianType(FString Type)
{
    PedType = Type;
    if (Type == "business") BaseSpeed = 50.0f;
    else if (Type == "worker") BaseSpeed = 70.0f;
    else if (Type == "tourist") BaseSpeed = 40.0f;
    else BaseSpeed = 60.0f;
}

void ACKPedestrianAIController::ReactToPlayer(float PlayerSpeed, float Distance)
{
    if (!GetPawn()) return;
    if (PlayerSpeed > 200.0f && Distance < 500.0f && !bFleeing) {
        bFleeing = true;
        GetWorld()->GetTimerManager().ClearTimer(WanderTimer);
        FleeDirection = (GetPawn()->GetActorLocation() - UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation()).GetSafeNormal();
        if (UCharacterMovementComponent* MoveComp = GetPawn()->FindComponentByClass<UCharacterMovementComponent>())
            MoveComp->MaxWalkSpeed = BaseSpeed * 3.0f;
        FVector FleeTarget = GetPawn()->GetActorLocation() + FleeDirection * 1000.0f;
        MoveToLocation(FleeTarget);
        FTimerHandle UnfleeTimer;
        GetWorld()->GetTimerManager().SetTimer(UnfleeTimer, FTimerDelegate::CreateLambda([this]() {
            bFleeing = false;
            if (GetPawn() && GetPawn()->FindComponentByClass<UCharacterMovementComponent>())
                GetPawn()->FindComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = BaseSpeed;
            Wander();
        }), 5.0f, false);
    } else if (PlayerSpeed <= 200.0f && Distance < 200.0f && !bFleeing) {
        ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (Player) GetPawn()->SetActorRotation((Player->GetActorLocation() - GetPawn()->GetActorLocation()).Rotation());
    }
}

void ACKPedestrianAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Player && GetPawn()) {
        float Dist = GetPawn()->GetDistanceTo(Player);
        float Speed = Player->GetVelocity().Size();
        ReactToPlayer(Speed, Dist);
    }
}

void ACKPedestrianAIController::Wander()
{
    if (!GetPawn() || bFleeing) return;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return;
    FVector NavLocation;
    if (NavSys->GetRandomReachablePointInRadius(WanderOrigin, 500.0f, NavLocation))
        MoveToLocation(NavLocation);
}
