#include "CKPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "CKCharacter.h"

ACKPlayerController::ACKPlayerController() { bShowMouseCursor = false; }

void ACKPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Create main HUD
    if (IsLocalPlayerController())
    {
        HUDWidget = CreateWidget<UCKHUDWidget>(this, LoadClass<UCKHUDWidget>(nullptr, TEXT("/Script/ConcreteKingdom.CKHUDWidget")));
        if (HUDWidget)
        {
            HUDWidget->AddToViewport(0);
            UE_LOG(LogTemp, Warning, TEXT("PlayerController: HUD created"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerController: HUD creation failed - create a Blueprint child of CKHUDWidget"));
        }

        // Create pause menu
        PauseMenu = CreateWidget<UCKPauseMenuWidget>(this, UCKPauseMenuWidget::StaticClass());
        if (PauseMenu) PauseMenu->ClosePauseMenu();

        // Create weapon wheel
        WeaponWheel = CreateWidget<UCKWeaponWheelWidget>(this, UCKWeaponWheelWidget::StaticClass());
        if (WeaponWheel) WeaponWheel->CloseWheel();

        // Create phone
        PhoneScreen = CreateWidget<UCKPhoneScreenWidget>(this, UCKPhoneScreenWidget::StaticClass());
        if (PhoneScreen) PhoneScreen->ClosePhone();
    }

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
