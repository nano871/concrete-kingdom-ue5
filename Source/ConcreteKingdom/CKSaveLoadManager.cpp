#include "CKSaveLoadManager.h"
#include "Kismet/GameplayStatics.h"
#include "CKSaveGame.h"

UCKSaveLoadManager::UCKSaveLoadManager() { PrimaryComponentTick.bCanEverTick = false; }

void UCKSaveLoadManager::AutoSave()
{
    UCKSaveGame* SG = Cast<UCKSaveGame>(UGameplayStatics::CreateSaveGameObject(UCKSaveGame::StaticClass()));
    if (SG) UGameplayStatics::SaveGameToSlot(SG, TEXT("autosave"), 0);
}

void UCKSaveLoadManager::QuickSave()
{
    UCKSaveGame* SG = Cast<UCKSaveGame>(UGameplayStatics::CreateSaveGameObject(UCKSaveGame::StaticClass()));
    if (SG) UGameplayStatics::SaveGameToSlot(SG, TEXT("quicksave"), 0);
}

void UCKSaveLoadManager::QuickLoad()
{
    UCKSaveGame* SG = Cast<UCKSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("quicksave"), 0));
    if (SG) UE_LOG(LogTemp, Warning, TEXT("Game loaded"));
}
