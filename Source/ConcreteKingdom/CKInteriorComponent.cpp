#include "CKInteriorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

UCKInteriorComponent::UCKInteriorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bInside = false;
}

void UCKInteriorComponent::EnterInterior(FString InteriorType)
{
    if (!GetWorld()) return;
    ACharacter* Char = Cast<ACharacter>(GetOwner());(FString InteriorType)
{
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char) return;

    OutsideLocation = Char->GetActorLocation();
    OutsideRotation = Char->GetActorRotation();

    if (InteriorType == "bank")
    {
        Char->SetActorLocation(FVector(0, 0, 200));
        Char->SetActorRotation(FRotator(0, 0, 0));
    }
    else if (InteriorType == "gunshop")
    {
        Char->SetActorLocation(FVector(0, 0, 100));
        Char->SetActorRotation(FRotator(0, 0, 0));
    }

    if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
    {
        PC->SetIgnoreLookInput(true);
        PC->SetIgnoreMoveInput(true);
    }
    bInside = true;
    UE_LOG(LogTemp, Warning, TEXT("Entered interior: %s"), *InteriorType);
}

void UCKInteriorComponent::ExitInterior()
{
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char) return;

    Char->SetActorLocation(OutsideLocation);
    Char->SetActorRotation(OutsideRotation);

    if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
    {
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
    }
    bInside = false;
    UE_LOG(LogTemp, Warning, TEXT("Exited interior"));
}
