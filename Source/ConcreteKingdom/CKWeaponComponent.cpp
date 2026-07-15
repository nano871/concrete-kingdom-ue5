// UE5.8 - Weapon system with aiming, shooting, buying, GTA V recoil patterns
#include "CKWeaponComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UCKWeaponComponent::UCKWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAiming = false;
    ShootCooldown = 0.0f;
    ShootTimer = 0.0f;
    EquippedWeapon = TEXT("");
    AimFOV = 50.0f;
    CurrentFOV = 90.0f;
    // Recoil state (from GTA V research)
    CurrentSpread = 0.03f;
    BaseSpread = 0.03f;
    BloomPerShot = 0.015f;
    SpreadRecovery = 0.1f;
    MaxSpread = 0.12f;
    SpreadDecayDelay = 0.0f;
}

void UCKWeaponComponent::DefineWeapons()
{
    Weapons.Empty();

    FWeaponDef Pistol;
    Pistol.Name = TEXT("pistol");
    Pistol.Damage = 25;
    Pistol.FireRate = 0.15f;
    Pistol.MaxAmmo = 30;
    Pistol.CurrentAmmo = 30;
    Pistol.Price = 200;
    Pistol.bOwned = false;
    Weapons.Add(TEXT("pistol"), Pistol);

    FWeaponDef Rifle;
    Rifle.Name = TEXT("rifle");
    Rifle.Damage = 40;
    Rifle.FireRate = 0.1f;
    Rifle.MaxAmmo = 60;
    Rifle.CurrentAmmo = 60;
    Rifle.Price = 800;
    Rifle.bOwned = false;
    Weapons.Add(TEXT("rifle"), Rifle);

    UE_LOG(LogTemp, Warning, TEXT("Weapons defined: pistol ($200), rifle ($800)"));
}

bool UCKWeaponComponent::BuyWeapon(FString WeaponName, int32& Money)
{
    FWeaponDef* W = Weapons.Find(WeaponName);
    if (!W || W->bOwned) return false;
    if (Money < W->Price) return false;

    Money -= W->Price;
    W->bOwned = true;
    W->CurrentAmmo = W->MaxAmmo;
    EquipWeapon(WeaponName);
    UE_LOG(LogTemp, Warning, TEXT("Bought %s! Money left: $%d"), *WeaponName, Money);
    return true;
}

void UCKWeaponComponent::EquipWeapon(FString WeaponName)
{
    FWeaponDef* W = Weapons.Find(WeaponName);
    if (!W || !W->bOwned) return;
    EquippedWeapon = WeaponName;
    ShootCooldown = W->FireRate;
    ShootTimer = 0.0f;
}

void UCKWeaponComponent::Shoot()
{
    if (EquippedWeapon.IsEmpty() || ShootTimer < ShootCooldown) return;
    FWeaponDef* W = Weapons.Find(EquippedWeapon);
    if (!W || W->CurrentAmmo <= 0) return;

    W->CurrentAmmo--;
    ShootTimer = 0.0f;
    SpreadDecayDelay = 0.5f; // pause recovery briefly after firing
    // Bloom increase per shot (recoil builds up)
    CurrentSpread = FMath::Min(MaxSpread, CurrentSpread + BloomPerShot);

    // Line trace from camera center
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char) return;
    UCameraComponent* Cam = Char->FindComponentByClass<UCameraComponent>();
    if (!Cam) return;

    FVector Start = Cam->GetComponentLocation();
    FVector Forward = Cam->GetForwardVector();
    // Apply recoil spread (GTA V bloom cone)
    float AimMultiplier = bAiming ? 0.6f : 1.0f;
    float Theta = FMath::FRand() * PI * 2.0f;
    float Phi = CurrentSpread * AimMultiplier * FMath::FRand();
    Forward = Forward.RotateAngleAxis(Phi * 180.0f / PI, Cam->GetUpVector());
    Forward = Forward.RotateAngleAxis(Theta * 180.0f / PI, Cam->GetRightVector());
    
    FVector End = Start + Forward * 10000.0f;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        // Apply damage
        AActor* HitActor = Hit.GetActor();
        if (HitActor)
        {
            FPointDamageEvent DamageEvent(W->Damage, Hit, Forward, nullptr);
            HitActor->TakeDamage(W->Damage, DamageEvent, nullptr, GetOwner());
            UE_LOG(LogTemp, Warning, TEXT("Hit %s for %d damage"), *HitActor->GetName(), W->Damage);
        }
    }
}

void UCKWeaponComponent::StartAim()
{
    bAiming = true;
    CurrentFOV = AimFOV;
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (Char)
    {
        if (UCameraComponent* Cam = Char->FindComponentByClass<UCameraComponent>())
            Cam->SetFieldOfView(CurrentFOV);
    }
}

void UCKWeaponComponent::StopAim()
{
    bAiming = false;
    CurrentFOV = 90.0f;
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (Char)
    {
        if (UCameraComponent* Cam = Char->FindComponentByClass<UCameraComponent>())
            Cam->SetFieldOfView(CurrentFOV);
    }
}

void UCKWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    ShootTimer += DeltaTime;

    // Spread recovery (GTA V: recovers ~0.1 rad/sec after 0.5s of not firing)
    if (SpreadDecayDelay > 0.0f)
    {
        SpreadDecayDelay -= DeltaTime;
    }
    else if (CurrentSpread > BaseSpread)
    {
        CurrentSpread = FMath::Max(BaseSpread, CurrentSpread - SpreadRecovery * DeltaTime);
    }
}

int32 UCKWeaponComponent::GetCurrentAmmo()
{
    FWeaponDef* W = Weapons.Find(EquippedWeapon);
    return W ? W->CurrentAmmo : 0;
}

TArray<FString> UCKWeaponComponent::GetOwnedWeapons()
{
    TArray<FString> Owned;
    for (auto& Pair : Weapons)
        if (Pair.Value.bOwned) Owned.Add(Pair.Key);
    return Owned;
}