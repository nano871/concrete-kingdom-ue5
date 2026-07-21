---
title: "CKPedestrianAIController — Add idle animations, path following on sidewalks"
topic_index: 9
topic: "IMPLEMENT: CKPedestrianAIController — add idle animations, path following on sidewalks"
date: 2026-07-21
researched_by: Nanov2_bot (cron)
confidence: strongly_supported
tags: [UE5.8, AI, pedestrian, animation, pathfinding, AIController, ACharacter, AnimMontage, UAnimInstance, sidewalk, NavMesh, open-world]
---

# Implementation Card: Pedestrian AI — Idle Animations & Sidewalk Path Following

## Summary

CKPedestrianAIController exists with basic wander/flee logic but **no animation playback** and **no sidewalk-constrained path following**. Pedestrians glide in T-pose and wander through roads instead of staying on sidewalks. This card fixes both.

Two changes required:
1. **CKPedestrianAIController.h/.cpp** — Add idle animation montage playback, animation state parameter setting, and tighter MoveToLocation parameters for sidewalk path following.
2. **Animation Blueprint (Content Browser asset)** — Must be created in the editor to read the animation state parameters and drive the character mesh.

---

## UE5.8 API Reference

### Key Classes and Includes

```cpp
// Animation
#include "Animation/AnimMontage.h"          // UAnimMontage — animation asset
#include "Animation/AnimInstance.h"          // UAnimInstance — runtime anim controller
#include "Components/SkeletalMeshComponent.h" // USkeletalMeshComponent — GetMesh()
#include "GameFramework/Character.h"         // ACharacter — PlayAnimMontage()

// Path Following
// Already included: "NavigationSystem.h", "AIController.h"
// MoveToLocation is a member of AAIController (no extra include)
```

### Build.cs Dependencies

Already present in `ConcreteKingdom.Build.cs`:
- `"AIModule"` — for AAIController, MoveToLocation
- `"NavigationSystem"` — for UNavigationSystemV1, FNavLocation
- `"Engine"` — for ACharacter, UAnimInstance, UAnimMontage

No new dependencies needed.

### AAIController::MoveToLocation — Full Signature

From UE5.8 source (`AIController.h`):

```cpp
UFUNCTION(BlueprintCallable, Category = "AI")
virtual EPathFollowingRequestResult::Type MoveToLocation(
    const FVector& Dest,
    float AcceptanceRadius = UPathFollowingComponent::DefaultAcceptanceRadius,  // -1 = use default
    bool bStopOnOverlap = true,
    bool bUsePathfinding = true,
    bool bProjectDestinationToNavigation = false,
    bool bCanStrafe = false,
    TSubclassOf<UNavigationQueryFilter> FilterClass = nullptr,
    bool bAllowPartialPath = true
);
```

| Parameter | Default | Recommended for Sidewalks | Why |
|-----------|---------|--------------------------|-----|
| `AcceptanceRadius` | -1 (defaults to 50) | **30.0f** | Pedestrians walk tight. 30 units = ~1 sidewalk tile width. |
| `bStopOnOverlap` | true | **true** | Stop when overlapping destination, not just when within radius. |
| `bUsePathfinding` | true | **true** | Must stay on NavMesh. |
| `bProjectDestinationToNavigation` | false | **true** | Ensures destination is on NavMesh before pathfinding. |
| `bCanStrafe` | false | **false** | Pedestrians face movement direction, not target. |
| `FilterClass` | nullptr | **nullptr** | Default navigation filter is fine. |
| `bAllowPartialPath` | true | **false** | CRITICAL: Don't accept partial paths — pedestrian should not walk off NavMesh edge. |

### Return Value (EPathFollowingRequestResult::Type)

```
// Check if path was found
EPathFollowingRequestResult::Type Result = MoveToLocation(...);
if (Result == EPathFollowingRequestResult::Failed) { /* handle */ }
```

### ACharacter::PlayAnimMontage — Full Signature

```cpp
UFUNCTION(BlueprintCallable, Category = "Animation")
virtual float PlayAnimMontage(
    class UAnimMontage* AnimMontage,
    float InPlayRate = 1.0f,
    FName StartSectionName = NAME_None
);
```

