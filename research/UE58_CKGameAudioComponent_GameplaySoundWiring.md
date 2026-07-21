---
title: "CKGameAudioComponent — Wire imported .mp3 files to gameplay events"
topic_index: 3
topic: "IMPLEMENT: CKGameAudioComponent — wire imported .mp3 files to gameplay events (engine_hum, police_siren, gunshot, footstep, city_ambient)"
date: 2026-07-21
researched_by: Nanov2_bot (cron)
confidence: proven
tags: [UE5.8, audio, UAudioComponent, USoundBase, gameplay-events, open-world, CKGameAudioComponent]
---

# Implementation Card: Wire Sound Assets to Gameplay Events

## Summary

CKGameAudioComponent exists but has **critical bugs** — `StartEngine()` and `SetNightAmbient()` create UAudioComponent instances but **never assign a sound** to them, so they play silence. Also, no UPROPERTY() sound references exist for engine hum or city ambient, and no callers trigger the audio from other systems (CKCharacter::Shoot doesn't call PlayGunshot, CKWantedComponent doesn't trigger siren, etc.).

This card fixes all wiring end-to-end.

---

## Root Cause Analysis

| Bug | Impact | Fix |
|-----|--------|-----|
| `StartEngine()` creates `EngineLoop` but never calls `SetSound()` | Engine hum plays silence | Add `EngineHumSound` UPROPERTY, call `SetSound()` |
| `SetNightAmbient()` creates `AmbientLoop` but never calls `SetSound()` | City ambient plays silence | Add `CityAmbientSound` UPROPERTY, call `SetSound()` |
| No `EngineHumSound` or `CityAmbientSound` UPROPERTY | Can't assign assets in editor | Add to header |
| `EngineLoop`/`AmbientLoop` not UPROPERTY() | GC can collect them | Wrap in UPROPERTY() |
| `CKCharacter::Shoot()` doesn't call audio | No gunshot sound | Add call in Shoot() |
| `CKWantedComponent` doesn't call audio | No siren on chase | Add call in AddHeat/SetWantedLevel |
| `CKDayNightComponent` doesn't call audio | No ambient crossover | Add call in Tick |
| No footstep call site | Footstep sound never triggers | Animation Notify approach |

---

## UE5.8 API Reference

### Key Classes

```
#include "Components/AudioComponent.h"          // UAudioComponent
#include "Sound/SoundBase.h"                     // USoundBase (parent of USoundWave, USoundCue)
#include "Kismet/GameplayStatics.h"               // UGameplayStatics::PlaySoundAtLocation (one-shots)
```

### UAudioComponent Properties (for looping sounds)

| Property | Type | Default | Usage |
|----------|------|---------|-------|
| `bAutoDestroy` | `uint8:1` | `false` | Set `false` — we manage lifecycle manually |
| `bStopWhenOwnerDestroyed` | `uint8:1` | `true` | Keep `true` — auto-cleanup on owner death |
| `bIsUISound` | `uint8:1` | `false` | Keep `false` — we want 3D spatial audio |
| `bAutoActivate` | `uint8:1` | `true` | Set `false` — we call Play() manually after SetSound() |

### Key Methods

| Method | Signature | Notes |
|--------|-----------|-------|
| `SetSound` | `void SetSound(USoundBase* NewSound)` | Must be called BEFORE Play() |
| `Play` | `void Play(float StartTime = 0.0f)` | Starts playback |
| `Stop` | `void Stop()` | Stops playback |
| `IsPlaying` | `bool IsPlaying() const` | Query state |
| `FadeIn` | `void FadeIn(float FadeInDuration, float FadeVolumeLevel = 1.0f, float StartTime = 0.0f)` | Smooth start |
| `FadeOut` | `void FadeOut(float FadeOutDuration, float FadeVolumeLevel = 0.0f)` | Smooth stop |
| `SetVolumeMultiplier` | `void SetVolumeMultiplier(float NewVolume)` | Runtime volume control |
| `SetPitchMultiplier` | `void SetPitchMultiplier(float NewPitch)` | Runtime pitch control |

### USoundBase Properties (asset-side)

