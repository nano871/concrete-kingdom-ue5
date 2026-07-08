#include "CKCityGenerator.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ACKCityGenerator::ACKCityGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    GridSizeX = 3;
    GridSizeY = 3;
    BlockSize = 800.0f;
    RoadWidth = 400.0f;
}

void ACKCityGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateCity();
}

void ACKCityGenerator::GenerateCity()
{
    ClearCity();

    // Find the cube mesh for buildings (UE5 starter content)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));

    UStaticMesh* BuildingMesh = CubeMesh.Object;
    UStaticMesh* RoadMesh = PlaneMesh.Object;
    if (!BuildingMesh || !RoadMesh) return;

    // ── Ground plane ──
    float TotalWidth = GridSizeX * (BlockSize + RoadWidth) + RoadWidth;
    float TotalDepth = GridSizeY * (BlockSize + RoadWidth) + RoadWidth;

    AActor* Ground = GetWorld()->SpawnActor<AActor>();
    Ground->SetActorLocation(FVector(0, 0, -5));
    UStaticMeshComponent* GroundMesh = NewObject<UStaticMeshComponent>(Ground);
    GroundMesh->SetStaticMesh(RoadMesh);
    GroundMesh->SetWorldScale3D(FVector(TotalWidth / 100.0f, TotalDepth / 100.0f, 1));
    GroundMesh->RegisterComponent();
    Ground->SetRootComponent(GroundMesh);
    SpawnedActors.Add(Ground);

    // ── Roads (grid pattern) ──
    for (int32 X = 0; X <= GridSizeX; X++)
    {
        float RoadX = -TotalWidth / 2 + X * (BlockSize + RoadWidth) + RoadWidth / 2;
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            float RoadZ = -TotalDepth / 2 + Y * (BlockSize + RoadWidth) + BlockSize / 2 + RoadWidth / 2;
            AActor* RoadActor = GetWorld()->SpawnActor<AActor>();
            RoadActor->SetActorLocation(FVector(RoadX, RoadZ, 0));
            UStaticMeshComponent* RoadMeshComp = NewObject<UStaticMeshComponent>(RoadActor);
            RoadMeshComp->SetStaticMesh(BuildingMesh);
            RoadMeshComp->SetWorldScale3D(FVector(RoadWidth / 100.0f, BlockSize / 100.0f, 0.1f));
            RoadMeshComp->RegisterComponent();
            RoadActor->SetRootComponent(RoadMeshComp);
            SpawnedActors.Add(RoadActor);
        }
    }

    // ── Buildings on each block ──
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            float CenterX = -TotalWidth / 2 + X * (BlockSize + RoadWidth) + RoadWidth + BlockSize / 2;
            float CenterY = -TotalDepth / 2 + Y * (BlockSize + RoadWidth) + RoadWidth + BlockSize / 2;

            // Subdivide block into 3x3 building plots
            for (int32 BX = 0; BX < 3; BX++)
            {
                for (int32 BY = 0; BY < 3; BY++)
                {
                    float PlotW = BlockSize / 3.0f * 0.85f;
                    float PlotD = BlockSize / 3.0f * 0.85f;
                    float PlotH = 200 + FMath::RandRange(100, 600);
                    FLinearColor Color = FLinearColor(
                        0.3f + FMath::FRand() * 0.2f,
                        0.3f + FMath::FRand() * 0.2f,
                        0.35f + FMath::FRand() * 0.25f
                    );

                    float PX = CenterX - BlockSize / 2 + (BX + 0.5f) * (BlockSize / 3.0f);
                    float PY = CenterY - BlockSize / 2 + (BY + 0.5f) * (BlockSize / 3.0f);

                    FVector Position(PX, PY, PlotH / 2.0f);
                    FVector Size(PlotW / 100.0f, PlotD / 100.0f, PlotH / 100.0f);

                    AActor* Building = GetWorld()->SpawnActor<AActor>();
                    Building->SetActorLocation(Position);
                    UStaticMeshComponent* BldgMesh = NewObject<UStaticMeshComponent>(Building);
                    BldgMesh->SetStaticMesh(BuildingMesh);
                    BldgMesh->SetWorldScale3D(Size);
                    BldgMesh->RegisterComponent();
                    Building->SetRootComponent(BldgMesh);
                    SpawnedActors.Add(Building);
                }
            }
        }
    }

    // ── Custom buildings (bank, landmarks, etc.) ──
    for (const FBuildingDef& B : CustomBuildings)
    {
        AActor* Bldg = GetWorld()->SpawnActor<AActor>();
        Bldg->SetActorLocation(FVector(B.Position.X, B.Position.Y, B.Height / 2.0f));
        UStaticMeshComponent* BldgMesh = NewObject<UStaticMeshComponent>(Bldg);
        BldgMesh->SetStaticMesh(BuildingMesh);
        BldgMesh->SetWorldScale3D(FVector(B.Size.X / 100.0f, B.Size.Y / 100.0f, B.Height / 100.0f));
        BldgMesh->RegisterComponent();
        Bldg->SetRootComponent(BldgMesh);
        SpawnedActors.Add(Bldg);
    }

    UE_LOG(LogTemp, Warning, TEXT("City generated: %d actors"), SpawnedActors.Num());
}

void ACKCityGenerator::SpawnRoad(FVector Start, FVector End, float Width) {}
void ACKCityGenerator::SpawnBuilding(FVector Position, FVector Size, float Height, FLinearColor Color) {}

void ACKCityGenerator::ClearCity()
{
    for (AActor* A : SpawnedActors)
    {
        if (A && A->IsValidLowLevel()) A->Destroy();
    }
    SpawnedActors.Empty();
}
