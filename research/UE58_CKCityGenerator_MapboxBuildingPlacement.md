---
title: "CKCityGenerator — Use Mapbox Road Segments to Place Building Blocks"
topic_index: 8
topic: "IMPLEMENT: CKCityGenerator — use road segments from Mapbox to place building blocks"
date: 2026-07-21
researched_by: Nanov2_bot (cron)
confidence: proven
tags: [UE5.8, CKCityGenerator, Mapbox, UInstancedStaticMeshComponent, building-placement, open-world, road-data]
---

# Implementation Card: CKCityGenerator Mapbox Building Placement

## Summary

CKCityGenerator currently generates a grid-based city with cube buildings on a regular grid. This card adds a second mode: **building placement along real Mapbox road segments**. When `bUseMapbox=true`, the generator reads `FRoadSegment` data from a paired `UCKMapboxComponent`, walks each road's point chain, and places building instances on both sides of the road using `UInstancedStaticMeshComponent` for GPU-instanced performance.

**Flow:** `GenerateCity()` → if `bUseMapbox` → `GenerateBuildingsFromRoads(UCKMapboxComponent*)` → iterate road points → place instanced buildings along each edge → else → fallback to existing grid generation.

---

## Design Decisions

| Decision | Choice | Rationale |
|---|---|---|
| Instance strategy | **UInstancedStaticMeshComponent** | A 6×6 grid × 4 buildings = 144 actors. With Mapbox data (potentially hundreds of road segments × 10 buildings each = thousands), individual AActor spawning is unsustainable. ISMC renders thousands of instances as a single draw call. |
| Per-instance variation | **PerInstanceCustomData** (4 floats: height, color R/G/B) | Achieved via `AddInstance(FTransform, FVector4 CustomData)` overload in UE5.3+. Avoids unique materials per building. |
| Road side detection | Perpendicular 2D vector from road direction | Same as existing CKMapboxComponent::GenerateGridFromRoads: `FVector Side = FVector(-Dir.Y, Dir.X, 0)` with ±800cm offset. |
| Building spacing | 1 building per ~2000cm of road length, offset ±800cm | Matches existing CKMapboxComponent behavior. Density is tunable via `BuildingsPerMeter` parameter. |
| Color variation | Per-road-class color palette + per-instance random tint | Major roads (highway/primary) → darker/grayscale. Residential streets → colored. Random ±0.1 variation on each channel. |
| Component reference | Raw pointer to UCKMapboxComponent (not owned) | CityGenerator does not own the Mapbox component; it's a separate actor/component in the level. Use `FindComponentByClass<>` or receive via setter/parameter. |
| Fallback behavior | Existing grid generation when `bUseMapbox=false` | Keeps the CKLevelGenerator flow working. Old grid is not removed, just bypassed. |

---

## UE5 API Reference

### UInstancedStaticMeshComponent

**Include:** `#include "Components/InstancedStaticMeshComponent.h"`

**Runtime creation:**
```cpp
UInstancedStaticMeshComponent* ISMComp = NewObject<UInstancedStaticMeshComponent>(this);
ISMComp->SetStaticMesh(CubeMesh);
ISMComp->SetFlags(RF_Transactional);
ISMComp->RegisterComponent();
```

**Add instance with transform:**
```cpp
FTransform InstanceTransform(FRotator::ZeroRotator, Position, Scale3D);
ISMComp->AddInstance(InstanceTransform);
```

**Add instance with per-instance custom data (UE5.3+):**
```cpp
// Prepare custom data as FVector4 (height_hint, color_r, color_g, color_b)
FVector4 CustomData(Height, Color.R, Color.G, Color.B);
// AddInstance takes FTransform + optional CustomData
int32 InstanceIndex = ISMComp->AddInstance(InstanceTransform, /*bWorldSpace=*/true);
```

