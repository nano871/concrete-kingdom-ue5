---
title: "CKDayNightComponent — add street light emissive toggle based on TOD"
topic_index: 5
topic: "IMPLEMENT: CKDayNightComponent — add street light emissive toggle based on TOD"
date: 2026-07-21
researched_by: Nanov2_bot (cron)
confidence: proven
tags: [UE5.8, day-night, emissive, street-lights, materials, UMaterialInstanceDynamic, CKDayNightComponent, world-simulation]
---

# Implementation Card: Street Light Emissive Toggle Based on Time of Day

## Summary

CKDayNightComponent already drives sun rotation and light color based on `TimeOfDay`. When `IsNight()` returns true (before 06:00 or after 20:00), street lights in the world should glow with a warm emissive color. During the day, their emissive should be turned off (black).

The implementation uses a **tag-based actor discovery** pattern: any actor tagged `"StreetLight"` is automatically discovered once in `BeginPlay`. When day→night transition is detected, a `UMaterialInstanceDynamic` is created per `UStaticMeshComponent` and its `"EmissiveColor"` vector parameter is set to a warm orange. On night→day, the emissive is set to black.

This mirrors the pattern already used by `CKTrafficLightActor::UpdateLightColor()` which calls `CreateAndSetMaterialInstanceDynamic(0)` and `SetVectorParameterValue(TEXT("EmissiveColor"), ...)`.

---

## UE5.8 API Reference

### Key Classes

| Class | Include | Purpose |
|-------|---------|---------|
| `UGameplayStatics` | `#include "Kismet/GameplayStatics.h"` | `GetAllActorsWithTag()` — find street lights by tag |
| `UStaticMeshComponent` | `#include "Components/StaticMeshComponent.h"` | Each street light's renderable mesh |
| `UMaterialInstanceDynamic` | `#include "Materials/MaterialInstanceDynamic.h"` | Runtime-modifiable material with emissive parameter |
| `UPrimitiveComponent` | `#include "Components/PrimitiveComponent.h"` | Parent of UStaticMeshComponent, has `CreateAndSetMaterialInstanceDynamic()` |

### Core Functions

#### UGameplayStatics::GetAllActorsWithTag

```cpp
static void GetAllActorsWithTag(
    const UObject* WorldContextObject,
    FName Tag,
    TArray<AActor*>& OutActors
);
```

- `WorldContextObject` — typically `GetWorld()` or `this`
- `Tag` — the `FName` tag to search for (actors must have this in their `Tags` array)
- `OutActors` — output array of actors

**Important:** This is an O(n) scan over all actors in the world. Call it once in `BeginPlay()`, not every frame. For large worlds, cache the results.

#### UPrimitiveComponent::CreateAndSetMaterialInstanceDynamic

```cpp
UMaterialInstanceDynamic* CreateAndSetMaterialInstanceDynamic(
    int32 ElementIndex
);
```

- `ElementIndex` — which material slot on the mesh (usually `0`)
- Returns a `UMaterialInstanceDynamic*` — the MID is automatically set as the override material for that slot
- Call once per component per element index — subsequent calls with the same index will create a new MID (so cache the pointer)

**Important:** In `CKTrafficLightActor::UpdateLightColor()`, `CreateAndSetMaterialInstanceDynamic(0)` is called every tick — this is wasteful but functional for a single traffic light. For street lights, we call it once in `BeginPlay` and cache the MID.

#### UMaterialInstanceDynamic::SetVectorParameterValue

```cpp
void SetVectorParameterValue(
    FName ParameterName,
    FLinearColor Value
);
```

- `ParameterName` — must match the parameter name in the material graph (case-sensitive). Convention: `"EmissiveColor"`
- `Value` — `FLinearColor`. For on: `FLinearColor(1.0f, 0.5f, 0.05f)` (warm orange). For off: `FLinearColor(0.0f, 0.0f, 0.0f)` (black)