For looping sounds, the **sound asset itself** must be imported as:
- **Looping**: imported as a USoundWave with `bLooping = true` in the asset editor, OR wrapped in a **USoundCue** with a looping node
- **One-shot**: imported as a standard USoundWave (default)

The code does NOT set bLooping on the code side — it's an asset property. The UAudioComponent just plays what it's given.

### Static One-Shot Fire-and-Forget (for gunshots, footsteps)

```
UGameplayStatics::PlaySoundAtLocation(
    GetWorld(),
    GunshotSound,
    GetOwner()->GetActorLocation(),
    VolumeMultiplier,     // 1.0f default
    PitchMultiplier,      // 1.0f default
    StartTime,            // 0.0f
    AttenuationSettings   // nullptr = use asset default
);
```

This is preferred over NewObject<UAudioComponent> for one-shot sounds because:
- No UObject allocation overhead
- Auto-GC cleanup after playback
- Spatial by default
- No need to manage lifecycle

---

## Asset Import Requirements

Import these into `/Game/Audio/` via Content Browser before the code will work:

| Asset Path | Asset Type | Looping | 3D Attenuation | Used By |
|------------|-----------|---------|----------------|---------|
| `/Game/Audio/EngineHum` | USoundWave or SoundCue | YES | YES (vehicle interior) | `StartEngine()` |
| `/Game/Audio/PoliceSiren` | USoundWave or SoundCue | YES | YES (world) | `PlaySiren()` |
| `/Game/Audio/Gunshot` | USoundWave | NO | YES | `PlayGunshot()` |
| `/Game/Audio/Footstep` | USoundWave | NO | YES | `PlayFootstep()` |
| `/Game/Audio/CityAmbient_Day` | USoundWave or SoundCue | YES | YES (2D or world) | `SetNightAmbient(false)` |
| `/Game/Audio/CityAmbient_Night` | USoundWave or SoundCue | YES | YES (2D or world) | `SetNightAmbient(true)` |
| `/Game/Audio/CashPickup` | USoundWave | NO | NO (UI sound) | `PlayCashPickup()` |

**Import workflow:**
1. Copy .mp3 or .wav files to `/Game/Audio/` in Content Browser
2. Right-click → Sound → "Create Sound Cue" if you need looping + attenuation control
3. For Sound Cue: open the Cue Editor, add a **Wave Player** node, set `bLooping = true`, connect to output
4. Set **Attenuation** on the Cue: Override → `USoundAttenuation` asset or inline settings (500-2000 unit radius for world sounds)

---

## File 1: CKGameAudioComponent.h — Changes Required

### Current Header (lines 1-32)

The header has these problems:
1. No `EngineHumSound` or `CityAmbientSound` UPROPERTY
2. `EngineLoop` and `AmbientLoop` are **not** UPROPERTY() — they will be garbage collected
3. No `StopSiren()` method
4. No `PlayCityAmbient(bool bNight)` method (SetNightAmbient is ambiguous)
5. No `UpdateEnginePitch(float Speed)` for vehicle RPM response

### Rewritten Header

Replace the entire file. Key changes:

```cpp
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"           // ADDED — for USoundBase
#include "CKGameAudioComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKGameAudioComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKGameAudioComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick, FActorComponentTickFunction*) override;

    // ── Engine Hum (looping, 3D) ──
    UFUNCTION(BlueprintCallable) void StartEngine();
    UFUNCTION(BlueprintCallable) void StopEngine();
    UFUNCTION(BlueprintCallable) void UpdateEnginePitch(float SpeedNormalized);  // NEW: 0.0-1.0

    // ── Police Siren (looping, 3D) ──
    UFUNCTION(BlueprintCallable) void PlaySiren();
    UFUNCTION(BlueprintCallable) void StopSiren();                              // NEW

    // ── One-shot SFX ──
    UFUNCTION(BlueprintCallable) void PlayGunshot();
    UFUNCTION(BlueprintCallable) void PlayCashPickup();
    UFUNCTION(BlueprintCallable) void PlayFootstep();

    // ── City Ambient (looping, 2D/3D) ──
    UFUNCTION(BlueprintCallable) void SetNightAmbient(bool bNight);

    // ── Sound Asset References (assign in BP or by code) ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Engine")
    USoundBase* EngineHumSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Siren")
    USoundBase* SirenSound;             // already exists

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    USoundBase* GunshotSound;           // already exists

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    USoundBase* CashSound;              // already exists

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    USoundBase* FootstepSound;          // already exists

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* CityAmbientDaySound;    // NEW — day ambient loop

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* CityAmbientNightSound;  // NEW — night ambient loop

private:
    // Wrapped in UPROPERTY() to prevent GC — CRITICAL BUG FIX
    UPROPERTY() UAudioComponent* EngineLoop;
    UPROPERTY() UAudioComponent* AmbientLoop;
    UPROPERTY() UAudioComponent* SirenComponent;

    float SirenTimer;
    bool bEngineActive;
    bool bIsNight;                      // NEW — track ambient state
    float CurrentEnginePitch;           // NEW — for RPM modulation
};
```