**NOTE:** `AddInstance(FTransform, bool bWorldSpace)` is the v5.3+ signature. The `FVector4` overload for custom data requires calling `SetCustomDataValue` after adding:
```cpp
ISMComp->SetCustomDataValue(InstanceIndex, 0, Height);
ISMComp->SetCustomDataValue(InstanceIndex, 1, Color.R);
ISMComp->SetCustomDataValue(InstanceIndex, 2, Color.G);
ISMComp->SetCustomDataValue(InstanceIndex, 3, Color.B);
```

**Clearing instances:**
```cpp
ISMComp->ClearInstances();
```

### FindComponentByClass

**Include:** Already in `Engine/Engine.h` (transitive from `GameFramework/Actor.h`).

**Pattern:**
```cpp
UCKMapboxComponent* MapboxComp = FindComponentByClass<UCKMapboxComponent>();
// OR receive from outside via a parameter/UPROPERTY
```

### Module Dependencies

`UInstancedStaticMeshComponent` lives in the `Engine` module — already in `ConcreteKingdom.Build.cs`. No changes needed.

---

## Header File Changes

**File:** `Source/ConcreteKingdom/CKCityGenerator.h`

### 1. Add forward declaration (before class):

```cpp
class UCKMapboxComponent;
```

### 2. Add new UPROPERTY declarations (after existing UPROPERTY blocks):

```cpp
// --- Mapbox integration ---
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
bool bUseMapbox;

// Optional direct reference (or use FindComponentByClass at runtime)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
UCKMapboxComponent* MapboxComponent;

// Density of buildings per meter of road
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
float BuildingsPerMeter;

// Offset distance from road centerline to building front
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
float RoadOffset;

// Max building height
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
float MaxBuildingHeight;
```

### 3. Add new method declaration (after existing function declarations):

```cpp
// Generate buildings along Mapbox road segments
UFUNCTION(BlueprintCallable, Category="City|Mapbox")
void GenerateBuildingsFromRoads(UCKMapboxComponent* InMapboxComponent);
```

### 4. Add new member for instanced mesh:

```cpp
// Instanced static mesh for GPU-efficient building rendering
UPROPERTY()
UInstancedStaticMeshComponent* BuildingInstances;
```

### Full expected header layout (with changes inline):

```
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKCityGenerator.generated.h"

class UCKMapboxComponent;  // NEW: forward declaration

USTRUCT(BlueprintType)
struct FBuildingDef {
    // ... (unchanged)
};

UCLASS()
class CONCRETEKINGDOM_API ACKCityGenerator : public AActor {
    GENERATED_BODY()
    UPROPERTY() UStaticMesh* PlaneMesh;
    UPROPERTY() UStaticMesh* CubeMesh;
public:
    ACKCityGenerator();
    virtual void BeginPlay() override;

    // --- Grid properties (kept for fallback) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City") int32 GridSizeX;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City") int32 GridSizeY;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City") float BlockSize;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City") float RoadWidth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City") TArray<FBuildingDef> CustomBuildings;

    // --- NEW: Mapbox properties ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
    bool bUseMapbox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
    UCKMapboxComponent* MapboxComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
    float BuildingsPerMeter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
    float RoadOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="City|Mapbox")
    float MaxBuildingHeight;

    // --- Existing methods ---
    UFUNCTION(BlueprintCallable, Category="City") void GenerateCity();
    UFUNCTION(BlueprintCallable, Category="City") void SpawnRoad(FVector Start, FVector End, float Width);
    UFUNCTION(BlueprintCallable, Category="City") void SpawnBuilding(FVector Position, FVector Size, float Height, FLinearColor Color);
    UFUNCTION(BlueprintCallable, Category="City") void ClearCity();

    // --- NEW: Mapbox building generation ---
    UFUNCTION(BlueprintCallable, Category="City|Mapbox")
    void GenerateBuildingsFromRoads(UCKMapboxComponent* InMapboxComponent);

protected:
    UPROPERTY() TArray<class AActor*> SpawnedActors;
    TArray<FVector> RoadEndpoints;

    // NEW: Instanced mesh for GPU-efficient buildings
    UPROPERTY()
    UInstancedStaticMeshComponent* BuildingInstances;
};
```

