#include "CKGameMode.h"
ACKGameMode::ACKGameMode() { WantedLevel = 0; Money = 0; }
void ACKGameMode::AddWantedLevel(int32 A) { WantedLevel = FMath::Clamp(WantedLevel + A, 0, 5); }
void ACKGameMode::AddMoney(int32 A) { Money += A; }
