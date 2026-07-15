#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKGameAudioComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKGameAudioComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKGameAudioComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick, FActorComponentTickFunction*) override;

    UFUNCTION(BlueprintCallable) void StartEngine();
    UFUNCTION(BlueprintCallable) void StopEngine();
    UFUNCTION(BlueprintCallable) void PlaySiren();
    UFUNCTION(BlueprintCallable) void PlayGunshot();
    UFUNCTION(BlueprintCallable) void PlayCashPickup();
    UFUNCTION(BlueprintCallable) void PlayFootstep();
    UFUNCTION(BlueprintCallable) void PlayHorn();
    UFUNCTION(BlueprintCallable) void SetNightAmbient(bool bNight);

    UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundWave* SirenSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundWave* GunshotSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundWave* CashSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundWave* FootstepSound;

private:
    UAudioComponent* EngineLoop;
    UAudioComponent* AmbientLoop;
    UPROPERTY() class UAudioComponent* SirenComponent;
    float SirenTimer;
    bool bEngineActive;
};
