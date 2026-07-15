#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKWeaponComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDef {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) int32 Damage;
    UPROPERTY(EditAnywhere) float FireRate;
    UPROPERTY(EditAnywhere) int32 MaxAmmo;
    UPROPERTY(EditAnywhere) int32 Price;
    UPROPERTY() int32 CurrentAmmo;
    UPROPERTY() bool bOwned;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKWeaponComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKWeaponComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable) void DefineWeapons();
    UFUNCTION(BlueprintCallable) bool BuyWeapon(FString WeaponName, int32& Money);
    UFUNCTION(BlueprintCallable) void EquipWeapon(FString WeaponName);
    UFUNCTION(BlueprintCallable) void Shoot();
    UFUNCTION(BlueprintCallable) void StartAim();
    UFUNCTION(BlueprintCallable) void StopAim();
    UFUNCTION(BlueprintCallable) bool IsAiming() { return bAiming; }
    UFUNCTION(BlueprintCallable) FString GetEquippedWeapon() { return EquippedWeapon; }
    UFUNCTION(BlueprintCallable) int32 GetCurrentAmmo();
    UFUNCTION(BlueprintCallable) TArray<FString> GetOwnedWeapons();

    UPROPERTY(BlueprintReadOnly) TMap<FString, FWeaponDef> Weapons;
    UPROPERTY(BlueprintReadOnly) bool bAiming;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AimFOV;

private:
    FString EquippedWeapon;
    float ShootCooldown;
    float ShootTimer;
    float CurrentFOV;
};