#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "CKVehiclePawn.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKVehiclePawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()
public:
    ACKVehiclePawn();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
    class UStaticMeshComponent* VehicleMesh;

    void MoveForward(const struct FInputActionValue& Value);
    void MoveRight(const struct FInputActionValue& Value);
    void Handbrake();
    void ReleaseHandbrake();
};