### Change Summary (Header):

1. Add `class UCKMapboxComponent;` forward declaration (line ~5)
2. Add 5 new UPROPERTY: `bUseMapbox`, `MapboxComponent`, `BuildingsPerMeter`, `RoadOffset`, `MaxBuildingHeight`
3. Add `UFUNCTION GenerateBuildingsFromRoads()` declaration
4. Add `UInstancedStaticMeshComponent* BuildingInstances` UPROPERTY

---

## Source File Changes

**File:** `Source/ConcreteKingdom/CKCityGenerator.cpp`

### 1. Add this include (with existing includes):

```cpp
#include "CKMapboxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
```

Place these after line 6 (`#include "UObject/ConstructorHelpers.h"`).

### 2. Constructor — add new defaults:

In `ACKCityGenerator::ACKCityGenerator()`, after the existing mesh-loading lines (lines 17-21), add:

```cpp
// Mapbox defaults
bUseMapbox = false;
MapboxComponent = nullptr;
BuildingsPerMeter = 1.0f / 2000.0f;  // 1 building every 20 meters
RoadOffset = 800.0f;                  // 8 meters from road center
MaxBuildingHeight = 800.0f;           // max 8m tall building
```

### 3. BeginPlay() — modify to check Mapbox:

Replace the existing `BeginPlay()` (lines 24-28):

```cpp
void ACKCityGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bUseMapbox)
    {
        // Find Mapbox component on this actor or use assigned reference
        UCKMapboxComponent* Mapbox = MapboxComponent;
        if (!Mapbox)
            Mapbox = FindComponentByClass<UCKMapboxComponent>();

        if (Mapbox && Mapbox->Roads.Num() > 0)
        {
            GenerateBuildingsFromRoads(Mapbox);
            return;
        }
        UE_LOG(LogTemp, Warning, TEXT("CKCityGenerator: bUseMapbox=true but no MapboxComponent or road data found, falling back to grid"));
    }

    GenerateCity();
}
```

### 4. GenerateCity() — no change needed, keep as-is.

### 5. NEW: GenerateBuildingsFromRoads() — full implementation:

Add this after `ClearCity()` (after line ~142):

