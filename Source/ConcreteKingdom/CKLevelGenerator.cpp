// UE5.8 - complete level generation from code
#include "CKLevelGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"

ACKLevelGenerator::ACKLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACKLevelGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateCompleteLevel();
}

void ACKLevelGenerator::GenerateCompleteLevel()
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (!CubeMesh || !PlaneMesh) return;

    const float TotalSize = 8000.0f;
    const float RoadW = 400.0f;

    // Ground
    AActor* Ground = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(0, 0, -5), FRotator::ZeroRotator, SpawnParams);
    UStaticMeshComponent* GroundComp = NewObject<UStaticMeshComponent>(Ground);
    GroundComp->SetStaticMesh(PlaneMesh);
    GroundComp->SetWorldScale3D(FVector(10, 10, 1));
    GroundComp->RegisterComponent();
    Ground->SetRootComponent(GroundComp);

    // 4x4 city block
    for (int32 Col = 0; Col < 4; Col++)
    {
        for (int32 Row = 0; Row < 4; Row++)
        {
            float CX = -TotalSize / 2 + RoadW + Col * (TotalSize / 3.5f) + (TotalSize / 7.0f);
            float CY = -TotalSize / 2 + RoadW + Row * (TotalSize / 3.5f) + (TotalSize / 7.0f);
            float H = 200 + FMath::RandRange(100, 600);
            float S = 0.8f;

            AActor* Bldg = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(CX, CY, H / 2), FRotator::ZeroRotator, SpawnParams);
            UStaticMeshComponent* BldgComp = NewObject<UStaticMeshComponent>(Bldg);
            BldgComp->SetStaticMesh(CubeMesh);
            BldgComp->SetWorldScale3D(FVector(S, S, H / 100.0f));
            BldgComp->RegisterComponent();
            Bldg->SetRootComponent(BldgComp);
        }
    }

    // Player start
    APlayerStart* PStart = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
        FVector(0, -TotalSize / 2 + 600, 100), FRotator::ZeroRotator, SpawnParams);

    UE_LOG(LogTemp, Warning, TEXT("Level generated"));
}
