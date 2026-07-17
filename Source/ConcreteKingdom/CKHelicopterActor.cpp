#include "CKHelicopterActor.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"

ACKHelicopterActor::ACKHelicopterActor()
{
    PrimaryActorTick.bCanEverTick = true;
    Speed = 500.0f;
    bActive = false;
    UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    SearchLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SearchLight"));
    SearchLight->SetupAttachment(RootComponent);
}

void ACKHelicopterActor::Deploy(FVector TargetLocation) { Target = TargetLocation + FVector(0, 0, 2000); bActive = true; SetActorLocation(Target); }

void ACKHelicopterActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bActive) return;
    FVector Dir = (Target - GetActorLocation()).GetSafeNormal();
    AddActorWorldOffset(Dir * Speed * DeltaTime);
}
