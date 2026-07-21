---
id: CARD-CKTV-010
version: 1
status: active
created: 2026-07-21
last_verified: 2026-07-21
confidence: proven
half_life: slow
tags: [ue5.8, traffic, vehicle, lighting, brake-lights, turn-signals, material-instance-dynamic]
links: []
uses: 0
---

# CKTrafficVehicleComponent — Brake Lights & Turn Signals

## Summary

Add brake lights (red, solid on braking) and turn signals (amber, pulsing on direction change) to AI traffic vehicles. Uses the **Optics** material slot on the car mesh (already present in the imported passenger car pack) via a runtime-created **UMaterialInstanceDynamic**.

The car mesh has 3 material slots per body variant: Body_0, Glass_0, Optics_0. The Optics slot controls the tail light/headlight lenses. By creating a dynamic instance of that material and setting scalar parameters, we can drive the emissive intensity for brake lights and turn signals — no mesh changes needed.

---

## UE5 API Reference

### Classes & Includes

| Purpose | Class | Include |
|---|---|---|
| Dynamic material instance | `UMaterialInstanceDynamic` | `#include "Materials/MaterialInstanceDynamic.h"` |
| Static mesh material access | `UStaticMeshComponent` | `#include "Components/StaticMeshComponent.h"` |
| Named parameter on MID | `SetScalarParameterValue(FName, float)` | (included via MID header) |
| Runtime material slot replacement | `SetMaterial(int32, UMaterialInterface*)` | `UStaticMeshComponent` |
| Get material from slot | `GetMaterial(int32)` | `UStaticMeshComponent` |
| Get static mesh | `GetStaticMesh()` | `UStaticMeshComponent` |
| Create MID from parent | `UMaterialInstanceDynamic::Create(Material, this)` | `Static` factory on `UMaterialInstanceDynamic` |

### Key API Patterns

**Creating a MID at runtime (BeginPlay context):**
```cpp
UMaterialInterface* OpticsMat = MeshComp->GetMaterial(OpticsSlotIndex);
if (OpticsMat && OpticsMat->IsValidLowLevel())
{
    OpticsMID = UMaterialInstanceDynamic::Create(OpticsMat, this);
    if (OpticsMID)
    {
        MeshComp->SetMaterial(OpticsSlotIndex, OpticsMID);
    }
}
```

**Setting scalar parameters every Tick:**
```cpp
if (OpticsMID)
{
    OpticsMID->SetScalarParameterValue(FName("EmissiveIntensity"), bBrakeLightsOn ? 5.0f : 0.0f);
    // Turn signal blink
    float BlinkPhase = FMath::Sin(TurnSignalTimer * 8.0f);
    float LeftBlink = (bLeftSignalOn && BlinkPhase > 0.0f) ? 1.0f : 0.0f;
    float RightBlink = (bRightSignalOn && BlinkPhase > 0.0f) ? 1.0f : 0.0f;
    OpticsMID->SetScalarParameterValue(FName("TurnSignalLeft"), LeftBlink);
    OpticsMID->SetScalarParameterValue(FName("TurnSignalRight"), RightBlink);
}
```

**Computing turn angle between two direction vectors:**
```cpp
float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CurrentDir.GetSafeNormal(), NextDir.GetSafeNormal())));
```

---

## Implementation Plan

### File 1: `CKTrafficVehicleComponent.h`

**Changes required:**

1. Add includes after line 5 (`#include "CKTrafficVehicleComponent.generated.h"`):
```cpp
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
```

2. Inside the class body, after the existing properties (after line 26 `float IDMAcceleration(...);`), add the new public section:

