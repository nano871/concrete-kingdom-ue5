#include "CKRoadblockActor.h"
#include "Components/StaticMeshComponent.h"

ACKRoadblockActor::ACKRoadblockActor()
{
    UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrier"));
    RootComponent = Mesh;
    Mesh->SetWorldScale3D(FVector(3, 1, 1));
}

void ACKRoadblockActor::Deploy(FVector Location) { SetActorLocation(Location); }
