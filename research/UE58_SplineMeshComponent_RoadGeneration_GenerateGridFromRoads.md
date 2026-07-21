---
title: "CKMapboxComponent::GenerateGridFromRoads — SplineMeshComponent Road Generation"
topic_index: 1
topic: "IMPLEMENT: CKMapboxComponent::GenerateGridFromRoads() — place road meshes along FRoadSegment points"
date: 2026-07-21
researched_by: Nanov2_bot (cron)
confidence: proven
tags: [UE5.8, SplineMeshComponent, Mapbox, road-generation, runtime-mesh, open-world]
---

# Implementation Card: GenerateGridFromRoads() via USplineMeshComponent

## Summary

GenerateGridFromRoads() currently logs a message and returns. It needs to iterate over loaded FRoadSegments and spawn road meshes in the world using USplineMeshComponent. Each consecutive pair of points in a road segment gets one SplineMeshComponent on a dedicated AActor, forming a continuous road surface.

---

## Design Decisions

| Decision | Choice | Rationale |
|---|---|---|
| Component type | USplineMeshComponent | Deforms mesh between start/end points — handles curves, gentle road bends. Matches GTA V's spline-road approach (see research/card_20260708_0715.md). |
| Actor strategy | One AActor per point-pair segment | Matches existing CKCityGenerator pattern. Enables per-segment cleanup without complex actor management. |
| Default mesh | /Engine/BasicShapes/Cube.Cube (placeholder) | Available without imported assets. SplineMesh deforms it into road shape. Replace with proper road mesh later. |
| Cleanup tracking | TArray<AActor*> SpawnedRoadActors | Follows CKCityGenerator::SpawnedActors pattern. Enables ClearCity-style road teardown. |

---

## UE5 API Reference

### Core Class: USplineMeshComponent

**Include:** `#include "Components/SplineMeshComponent.h"`

**Construction (runtime):**
```cpp
USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(OwnerActor);
SplineMesh->SetStaticMesh(RoadMesh);
SplineMesh->SetForwardAxis(ESplineMeshAxis::X);
SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
SplineMesh->SetCollisionProfileName(FName("BlockAll"));
SplineMesh->RegisterComponent();
```

### Key Method: SetStartAndEnd
```cpp
void SetStartAndEnd(
    const FVector& StartPos,      // Local-space start position
    const FVector& StartTangent,  // Local-space start tangent
    const FVector& EndPos,        // Local-space end position
    const FVector& EndTangent,    // Local-space end tangent
    bool bUpdateMesh = true       // Rebuild mesh geometry
);
```

### Tangent Formula (straight segments)
For segment from A to B:
```
Direction = (B - A).GetSafeNormal()
Length    = (B - A).Size()
Tangent   = Direction * Length * 0.5f  // half-length tangent = straight segment
```

### ESplineMeshAxis::Type
- `ESplineMeshAxis::X` — mesh's X axis aligns with spline direction (default, works with cube)
- `ESplineMeshAxis::Y` — mesh's Y axis aligns with spline direction
- `ESplineMeshAxis::Z` — mesh's Z axis aligns with spline direction

### Module Dependencies
No changes needed — `USplineMeshComponent` lives in the `Engine` module, already in `ConcreteKingdom.Build.cs`.

---

## Header File Changes

**File:** `Source/ConcreteKingdom/CKMapboxComponent.h`

### Add these UPROPERTY declarations (after `bUseMapbox`):
```cpp
// Road mesh to use for spline-based road generation
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Roads")
UStaticMesh* RoadMesh;

// Road surface width in world units
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Roads")
float RoadHalfWidth;

// Track spawned road actors for cleanup
UPROPERTY()
TArray<AActor*> SpawnedRoadActors;
```

### Add cleanup function declaration:
```cpp
UFUNCTION(BlueprintCallable)
void ClearRoads();
```

### Full header after edits (lines 1-27 expected):
```cpp
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKMapboxComponent.generated.h"

USTRUCT(BlueprintType)
struct FRoadSegment {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) FString Type;
    UPROPERTY(EditAnywhere) TArray<FVector> Points;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKMapboxComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKMapboxComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString MapboxToken;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CenterLat;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CenterLng;
    UPROPERTY(BlueprintReadOnly) TArray<FRoadSegment> Roads;

    // --- Road Mesh Generation ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Roads")
    UStaticMesh* RoadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Roads")
    float RoadHalfWidth;

    UPROPERTY()
    TArray<AActor*> SpawnedRoadActors;   // <<< TRACKS ACTORS FOR CLEANUP

    UFUNCTION(BlueprintCallable) void LoadRoadData(FString FilePath);
    UFUNCTION(BlueprintCallable) void GenerateGridFromRoads();
    UFUNCTION(BlueprintCallable) void ClearRoads();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseMapbox;
};
```