```cpp
    // ── Brake lights & turn signals ──────────────────────────────────────
    /** Override to enable ticking */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void BeginPlay() override;

    /** Call this from the traffic manager when acceleration is negative (braking). */
    void SetBraking(bool bBraking);

    /**
     * Call this from the traffic manager when the vehicle's movement direction
     * changes between consecutive waypoints. Computes turn angle and activates
     * the appropriate turn signal.
     * @param CurrentDir  Direction from current pos to this waypoint.
     * @param NextDir     Direction from this waypoint to the next waypoint.
     */
    void SetTurnDirection(const FVector& CurrentDir, const FVector& NextDir);

    UPROPERTY() bool bBrakeLightsOn;
    UPROPERTY() bool bLeftSignalOn;
    UPROPERTY() bool bRightSignalOn;

    /** Accumulated time for turn signal blink animation (radians). */
    UPROPERTY() float TurnSignalTimer;

    /** Deceleration threshold (cm/s²) below which brake lights activate. Default -50. */
    UPROPERTY() float BrakeLightThreshold;

    /** Degrees of direction change to trigger turn signal. Default 15. */
    UPROPERTY() float TurnAngleThreshold;

    /** Material slot index for the Optics material. Default 2 (Body=0, Glass=1, Optics=2). */
    UPROPERTY() int32 OpticsMaterialSlotIndex;

    /** Dynamic material instance for the Optics lens material. */
    UPROPERTY() UMaterialInstanceDynamic* OpticsMID;
```

### File 2: `CKTrafficVehicleComponent.cpp`

**Changes required:**

1. **Constructor** — add after line 12 (`bInitialized = false;`):
```cpp
    PrimaryComponentTick.bCanEverTick = true;
    bBrakeLightsOn = false;
    bLeftSignalOn = false;
    bRightSignalOn = false;
    TurnSignalTimer = 0.0f;
    BrakeLightThreshold = -50.0f;       // cm/s² — gentle braking threshold
    TurnAngleThreshold = 15.0f;         // degrees
    OpticsMaterialSlotIndex = 2;        // Body=0, Glass=1, Optics=2
    OpticsMID = nullptr;
```

2. **Add BeginPlay method** (after constructor, before IDMAcceleration):
```cpp
void UCKTrafficVehicleComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Owner->GetRootComponent());
    if (!MeshComp) return;

    // Get the Optics material from the expected slot index
    UMaterialInterface* OpticsMat = MeshComp->GetMaterial(OpticsMaterialSlotIndex);
    if (!OpticsMat || !OpticsMat->IsValidLowLevel())
    {
        // Fallback: scan all material slots for one containing "Optics" in name
        for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
        {
            UMaterialInterface* Mat = MeshComp->GetMaterial(i);
            if (Mat && Mat->GetName().Contains(TEXT("Optics")))
            {
                OpticsMat = Mat;
                OpticsMaterialSlotIndex = i;
                break;
            }
        }
    }

    if (OpticsMat && OpticsMat->IsValidLowLevel())
    {
        OpticsMID = UMaterialInstanceDynamic::Create(OpticsMat, this);
        if (OpticsMID)
        {
            MeshComp->SetMaterial(OpticsMaterialSlotIndex, OpticsMID);
            UE_LOG(LogTemp, Verbose, TEXT("CKTrafficVehicle: Created Optics MID for brake/turn signals"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CKTrafficVehicle: No Optics material found on mesh — brake lights disabled"));
    }
}
```

3. **Add TickComponent method** (after BeginPlay):
```cpp
void UCKTrafficVehicleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TurnSignalTimer += DeltaTime;

    if (!OpticsMID) return;

    // ── Brake lights ────────────────────────────────────────────────
    // GTA V style: brake lights are fully on when braking, fully off otherwise
    OpticsMID->SetScalarParameterValue(FName("EmissiveIntensity"), bBrakeLightsOn ? 5.0f : 0.0f);

    // ── Turn signals ────────────────────────────────────────────────
    // Blink at ~4 Hz (8.0 rad/s → sin oscillates at ~1.27 Hz, 2 blinks per cycle)
    float Blink = (FMath::Sin(TurnSignalTimer * 8.0f) > 0.0f) ? 1.0f : 0.0f;

    // Set left/right turn signal blink values
    // Use different parameter names — the material must have these parameters exposed
    OpticsMID->SetScalarParameterValue(FName("TurnSignalLeft"), (bLeftSignalOn ? Blink : 0.0f));
    OpticsMID->SetScalarParameterValue(FName("TurnSignalRight"), (bRightSignalOn ? Blink : 0.0f));
}
```

