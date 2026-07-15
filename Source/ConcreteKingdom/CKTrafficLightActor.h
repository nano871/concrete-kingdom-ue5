#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKTrafficLightActor.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKTrafficLightActor : public AActor {
    GENERATED_BODY()
public:
    ACKTrafficLightActor();
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bNorthSouthGreen;
    float Timer;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GreenDuration;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float YellowDuration;

    UPROPERTY() class UStaticMeshComponent* Mesh;
    void UpdateLightColor();
};
