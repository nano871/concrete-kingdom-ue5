// Day/night cycle - rotates sun, ambient color blending, IsNight query
#include "CKDayNightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UCKDayNightComponent::UCKDayNightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    TimeOfDay = 12.0f; // noon start
    DaySpeed = 0.1f;   // 1 game hour = ~10 real seconds
    SunHeight = 45.0f;
    DayColor = FLinearColor(1.0f, 0.95f, 0.8f);
    NightColor = FLinearColor(0.1f, 0.1f, 0.3f);
    CurrentColor = DayColor;
    SunAltitude = 0.0f;
    SunLight = nullptr;
}

void UCKDayNightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeOfDay += DeltaTime * DaySpeed;
    if (TimeOfDay > 24.0f) TimeOfDay -= 24.0f;

    // Sun altitude: 0 at midnight, 90 at noon
    SunAltitude = FMath::Sin(TimeOfDay / 24.0f * PI * 2.0f) * SunHeight;

    // Find or cache the directional light
    if (!SunLight)
    {
        TArray<AActor*> Lights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), Lights);
        if (Lights.Num() > 0) SunLight = Cast<ADirectionalLight>(Lights[0]);
    }

    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(SunAltitude, 0, 0));

        // Blend between day and night colors
        float DayFactor = FMath::Clamp((SunAltitude + 10.0f) / 60.0f, 0.0f, 1.0f);
        CurrentColor = FLinearColor::LerpUsingHSV(NightColor, DayColor, DayFactor);
        SunLight->SetLightColor(CurrentColor);
    }
}

bool UCKDayNightComponent::IsNight()
{
    return TimeOfDay < 6.0f || TimeOfDay > 20.0f;
}

float UCKDayNightComponent::GetSunAltitude()
{
    return SunAltitude;
}