#### AActor access pattern

```cpp
// Get all static mesh components on an actor
TArray<UStaticMeshComponent*> MeshComps;
Actor->GetComponents<UStaticMeshComponent>(MeshComps);
// Or GetComponentsByClass(UStaticMeshComponent::StaticClass())
```

### FLinearColor Constants

| Color | Constructor | Use |
|-------|-------------|-----|
| Orange emissive | `FLinearColor(1.0f, 0.45f, 0.05f)` | Warm street light glow (night) |
| Dim orange | `FLinearColor(0.02f, 0.01f, 0.0f)` | Transitional near-dusk |
| Black (off) | `FLinearColor(0.0f, 0.0f, 0.0f)` | Daytime (no emissive) |

---

## Data Model: FStreetLightCache

Each street light actor may have multiple `UStaticMeshComponent` children (pole, bulb housing, lens). Each mesh component needs its own `UMaterialInstanceDynamic`. We store a flat map:

```
Cache: TMap<AActor*, TArray<UStaticMeshComponent*>>  // keyed by actor for easy iteration
MID Cache: TMap<UStaticMeshComponent*, UMaterialInstanceDynamic*>  // per-component MID
```

Or simpler: store a flat `TArray<UMaterialInstanceDynamic*>` of all MIDs. Fewer data structures, same result.

**Recommended approach** — store two flat arrays:

```cpp
TArray<UStaticMeshComponent*> StreetLightMeshes;   // all mesh components with emissive
TArray<UMaterialInstanceDynamic*> StreetLightMIDs;  // corresponding MIDs (parallel array)
```

This avoids per-actor iteration overhead at toggle time — just loop over the flat arrays.

---

## File 1: CKDayNightComponent.h — Changes Required

### Current Header (36 lines)

No street light support exists. We need to add:

1. **Includes**: `"Components/StaticMeshComponent.h"`, `"Materials/MaterialInstanceDynamic.h"`
2. **Member variables** for caching street light meshes and MIDs
3. **Helper function** declaration to build the cache
4. **Night state** tracking variable

### Changed Header

**Lines to add in the header** (after line 27, before the closing `};`):

```cpp
// ── Street Light Emissive Control (NEW) ──
public:
    /** Rebuild the street light MID cache (call on BeginPlay or when world changes) */
    UFUNCTION(BlueprintCallable, Category = "Time|StreetLights")
    void RebuildStreetLightCache();

private:
    /** All static mesh components on tagged street light actors */
    UPROPERTY()
    TArray<UStaticMeshComponent*> StreetLightMeshes;

    /** Dynamic material instances for each street light mesh component (parallel array) */
    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> StreetLightMIDs;

    /** Track the most recent IsNight() result to detect transitions */
    bool bPreviousIsNight;
```

**Also add these includes at the top** (after line 4, before the generated.h):

```cpp
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
```

### Full changed header preview

Replace lines 33-36 (the `private:` section and closing brace) with:

```cpp
private:
    class ADirectionalLight* SunLight;
    FLinearColor DayColor;
    FLinearColor NightColor;
    FLinearColor CurrentColor;

    // ── Street Light Emissive (NEW) ──
public:
    UFUNCTION(BlueprintCallable, Category = "Time|StreetLights")
    void RebuildStreetLightCache();

private:
    UPROPERTY()
    TArray<UStaticMeshComponent*> StreetLightMeshes;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> StreetLightMIDs;

    bool bPreviousIsNight;
};
```

**Full includes section** (lines 3-5):

```cpp
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CKDayNightComponent.generated.h"
```

---

## File 2: CKDayNightComponent.cpp — Changes Required

### 1. Add includes (lines 1-5)

**Current:**
```cpp
// Day/night cycle - rotates sun, ambient color blending, IsNight query
#include "CKDayNightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
```

**New — add these includes after line 5:**
```cpp
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
```

