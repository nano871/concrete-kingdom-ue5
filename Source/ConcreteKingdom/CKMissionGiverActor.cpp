#include "CKMissionGiverActor.h"
ACKMissionGiverActor::ACKMissionGiverActor() { PrimaryActorTick.bCanEverTick = false; }
void ACKMissionGiverActor::OfferMission(FString MID) { MissionID = MID; UE_LOG(LogTemp, Warning, TEXT("Mission offered: %s"), *MID); }
void ACKMissionGiverActor::AcceptMission() { UE_LOG(LogTemp, Warning, TEXT("Mission accepted: %s"), *MissionID); }
void ACKMissionGiverActor::DeclineMission() { UE_LOG(LogTemp, Warning, TEXT("Mission declined")); }