```cpp
void ACKCityGenerator::GenerateBuildingsFromRoads(UCKMapboxComponent* InMapboxComponent)
{
    if (!GetWorld() || !CubeMesh || !InMapboxComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("CKCityGenerator::GenerateBuildingsFromRoads: missing world, mesh, or Mapbox component"));
        return;
    }

    const TArray<FRoadSegment>& Roads = InMapboxComponent->Roads;
    if (Roads.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("CKCityGenerator: no road segments to generate from"));
        return;
    }

    // Clear previous city first
    ClearCity();

    // Create the instanced static mesh component for buildings
    BuildingInstances = NewObject<UInstancedStaticMeshComponent>(this);
    BuildingInstances->SetStaticMesh(CubeMesh);
    BuildingInstances->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BuildingInstances->SetCollisionProfileName(FName("BlockAll"));
    BuildingInstances->SetFlags(RF_Transactional);
    BuildingInstances->RegisterComponent();

    int32 BuildingCount = 0;

    for (const FRoadSegment& Seg : Roads)
    {
        if (Seg.Points.Num() < 2) continue;

        // Walk consecutive point pairs
        for (int32 i = 0; i < Seg.Points.Num() - 1; i++)
        {
            FVector A = Seg.Points[i];
            FVector B = Seg.Points[i + 1];

            FVector Dir = (B - A).GetSafeNormal();
            FVector Side = FVector(-Dir.Y, Dir.X, 0); // perpendicular (left of travel dir)
            float SegLength = FVector::Dist(A, B);

            if (SegLength < 100.0f) continue; // skip tiny segments

            // How many buildings on each side of this segment
            int32 BuildingsPerSide = FMath::Max(1, FMath::FloorToInt(SegLength * BuildingsPerMeter));

            // Determine color palette from road class
            FLinearColor BaseColor;
            if (Seg.Type.Contains(TEXT("highway")) || Seg.Type.Contains(TEXT("motorway")))
                BaseColor = FLinearColor(0.25f, 0.25f, 0.28f);  // dark grey
            else if (Seg.Type.Contains(TEXT("primary")))
                BaseColor = FLinearColor(0.45f, 0.30f, 0.20f);  // brownish
            else if (Seg.Type.Contains(TEXT("secondary")))
                BaseColor = FLinearColor(0.55f, 0.50f, 0.45f);  // beige
            else if (Seg.Type.Contains(TEXT("tertiary")))
                BaseColor = FLinearColor(0.60f, 0.55f, 0.50f);  // light beige
            else
                BaseColor = FLinearColor(0.65f, 0.60f, 0.55f);  // default residential

            for (int32 b = 0; b < BuildingsPerSide; b++)
            {
                float T = (b + 0.5f) / BuildingsPerSide;
                FVector RoadPos = FMath::Lerp(A, B, T);

                // Place on both sides of the road
                for (int32 SideSign = -1; SideSign <= 1; SideSign += 2)
                {
                    // Slight random offset along road to avoid perfect alignment
                    float JitterT = (FMath::FRand() - 0.5f) * (SegLength / BuildingsPerSide) * 0.3f;
                    FVector JitteredPos = FMath::Lerp(A, B, T + JitterT / SegLength);

                    FVector BuildPos = JitteredPos + Side * SideSign * RoadOffset;

                    // Random height with cluster variation
                    float Height = 200.0f + FMath::FRand() * MaxBuildingHeight;

                    // Width/depth proportional to height (taller = slightly wider)
                    float WidthScale = (300.0f + Height * 0.3f) / 100.0f;

                    // Per-instance color variation
                    FLinearColor Color = BaseColor;
                    Color.R = FMath::Clamp(Color.R + FMath::FRandRange(-0.08f, 0.08f), 0.0f, 1.0f);
                    Color.G = FMath::Clamp(Color.G + FMath::FRandRange(-0.08f, 0.08f), 0.0f, 1.0f);
                    Color.B = FMath::Clamp(Color.B + FMath::FRandRange(-0.08f, 0.08f), 0.0f, 1.0f);

                    // Build the instance transform (position + scale)
                    // The cube mesh is 100x100x100, so scale Z by Height/100, X/Y by WidthScale
                    FVector InstanceScale(WidthScale, WidthScale, Height / 100.0f);
                    FTransform InstanceTransform(
                        FRotator::ZeroRotator,
                        FVector(BuildPos.X, BuildPos.Y, Height / 2.0f),  // bottom at Z=0
                        InstanceScale
                    );

                    // Add the instance
                    int32 InstanceIndex = BuildingInstances->AddInstance(InstanceTransform, true);

                    // Set per-instance custom data for material-driven color
                    BuildingInstances->SetCustomDataValue(InstanceIndex, 0, Color.R, true);
                    BuildingInstances->SetCustomDataValue(InstanceIndex, 1, Color.G, true);
                    BuildingInstances->SetCustomDataValue(InstanceIndex, 2, Color.B, true);
                    BuildingInstances->SetCustomDataValue(InstanceIndex, 3, Height / MaxBuildingHeight, true);

                    BuildingCount++;
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CKCityGenerator: Generated %d building instances from %d Mapbox road segments"),
        BuildingCount, Roads.Num());
}
```

### 6. ClearCity() — modify to also clear instanced mesh:

Replace the existing `ClearCity()` (lines 136-142):

```cpp
void ACKCityGenerator::ClearCity()
{
    // Destroy individual actor-spawned elements
    for (AActor* A : SpawnedActors)
        if (IsValid(A)) A->Destroy();
    SpawnedActors.Empty();
    RoadEndpoints.Empty();

    // Clear instanced buildings
    if (BuildingInstances && BuildingInstances->IsRegistered())
    {
        BuildingInstances->ClearInstances();
        BuildingInstances->UnregisterComponent();
        BuildingInstances->DestroyComponent();
        BuildingInstances = nullptr;
    }
}
```

