#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "CKPlayerController.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKPlayerController : public APlayerController {
    GENERATED_BODY()
public:
    ACKPlayerController();
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // Legacy input bindings (works without EnhancedInput assets)
    void OnMove(const struct FInputActionValue& V);
    void OnLook(const struct FInputActionValue& V);
    void OnStartSprint();
    void OnStopSprint();
    void OnJump();
    void OnInteract();
    void OnShoot();
    void OnOpenMissions();
    void OnPause();
    void OnOpenWeaponWheel();
    void OnOpenPhone();

    // UI Widgets — created in BeginPlay
    UPROPERTY() class UCKHUDWidget* HUDWidget;
    UPROPERTY() class UCKPauseMenuWidget* PauseMenu;
    UPROPERTY() class UCKWeaponWheelWidget* WeaponWheel;
    UPROPERTY() class UCKPhoneScreenWidget* PhoneScreen;
    UPROPERTY() class UCKMissionWidget* MissionWidget;

    // EnhancedInput properties (populated by project assets or left null)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputMappingContext* InputMapping;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* MoveAction;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* LookAction;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* SprintAction;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* JumpAction;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* InteractAction;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* ShootAction;
};
