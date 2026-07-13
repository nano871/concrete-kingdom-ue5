#include "CKPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "CKCharacter.h"

ACKPlayerController::ACKPlayerController() { bShowMouseCursor = false; }

void ACKPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* SS = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (InputMapping) SS->AddMappingContext(InputMapping, 0);
        }
    }
}

void ACKPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (MoveAction) EI->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACKPlayerController::OnMove);
        if (LookAction) EI->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACKPlayerController::OnLook);
        if (SprintAction) EI->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ACKPlayerController::OnStartSprint);
        if (SprintAction) EI->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACKPlayerController::OnStopSprint);
        if (JumpAction) EI->BindAction(JumpAction, ETriggerEvent::Started, this, &ACKPlayerController::OnJump);
        if (InteractAction) EI->BindAction(InteractAction, ETriggerEvent::Started, this, &ACKPlayerController::OnInteract);
        if (ShootAction) EI->BindAction(ShootAction, ETriggerEvent::Started, this, &ACKPlayerController::OnShoot);
    }
}

void ACKPlayerController::OnMove(const FInputActionValue& V)
{
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->Move(V);
}
void ACKPlayerController::OnLook(const FInputActionValue& V)
{
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->Look(V);
}
void ACKPlayerController::OnStartSprint()
{
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->StartSprint();
}

void ACKPlayerController::OnStopSprint()
{
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->StopSprint();
}

void ACKPlayerController::OnJump()
{
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->StartJump();
}
void ACKPlayerController::OnInteract()
{
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->Interact();
}
void ACKPlayerController::OnShoot()
{
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->Shoot();
}