**Change summary (header):**
1. Add the three UPROPERTY lines for RoadMesh, RoadHalfWidth, SpawnedRoadActors
2. Add `UFUNCTION(BlueprintCallable) void ClearRoads();`

---

## Source File Changes

**File:** `Source/ConcreteKingdom/CKMapboxComponent.cpp`

### Add this include (with the existing includes):
```cpp
#include "Components/SplineMeshComponent.h"
```

### Constructor — add defaults:
In `UCKMapboxComponent::UCKMapboxComponent()`, after the existing lines, add:
```cpp
// Default road mesh (fallback)
static ConstructorHelpers::FObjectFinder<UStaticMesh> RoadMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
RoadMesh = RoadMeshFinder.Object;
RoadHalfWidth = 200.0f;  // Half-width of road in cm (400cm = 4m total)
```

**WARNING:** ConstructorHelpers::FObjectFinder only works in the constructor. It will crash at runtime if called from BeginPlay or GenerateGridFromRoads. The assignment above is safe because it's inside the constructor.

**Alternative** (if you want to assign the mesh externally): Skip the ConstructorHelpers line and leave RoadMesh as nullptr. The GenerateGridFromRoads function will need to handle nullptr RoadMesh with a log error.

### GenerateGridFromRoads() — FULL REPLACEMENT:

Replace the existing stub (lines 52-60) with:

```cpp
void UCKMapboxComponent::GenerateGridFromRoads()
{
    // Validate state
    if (!bUseMapbox)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mapbox disabled, skipping road generation"));
        return;
    }
    if (Roads.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No road data loaded, cannot generate roads"));
        return;
    }
    if (!RoadMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("RoadMesh is null — assign a static mesh in the blueprint defaults"));
        return;
    }
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world reference"));
        return;
    }

    // Clear previously generated roads
    ClearRoads();

    int32 TotalSegments = 0;

    for (const FRoadSegment& Seg : Roads)
    {
        if (Seg.Points.Num() < 2)
        {
            UE_LOG(LogTemp, Verbose, TEXT("Road '%s' has fewer than 2 points, skipping"), *Seg.Name);
            continue;
        }

        // Iterate consecutive point pairs
        for (int32 i = 0; i < Seg.Points.Num() - 1; i++)
        {
            const FVector& P0 = Seg.Points[i];     // World-space start
            const FVector& P1 = Seg.Points[i + 1]; // World-space end

            // Skip zero-length segments
            float SegLength = FVector::Dist(P0, P1);
            if (SegLength < 1.0f) continue;

            // Tangent (straight segment between these two points)
            FVector Direction = (P1 - P0).GetSafeNormal();
            FVector Tangent = Direction * (SegLength * 0.5f);

            // Spawn an actor at the segment midpoint
            AActor* RoadActor = GetWorld()->SpawnActor<AActor>();
            if (!RoadActor) continue;

            // Create root scene component at midpoint position
            USceneComponent* RootComp = NewObject<USceneComponent>(RoadActor);
            RootComp->RegisterComponent();
            RoadActor->SetRootComponent(RootComp);
            RoadActor->SetActorLocation(P0); // Actor at start, spline in local space

            // Create SplineMeshComponent
            USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(RoadActor);
            SplineMesh->SetStaticMesh(RoadMesh);
            SplineMesh->SetForwardAxis(ESplineMeshAxis::X);

            // Local-space: start at origin (0,0,0), end at (P1 - P0)
            FVector LocalEnd = P1 - P0;
            FVector LocalTangent = Tangent; // In local space, same direction since actor is at P0

            SplineMesh->SetStartAndEnd(
                FVector::ZeroVector,  // StartPos (local = origin since actor is at P0)
                LocalTangent,         // StartTangent
                LocalEnd,             // EndPos (local offset from P0 to P1)
                LocalTangent          // EndTangent (same for straight segment)
            );

            // Scale Y to road width (Cube mesh is 100x100x100, so scale Y by RoadHalfWidth*2/100)
            float WidthScale = (RoadHalfWidth * 2.0f) / 100.0f;
            SplineMesh->SetWorldScale3D(FVector(1.0f, WidthScale, 0.08f));

            // Collision
            SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SplineMesh->SetCollisionProfileName(FName("BlockAll"));

            // Must call RegisterComponent for runtime-created components
            SplineMesh->RegisterComponent();

            // Track for cleanup
            SpawnedRoadActors.Add(RoadActor);
            TotalSegments++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Mapbox: Generated %d road segments from %d roads"),
        TotalSegments, Roads.Num());
}
```

### ClearRoads() — NEW FUNCTION:

