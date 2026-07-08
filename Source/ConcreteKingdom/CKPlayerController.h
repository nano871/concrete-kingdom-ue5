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
    UPROPERTY(EditAnywhere, BlueprintReadOnly) class UInputMappingContext* InputMapping;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) class UInputAction* MoveAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) class UInputAction* LookAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) class UInputAction* JumpAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) class UInputAction* InteractAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) class UInputAction* ShootAction;
    void OnMove(const FInputActionValue& V);
    void OnLook(const FInputActionValue& V);
    void OnJump(); void OnInteract(); void OnShoot();
};