Returns: length of montage in seconds, or 0.0f if failed.

### UAnimInstance — Setting Animation Parameters

The Animation Blueprint reads variables from the UAnimInstance. In C++, we access it via the character's mesh and set float/bool parameters:

```cpp
// Get the AnimInstance from the character's skeletal mesh
UAnimInstance* AnimInst = GetPawn()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();
// OR via ACharacter cast:
UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();

// Set parameters the Animation Blueprint reads
AnimInst->SetFloatParameter(TEXT("Speed"), CurrentSpeed);
AnimInst->SetBoolParameter(TEXT("bIsMoving"), bIsMoving);
AnimInst->SetBoolParameter(TEXT("bIsFleeing"), bFleeing);
```

**Important:** `SetFloatParameter`/`SetBoolParameter` require the Animation Blueprint to have matching parameter names in its variable list. These are NOT automatic — they must be manually created in the AnimBP (see § Asset Requirements below).

| Method | Signature | Notes |
|--------|-----------|-------|
| `SetFloatParameter` | `void SetFloatParameter(FName ParamName, float Value)` | Sets a float variable in the AnimBP. Thread-safe variant: `SetFloatParameter_ThreadSafe`. |
| `SetBoolParameter` | `void SetBoolParameter(FName ParamName, bool Value)` | Sets a bool variable in the AnimBP. |
| `Montage_Play` | `float Montage_Play(UAnimMontage* Montage, float InPlayRate = 1.0f, EMontagePlayReturnType ReturnType = EMontagePlayReturnType::MontageLength, float InTimeToStartMontageAt = 0.0f, bool bStopAllMontages = true)` | Plays a montage directly on the AnimInstance. |

---

## Asset Requirements (Content Browser)

These assets must be created in the Content Browser before the code will produce visible results:

### Animation Blueprint

Create: Right-click a skeletal mesh → Create → Anim Blueprint → Name: `ABP_Pedestrian`

In the AnimBP:
1. **Variables tab**: Create these variables:
   - `Speed` (float, default 0.0) — set from C++ via `SetFloatParameter(TEXT("Speed"), ...)`
   - `bIsMoving` (bool, default false) — set from C++ via `SetBoolParameter(TEXT("bIsMoving"), ...)`
   - `bIsFleeing` (bool, default false) — set from C++ via `SetBoolParameter(TEXT("bIsFleeing"), ...)`
2. **AnimGraph**: Create a state machine with:
   - **Idle** state → plays idle animation blend (e.g., `Idle_Stand`, `Idle_LookAround`)
   - **Walk** state → plays walk cycle, uses Speed to blend between walk speeds
   - **Flee** state → plays run cycle
   - Transitions: Idle→Walk when `bIsMoving == true`, Walk→Idle when `bIsMoving == false`, anything→Flee when `bIsFleeing == true`, Flee→Walk when `bIsFleeing == false && bIsMoving == true`, Flee→Idle when `bIsFleeing == false && bIsMoving == false`
