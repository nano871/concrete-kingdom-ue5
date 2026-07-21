#include "CKCoverComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCKCoverComponent::UCKCoverComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bInCover = false;
    bBlindFiring = false;
}

void UCKCoverComponent::FindCover()
{
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char) return;
    FVector Start = Char->GetActorLocation();
    FVector Forwards[] = { Char->GetActorForwardVector(), -Char->GetActorForwardVector(),
                           Char->GetActorRightVector(), -Char->GetActorRightVector() };

    for (FVector Dir : Forwards)
    {
        FHitResult Hit;
        FVector End = Start + Dir * 200.0f;
        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
        {
            if (Hit.bBlockingHit && Hit.GetActor())
            {
                CoverDirection = Dir;
                bInCover = true;
                Char->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
                UE_LOG(LogTemp, Warning, TEXT("In cover!"));
                return;
            }
        }
    }
    bInCover = false;
}

void UCKCoverComponent::ToggleCover()
{
    if (bInCover)
    {
        bInCover = false;
        if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
            Char->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
    }
    else
    {
        FindCover();
    }
}

void UCKCoverComponent::BlindFire()
{
    if (!bInCover) return;
    bBlindFiring = true;
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char || !GetWorld()) return;
    // Fire with high spread in cover direction
    FVector Origin = Char->GetActorLocation() + Char->GetActorForwardVector() * 50.0f;
    FVector SpreadDir = CoverDirection + FMath::VRand() * 0.5f; // high spread
    FVector End = Origin + SpreadDir * 2000.0f;
    FHitResult Hit;
    GetWorld()->LineTraceSingleByChannel(Hit, Origin, End, ECC_Visibility);
    if (Hit.bBlockingHit && Hit.GetActor())
    {
        FPointDamageEvent DmgEvent(20.0f, Hit, -SpreadDir, nullptr);
        Hit.GetActor()->TakeDamage(20.0f, DmgEvent, Char->GetController(), Char);
    }
    UE_LOG(LogTemp, Warning, TEXT("[COVER] Blind fire! High spread, no headshots"));
}

void UCKCoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
