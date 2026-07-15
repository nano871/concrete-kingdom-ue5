#include "CKTrafficLightActor.h"
#include "Components/StaticMeshComponent.h"

ACKTrafficLightActor::ACKTrafficLightActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bNorthSouthGreen = true;
    GreenDuration = 10.0f;
    YellowDuration = 2.0f;
    Timer = 0.0f;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
}

void ACKTrafficLightActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Timer += DeltaTime;
    float TotalCycle = GreenDuration * 2 + YellowDuration * 2;

    if (Timer > TotalCycle) Timer -= TotalCycle;

    if (Timer < GreenDuration) bNorthSouthGreen = true;
    else if (Timer < GreenDuration + YellowDuration) bNorthSouthGreen = true; // yellow
    else if (Timer < GreenDuration + YellowDuration + GreenDuration) bNorthSouthGreen = false;
    else bNorthSouthGreen = false; // yellow for cross street

    UpdateLightColor();
}

void ACKTrafficLightActor::UpdateLightColor()
{
    // Visual would be set here with material changes
}