3. **Event Graph**: Call `SetFloatParameter` from the `TryGetPawnOwner` → GetVelocity → VectorLength to get Speed (this is the fallback if C++ doesn't set it — but the C++ code WILL set it).

### Animation Montages (for idle animations)

Create one or more UAnimMontage assets:
- `/Game/Animations/Pedestrian/Idle_Stand` — simple standing idle (looping)
- `/Game/Animations/Pedestrian/Idle_LookAround` — idle with head-turning (looping or section-based)
- `/Game/Animations/Pedestrian/Idle_Phone` — idle checking phone (looping)

Each montage should be set to **Looping** in the Montage editor (Section → Loop toggle).

### Skeletal Mesh Assignment

The pedestrian character blueprint (e.g., `BP_PedestrianCharacter`) must have:
1. A **Skeletal Mesh** assigned (e.g., `SK_Mannequin` or custom pedestrian mesh)
2. The **Animation Blueprint** (`ABP_Pedestrian`) assigned in the mesh component's Anim Class field
3. An **Animation Montage** referenced in the `IdleMontages` array

---

## File 1: CKPedestrianAIController.h — Changes Required

### Current Header (23 lines)

The header currently has: Wander(), flee logic, but no animation support and no sidewalk path following.

### New Header

Replace the entire file with:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CKPedestrianAIController.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKPedestrianAIController : public AAIController {
    GENERATED_BODY()
public:
    ACKPedestrianAIController();
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable) void SetPedestrianType(FString Type);
    UFUNCTION(BlueprintCallable) void ReactToPlayer(float PlayerSpeed, float Distance);

    // ── Animation ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<class UAnimMontage*> IdleMontages;          // Random idle animations to play

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float IdleMontagePlayRate;                         // Speed multiplier for idle anims

    // ── Path Following ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float SidewalkAcceptanceRadius;                    // How close to destination before stopping

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bAllowPartialPaths;                           // Allow partial paths (false = must reach full path)

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayRandomIdle();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopIdle();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationState();

protected:
    void Wander();
    void MoveToOnSidewalk(FVector Destination);

    FVector WanderOrigin;
    FTimerHandle WanderTimer;
    FString PedType;
    float BaseSpeed;
    bool bFleeing;
    FVector FleeDirection;

    // Animation state
    bool bIsMoving;
    class UAnimMontage* CurrentIdleMontage;            // Track which montage is playing
    FTimerHandle IdleTimer;                            // Timer for next idle animation
};
```

### What changed (line-by-line):

| Lines | Change | Reason |
|-------|--------|--------|
| 1-4 | Pragma + includes | No change needed. CoreMinimal, AIController, generated.h present. |
| 21-22 | Added `IdleMontages` UPROPERTY + `IdleMontagePlayRate` | Asset references for random idle animations. Array so multiple idles can be assigned in BP. |
| 25-26 | Added `SidewalkAcceptanceRadius`, `bAllowPartialPaths` | Path following parameters for tight sidewalk navigation. |
| 28-29 | Added `PlayRandomIdle()`, `StopIdle()` | Public API for animation control. |
| 30-31 | Added `UpdateAnimationState()` | Called every tick to update AnimBP parameters. |
| 37 | Changed `Wander()` from private to protected | Now calls `MoveToOnSidewalk()` instead of raw `MoveToLocation()`. |
| 38 | Added `MoveToOnSidewalk(FVector)` | New method wrapping MoveToLocation with sidewalk-appropriate parameters. |
| 44-45 | Added `bIsMoving`, `CurrentIdleMontage`, `IdleTimer` | Animation state tracking. |

---

## File 2: CKPedestrianAIController.cpp — Changes Required

### Constructor — Add defaults

**Current code (lines 8-12):**
```cpp
ACKPedestrianAIController::ACKPedestrianAIController()
{
    BaseSpeed = 60.0f;
    bFleeing = false;
}
```

**New code:**
```cpp
ACKPedestrianAIController::ACKPedestrianAIController()
{
    BaseSpeed = 60.0f;
    bFleeing = false;
    bIsMoving = false;
    CurrentIdleMontage = nullptr;
    IdleMontagePlayRate = 1.0f;
    SidewalkAcceptanceRadius = 30.0f;     // ~1 sidewalk tile width
    bAllowPartialPaths = false;            // Must reach destination on NavMesh
}
```

### BeginPlay() — Add animation start

**Current code (lines 14-21):**
```cpp
void ACKPedestrianAIController::BeginPlay()
{
    Super::BeginPlay();
    if (GetPawn()) {
        WanderOrigin = GetPawn()->GetActorLocation();
        GetWorld()->GetTimerManager().SetTimer(WanderTimer, this, &ACKPedestrianAIController::Wander, 3.0f, true);
    }
}
```

**New code:**
```cpp
void ACKPedestrianAIController::BeginPlay()
{
    Super::BeginPlay();
    if (GetPawn()) {
        WanderOrigin = GetPawn()->GetActorLocation();

        // Start wandering after a delay
        GetWorld()->GetTimerManager().SetTimer(WanderTimer, this, &ACKPedestrianAIController::Wander, 3.0f, true);

        // Play initial idle animation
        PlayRandomIdle();
    }
}
```

### After SetPedestrianType() — Add Wander() after flee ends

**Current code (lines 32-54):** ReactToPlayer() has a flee timer that calls Wander() after 5 seconds. This is fine — keep as-is.

**BUT** — the Wander() call at line 49 should be changed to `Wander()` (already is). It also needs to restart the idle animation cycle. Add this to the flee-end lambda:

**Change in ReactToPlayer() (around line 49):**
```cpp
// Inside the UnfleeTimer lambda, after the existing code:
GetWorld()->GetTimerManager().SetTimer(
    UnfleeTimer,
    FTimerDelegate::CreateLambda([this]() {
        bFleeing = false;
        if (GetPawn() && GetPawn()->FindComponentByClass<UCharacterMovementComponent>())
            GetPawn()->FindComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = BaseSpeed;
        Wander();
        // ADD: Restart idle animation after fleeing
        PlayRandomIdle();
    }),
    5.0f, false
);
```

### Tick() — Add animation state update

**Current code (lines 56-65):**
```cpp
void ACKPedestrianAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Player && GetPawn()) {
        float Dist = GetPawn()->GetDistanceTo(Player);
        float Speed = Player->GetVelocity().Size();
        ReactToPlayer(Speed, Dist);
    }
}
```

**New code:**
```cpp
void ACKPedestrianAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ── Update animation state every frame ──
    UpdateAnimationState();

    // ── Player reactivity ──
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Player && GetPawn()) {
        float Dist = GetPawn()->GetDistanceTo(Player);
        float Speed = Player->GetVelocity().Size();
        ReactToPlayer(Speed, Dist);
    }
}
```

### ADD — MoveToOnSidewalk method (insert after Tick, before Wander)

```cpp
void ACKPedestrianAIController::MoveToOnSidewalk(FVector Destination)
{
    if (!GetPawn()) return;

    EPathFollowingRequestResult::Type Result = MoveToLocation(
        Destination,
        SidewalkAcceptanceRadius,   // 30.0f — tight sidewalk stop
        true,                        // bStopOnOverlap — stop when overlapping dest
        true,                        // bUsePathfinding — must use NavMesh
        true,                        // bProjectDestinationToNavigation — snap to NavMesh
        false,                       // bCanStrafe — face movement direction
        nullptr,                     // FilterClass — default nav filter
        bAllowPartialPaths           // false — don't accept partial paths
    );

    if (Result == EPathFollowingRequestResult::Failed)
    {
        // If can't reach exact destination, try a nearby point
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSys)
        {
            FNavLocation NavLocation;
            if (NavSys->GetRandomReachablePointInRadius(Destination, 200.0f, NavLocation))
            {
                MoveToLocation(
                    NavLocation.Location,
                    SidewalkAcceptanceRadius,
                    true, true, true, false, nullptr, false
                );
            }
        }
    }
}
```

### ADD — PlayRandomIdle method (insert after MoveToOnSidewalk)

```cpp
void ACKPedestrianAIController::PlayRandomIdle()
{
    if (!GetPawn() || bFleeing || bIsMoving) return;

    // Don't interrupt if currently playing an idle
    if (CurrentIdleMontage && GetPawn()->GetCurrentMontage() == CurrentIdleMontage) return;

    // Pick a random idle montage from the array
    if (IdleMontages.Num() == 0) return;

    int32 Index = FMath::RandRange(0, IdleMontages.Num() - 1);
    UAnimMontage* SelectedMontage = IdleMontages[Index];
    if (!SelectedMontage) return;

    // Play on the character mesh
    ACharacter* Char = Cast<ACharacter>(GetPawn());
    if (Char)
    {
        Char->PlayAnimMontage(SelectedMontage, IdleMontagePlayRate);
        CurrentIdleMontage = SelectedMontage;
        UE_LOG(LogTemp, Verbose, TEXT("[PED] Playing idle montage #%d"), Index);
    }

    // Schedule next idle animation (random interval 3-8 seconds if still idle)
    GetWorld()->GetTimerManager().ClearTimer(IdleTimer);
    GetWorld()->GetTimerManager().SetTimer(
        IdleTimer,
        this,
        &ACKPedestrianAIController::PlayRandomIdle,
        FMath::RandRange(3.0f, 8.0f),
        false
    );
}
```

### ADD — StopIdle method (insert after PlayRandomIdle)

```cpp
void ACKPedestrianAIController::StopIdle()
{
    if (!GetPawn()) return;

    ACharacter* Char = Cast<ACharacter>(GetPawn());
    if (Char && Char->GetMesh())
    {
        UAnimInstance* AnimInst = Char->GetMesh()->GetAnimInstance();
        if (AnimInst && CurrentIdleMontage)
        {
            AnimInst->Montage_Stop(0.25f, CurrentIdleMontage);  // 0.25s blend-out
            CurrentIdleMontage = nullptr;
        }
    }

    GetWorld()->GetTimerManager().ClearTimer(IdleTimer);
}
```

### ADD — UpdateAnimationState method (insert after StopIdle)

```cpp
void ACKPedestrianAIController::UpdateAnimationState()
{
    if (!GetPawn()) return;

    ACharacter* Char = Cast<ACharacter>(GetPawn());
    if (!Char || !Char->GetMesh()) return;

    UAnimInstance* AnimInst = Char->GetMesh()->GetAnimInstance();
    if (!AnimInst) return;

    // Calculate movement speed of the pawn
    float CurrentSpeed = GetPawn()->GetVelocity().Size();
    bool bNowMoving = CurrentSpeed > 10.0f;  // Threshold: 10 cm/s = walking

    // State transitions
    if (bNowMoving && !bIsMoving)
    {
        // Just started moving — stop idle animation
        StopIdle();
    }
    else if (!bNowMoving && bIsMoving)
    {
        // Just stopped moving — start idle animation
        PlayRandomIdle();
    }

    bIsMoving = bNowMoving;

    // Set animation parameters for the Animation Blueprint
    AnimInst->SetFloatParameter(TEXT("Speed"), CurrentSpeed);
    AnimInst->SetBoolParameter(TEXT("bIsMoving"), bIsMoving);
    AnimInst->SetBoolParameter(TEXT("bIsFleeing"), bFleeing);
}
```

### Wander() — Use MoveToOnSidewalk instead of raw MoveToLocation

**Current code (lines 67-75):**
```cpp
void ACKPedestrianAIController::Wander()
{
    if (!GetPawn() || bFleeing) return;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return;
    FVector NavLocation;
    if (NavSys->GetRandomReachablePointInRadius(WanderOrigin, 500.0f, NavLocation))
        MoveToLocation(NavLocation);
}
```

**New code:**
```cpp
void ACKPedestrianAIController::Wander()
{
    if (!GetPawn() || bFleeing) return;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return;

    FNavLocation NavLocation;
    if (NavSys->GetRandomReachablePointInRadius(WanderOrigin, 500.0f, NavLocation))
    {
        // Use sidewalk-constrained path following
        MoveToOnSidewalk(NavLocation.Location);
    }
}
```

---

## Compile Errors to Watch For

| Error | Cause | Fix |
|-------|-------|-----|
| `'EPathFollowingRequestResult' is not a type` | Missing AIModule include | Ensure `#include "AIController.h"` (pulls in PathFollowingComponent via AIController.h). AIModule is already in Build.cs. |
| `'UAnimMontage' is not a type` | Missing animation include | Add `#include "Animation/AnimMontage.h"` to the .cpp file. |
| `'UAnimInstance' is not a type` | Missing animation include | Add `#include "Animation/AnimInstance.h"` to the .cpp file. |
| `'SetFloatParameter' is not a member of 'UAnimInstance'` | Wrong API | Use `AnimInst->SetFloatParameter(FName, float)` — it's a UFUNCTION. Verify spelling (capital P). |
| `'Montage_Stop' is not a member` | Missing include | `Montage_Stop` is on UAnimInstance, requires `#include "Animation/AnimInstance.h"`. |
| `'GetCurrentMontage' is not a member of 'AActor'` | Wrong class | `GetCurrentMontage()` is on UAnimInstance, not ACharacter. Use `AnimInst->GetCurrentMontage()`. |
| `'PlayAnimMontage' is not a member` | Wrong class | `PlayAnimMontage` is on ACharacter, not AAIController. Cast to ACharacter first. |
| `Cannot convert from 'FNavLocation' to 'const FVector&'` | Wrong argument type | FNavLocation.Location is the FVector. Use `.Location` to destructure. |
| `'GetMesh' is not a member of 'AActor'` | Need to cast to ACharacter | `Cast<ACharacter>(GetPawn())->GetMesh()` — guard with null check. |
| `Error: 'GetAnimInstance': not a member of 'USceneComponent'` | Wrong method | `GetMesh()` returns `USkeletalMeshComponent*`, then call `GetAnimInstance()` on that. |
| Linker error: `unresolved external symbol for UAnimInstance::SetFloatParameter` | Missing module | Add `"Engine"` to Build.cs (already present). |