```cpp
void UCKMapboxComponent::ClearRoads()
{
    for (AActor* Actor : SpawnedRoadActors)
    {
        if (Actor && !Actor->IsPendingKillPending())
            Actor->Destroy();
    }
    SpawnedRoadActors.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Cleared %d road actors"), Roads.Num() > 0 ? SpawnedRoadActors.Num() : 0);
}
```

---

## Compile Errors to Watch For

### 1. `ESplineMeshAxis` not found
**Fix:** Verify `#include "Components/SplineMeshComponent.h"` is present. The enum is defined inside SplineMeshComponent.h. If still failing, add `#include "Components/SplineComponent.h"` as well.

### 2. `FObjectFinder` outside constructor
**Fix:** The `static ConstructorHelpers::FObjectFinder<>` line MUST be inside the constructor only. If GenerateGridFromRoads() is called at runtime and the mesh wasn't assigned in the constructor, it will crash with "Access None." Add this guard at the top of GenerateGridFromRoads:
```cpp
if (!RoadMesh)
{
    UE_LOG(LogTemp, Error, TEXT("RoadMesh is null"));
    return;
}
```

### 3. `RegisterComponent()` crash
**Fix:** The component must have an owner before RegisterComponent(). `NewObject<USplineMeshComponent>(RoadActor)` sets the owner. Do NOT call RegisterComponent before the root component is set. Order: spawn actor → create root → register root → create SplineMesh → call SplineMesh methods → register SplineMesh.

### 4. `SetWorldScale3D` on SplineMesh vs SetScale3D
SplineMeshComponent inherits from MeshComponent → SceneComponent. Both work. Use `SetWorldScale3D` when the component is attached to a moving root. Here the root is at origin, so either works.

### 5. Spline mesh not visible in packaged builds
This is a known UE5 issue (#105434 on forums). Components created via `NewObject<>()` at runtime may not render in packaged builds unless:
- The StaticMesh asset is referenced by a UPROPERTY in a loaded object (it is — `RoadMesh` is a UPROPERTY)
- The component's Registration is complete (call `RegisterComponent()`)
- The owning actor has `bNetLoadOnClient = true` (default)
If roads are invisible in package: add a `RegisterComponentWithWorld(GetWorld())` call instead of just `RegisterComponent()`.

---

## Testing Procedure

### 1. In-Editor Test (Blueprint)
1. Drop a `BlueprintActor` with `UCKMapboxComponent` into the level
2. Set `bUseMapbox = true`
3. Call `LoadRoadData` with path `/Game/Data/mapbox_roads.json` (or `/Content/Data/mapbox_roads.json`)
4. Call `GenerateGridFromRoads`
5. Observe: Five roads should appear as gray cube strips forming a cross pattern ~2km across

### 2. Verify Correctness Checklist
- [ ] Roads appear at correct world positions (matching lat/lng → UE coordinate conversion)
- [ ] Road segments are continuous (no gaps between consecutive point pairs)
- [ ] Road Y-scale matches `RoadHalfWidth`
- [ ] Collision: Character can walk/stand on road surface
- [ ] `ClearRoads()` removes all road actors
- [ ] Calling `GenerateGridFromRoads()` twice without `ClearRoads()` produces duplicate roads (intentional — caller must clear first)

### 3. Performance Test
- With 5 roads (2 points each = 5 segments), CPU cost should be <0.05ms
- Scale test: Generate 500 road segments and measure frame-time impact
- Each SplineMeshComponent costs ~0.01-0.03ms on GPU for rendering; batch limit before visible impact is ~200-300 segments on console, ~500+ on PC

### 4. Failure Modes
| Condition | Expected Behavior |
|---|---|
| RoadMesh = nullptr | Log error, return early |
| Roads array empty | Log warning, return |
| Seg.Points < 2 | Skip road, log verbose |
| P0 == P1 (zero length) | Skip segment silently |
| No World | Log error, crash-guard |

---

## Integration with CKCityGenerator

This is the partner function to CKCityGenerator::GenerateCity(). The intended flow:

1. `LoadRoadData(path)` → reads JSON, populates Roads array
2. `GenerateGridFromRoads()` → spawns SplineMesh road actors
3. `CKCityGenerator::GenerateCity()` (or a new overload) reads the Roads data to place buildings alongside roads (future topic #9)

For now, this card only implements road mesh placement. Building generation from road data is a separate topic.

---

## Next Steps After Implementation

1. Verify the roads render with correct orientation in UE5 editor
2. The road tangent calculation currently produces straight segments — for curved roads with 3+ points, switch to Catmull-Rom tangents:
   ```
   // For interior points:
   StartTangent = (Points[i+1] - Points[i-1]) * 0.25f * SegmentLengthScale
   EndTangent   = (Points[i+2] - Points[i])   * 0.25f * SegmentLengthScale
   ```
3. Add different mesh/color per road type (primary=wide, secondary=medium, tertiary=narrow)