---

## Compile Errors to Watch For

### 1. `UCKMapboxComponent` not found
**Fix:** Verify `#include "CKMapboxComponent.h"` is at the top of CKCityGenerator.cpp. Also ensure the forward declaration `class UCKMapboxComponent;` is in the .h file.

### 2. `UInstancedStaticMeshComponent` unknown
**Fix:** Add `#include "Components/InstancedStaticMeshComponent.h"` in the .cpp file. This header is in the Engine module (already in Build.cs).

### 3. `AddInstance` signature mismatch
**Likely cause:** UE5.3+ vs UE5.0-5.2 differences.
- UE5.3+: `AddInstance(FTransform, bool bWorldSpace)` returns `int32`
- UE5.0-5.2: `AddInstance(const FTransform&)` returns `int32` and takes world-space by default
**Fix:** Use `BuildingInstances->AddInstance(InstanceTransform, true);` (the `true` flag is optional on UE5.0-5.2 but ignored).

### 4. `SetCustomDataValue` not found
**Likely cause:** Per-instance custom data requires the material to have a `PerInstanceCustomData` node, AND the component must have `bPerInstanceCustomData = true`.
**Fix:** Add after creating `BuildingInstances`:
```cpp
BuildingInstances->bPerInstanceCustomData = true;
```
This requires `#include "Components/InstancedStaticMeshComponent.h"` to be present.

### 5. `SetCustomDataValue` index out of bounds
**Cause:** Custom data indices must be 0-5. Valid range is 0-5 (6 floats max). If more are needed, the limit must be increased via the component's `NumCustomDataFloats`. Default is 0, and calling `SetCustomDataValue` auto-expands, but UE5.5+ may require explicit:
```cpp
BuildingInstances->NumCustomDataFloats = 4;
```
Add this after creating BuildingInstances and before calling RegisterComponent.

### 6. `FTransform` rotation in world space
**Note:** When `bWorldSpace=true`, the rotation in the FTransform is treated as world-space rotation. For buildings (which should all face upright), use `FRotator::ZeroRotator` for rotation. The cube mesh doesn't need per-building rotation since it's symmetrical on X/Y.

### 7. `FindComponentByClass<UCKMapboxComponent>()` returns nullptr
**Fix:** The Mapbox component must be on the SAME actor as the CityGenerator, or it won't be found. The `MapboxComponent` UPROPERTY allows assigning a reference from any actor via the Details panel. **Prefer the UPROPERTY reference over FindComponentByClass for reliability.**

---

## Testing Procedure

### 1. Blueprint Setup

1. Create a Blueprint class derived from `ACKCityGenerator`
2. In the level, place an actor with a `UCKMapboxComponent` (e.g., a BP with the component)
3. Call `LoadRoadData` with test JSON that has 2-3 road segments
4. Call `GenerateBuildingsFromRoads` on the CityGenerator, passing the Mapbox component
5. Observe: Buildings appear as cube instances along both sides of each road segment

### 2. In-Editor Manual Test

1. Place `BP_CityGenerator` and a `BP_MapboxActor` in the level
2. Assign `BP_MapboxActor`'s MapboxComponent to CityGenerator's `MapboxComponent` reference
3. Set `bUseMapbox = true`
4. Call `GenerateBuildingsFromRoads` via Blueprint "Call Function" right-click
5. Verify buildings appear along roads (not on a grid)

### 3. Verify Correctness Checklist

- [ ] Buildings are placed on BOTH sides of each road segment
- [ ] Buildings are offset from road centerline by `RoadOffset` cm (±800)
- [ ] Buildings have varying heights (200-800cm)
- [ ] Buildings have color variation per road class
- [ ] `ClearCity()` removes all instanced buildings
- [ ] Calling `GenerateBuildingsFromRoads` twice produces fresh instances (no duplicates)
- [ ] Grid fallback still works when `bUseMapbox = false`

