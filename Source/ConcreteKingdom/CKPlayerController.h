#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "CKPlayerController.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKPlayerController : public APlayerController {
    GENERATED_BODY()
public:
    ACKPlayerController();
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* InputMapping;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* SprintAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* JumpAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* InteractAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* ShootAction;

    void OnMove(const FInputActionValue& V);
    void OnLook(const FInputActionValue& V);
    void OnStartSprint();
    void OnStopSprint();
    void OnJump();
    void OnInteract();
    void OnShoot();
};
