#include "CKMinimapCameraActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"

ACKMinimapCameraActor::ACKMinimapCameraActor()
{
    PrimaryActorTick.bCanEverTick = true;
    CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
    RootComponent = CaptureComponent;
    CaptureComponent->SetRelativeRotation(FRotator(-90, 0, 0));
}

void ACKMinimapCameraActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Player) SetActorLocation(Player->GetActorLocation() + FVector(0, 0, 5000));
}