### 2. Initialize bPreviousIsNight in constructor (after line 17)

**Current (line 18 only):**
```cpp
SunLight = nullptr;
```

**New — add after `SunLight = nullptr;` (line 18):**
```cpp
bPreviousIsNight = IsNight();  // initialize to match current TOD state
```

### 3. Add BeginPlay override (NEW — insert after constructor, before TickComponent)

**Add this block** (insert after line 19 closing brace):

```cpp
void UCKDayNightComponent::BeginPlay()
{
    Super::BeginPlay();
    RebuildStreetLightCache();
    UE_LOG(LogTemp, Verbose, TEXT("[DAYNIGHT] Street light cache built, %d meshes"), StreetLightMeshes.Num());
}
```

**Also add declaration in header:**

```cpp
virtual void BeginPlay() override;
```

### 4. RebuildStreetLightCache() implementation (NEW — insert after IsNight())

**Add after `GetSunAltitude()`** (after line 57):

```cpp
void UCKDayNightComponent::RebuildStreetLightCache()
{
    if (!GetWorld()) return;

    // Clear old cache
    StreetLightMeshes.Empty();
    StreetLightMIDs.Empty();

    // Find all actors tagged "StreetLight"
    TArray<AActor*> StreetLightActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(TEXT("StreetLight")), StreetLightActors);

    for (AActor* Actor : StreetLightActors)
    {
        if (!Actor) continue;

        // Get all static mesh components on this actor
        TArray<UStaticMeshComponent*> MeshComps;
        Actor->GetComponents<UStaticMeshComponent>(MeshComps);

        for (UStaticMeshComponent* MeshComp : MeshComps)
        {
            if (!MeshComp) continue;

            // Create a dynamic material instance for element 0
            UMaterialInstanceDynamic* MID = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
            if (MID)
            {
                StreetLightMeshes.Add(MeshComp);
                StreetLightMIDs.Add(MID);

                // Set initial emissive based on current TOD
                bool bNight = IsNight();
                FLinearColor EmissiveColor = bNight
                    ? FLinearColor(1.0f, 0.45f, 0.05f)   // warm orange glow
                    : FLinearColor(0.0f, 0.0f, 0.0f);    // off (black)
                MID->SetVectorParameterValue(TEXT("EmissiveColor"), EmissiveColor);
            }
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("[DAYNIGHT] RebuildStreetLightCache: %d actors, %d meshes"),
        StreetLightActors.Num(), StreetLightMeshes.Num());
}
```

### 5. Modify TickComponent — add TOD transition detection (after SunLight color update, line 47)

**Current code (lines 39-47):**
```cpp
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(SunAltitude, 0, 0));

        // Blend between day and night colors
        float DayFactor = FMath::Clamp((SunAltitude + 10.0f) / 60.0f, 0.0f, 1.0f);
        CurrentColor = FLinearColor::LerpUsingHSV(NightColor, DayColor, DayFactor);
        SunLight->SetLightColor(CurrentColor);
    }
}
```

**New code — add after the SunLight color block (after line 47, before the closing `}` of TickComponent):**

```cpp
    // ── Street Light Emissive Toggle ──
    bool bCurrentIsNight = IsNight();
    if (bCurrentIsNight != bPreviousIsNight)
    {
        bPreviousIsNight = bCurrentIsNight;

        FLinearColor TargetEmissive = bCurrentIsNight
            ? FLinearColor(1.0f, 0.45f, 0.05f)   // warm orange glow
            : FLinearColor(0.0f, 0.0f, 0.0f);    // off

        for (UMaterialInstanceDynamic* MID : StreetLightMIDs)
        {
            if (MID)
            {
                MID->SetVectorParameterValue(TEXT("EmissiveColor"), TargetEmissive);
            }
        }

        UE_LOG(LogTemp, Verbose, TEXT("[DAYNIGHT] Street lights %s"),
            bCurrentIsNight ? TEXT("ON (night)") : TEXT("OFF (day)"));
    }
```

