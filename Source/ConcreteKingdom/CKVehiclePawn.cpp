#include "CKVehiclePawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

ACKVehiclePawn::ACKVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 700.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void ACKVehiclePawn::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
void ACKVehiclePawn::SetupPlayerInputComponent(UInputComponent* PIC) { Super::SetupPlayerInputComponent(PIC); }

void ACKVehiclePawn::MoveForward(const FInputActionValue& Value)
{
    float Scale = Value.Get<float>();
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovementComponent())
        MoveComp->SetThrottleInput(Scale);
}

void ACKVehiclePawn::MoveRight(const FInputActionValue& Value)
{
    float Scale = Value.Get<float>();
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovementComponent())
        MoveComp->SetSteeringInput(Scale);
}

void ACKVehiclePawn::Handbrake()
{
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovementComponent())
        MoveComp->SetHandbrakeInput(true);
}

void ACKVehiclePawn::ReleaseHandbrake()
{
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovementComponent())
        MoveComp->SetHandbrakeInput(false);
}