4. **Add SetBraking method** (after TickComponent):
```cpp
void UCKTrafficVehicleComponent::SetBraking(bool bBraking)
{
    bBrakeLightsOn = bBraking;
}
```

5. **Add SetTurnDirection method** (after SetBraking):
```cpp
void UCKTrafficVehicleComponent::SetTurnDirection(const FVector& CurrentDir, const FVector& NextDir)
{
    if (CurrentDir.IsNearlyZero() || NextDir.IsNearlyZero())
    {
        bLeftSignalOn = false;
        bRightSignalOn = false;
        return;
    }

    FVector C = CurrentDir.GetSafeNormal();
    FVector N = NextDir.GetSafeNormal();

    // Compute signed turn angle using cross product Z component
    float Dot = FVector::DotProduct(C, N);
    Dot = FMath::Clamp(Dot, -1.0f, 1.0f);
    float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

    if (AngleDeg < TurnAngleThreshold)
    {
        // Going straight — no turn signal
        bLeftSignalOn = false;
        bRightSignalOn = false;
        return;
    }

    // Determine turn direction: cross product Z tells us left/right
    // In UE5's coordinate system (X=forward, Y=right, Z=up):
    // Negative Z cross = left turn, Positive Z cross = right turn
    FVector Cross = FVector::CrossProduct(C, N);
    if (Cross.Z > 0.0f)
    {
        bRightSignalOn = true;
        bLeftSignalOn = false;
    }
    else
    {
        bLeftSignalOn = true;
        bRightSignalOn = false;
    }
}
```

### File 3: `CKTrafficManager.cpp`

**Changes required in `UpdateVehicles`:**

1. **After computing acceleration** (line 193, `float Accel = Follower->IDMAcceleration(...)`), add:
```cpp
        // Set brake lights state based on IDM acceleration
        Follower->SetBraking(Accel < Follower->BrakeLightThreshold);
```

2. **Before the waypoint increment** (before line 204 `Follower->CurrentWaypoint++`), detect the upcoming turn direction. The best place is at the top of the loop, after we have the current Target and before we move:
```cpp
        // ── Detect turn for turn signals ──────────────────────────────
        // Look ahead to the next waypoint to compute the turn angle
        int32 NextWpIdx = Follower->CurrentWaypoint + 1;
        if (Lane.Waypoints.IsValidIndex(NextWpIdx))
        {
            FVector DirToCurrent = (Target - Current).GetSafeNormal();
            FVector DirToNext = (Lane.Waypoints[NextWpIdx] - Target).GetSafeNormal();
            if (!DirToCurrent.IsNearlyZero() && !DirToNext.IsNearlyZero())
            {
                Follower->SetTurnDirection(DirToCurrent, DirToNext);
            }
        }
        else
        {
            // No more waypoints — turn off signals
            Follower->SetTurnDirection(FVector::ZeroVector, FVector::ZeroVector);
        }
```

This block should be placed right after the lead vehicle distance calculation (after line 190) and before the IDM acceleration calculation — or, more practically, right before the existing `// IDM acceleration` comment (line 192). The turn signal should activate before the vehicle reaches the waypoint, so the driver sees the signal in advance.

---

## Compile Errors to Watch For

| Error | Cause | Fix |
|---|---|---|
| `'PrimaryComponentTick' is not a member of 'UCKTrafficVehicleComponent'` | Missing `GENERATED_BODY()` or wrong base class | Component must inherit `UActorComponent` and have `GENERATED_BODY()` |
| `'UMaterialInstanceDynamic' : no appropriate default constructor` | Missing `#include "Materials/MaterialInstanceDynamic.h"` | Add the include |
| `'BeginPlay' : function does not take 0 arguments` | Signature mismatch — must be `virtual void BeginPlay() override;` | Add `override` keyword |
| `'SetScalarParameterValue' is not a member of 'UMaterialInstanceDynamic'` | Wrong include or UE5 API change | Use `SetScalarParameterValue(FName, float)` — it's on `UMaterialInstanceDynamic` not `UMaterialInstance` |
| `'IsValidLowLevel' : is not a member` | Wrong UE5 version check | Use `IsValid(OpticsMat)` instead (UE5 preferred pattern) |
| `FVector::CrossProduct` compile error | Wrong namespace | `FVector::CrossProduct` is correct static method |
| `LNK2019 unresolved external symbol` for UMaterialInstanceDynamic | Missing module dependency | Add `"MaterialInstanceDynamic"` is in `Engine` module — already in Build.cs |
| `'GetNumMaterials' : is not a member of 'UStaticMeshComponent'` | UE5.8 API change | Use `MeshComp->GetStaticMesh()->GetMaterialIndex(...)` or iterate `GetMaterial(i)` until null |