### Full TickComponent delta

The final TickComponent should look like:

```cpp
void UCKDayNightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!GetWorld()) return;

    TimeOfDay += DeltaTime * DaySpeed;
    if (TimeOfDay > 24.0f) TimeOfDay -= 24.0f;

    // Sun altitude
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

        float DayFactor = FMath::Clamp((SunAltitude + 10.0f) / 60.0f, 0.0f, 1.0f);
        CurrentColor = FLinearColor::LerpUsingHSV(NightColor, DayColor, DayFactor);
        SunLight->SetLightColor(CurrentColor);
    }

    // ── Street Light Emissive Toggle ──
    bool bCurrentIsNight = IsNight();
    if (bCurrentIsNight != bPreviousIsNight)
    {
        bPreviousIsNight = bCurrentIsNight;

        FLinearColor TargetEmissive = bCurrentIsNight
            ? FLinearColor(1.0f, 0.45f, 0.05f)
            : FLinearColor(0.0f, 0.0f, 0.0f);

        for (UMaterialInstanceDynamic* MID : StreetLightMIDs)
        {
            if (MID)
                MID->SetVectorParameterValue(TEXT("EmissiveColor"), TargetEmissive);
        }

        UE_LOG(LogTemp, Verbose, TEXT("[DAYNIGHT] Street lights %s"),
            bCurrentIsNight ? TEXT("ON (night)") : TEXT("OFF (day)"));
    }
}
```

---

## File 3: Street Light Actor Blueprint Setup (Content Browser, not C++)

The C++ code discovers actors tagged `"StreetLight"`. The actual street light actor can be:

### Option A: Existing actor tagged in the level
1. Place any `UStaticMesh` actor in the level (e.g., a lamp post mesh)
2. In the **Details Panel → Actor → Tags**, add a tag: `StreetLight`
3. Assign a material that has an `"EmissiveColor"` vector parameter

### Option B: Create a Blueprint street light
1. Right-click in Content Browser → Blueprint Class → Actor
2. Name: `BP_StreetLight`
3. Add a `UStaticMeshComponent` as root
4. Assign a street lamp mesh
5. In the **Class Defaults → Tags**, add `"StreetLight"`

### Required Material Setup

The material on the street light mesh **must** have a **Vector Parameter** named `"EmissiveColor"` for the C++ code to control. If the material doesn't expose this parameter, the `SetVectorParameterValue` call silently does nothing.

**Recommended material graph (in UE5 Material Editor):**

```
TextureSample (BaseColor) → 
                             Multiply → MainMaterial (Emissive Color input)
VectorParameter("EmissiveColor") →     [multiply by a small intensity if needed]
```

Or simpler:
1. Right-click in the material graph → `VectorParameter`
2. Name: `EmissiveColor`
3. Default value: `(0, 0, 0)` (off)
4. Connect the output of the VectorParameter node to the **Emissive Color** input of the main material node
5. Set a small intensity multiplier (e.g., `Multiply * 2.0`) to boost the night glow

**Material Instance approach** (recommended for multiple street lights):
1. Create a **Material** (`M_StreetLight`) with the `EmissiveColor` parameter
2. Create **Material Instances** for variations (warm white, orange, etc.)
3. Assign instances to different street light meshes
4. The C++ code creates a **Dynamic** Material Instance from whatever material is assigned, so any material with the `EmissiveColor` param will work

---

## Compile Errors to Watch For

