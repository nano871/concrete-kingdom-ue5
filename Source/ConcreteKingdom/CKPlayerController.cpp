#include "CKPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "CKCharacter.h"
#include "CKHUDWidget.h"
#include "CKPauseMenuWidget.h"
#include "CKWeaponWheelWidget.h"
#include "CKPhoneScreenWidget.h"
#include "CKMissionWidget.h"
#include "CKGameMode.h"
#include "CKVehiclePawn.h"
#include "Blueprint/UserWidget.h"

ACKPlayerController::ACKPlayerController() { bShowMouseCursor = false; MissionWidget = nullptr; }

void ACKPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalPlayerController()) return;

    // Create main HUD
    HUDWidget = CreateWidget<UCKHUDWidget>(this, UCKHUDWidget::StaticClass());
    if (HUDWidget) { HUDWidget->AddToViewport(0); UE_LOG(LogTemp, Warning, TEXT("[PC] HUD created")); }
    else UE_LOG(LogTemp, Warning, TEXT("[PC] HUD failed - need BP child"));

    // Create pause menu
    PauseMenu = CreateWidget<UCKPauseMenuWidget>(this, UCKPauseMenuWidget::StaticClass());
    if (PauseMenu) PauseMenu->ClosePauseMenu();

    // Create weapon wheel
    WeaponWheel = CreateWidget<UCKWeaponWheelWidget>(this, UCKWeaponWheelWidget::StaticClass());
    if (WeaponWheel) WeaponWheel->CloseWheel();

    // Create phone
    PhoneScreen = CreateWidget<UCKPhoneScreenWidget>(this, UCKPhoneScreenWidget::StaticClass());
    if (PhoneScreen) PhoneScreen->ClosePhone();

    // Create mission widget
    MissionWidget = CreateWidget<UCKMissionWidget>(this, UCKMissionWidget::StaticClass());
    if (MissionWidget) { MissionWidget->AddToViewport(1); MissionWidget->Hide(); }

    // EnhancedInput mapping context
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

    // Fallback: bind classic input actions (works without EnhancedInput assets)
    if (UInputComponent* IC = InputComponent)
    {
        IC->BindAction("OpenMissions", IE_Pressed, this, &ACKPlayerController::OnOpenMissions);
        IC->BindAction("Pause", IE_Pressed, this, &ACKPlayerController::OnPause);
        IC->BindAction("OpenWeaponWheel", IE_Pressed, this, &ACKPlayerController::OnOpenWeaponWheel);
        IC->BindAction("OpenPhone", IE_Pressed, this, &ACKPlayerController::OnOpenPhone);
        IC->BindAction("Interact", IE_Pressed, this, &ACKPlayerController::OnInteract);
        IC->BindAction("EnterVehicle", IE_Pressed, this, &ACKPlayerController::OnEnterExitVehicle);
        IC->BindAction("Jump", IE_Pressed, this, &ACKPlayerController::OnJump);
        IC->BindAction("Shoot", IE_Pressed, this, &ACKPlayerController::OnShoot);
        IC->BindAction("Sprint", IE_Pressed, this, &ACKPlayerController::OnStartSprint);
        IC->BindAction("Sprint", IE_Released, this, &ACKPlayerController::OnStopSprint);
    }

    // EnhancedInput (if IMC assets exist in project)
    if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (MoveAction) EI->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACKPlayerController::OnMove);
        if (LookAction) EI->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACKPlayerController::OnLook);
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

void ACKPlayerController::OnOpenMissions()
{
    if (!MissionWidget) return;
    if (MissionWidget->bVisible)
    {
        MissionWidget->Hide();
        SetShowMouseCursor(false);
        FInputModeGameOnly Mode;
        SetInputMode(Mode);
    }
    else
    {
        // Fetch available missions from GameMode
        if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
        {
            TArray<FString> Avail;
            TArray<FString> Completed;
            if (GM->Missions)
            {
                Avail = GM->Missions->GetAvailableMissions();
                // Completed list not yet fetched from save system
            }
            MissionWidget->UpdateMissionList(Avail, Completed);
        }
        MissionWidget->Show();
        FInputModeGameAndUI Mode;
        Mode.SetWidgetToFocus(MissionWidget->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(Mode);
    }
}

void ACKPlayerController::OnEnterExitVehicle()
{
    // If currently in a vehicle, exit back to character
    if (GetPawn() && GetPawn()->IsA<ACKVehiclePawn>())
    {
        // Find the player character and possess it
        if (ACKCharacter* Char = Cast<ACKCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()))
        {
            // Actually, just unpossess and respawn at vehicle location
            FVector ExitLoc = GetPawn()->GetActorLocation() + FVector(200, 0, 0);
            GetPawn()->Destroy();
            // Spawn character at exit location (simplified - just log for now)
            UE_LOG(LogTemp, Warning, TEXT("[VEHICLE] Exited vehicle"));
        }
        else
        {
            // Find character actor in world
            for (TActorIterator<ACKCharacter> It(GetWorld()); It; ++It)
            {
                Possess(*It);
                UE_LOG(LogTemp, Warning, TEXT("[VEHICLE] Returned to character"));
                return;
            }
        }
        return;
    }

    // On-foot: find nearest vehicle and enter it
    ACKCharacter* Char = Cast<ACKCharacter>(GetPawn());
    if (!Char) return;

    // Simple proximity check - find nearest vehicle pawn
    ACKVehiclePawn* NearestVehicle = nullptr;
    float BestDist = 200.0f; // 2 meter range

    for (TActorIterator<ACKVehiclePawn> It(GetWorld()); It; ++It)
    {
        ACKVehiclePawn* V = *It;
        if (!V) continue;
        float D = FVector::Dist(Char->GetActorLocation(), V->GetActorLocation());
        if (D < BestDist) { BestDist = D; NearestVehicle = V; }
    }

    if (NearestVehicle)
    {
        // Enter vehicle: possess the vehicle pawn
        Possess(NearestVehicle);
        NearestVehicle->EnableInput(this);
        UE_LOG(LogTemp, Warning, TEXT("[VEHICLE] Entered vehicle"));
    }
}

void ACKPlayerController::OnPause()
{
    if (PauseMenu)
    {
        if (PauseMenu->IsPaused()) PauseMenu->ClosePauseMenu();
        else PauseMenu->OpenPauseMenu();
    }
}

void ACKPlayerController::OnOpenWeaponWheel()
{
    if (WeaponWheel)
    {
        if (WeaponWheel->IsOpen()) { WeaponWheel->CloseWheel(); }
        else { WeaponWheel->OpenWheel(); }
    }
}

void ACKPlayerController::OnOpenPhone()
{
    if (PhoneScreen)
    {
        if (PhoneScreen->IsOpen()) PhoneScreen->ClosePhone();
        else PhoneScreen->OpenPhone();
    }
}