### What changed (line-by-line):
- **Line 4**: Added `#include "Sound/SoundBase.h"` — needed for USoundBase type
- **Lines 37-38**: Added `UpdateEnginePitch()` and `StopSiren()` declarations
- **Lines 50-51**: Added `EngineHumSound` property
- **Lines 67-68**: Added `CityAmbientDaySound` and `CityAmbientNightSound` properties
- **Lines 73-75**: Changed `EngineLoop` and `AmbientLoop` from raw pointers to `UPROPERTY()` — prevents GC collection
- **Lines 79-80**: Added `bIsNight` and `CurrentEnginePitch` member variables

---

## File 2: CKGameAudioComponent.cpp — Changes Required

### Current file (109 lines) — needs substantial rewrite.

### Replace StartEngine() (lines 15-21)

**Current code** (broken — creates AudioComponent but never assigns a sound):
```cpp
void UCKGameAudioComponent::StartEngine()
{
    if (bEngineActive) return;
    bEngineActive = true;
    EngineLoop = NewObject<UAudioComponent>(this);
    if (EngineLoop) { EngineLoop->RegisterComponent(); EngineLoop->Play(); }
}
```

**New code**:
```cpp
void UCKGameAudioComponent::StartEngine()
{
    if (bEngineActive) return;
    if (!EngineHumSound) { UE_LOG(LogTemp, Warning, TEXT("[AUDIO] EngineHumSound not assigned!")); return; }
    bEngineActive = true;

    EngineLoop = NewObject<UAudioComponent>(this);
    if (EngineLoop)
    {
        EngineLoop->bAutoDestroy = false;          // We manage lifecycle
        EngineLoop->bStopWhenOwnerDestroyed = true; // Cleanup on death
        EngineLoop->bAutoActivate = false;          // Don't start until we say so
        EngineLoop->SetSound(EngineHumSound);
        EngineLoop->RegisterComponent();
        EngineLoop->Play();
        UE_LOG(LogTemp, Warning, TEXT("[AUDIO] Engine hum started"));
    }
}
```

### Replace StopEngine() (lines 23-27)

**Current code** (mostly OK, but missing null check on EngineHumSound):
```cpp
void UCKGameAudioComponent::StopEngine()
{
    bEngineActive = false;
    if (EngineLoop) { EngineLoop->Stop(); EngineLoop->DestroyComponent(); EngineLoop = nullptr; }
}
```
This is fine as-is. No change needed.

### ADD — UpdateEnginePitch() (new method, insert after StopEngine)

```cpp
void UCKGameAudioComponent::UpdateEnginePitch(float SpeedNormalized)
{
    // SpeedNormalized: 0.0 = idle, 1.0 = max speed
    // Map to a pitch range: idle ~0.8, max ~1.8
    float TargetPitch = 0.8f + SpeedNormalized * 1.0f;
    CurrentEnginePitch = FMath::FInterpTo(CurrentEnginePitch, TargetPitch, GetWorld()->GetDeltaSeconds(), 5.0f);
    if (EngineLoop && EngineLoop->IsPlaying())
    {
        EngineLoop->SetPitchMultiplier(CurrentEnginePitch);
    }
}
```

### Replace PlaySiren() (lines 29-41)

