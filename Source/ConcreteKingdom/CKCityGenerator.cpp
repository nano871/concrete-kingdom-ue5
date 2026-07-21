// UE5.8 - procedural city generator
#include "CKCityGenerator.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACKCityGenerator::ACKCityGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
    GridSizeX = 3;
    GridSizeY = 3;
    BlockSize = 800.0f;
    RoadWidth = 400.0f;

    // Load meshes in constructor (only place FObjectFinder is allowed)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
    CubeMesh = CubeFinder.Object;
    PlaneMesh = PlaneFinder.Object;
}

void ACKCityGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateCity();
}

void ACKCityGenerator::GenerateCity()
{
    if (!GetWorld() || !CubeMesh || !PlaneMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("CKCityGenerator: Cannot load basic meshes"));
        return;
    }

    float TotalWidth = GridSizeX * (BlockSize + RoadWidth) + RoadWidth;
    float TotalHeight = GridSizeY * (BlockSize + RoadWidth) + RoadWidth;

    // Ground
    AActor* Ground = GetWorld()->SpawnActor<AActor>();
    UStaticMeshComponent* GroundComp = NewObject<UStaticMeshComponent>(Ground);
    GroundComp->SetStaticMesh(PlaneMesh);
    GroundComp->SetWorldScale3D(FVector(TotalWidth / 100.0f, TotalHeight / 100.0f, 1));
    GroundComp->RegisterComponent();
    Ground->SetRootComponent(GroundComp);
    Ground->SetActorLocation(FVector(0, 0, -5));

    // Roads and buildings
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            float CX = -TotalWidth / 2 + RoadWidth + X * (BlockSize + RoadWidth) + BlockSize / 2 + RoadWidth / 2;
            float CY = -TotalHeight / 2 + RoadWidth + Y * (BlockSize + RoadWidth) + BlockSize / 2 + RoadWidth / 2;

            // Roads
            AActor* NSR = GetWorld()->SpawnActor<AActor>();
            UStaticMeshComponent* NSComp = NewObject<UStaticMeshComponent>(NSR);
            NSComp->SetStaticMesh(CubeMesh);
            NSComp->SetWorldScale3D(FVector(RoadWidth / 100.0f, TotalHeight / 100.0f, 0.08f));
            NSComp->RegisterComponent();
            NSR->SetRootComponent(NSComp);
            NSR->SetActorLocation(FVector(CX, 0, -4.8f));

            AActor* EWR = GetWorld()->SpawnActor<AActor>();
            UStaticMeshComponent* EWComp = NewObject<UStaticMeshComponent>(EWR);
            EWComp->SetStaticMesh(CubeMesh);
            EWComp->SetWorldScale3D(FVector(TotalWidth / 100.0f, RoadWidth / 100.0f, 0.08f));
            EWComp->RegisterComponent();
            EWR->SetRootComponent(EWComp);
            EWR->SetActorLocation(FVector(0, CY, -4.8f));

            // Buildings
            for (int32 BX = 0; BX < 2; BX++)
            {
                for (int32 BY = 0; BY < 2; BY++)
                {
                    float BXPos = CX - BlockSize / 4 + BX * BlockSize / 2;
                    float BYPos = CY - BlockSize / 4 + BY * BlockSize / 2;
                    float BH = 200 + FMath::FRand() * 600;

                    AActor* Bldg = GetWorld()->SpawnActor<AActor>();
                    UStaticMeshComponent* BComp = NewObject<UStaticMeshComponent>(Bldg);
                    BComp->SetStaticMesh(CubeMesh);
                    BComp->SetWorldScale3D(FVector(BlockSize / 5.0f / 100.0f, BlockSize / 5.0f / 100.0f, BH / 100.0f));
                    BComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    BComp->SetCollisionProfileName(FName("BlockAll"));
                    BComp->RegisterComponent();
                    Bldg->SetRootComponent(BComp);
                    Bldg->SetActorLocation(FVector(BXPos, BYPos, 0));
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("City generated: %dx%d blocks"), GridSizeX, GridSizeY);
}

void ACKCityGenerator::SpawnRoad(FVector Start, FVector End, float Width)
{
    if (!GetWorld() || !CubeMesh) return;
    AActor* Road = GetWorld()->SpawnActor<AActor>();
    if (!Road) return;
    UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Road);
    Comp->SetStaticMesh(CubeMesh);
    FVector Mid = (Start + End) * 0.5f;
    float Length = FVector::Dist(Start, End);
    Comp->SetWorldScale3D(FVector(Width / 100.0f, Length / 100.0f, 0.08f));
    Comp->RegisterComponent();
    Road->SetRootComponent(Comp);
    Road->SetActorLocation(Mid);
    FRotator Rot = (End - Start).Rotation();
    Road->SetActorRotation(FRotator(0, Rot.Yaw, 0));
    SpawnedActors.Add(Road);
    RoadEndpoints.Add(Start); RoadEndpoints.Add(End);
}

void ACKCityGenerator::SpawnBuilding(FVector Position, FVector Size, float Height, FLinearColor Color)
{
    if (!GetWorld() || !CubeMesh) return;
    AActor* Bldg = GetWorld()->SpawnActor<AActor>();
    if (!Bldg) return;
    UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Bldg);
    Comp->SetStaticMesh(CubeMesh);
    Comp->SetWorldScale3D(FVector(Size.X / 100.0f, Size.Y / 100.0f, Height / 100.0f));
    Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Comp->SetCollisionProfileName(FName("BlockAll"));
    Comp->RegisterComponent();
    Bldg->SetRootComponent(Comp);
    Bldg->SetActorLocation(Position);
    SpawnedActors.Add(Bldg);
}

void ACKCityGenerator::ClearCity()
{
    for (AActor* A : SpawnedActors)
        if (IsValid(A)) A->Destroy();
    SpawnedActors.Empty();
    RoadEndpoints.Empty();
}
