#include "CKGameMode.h"

ACKGameMode::ACKGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    WantedLevel = 0;
    Money = 0;
}

void ACKGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("Concrete Kingdom started! Money: $%d, Wanted: %d"), Money, WantedLevel);
}

void ACKGameMode::AddWantedLevel(int32 Amount)
{
    WantedLevel = FMath::Clamp(WantedLevel + Amount, 0, 5);
    UE_LOG(LogTemp, Warning, TEXT("Wanted level: %d"), WantedLevel);
}

void ACKGameMode::AddMoney(int32 Amount)
{
    Money += Amount;
    UE_LOG(LogTemp, Warning, TEXT("Money: $%d"), Money);
}