| Error | Cause | Fix |
|-------|-------|-----|
| `'UStaticMeshComponent' is not a known type` | Missing include in header | Add `#include "Components/StaticMeshComponent.h"` |
| `'UMaterialInstanceDynamic' is not a known type` | Missing include in header | Add `#include "Materials/MaterialInstanceDynamic.h"` |
| `'CreateAndSetMaterialInstanceDynamic' is not a member of 'UStaticMeshComponent'` | Wrong include | Inherited from `UPrimitiveComponent` — `#include "Components/PrimitiveComponent.h"` should already be pulled in via StaticMeshComponent.h |
| `'GetComponents<T>' is not a member of 'AActor'` | Wrong UE5 API | Use `Actor->GetComponents<UStaticMeshComponent>(MeshComps)` — requires `#include "Components/StaticMeshComponent.h"` in the .cpp |
| `'SetVectorParameterValue' is not a member of 'UMaterialInstanceDynamic'` | Wrong include | Must include `"Materials/MaterialInstanceDynamic.h"` (not just `"Engine/Material.h"`) |
| `'UGameplayStatics::GetAllActorsWithTag' unresolved` | Missing module dependency | Already included via `Kismet/GameplayStatics.h` — verify it's in the .cpp |
| `LNK2019: unresolved external symbol for UMaterialInstanceDynamic` | Missing module | Add `MaterialInstanceDynamic` to `PublicDependencyModuleNames` in `Build.cs` if not already there |
| `'GetComponents<UStaticMeshComponent>' does not compile` | Template not instantiated | Add `#include "Components/StaticMeshComponent.h"` before calling `GetComponents` |
| `'FLinearColor' is not a known type in new code` | Missing include | Add `#include "Math/Color.h"` — usually pulled by CoreMinimal.h, but add explicitly if needed |
| `warning C4267: conversion from 'size_t' to 'int32'` | Using `.Num()` without cast | StreetLightMeshes.Num() returns `int32` in UE5, so no cast needed |
| `UPROPERTY() on TArray<UMaterialInstanceDynamic*>` compile warning | UPROPERTY on raw pointer array | This is fine in UE5 — UPROPERTY() on a TArray of UObject* ensures GC traversal |

### Build.cs check

Verify that `ConcreteKingdom.Build.cs` (or similar) has these in `PublicDependencyModuleNames`:

```
"Engine",
"Kismet"     // for GameplayStatics
```

`UMaterialInstanceDynamic` is in the `Engine` module, so if `Engine` is already listed, no changes needed to Build.cs.

---

## Testing Plan

### Test 1: Compile
1. Open project in UE5.8
2. Build (Ctrl+Shift+F5)
3. Fix any compile errors (see table above)
4. Expected: no errors, link succeeds

### Test 2: No Street Lights in Level (null-safe)
1. Press Play with NO actors tagged "StreetLight"
2. Verify: no crash, log shows `[DAYNIGHT] Street light cache built, 0 meshes`
3. Verify: day/night cycle still works, sun rotates, light color blends

### Test 3: Single Street Light in Level
1. Place a cube/cylinder in the level
2. Give it a material with `"EmissiveColor"` vector parameter
3. Tag it `"StreetLight"`
4. Set `TimeOfDay = 21.0` (night) in CKDayNightComponent defaults
5. Press Play
6. Verify: street light emissive is warm orange/glowing
7. Change `TimeOfDay` to `12.0` — verify emissive turns black/off

### Test 4: Day/Night Transition
1. Place a street light actor in the level
2. Press Play
3. Wait for `TimeOfDay` to cross 20:00 (or 06:00)
4. Verify: street light emissive toggles ON at dusk, OFF at dawn
5. Verify: Output Log shows `[DAYNIGHT] Street lights ON (night)` and `[DAYNIGHT] Street lights OFF (day)`

### Test 5: Multiple Street Lights
1. Place 3+ street light actors at different locations
2. Verify: all toggle simultaneously at the transition point
3. Verify: each has the correct emissive intensity

### Test 6: Material Without EmissiveColor Parameter
1. Assign a plain material (e.g., `M_Basic_Floor`) to one street light mesh
2. Verify: no crash — `SetVectorParameterValue` on a MID where the parameter doesn't exist silently fails
3. Log is unaffected