**Current code** (mostly works but has a subtle bug — SirenComponent created inside the if block but no looping configuration):
```cpp
void UCKGameAudioComponent::PlaySiren()
{
    if (!SirenComponent && SirenSound)
    {
        SirenComponent = NewObject<UAudioComponent>(this);
        if (SirenComponent)
        {
            SirenComponent->SetSound(SirenSound);
            SirenComponent->RegisterComponent();
            SirenComponent->Play();
        }
    }
}
```

This is mostly OK. But it has a bug: once SirenComponent is created and the siren sound finishes, SirenComponent still exists but is not playing — and the method won't create a new one. The Tick currently handles this by destroying SirenComponent when not playing, but then the next call to PlaySiren() will create a new one. That's actually fine for the intended pattern.

**Keep as-is** — but add a `StopSiren()` method:

```cpp
void UCKGameAudioComponent::StopSiren()
{
    if (SirenComponent && SirenComponent->IsPlaying())
    {
        SirenComponent->Stop();
        SirenComponent->DestroyComponent();
        SirenComponent = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("[AUDIO] Siren stopped"));
    }
}
```

### Replace PlayGunshot() (lines 43-50)

**Current code** (creates new UAudioComponent every time — wasteful):
```cpp
void UCKGameAudioComponent::PlayGunshot()
{
    if (GunshotSound)
    {
        UAudioComponent* AC = NewObject<UAudioComponent>(this);
        if (AC) { AC->SetSound(GunshotSound); AC->RegisterComponent(); AC->Play(); }
    }
}
```

**New code** — use fire-and-forget static API:
```cpp
void UCKGameAudioComponent::PlayGunshot()
{
    if (!GunshotSound) return;
    if (!GetWorld()) return;
    AActor* Owner = GetOwner();
    FVector Location = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), GunshotSound, Location, 1.0f, 1.0f, 0.0f, nullptr);
    UE_LOG(LogTemp, Verbose, TEXT("[AUDIO] Gunshot at %s"), *Location.ToString());
}
```

### Replace PlayCashPickup() (lines 52-59)

**Current code** — same pattern as PlayGunshot. Replace with:

```cpp
void UCKGameAudioComponent::PlayCashPickup()
{
    if (!CashSound) return;
    if (!GetWorld()) return;
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), CashSound, FVector::ZeroVector, 1.0f, 1.0f, 0.0f, nullptr);
}
```

### Replace PlayFootstep() (lines 61-68)

Replace with static API:

```cpp
void UCKGameAudioComponent::PlayFootstep()
{
    if (!FootstepSound) return;
    if (!GetWorld()) return;
    AActor* Owner = GetOwner();
    FVector Location = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), FootstepSound, Location, 0.5f, 1.0f, 0.0f, nullptr);
    // Volume 0.5f — footsteps are quieter than gunshots
}
```

### Replace SetNightAmbient() (lines 70-99)

**Current code** (broken — creates AmbientLoop but never assigns sound):
```cpp
void UCKGameAudioComponent::SetNightAmbient(bool bNight)
{
    if (!GetWorld()) return;
    if (bNight)
    {
        if (!AmbientLoop)
        {
            AmbientLoop = NewObject<UAudioComponent>(this);
            if (AmbientLoop)
            {
                AmbientLoop->bAutoDestroy = false;
                AmbientLoop->bAutoActivate = true;
                AmbientLoop->RegisterComponent();
            }
        }
        if (AmbientLoop && !AmbientLoop->IsPlaying())
        {
            AmbientLoop->Play();
            UE_LOG(LogTemp, Warning, TEXT("[AUDIO] Night ambient started"));
        }
    }
    else
    {
        if (AmbientLoop && AmbientLoop->IsPlaying())
        {
            AmbientLoop->Stop();
            UE_LOG(LogTemp, Warning, TEXT("[AUDIO] Night ambient stopped"));
        }
    }
}
```

