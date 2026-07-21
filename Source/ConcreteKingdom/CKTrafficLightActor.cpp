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
    if (!Mesh) return;
    float CyclePos = FMath::Fmod(Timer, GreenDuration * 2 + YellowDuration * 2);
    bool bIsYellow = (CyclePos >= GreenDuration && CyclePos < GreenDuration + YellowDuration) ||
                     (CyclePos >= GreenDuration * 2 + YellowDuration && CyclePos < GreenDuration * 2 + YellowDuration * 2);
    bool bIsGreen = (CyclePos < GreenDuration) || 
                    (CyclePos >= GreenDuration + YellowDuration && CyclePos < GreenDuration * 2 + YellowDuration);
    FLinearColor LightColor = FLinearColor::Red;
    if (bIsGreen && bNorthSouthGreen) LightColor = FLinearColor::Green;
    else if (bIsYellow) LightColor = FLinearColor::Yellow;
    // Create or reuse a dynamic material instance
    UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(0);
    if (Mat) Mat->SetVectorParameterValue(TEXT("EmissiveColor"), LightColor);
}
