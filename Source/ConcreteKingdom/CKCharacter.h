#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CKCharacter.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ACKCharacter();
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* CameraBoom;

    void Move(const struct FInputActionValue& Value);
    void Look(const struct FInputActionValue& Value);
    void StartJump();
    void StartSprint();
    void StopSprint();
    void Interact();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bHasWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 Ammo;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Shoot();
};
