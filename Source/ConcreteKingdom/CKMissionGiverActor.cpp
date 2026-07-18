#include "CKMissionGiverActor.h"
#include "Components/StaticMeshComponent.h"

ACKMissionGiverActor::ACKMissionGiverActor()
{
    PrimaryActorTick.bCanEverTick = false;
    UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissionGiverMesh"));
    RootComponent = Mesh;
    Mesh->SetWorldScale3D(FVector(0.5, 0.5, 1.5));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionProfileName(FName("BlockAll"));
    bMissionOffered = false;
}

void ACKMissionGiverActor::OfferMission(FString MID)
{
    MissionID = MID;
    bMissionOffered = true;
    // Show visual marker above NPC
    UE_LOG(LogTemp, Warning, TEXT("[MISSION] Offered: %s"), *MID);
}

void ACKMissionGiverActor::AcceptMission()
{
    if (!bMissionOffered) return;
    bMissionOffered = false;
    // Hide marker, mission is now active
    SetActorHiddenInGame(true);
    UE_LOG(LogTemp, Warning, TEXT("[MISSION] Accepted: %s"), *MissionID);
}

void ACKMissionGiverActor::DeclineMission()
{
    bMissionOffered = false;
    UE_LOG(LogTemp, Warning, TEXT("[MISSION] Declined: %s"), *MissionID);
}