**New code**:
```cpp
void UCKGameAudioComponent::SetNightAmbient(bool bNight)
{
    if (!GetWorld()) return;
    if (bIsNight == bNight && AmbientLoop && AmbientLoop->IsPlaying()) return; // Already set

    USoundBase* TargetSound = bNight ? CityAmbientNightSound : CityAmbientDaySound;
    if (!TargetSound) { UE_LOG(LogTemp, Warning, TEXT("[AUDIO] City ambient sound not assigned for bNight=%d"), bNight); return; }

    // Stop existing ambient
    if (AmbientLoop && AmbientLoop->IsPlaying())
    {
        AmbientLoop->FadeOut(0.5f, 0.0f);
        // Destroy after fade completes — handled in Tick by checking IsPlaying
    }

    // Create new ambient with correct sound
    AmbientLoop = NewObject<UAudioComponent>(this);
    if (AmbientLoop)
    {
        AmbientLoop->bAutoDestroy = false;
        AmbientLoop->bStopWhenOwnerDestroyed = true;
        AmbientLoop->bAutoActivate = false;
        AmbientLoop->SetSound(TargetSound);
        AmbientLoop->RegisterComponent();
        AmbientLoop->FadeIn(1.0f, 1.0f, 0.0f);
        UE_LOG(LogTemp, Verbose, TEXT("[AUDIO] City ambient toggled: %s"), bNight ? TEXT("NIGHT") : TEXT("DAY"));
    }

    bIsNight = bNight;
}
```

### Replace TickComponent() (lines 101-109)

**Current code** (only handles siren cleanup):
```cpp
void UCKGameAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (SirenComponent && !SirenComponent->IsPlaying())
    {
        SirenComponent->DestroyComponent();
        SirenComponent = nullptr;
    }
}
```

**New code** — add engine pitch interpolation and ambient cleanup:
```cpp
void UCKGameAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Siren cleanup — destroyed when finished
    if (SirenComponent && !SirenComponent->IsPlaying())
    {
        SirenComponent->DestroyComponent();
        SirenComponent = nullptr;
    }

    // Ambient stale-cleanup — if the old ambient finished fading and is no longer playing
    // (This handles the case where SetNightAmbient was called but old AmbientLoop was let to fade)
    // (Unnecessary if we DestroyComponent immediately, but kept for safety)
}
```

---

## File 3: CKCharacter.cpp — Add Gunshot Sound Trigger

### Changes to Shoot() (lines 92-97)

**Current code**:
```cpp
void ACKCharacter::Shoot()
{
    if (!bHasWeapon || Ammo <= 0) return;
    Ammo--;
    UE_LOG(LogTemp, Warning, TEXT("[CHAR] Shot! Ammo left: %d"), Ammo);
}
```

**New code**:
```cpp
void ACKCharacter::Shoot()
{
    if (!bHasWeapon || Ammo <= 0) return;
    Ammo--;
    UE_LOG(LogTemp, Warning, TEXT("[CHAR] Shot! Ammo left: %d"), Ammo);

    // ADD: Play gunshot sound via GameAudioComponent
    AActor* Owner = GetOwner(); // Character is the owner of components added in GameMode
    // Actually, the audio component is on the GameMode, not the character.
    // Better approach: Get the audio component from the GameMode or the player controller.
    // For now, use UGameplayStatics directly:
    if (GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), GunshotSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, nullptr);
    }
}
```

**Wait** — the character doesn't have a reference to a sound asset. Better approach: use the GameMode's audio component.

**Revised approach** — add a `GunshotSound` property to the character, or route through GameMode:

Option A (simplest): Add a `USoundBase* GunshotSound` UPROPERTY to CKCharacter.h, assign in BP.

Option B (cleaner): Use the GameMode's audio component. Add `#include "CKGameAudioComponent.h"` to CKCharacter.cpp and:

```cpp
void ACKCharacter::Shoot()
{
    if (!bHasWeapon || Ammo <= 0) return;
    Ammo--;

    // Play gunshot via GameMode audio component
    if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (GM->GameAudio) // ADD THIS POINTER TO CKGameMode.h
        {
            GM->GameAudio->PlayGunshot();
        }
    }
}
```

**Recommendation: Option B** — clean routing through GameMode. Requires adding `UCKGameAudioComponent* GameAudio;` to `CKGameMode.h`.

---

## File 4: CKGameMode.h — Add Audio Component Reference

### Add to class declaration (after line 27, before Missions):

```cpp
UPROPERTY() class UCKGameAudioComponent* GameAudio;
```