---

## How to Test

### Prerequisites (set up in Content Browser before testing)

1. Create a skeletal mesh character blueprint: `BP_PedestrianCharacter`
   - Inherits from `ACharacter` (the default UE5 Character class)
   - Assign a skeletal mesh (e.g., `SK_Mannequin` from starter content)
   - Assign the Animation Blueprint (`ABP_Pedestrian`) to the mesh component
   - Set the AI Controller class to `CKPedestrianAIController`
   - Set `Auto Possess AI` to `Placed In World` or `Spawned`

2. Create the Animation Blueprint: `ABP_Pedestrian`
   - Variables: `Speed` (float), `bIsMoving` (bool), `bIsFleeing` (bool)
   - State machine with Idle/Walk/Flee states
   - Walk state uses Speed to blend

3. Create idle montages: `Idle_Stand`, `Idle_LookAround`, `Idle_Phone`
   - Imported as UAnimMontage with looping enabled
   - Assign to `IdleMontages` array in the `CKPedestrianAIController` defaults

### Test Plan

**Test 1: Idle animation plays on BeginPlay**
```
1. Place BP_PedestrianCharacter in the level
2. Press Play
3. EXPECTED: Pedestrian plays one of the idle montages (e.g., standing, looking around)
4. If not playing: Check AnimBP is assigned to the mesh, montages are assigned to IdleMontages array
```

