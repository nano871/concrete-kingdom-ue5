#include "CKPauseMenuWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UCKPauseMenuWidget::OpenPauseMenu() { bIsPaused = true; SetVisibility(ESlateVisibility::Visible); UGameplayStatics::SetGamePaused(GetWorld(), true); }
void UCKPauseMenuWidget::ClosePauseMenu() { bIsPaused = false; SetVisibility(ESlateVisibility::Hidden); UGameplayStatics::SetGamePaused(GetWorld(), false); }
void UCKPauseMenuWidget::SaveGame() { UE_LOG(LogTemp, Warning, TEXT("Game saved")); }
void UCKPauseMenuWidget::LoadGame() { UE_LOG(LogTemp, Warning, TEXT("Game loaded")); }
void UCKPauseMenuWidget::QuitToMainMenu() { UGameplayStatics::OpenLevel(GetWorld(), FName("MainMenu")); }