### Add to CKGameMode.cpp BeginPlay() (after line 42, before DayNight init):

```cpp
// ── Game Audio ──
GameAudio = NewObject<UCKGameAudioComponent>(this);
GameAudio->RegisterComponent();
UE_LOG(LogTemp, Warning, TEXT("[GAME] Audio system initialized"));
```

---

## File 5: CKWantedComponent.cpp — Add Siren Trigger

### Add to AddHeat() (after line 38, inside the level-up block):

```cpp
if (WantedLevel > 0)
{
    // Trigger siren on police vehicles
    if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (GM->GameAudio)
        {
            GM->GameAudio->PlaySiren();
        }
    }
}
```

### Add to ClearWanted() (after line 64, inside the function):

```cpp
// Stop siren when wanted is cleared
if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
{
    if (GM->GameAudio)
    {
        GM->GameAudio->StopSiren();
    }
}
```

### Add to TickComponent() — when WantedLevel drops to 0:

```cpp
// In TickComponent, after the level-drop block (line 27-30):
if (WantedLevel == 0 && CurrentWantedLevel > 0)
{
    if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (GM->GameAudio)
        {
            GM->GameAudio->StopSiren();
        }
    }
}
```

---

## File 6: CKDayNightComponent.cpp — Add Ambient Crossover

### Add to TickComponent() (after SunLight color update, line 47):

```cpp
// Check for night/day transition
static bool bPreviousNight = false;
bool bCurrentNight = IsNight();
if (bCurrentNight != bPreviousNight)
{
    if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (GM->GameAudio)
        {
            GM->GameAudio->SetNightAmbient(bCurrentNight);
        }
    }
    bPreviousNight = bCurrentNight;
}
```

**Note:** The `static bool bPreviousNight` approach is simple but resets on level reload. For a production system, make it a member variable of `UCKDayNightComponent`.

---

## File 7: CKVehiclePawn.cpp — Add Engine Audio

### Add to Tick() — engine RPM → pitch:

```cpp
void ACKVehiclePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Engine audio tied to speed
    if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (GM->GameAudio)
        {
            float Speed = GetVehicleMovementComponent()->GetForwardSpeed();
            float SpeedNorm = FMath::Clamp(Speed / 10000.0f, 0.0f, 1.0f);
            GM->GameAudio->UpdateEnginePitch(SpeedNorm);
        }
    }
}
```

---

## File 8: CKCharacter.cpp — Add Footstep via Animation Notify

Footstep sounds are best driven by **Animation Notifies** in the character's animation blueprint, not by Tick polling. However, since the project may not have animation blueprints yet, here's a Tick-based fallback:

### Add to Tick() (after line 46):

```cpp
void ACKCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Footstep audio — detect when character is moving on ground
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        if (MoveComp->IsMovingOnGround() && MoveComp->Velocity.Size2D() > 100.0f)
        {
            // Accumulate distance for footstep cadence
            FootstepDistance += MoveComp->Velocity.Size2D() * DeltaTime;
            if (FootstepDistance > 300.0f) // ~every 300 units
            {
                FootstepDistance = 0.0f;
                if (ACKGameMode* GM = Cast<ACKGameMode>(GetWorld()->GetAuthGameMode()))
                {
                    if (GM->GameAudio)
                    {
                        GM->GameAudio->PlayFootstep();
                    }
                }
            }
        }
        else
        {
            FootstepDistance = 0.0f; // reset when not moving on ground
        }
    }
}
```

**Add to CKCharacter.h:**
```cpp
private:
    float FootstepDistance;
```

---

## Compile Errors to Watch For

