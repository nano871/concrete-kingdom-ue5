#include "CKPauseMenuWidget.h"
#include "CKSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UCKPauseMenuWidget::OpenPauseMenu() { bIsPaused = true; SetVisibility(ESlateVisibility::Visible); UGameplayStatics::SetGamePaused(GetWorld(), true); }
void UCKPauseMenuWidget::ClosePauseMenu() { bIsPaused = false; SetVisibility(ESlateVisibility::Hidden); UGameplayStatics::SetGamePaused(GetWorld(), false); }
void UCKPauseMenuWidget::SaveGame()
{
    if (UCKSaveGame* SG = Cast<UCKSaveGame>(UGameplayStatics::CreateSaveGameObject(UCKSaveGame::StaticClass())))
    {
        if (UGameplayStatics::SaveGameToSlot(SG, TEXT("save_slot"), 0))
            UE_LOG(LogTemp, Warning, TEXT("[SAVE] Game saved to slot save_slot"));
    }
}
void UCKPauseMenuWidget::LoadGame()
{
    if (UCKSaveGame* SG = Cast<UCKSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("save_slot"), 0)))
    {
        UE_LOG(LogTemp, Warning, TEXT("[SAVE] Game loaded from save_slot"));
    }
}
void UCKPauseMenuWidget::QuitToMainMenu() { UGameplayStatics::OpenLevel(GetWorld(), FName("MainMenu")); }