**Test 2: Wander animation state transitions**
```
1. Observe pedestrian
2. EXPECTED: Alternates between idle animation (standing still) and walk animation (wandering)
3. Walk animation should play when moving (Speed > 10 cm/s), idle when stopped
4. If transitions are wrong: Check UpdateAnimationState() is called in Tick, check AnimBP variable names match
```

**Test 3: Sidewalk path following**
```
1. Place NavMeshBoundsVolume covering the sidewalk area only (not the road)
2. Place pedestrian on the sidewalk
3. EXPECTED: Pedestrian wanders within the NavMesh area (sidewalk), never walks into the road
4. If pedestrian walks into road: Check NavMesh coverage, reduce wander radius from 500 to 300
```

**Test 4: Flee animation**
```
1. Approach pedestrian at high speed (run toward them)
2. EXPECTED: Pedestrian transitions to flee animation (run), speed increases 3x
3. After 5 seconds, pedestrian calms down and returns to walk/idle
4. If flee animation doesn't play: Check bIsFleeing parameter is reaching AnimBP
```

**Test 5: Multiple pedestrians**
```
1. Spawn 10-20 pedestrians in the level
2. EXPECTED: Each plays a random idle animation, wanders independently
3. Performance: Check frame time — 20 pedestrians should be < 0.5ms total AI cost
4. If frame time is high: Consider reducing Tick frequency or using less frequent UpdateAnimationState()
```

