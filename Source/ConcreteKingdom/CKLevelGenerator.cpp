#include "CKLevelGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/PlayerStartPIE.h"
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

    // ── Find meshes ──
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    UStaticMesh* BuildingMesh = CubeMesh.Object;
    UStaticMesh* FlatMesh = PlaneMesh.Object;
    if (!BuildingMesh || !FlatMesh) return;

    // ── Ground ──
    AActor* Ground = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(0, 0, -5), FRotator::ZeroRotator, SpawnParams);
    UStaticMeshComponent* GroundComp = NewObject<UStaticMeshComponent>(Ground);
    GroundComp->SetStaticMesh(FlatMesh);
    GroundComp->SetWorldScale3D(FVector(8, 8, 1));
    GroundComp->RegisterComponent();
    Ground->SetRootComponent(GroundComp);

    // ── Buildings in a city block ──
    const float TotalSize = 8000.0f;
    const float RoadW = 400.0f;
    const float BlockW = (TotalSize - RoadW * 3) / 4.0f;

    for (int32 Row = 0; Row < 4; Row++)
    {
        for (int32 Col = 0; Col < 4; Col++)
        {
            // Roads
            if (Row < 3 && Col < 3)
            {
                float W = RoadW / 100.0f;
                float L = BlockW / 100.0f;
                float CX = -TotalSize / 2 + RoadW + Col * (BlockW + RoadW) + BlockW / 2;
                float CY = -TotalSize / 2 + RoadW + Row * (BlockW + RoadW) + BlockW / 2;

                // N-S road segment
                AActor* NSRoad = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(CX, CY, -4.5f), FRotator::ZeroRotator, SpawnParams);
                UStaticMeshComponent* NSComp = NewObject<UStaticMeshComponent>(NSRoad);
                NSComp->SetStaticMesh(BuildingMesh);
                NSComp->SetWorldScale3D(FVector(W, L, 0.1f));
                NSComp->RegisterComponent();
                NSRoad->SetRootComponent(NSComp);
            }

            // Buildings on each plot
            if (Row < 4 && Col < 4)
            {
                float CX = -TotalSize / 2 + RoadW + Col * (BlockW + RoadW) + RoadW + BlockW / 4;
                float CY = -TotalSize / 2 + RoadW + Row * (BlockW + RoadW) + RoadW + BlockW / 4;

                for (int32 BX = 0; BX < 2; BX++)
                {
                    for (int32 BY = 0; BY < 2; BY++)
                    {
                        float PX = CX + BX * BlockW / 2.5f;
                        float PY = CY + BY * BlockW / 2.5f;
                        float H = 200 + FMath::RandRange(100, 600);
                        float S = BlockW / 3.5f / 100.0f;

                        AActor* Bldg = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(PX, PY, H / 2), FRotator::ZeroRotator, SpawnParams);
                        UStaticMeshComponent* BldgComp = NewObject<UStaticMeshComponent>(Bldg);
                        BldgComp->SetStaticMesh(BuildingMesh);
                        BldgComp->SetWorldScale3D(FVector(S, S, H / 100.0f));
                        BldgComp->RegisterComponent();
                        Bldg->SetRootComponent(BldgComp);
                    }
                }
            }
        }
    }

    // ── Player Start ──
    APlayerStart* PStart = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
        FVector(0, -TotalSize / 2 + 500, 100), FRotator::ZeroRotator, SpawnParams);

    // ── Lighting (basic directional light if none exists) ──
    AActor* Sun = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(1000, 0, 2000), FRotator(-45, 0, 0), SpawnParams);
    UStaticMeshComponent* SunComp = NewObject<UStaticMeshComponent>(Sun);
    SunComp->RegisterComponent();
    Sun->SetRootComponent(SunComp);
    SunComp->SetVisibility(false);

    UE_LOG(LogTemp, Warning, TEXT("Level generated: 4x4 city block with roads and player start"));
}