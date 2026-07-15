#include "CKTrafficVehicleComponent.h"

UCKTrafficVehicleComponent::UCKTrafficVehicleComponent()
{
    Speed = 0.0f;
    DesiredSpeed = 600.0f;
    TimeHeadway = 1.8f;
    MaxAcceleration = 200.0f;
    ComfortableBraking = 150.0f;
    MinimumGap = 200.0f;
    bInitialized = false;
}

// Intelligent Driver Model (IDM) — from Treiber et al. 2000
// Returns acceleration in cm/s² to maintain safe following distance
float UCKTrafficVehicleComponent::IDMAcceleration(float CurrentSpeed, float DistanceToLead, float LeadSpeed)
{
    if (!bInitialized) return 0.0f;

    // IDM parameters
    const float Delta = 4.0f; // acceleration exponent
    const float a = MaxAcceleration;
    const float b = ComfortableBraking;
    const float T = TimeHeadway;
    const float s0 = MinimumGap;
    const float v0 = DesiredSpeed;

    // Desired gap (speed-dependent following distance)
    float EffectiveGap = s0 + CurrentSpeed * T;
    if (DistanceToLead > 0.0f && DistanceToLead < 100000.0f)
    {
        // Add dynamic braking term
        float SpeedDiff = CurrentSpeed - LeadSpeed;
        EffectiveGap += CurrentSpeed * SpeedDiff / (2.0f * FMath::Sqrt(a * b));
    }

    // IDM acceleration formula
    float FreeRoad = FMath::Pow(CurrentSpeed / v0, Delta);
    float Interaction = 0.0f;
    if (EffectiveGap > 0.0f && DistanceToLead > 0.0f)
    {
        Interaction = FMath::Pow(EffectiveGap / DistanceToLead, 2.0f);
    }

    float Accel = a * (1.0f - FreeRoad - Interaction);
    return FMath::Clamp(Accel, -b * 2.0f, a);
}