### Verification Checklist

| Check | Pass Criteria |
|-------|---------------|
| Idle animation plays | Pedestrian visible idle at start |
| Walk animation plays | Pedestrian plays walk cycle while moving |
| Flee animation plays | High-speed player approach triggers flee |
| Stays on sidewalk | Pedestrian never leaves NavMesh area |
| Multiple pedestrians | 10+ pedestrians animate independently with no jitter |
| Movement stops at destination | Pedestrian stops within 30 units of wander target |
| No compiler warnings | Clean build in UE5.8 |
| No null pointer crashes | All GetPawn()/GetMesh() calls guarded |

---

## Edge Cases & Failure Modes

| Failure Mode | Symptom | Fix |
|-------------|---------|-----|
| No skeletal mesh on character | Crash in BeginPlay casting to ACharacter | Guard with `if (!GetPawn() || !GetPawn()->GetClass()->IsChildOf(ACharacter::StaticClass())) return;` |
| No AnimInstance on mesh | SetFloatParameter crashes | Guard with `if (!AnimInst) return;` (already in UpdateAnimationState) |
| NavMesh not built | Wander() never finds a point | Check P key in editor to show NavMesh — must cover sidewalk areas |
| Acceptance radius too large | Pedestrian stops in middle of road | Set to 30.0f or less |
| bAllowPartialPaths = true | Pedestrian walks off NavMesh edge | Set to false (as specified) |
| Montage not set to looping | One-shot idle animation plays once, character freezes | In Montage editor, set the section to loop |
| AnimBP variables don't match C++ names | SetFloatParameter sets nothing silently | Verify exact spelling of `Speed`, `bIsMoving`, `bIsFleeing` in both C++ and AnimBP |