### Test 7: Level Restart
1. Press Play, wait for night, verify lights on
2. `RestartLevel` in console
3. Verify: `RebuildStreetLightCache()` is called in `BeginPlay` — lights work after restart
4. Verify: no dangling pointers, no memory leak

### Test 8: Performance
1. Open `Stat GPU` and `Stat Engine`
2. Place 50+ street light actors
3. Verify: frame time impact at transition point is <0.1ms (just setting a vector parameter)
4. Verify: no per-frame overhead (the emissive toggle only runs on transition, not every tick)

---

## Implementation Order

| Step | File | Change |
|------|------|--------|
| 1 | `CKDayNightComponent.h` | Add includes: `StaticMeshComponent.h`, `MaterialInstanceDynamic.h` |
| 2 | `CKDayNightComponent.h` | Add `BeginPlay()` override declaration |
| 3 | `CKDayNightComponent.h` | Add `RebuildStreetLightCache()` public function |
| 4 | `CKDayNightComponent.h` | Add private members: `StreetLightMeshes`, `StreetLightMIDs`, `bPreviousIsNight` |
| 5 | `CKDayNightComponent.cpp` | Add includes for `StaticMeshComponent.h`, `MaterialInstanceDynamic.h` |
| 6 | `CKDayNightComponent.cpp` | Add `bPreviousIsNight = IsNight();` in constructor |
| 7 | `CKDayNightComponent.cpp` | Add `BeginPlay()` override with `RebuildStreetLightCache()` |
| 8 | `CKDayNightComponent.cpp` | Add `RebuildStreetLightCache()` implementation |
| 9 | `CKDayNightComponent.cpp` | Add TOD detection block in `TickComponent` |
| 10 | Content Browser | Create `M_StreetLight` material with `EmissiveColor` parameter |
| 11 | Content Browser | Create `BP_StreetLight` blueprint or tag existing actors |
| 12 | PIE Test | Verify toggle works at dusk/dawn |

Each step is independently testable — commit after each step.

---

## Design Decisions

### Why tag-based discovery instead of a custom actor class?
- **Looser coupling.** Any actor with a mesh and the right tag can become a street light
- **No new C++ class needed.** Designers can use blueprints or even existing static mesh actors
- **Easier level design.** No need to replace actors with a specific C++ class

### Why parallel arrays instead of TMap?
- `TArray<UStaticMeshComponent*>` + `TArray<UMaterialInstanceDynamic*>` is simpler than `TMap<UStaticMeshComponent*, UMaterialInstanceDynamic*>`
- Iteration over both arrays is O(n) with good cache locality
- Both arrays are UPROPERTY() — prevents GC of the MIDs

### Why CreateAndSetMaterialInstanceDynamic(0) instead of CreateDynamicMaterialInstance?
- `CreateAndSetMaterialInstanceDynamic(0)` creates a MID AND sets it as the override material for element 0 in one call
- `CreateDynamicMaterialInstance(0)` creates a MID but does NOT set it as the override — you'd need a second call to `SetMaterial()`
- `CreateAndSetMaterialInstanceDynamic` is simpler and matches the pattern already in `CKTrafficLightActor`

### Why only material element 0?
- Street lights typically have one material slot (the whole mesh uses one material)
- If a street light has multiple material elements (e.g., bulb glass, metal pole), only element 0 gets the MID
- For multi-element support, iterate `GetNumMaterials()` and create MIDs for each — but that's overengineering for v1

### Why not interpolate emissive over dusk/dawn?
- Binary toggle (on/off) is simpler and matches the GTA V behavior where street lights snap on/off at specific TOD thresholds
- Interpolation can be added later: `FLinearColor::LerpUsingHSV(off, on, DuskFactor)` — approximately 10 lines of code
- The emissive color can be set per-frame if needed, but the binary transition is more noticeable and gameplay-relevant