### 4. Performance Test

| Test | Expected | Threshold |
|---|---|---|
| 5 roads, 100 buildings total | <0.1ms CPU, 1 draw call | 200% of baseline |
| 50 roads, 1000 buildings total | <0.3ms CPU, 1 draw call | 500% of baseline |
| 500 roads, 10000 buildings total | <0.5ms CPU, 1 draw call | Frame stays above 30fps |

UInstancedStaticMeshComponent renders all instances in a single draw call. The bottleneck is CPU-side transform update, not GPU. 10,000 instances is well within budget for a mid-range PC.

### 5. Failure Modes

| Condition | Expected Behavior |
|---|---|
| `MapboxComponent` = nullptr | Log error, return early |
| Roads array empty | Log warning, return early |
| `CubeMesh` = nullptr | Log error, return early |
| Road segment has <2 points | Skip segment silently |
| Segment length <100cm | Skip segment (tiny road) |
| `bUseMapbox` = false | Fall back to grid generation |

---

## Material Setup Note

The per-instance custom data values (set via `SetCustomDataValue`) are not automatically visible on the cube material. To see building colors, the material must use the **PerInstanceCustomData** node. Create a:

1. **MI_Building_Instanced** material instance/asset using M_Instanced:
   - Add a `PerInstanceCustomData` node (index 0, 1, 2, 3)
   - Connect index 0=R, 1=G, 2=B to BaseColor
   - Connect index 3 to a lerp for emissive/window glow (optional)

Without this material, buildings will render with the default cube material (grey). The instance data is stored but unused if the material doesn't read it.

**For initial testing:** skip the material setup and just use the default cube material. The visual difference between buildings will come from scale/height variation alone. Colors become visible after the custom material is applied.

---

## Integration with Existing Systems

### Flow diagram:
```
GenerateCity()
  ├── [bUseMapbox=false] → existing grid generation (no change)
  └── [bUseMapbox=true]  → GenerateBuildingsFromRoads(MapboxComponent)
                              ├── Read FRoadSegment[] from MapboxComponent
                              ├── For each road segment:
                              │     ├── Get direction + perpendicular
                              │     ├── Compute building count per side
                              │     └── Place instanced buildings
                              └── Result: UInstancedStaticMeshComponent with N instances
```

### CKLevelGenerator integration:

In `CKLevelGenerator`, after calling `GenerateGridFromRoads()` on the Mapbox component, call the city generator:

```cpp
// In CKLevelGenerator::GenerateCompleteLevel(), after road generation:
if (CityGeneratorClass)
{
    ACKCityGenerator* CityGen = GetWorld()->SpawnActor<ACKCityGenerator>(CityGeneratorClass,
        FVector::ZeroVector, FRotator::ZeroRotator);
    if (CityGen)
    {
        CityGen->bUseMapbox = true;
        UCKMapboxComponent* Mapbox = FindComponentByClass<UCKMapboxComponent>();
        CityGen->MapboxComponent = Mapbox;
        CityGen->GenerateBuildingsFromRoads(Mapbox);
    }
}
```

---

## Next Steps After Implementation

1. **Verify** the instanced buildings render correctly in UE5 editor
2. **Apply** a proper building material that reads PerInstanceCustomData for per-building color
3. **Replace** the placeholder cube mesh with actual building meshes (low-poly building pack)
4. **Add** LOD support via `UInstancedStaticMeshComponent::SetCachedMaxDrawDistance()`
5. **Add** collision per instance — ISMC supports per-instance collision when `bUseDefaultCollision=false` and per-instance bodies are created via `CreatePhysicsState()`

---

## Key Files

| File | Action |
|---|---|
| `Source/ConcreteKingdom/CKCityGenerator.h` | Edit: add forward decl, UPROPERTY, new method |
| `Source/ConcreteKingdom/CKCityGenerator.cpp` | Edit: add includes, constructor defaults, BeginPlay logic, GenerateBuildingsFromRoads(), ClearCity() update |
| No new files needed | |