---

## Summary of All Changes

### CKPedestrianAIController.h

| What | Detail |
|------|--------|
| New UPROPERTY | `IdleMontages` — TArray<UAnimMontage*> |
| New UPROPERTY | `IdleMontagePlayRate` — float |
| New UPROPERTY | `SidewalkAcceptanceRadius` — float (default 30.0) |
| New UPROPERTY | `bAllowPartialPaths` — bool (default false) |
| New method | `PlayRandomIdle()` — picks and plays random idle montage |
| New method | `StopIdle()` — stops current idle montage with blend-out |
| New method | `UpdateAnimationState()` — sets AnimBP parameters |
| New method | `MoveToOnSidewalk(FVector)` — MoveToLocation with sidewalk params |
| New member | `bIsMoving` — bool animation state |
| New member | `CurrentIdleMontage` — UAnimMontage* tracking |
| New member | `IdleTimer` — FTimerHandle for idle animation scheduling |

### CKPedestrianAIController.cpp

| What | Line(s) | Change |
|------|---------|--------|
| Constructor | 8-12 | Initialize new members: bIsMoving=false, SidewalkAcceptanceRadius=30.0f, bAllowPartialPaths=false |
| BeginPlay | 14-21 | Add `PlayRandomIdle()` call after initial setup |
| ReactToPlayer | ~49 | Add `PlayRandomIdle()` in flee-end lambda |
| Tick | 56-65 | Add `UpdateAnimationState()` call at top |
| NEW | after Tick | `MoveToOnSidewalk(FVector)` — sidewalk-constrained path following |
| NEW | after MoveToOnSidewalk | `PlayRandomIdle()` — random montage selection + playback |
| NEW | after PlayRandomIdle | `StopIdle()` — blend-out and timer cleanup |
| NEW | after StopIdle | `UpdateAnimationState()` — speed/moving/fleeing → AnimBP |
| Wander | 67-75 | Replace `MoveToLocation(NavLocation)` with `MoveToOnSidewalk(NavLocation.Location)` |

### New Includes (add to .cpp)

```cpp
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
```

---

## Next Steps After This Card

1. Local AI creates the .cpp/.h changes
2. Artist creates the Animation Blueprint and idle montages in Content Browser
3. Designer places `BP_PedestrianCharacter` in the level or wires spawn logic into `CKCityGenerator`
4. QA tests the 5 test cases above
5. If successful, pop page 9 off research_topics.txt and proceed to topic 10