---

## How to Test the Implementation

### Test 1: Compile check
1. Open the project in UE5.8
2. Build the game module (Ctrl+Shift+F5 or Developer Command Prompt → `dotnet build`)
3. Fix any compile errors (see table above)

### Test 2: Optics material slot verification
1. In the editor, open the Sedan static mesh (`/Game/Models/passenger_car_pack/scene/Sedan.Sedan`)
2. Check the material slots panel — verify that slot 0 is Body, slot 1 is Glass, slot 2 is Optics
3. If the slot order differs, update `OpticsMaterialSlotIndex` in the component constructor
4. If Optics material doesn't exist, you'll need to check the fallback material scanning logic

### Test 3: Material parameter setup
1. Open the Optics material in the editor
2. Ensure it has scalar parameters named:
   - `EmissiveIntensity` — controls overall light brightness (0 = off, 5 = full)
   - `TurnSignalLeft` — 0 or 1 for left turn signal
   - `TurnSignalRight` — 0 or 1 for right turn signal
3. If the parameters have different names, update the `FName()` calls in `TickComponent`
4. If the material uses a VectorParameter for color instead of Scalar, change to `SetVectorParameterValue`

**IMPORTANT:** The car pack's imported Optics material may not have these parameters. The local AI implementing this should:
   - Open the Optics material in the editor
   - Add scalar parameters with the exact names above
   - Or, if the material is a simple texture-only material, create a material instance with emissive control

### Test 4: Runtime visual test
1. Play in editor (PIE)
2. Spawn the traffic manager with `DesiredDensity = 1.0`
3. Watch a traffic vehicle approach a turn:
   - **Expected:** Turn signal blinks (amber) 15+ degrees before the waypoint
   - **Expected:** Brake lights turn red when the vehicle slows down (IDM acceleration < -50)
4. Verify with `ShowDebug` or log output:
   ```
   UE_LOG(LogTemp, Warning, TEXT("Brake: %d Left: %d Right: %d"), bBrakeLightsOn, bLeftSignalOn, bRightSignalOn);
   ```

### Test 5: Edge cases
- Vehicle on last waypoint (no next waypoint) → turn signals should be off
- Vehicle going straight (0° turn angle) → turn signals off
- Vehicle at max speed, no lead vehicle → no brake lights
- Vehicle slowing for lead vehicle → brake lights on
- Optics material not found → log warning, no crash

---

## Dependencies

- None beyond existing `Engine` module (already in `ConcreteKingdom.Build.cs`)
- Car mesh must have an Optics material slot with emissive scalar parameters
- CKTrafficManager must call `SetBraking()` and `SetTurnDirection()` in its update loop

---

## Notes for the Local AI

1. **The `TickComponent` signature** in UE5.8 is:
   `virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;`
2. **The `BeginPlay` signature** is always `virtual void BeginPlay() override;`
3. **SetMaterial** on a `UStaticMeshComponent` replaces the material at the given slot index — this is safe to call at runtime.
4. **The turn signal blink frequency** of 8.0 rad/s gives ~1.27 Hz (2 blinks per cycle). GTA V uses ~1.5 Hz. Adjust this constant if needed.
5. **The BrakeLightThreshold** of -50 cm/s² means the brake lights come on during gentle deceleration. For harder braking, lower values (more negative) mean harder braking required. GTA V lights brake lights even on light engine braking, so -50 is reasonable.