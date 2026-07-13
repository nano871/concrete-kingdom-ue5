#include "CKCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "CKGameMode.h"
#include "Engine/World.h"

ACKCharacter::ACKCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 500.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    bHasWeapon = false;
    Ammo = 0;
}

void ACKCharacter::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
void ACKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) { Super::SetupPlayerInputComponent(PlayerInputComponent); }

void ACKCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MoveVector = Value.Get<FVector2D>();
    if (Controller) {
        AddMovementInput(GetActorForwardVector(), MoveVector.Y);
        AddMovementInput(GetActorRightVector(), MoveVector.X);
    }
}

void ACKCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);
}

void ACKCharacter::StartJump() { Jump(); }

void ACKCharacter::Interact()
{
    if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
    {
        GM->AddMoney(50);
        UE_LOG(LogTemp, Warning, TEXT("Interacted! Money: $%d"), GM->Money);
    }
}

void ACKCharacter::Shoot()
{
    if (!bHasWeapon || Ammo <= 0) return;
    Ammo--;
    UE_LOG(LogTemp, Warning, TEXT("Shot! Ammo left: %d"), Ammo);
}
