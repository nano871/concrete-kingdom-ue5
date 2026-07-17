#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CKGameMode.generated.h"
UCLASS()
class CONCRETEKINGDOM_API ACKGameMode : public AGameModeBase {
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    UFUNCTION(BlueprintCallable) void AddMoney(int32 Amount);
    UFUNCTION(BlueprintCallable) void AddWantedLevel(int32 Amount);
    UFUNCTION(BlueprintCallable) void OnPlayerSpawned(class ACKCharacter* Player);

    UPROPERTY() class UCKWantedComponent* WantedSystem;
    UPROPERTY() class UCKDayNightComponent* DayNight;
    UPROPERTY() class UCKDistrictComponent* Districts;
    UPROPERTY() class UCKFactionComponent* Factions;
    UPROPERTY() class UCKReputationComponent* Reputation;
    UPROPERTY() class UCKWeaponComponent* Weapons;
    UPROPERTY() class UCKTrafficManager* Traffic;
    UPROPERTY() class UCKBusinessComponent* Economy;
    UPROPERTY() class UCKMissionComponent* Missions;
public:
    ACKGameMode();
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 WantedLevel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Money;
    UFUNCTION(BlueprintCallable) void AddWantedLevel(int32 A);
    UFUNCTION(BlueprintCallable) void AddMoney(int32 A);
};