#include "CKGameAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Sound/SoundWave.h"

UCKGameAudioComponent::UCKGameAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    EngineLoop = nullptr;
    AmbientLoop = nullptr;
    SirenTimer = 0.0f;
    bEngineActive = false;
}

void UCKGameAudioComponent::StartEngine()
{
    if (bEngineActive) return;
    bEngineActive = true;
    EngineLoop = NewObject<UAudioComponent>(this);
    if (EngineLoop) { EngineLoop->RegisterComponent(); EngineLoop->Play(); }
}

void UCKGameAudioComponent::StopEngine()
{
    bEngineActive = false;
    if (EngineLoop) { EngineLoop->Stop(); EngineLoop->DestroyComponent(); EngineLoop = nullptr; }
}

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

void UCKGameAudioComponent::PlayGunshot()
{
    if (GunshotSound)
    {
        UAudioComponent* AC = NewObject<UAudioComponent>(this);
        if (AC) { AC->SetSound(GunshotSound); AC->RegisterComponent(); AC->Play(); }
    }
}

void UCKGameAudioComponent::PlayCashPickup()
{
    if (CashSound)
    {
        UAudioComponent* AC = NewObject<UAudioComponent>(this);
        if (AC) { AC->SetSound(CashSound); AC->RegisterComponent(); AC->Play(); }
    }
}

void UCKGameAudioComponent::PlayFootstep()
{
    if (FootstepSound)
    {
        UAudioComponent* AC = NewObject<UAudioComponent>(this);
        if (AC) { AC->SetSound(FootstepSound); AC->RegisterComponent(); AC->Play(); }
    }
}

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

void UCKGameAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (SirenComponent && !SirenComponent->IsPlaying())
    {
        SirenComponent->DestroyComponent();
        SirenComponent = nullptr;
    }
}