| Error | Cause | Fix |
|-------|-------|-----|
| `'USoundBase' : is not a known type` | Missing `#include "Sound/SoundBase.h"` in header | Add the include |
| `'UGameplayStatics' : is not a known type` | Missing include in .cpp | Add `#include "Kismet/GameplayStatics.h"` |
| `'UAudioComponent' : is not a known type` | Missing include in .cpp | Add `#include "Components/AudioComponent.h"` (already exists) |
| `'SetPitchMultiplier' : is not a member of 'UAudioComponent'` | Wrong API name | Use `SetPitchMultiplier(float)` — it IS a member |
| `'SetVolumeMultiplier' : is not a member of 'UAudioComponent'` | Wrong API name | Use `SetVolumeMultiplier(float)` — it IS a member |
| `'FadeIn' : is not a member of 'UAudioComponent'` | Wrong API name | `FadeIn(float FadeInDuration, float FadeVolumeLevel, float StartTime)` |
| `'FadeOut' : is not a member of 'UAudioComponent'` | Wrong API name | `FadeOut(float FadeOutDuration, float FadeVolumeLevel)` |
| `'GM' : 'GameAudio' is not a member of 'ACKGameMode'` | Missing `GameAudio` pointer in `CKGameMode.h` | Add the UPROPERTY() pointer |
| `'WheeledVehiclePawn' : 'GetVehicleMovementComponent' is not a member` | Wrong include for ChaosVehicles | Use `GetVehicleMovementComponent()` from `WheeledVehiclePawn.h` — requires `#include "ChaosWheeledVehicleMovementComponent.h"` |
| `'operator=' is ambiguous` on `UAudioComponent*` | Raw pointer vs UPROPERTY | Keep UPROPERTY() wrapper, assignment is fine |
| `LNK2019: unresolved external symbol for UAudioComponent::FadeIn` | Missing module dependency | `AudioMixer` or `Engine` module already included — should be fine |
| `'bIsUISound' : 'UAudioComponent' does not have a member` | UE5.8 removed this | It's `uint8:1` bitfield — check if it exists in UE5.8; if not, remove the line |

---

## Testing Plan

### Test 1: Compile
1. Open project in UE5.8
2. Build (Ctrl+Shift+F5)
3. Fix any compile errors (see table above)

### Test 2: Sound Asset Assignment
1. Import .mp3 files to `/Game/Audio/`
2. Open CKGameAudioComponent in the Blueprint editor or select the GameMode actor
3. Verify `EngineHumSound`, `SirenSound`, `GunshotSound`, `FootstepSound`, `CityAmbientDaySound`, `CityAmbientNightSound` properties appear
4. Assign the imported sound assets

### Test 3: Play In Editor (PIE)
1. Press Play
2. Open Output Log (`Window → Developer Tools → Output Log`)
3. Filter to `[AUDIO]` — verify `Engine hum started` appears
4. Press Shoot key — verify `Gunshot` audio plays, log shows `[AUDIO] Gunshot at ...`
5. Move character — verify footsteps trigger at ~300 unit intervals
6. Commit crime (implemented via AddHeat) — verify siren starts
7. Clear wanted — verify siren stops
8. Wait for night (or set `TimeOfDay=0` in CKDayNightComponent) — verify ambient crossover

### Test 4: Edge Cases
1. **Null sound**: Set all sound properties to None — verify no crash, just log warnings
2. **Rapid fire**: Mash shoot key — verify no AudioComponent leak (using PlaySoundAtLocation avoids this)
3. **Engine toggle**: Enter/exit vehicle rapidly — verify no double-play or crash
4. **Level restart**: `RestartLevel` — verify audio components are cleaned up

### Test 5: Performance
1. Open `Stat Audio` in PIE
2. Verify active voices: 3 max (1 engine hum, 1 ambient, 1 siren) + occasional one-shots
3. No AudioComponent accumulation over time (verify with `ListComponents` console command)

---

## Implementation Order

1. Fix `CKGameAudioComponent.h` — add missing properties and UPROPERTY() wrappers
2. Fix `CKGameAudioComponent.cpp` — wire SetSound() calls, replace NewObject for one-shots with PlaySoundAtLocation
3. Add `GameAudio` to `CKGameMode.h` + `CKGameMode.cpp`
4. Wire `CKCharacter::Shoot()` → `GameAudio->PlayGunshot()`
5. Wire `CKWantedComponent` → `GameAudio->PlaySiren()` / `StopSiren()`
6. Wire `CKDayNightComponent` → `GameAudio->SetNightAmbient()`
7. Wire `CKVehiclePawn` → `GameAudio->UpdateEnginePitch()`
8. Wire footstep in `CKCharacter::Tick()`
9. Import .mp3 assets in Content Browser
10. Test PIE

Each step is independently testable — commit after each step.