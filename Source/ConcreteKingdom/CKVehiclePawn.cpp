// UE5.8 - Chaos Vehicle API
#include "CKVehiclePawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

ACKVehiclePawn::ACKVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Try to load imported car mesh; fall back to engine cube
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CarMeshFinder(TEXT("/Game/Models/passenger_car_pack/scene.scene"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

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
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 8
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovement())
        MoveComp->SetThrottleInput(Scale);
#else
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovementComponent())
        MoveComp->SetThrottleInput(Scale);
#endif
}

void ACKVehiclePawn::MoveRight(const FInputActionValue& Value)
{
    float Scale = Value.Get<float>();
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 8
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovement())
        MoveComp->SetSteeringInput(Scale);
#else
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovementComponent())
        MoveComp->SetSteeringInput(Scale);
#endif
}

void ACKVehiclePawn::Handbrake()
{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 8
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovement())
        MoveComp->SetHandbrakeInput(true);
#else
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovementComponent())
        MoveComp->SetHandbrakeInput(true);
#endif
}

void ACKVehiclePawn::ReleaseHandbrake()
{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 8
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovement())
        MoveComp->SetHandbrakeInput(false);
#else
    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovementComponent())
        MoveComp->SetHandbrakeInput(false);
#endif
}