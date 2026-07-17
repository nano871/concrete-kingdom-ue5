#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKMinimapCameraActor.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKMinimapCameraActor : public AActor {
    GENERATED_BODY()
public:
    ACKMinimapCameraActor();
    virtual void Tick(float DeltaTime) override;
    UPROPERTY() class USceneCaptureComponent2D* CaptureComponent;
};
