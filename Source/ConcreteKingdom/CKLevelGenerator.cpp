// UE5.8 - Complete level generation
#include "CKLevelGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ACKLevelGenerator::ACKLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Load meshes in constructor (only place FObjectFinder is allowed)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
    CubeMesh = CubeFinder.Object;
    PlaneMesh = PlaneFinder.Object;
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

    if (!CubeMesh || !PlaneMesh) return;

    const float WorldSize = 16000.0f; // expanded 8x8 city
    const float RoadWidth = 400.0f;
    const float BlockSize = (WorldSize - RoadWidth * 5) / 6.0f;

    // ── Ground ──
    AActor* Ground = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(0, 0, -5), FRotator::ZeroRotator, SpawnParams);
    UStaticMeshComponent* GroundComp = NewObject<UStaticMeshComponent>(Ground);
    GroundComp->SetStaticMesh(PlaneMesh);
    GroundComp->SetWorldScale3D(FVector(WorldSize / 100.0f, WorldSize / 100.0f, 1));
    GroundComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GroundComp->RegisterComponent();
    Ground->SetRootComponent(GroundComp);

    // ── Roads (grid) ──
    for (int32 i = 0; i < 4; i++)
    {
        float Pos = -WorldSize / 2 + i * (BlockSize + RoadWidth) + RoadWidth + BlockSize / 2;

        // N-S road
        AActor* NSR = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(Pos, 0, -4.8f), FRotator::ZeroRotator, SpawnParams);
        UStaticMeshComponent* NSComp = NewObject<UStaticMeshComponent>(NSR);
        NSComp->SetStaticMesh(CubeMesh);
        NSComp->SetWorldScale3D(FVector(RoadWidth / 100.0f, WorldSize / 100.0f, 0.08f));
        NSComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        NSComp->RegisterComponent();
        NSR->SetRootComponent(NSComp);

        // E-W road
        AActor* EWR = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(0, Pos, -4.8f), FRotator::ZeroRotator, SpawnParams);
        UStaticMeshComponent* EWComp = NewObject<UStaticMeshComponent>(EWR);
        EWComp->SetStaticMesh(CubeMesh);
        EWComp->SetWorldScale3D(FVector(WorldSize / 100.0f, RoadWidth / 100.0f, 0.08f));
        EWComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        EWComp->RegisterComponent();
        EWR->SetRootComponent(EWComp);
    }

    // ── Buildings in each block ──
    for (int32 Col = 0; Col < 6; Col++)
    {
        for (int32 Row = 0; Row < 6; Row++)
        {
            float CX = -WorldSize / 2 + RoadWidth + Col * (BlockSize + RoadWidth) + RoadWidth / 2 + BlockSize / 5;
            float CY = -WorldSize / 2 + RoadWidth + Row * (BlockSize + RoadWidth) + RoadWidth / 2 + BlockSize / 4;

            for (int32 BX = 0; BX < 2; BX++)
            {
                for (int32 BY = 0; BY < 2; BY++)
                {
                    float PX = CX + BX * BlockSize / 2.5f;
                    float PY = CY + BY * BlockSize / 2.5f;
                    float H = 200 + FMath::RandRange(100, 800);
                    float S = BlockSize / 3.5f / 100.0f;

                    AActor* Bldg = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(PX, PY, H / 2), FRotator::ZeroRotator, SpawnParams);
                    UStaticMeshComponent* BldgComp = NewObject<UStaticMeshComponent>(Bldg);
                    BldgComp->SetStaticMesh(CubeMesh);
                    BldgComp->SetWorldScale3D(FVector(S, S, H / 100.0f));
                    BldgComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    BldgComp->SetCollisionProfileName(FName("BlockAll"));
                    BldgComp->RegisterComponent();
                    Bldg->SetRootComponent(BldgComp);
                }
            }
        }
    }

    // ── Player Start ──
    APlayerStart* PStart = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
        FVector(0, -WorldSize / 2 + 800, 100), FRotator::ZeroRotator, SpawnParams);

    // ── Lighting ──
    ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(),
        FVector(0, 0, 3000), FRotator(-45, 0, 0), SpawnParams);
    if (Sun)
    {
        Sun->SetMobility(EComponentMobility::Stationary);
        Sun->SetActorRotation(FRotator(-45, 30, 0));
    }

    // ── Sky Atmosphere (blue sky gradient) ──
    AActor* SkyAtm = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector(0, 0, 0), FRotator::ZeroRotator, SpawnParams);
    if (SkyAtm) SkyAtm->SetActorLabel(TEXT("SkyAtmosphere"));

    // ── Sky Light (ambient skylight from sky) ──
    ASkyLight* SkyLight = GetWorld()->SpawnActor<ASkyLight>(ASkyLight::StaticClass(),
        FVector(0, 0, 500), FRotator::ZeroRotator, SpawnParams);
    if (SkyLight)
    {
        SkyLight->GetLightComponent()->SetIntensity(1.0f);
        SkyLight->GetLightComponent()->SetIndirectLightingIntensity(1.0f);
    }

    // ── Exponential Height Fog (atmospheric depth) ──
    AExponentialHeightFog* Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(),
        FVector(0, 0, 0), FRotator::ZeroRotator, SpawnParams);
    if (Fog)
    {
        UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(0.02f);
            FogComp->FogHeightFalloff = 0.2f;
        }
    }

    
    // ── Traffic Lights at major intersections ──
    for (int32 TI = 0; TI < 4; TI++)
    {
        float TX = -WorldSize / 2 + TI * (BlockSize + RoadWidth) + RoadWidth + BlockSize / 2;
        float TY = -WorldSize / 2 + TI * (BlockSize + RoadWidth) + RoadWidth + BlockSize / 2;
        ACKTrafficLightActor* TL = GetWorld()->SpawnActor<ACKTrafficLightActor>(ACKTrafficLightActor::StaticClass(),
            FVector(TX, TY, 0), FRotator::ZeroRotator, SpawnParams);
        if (TL) TL->SetActorLabel(FString::Printf(TEXT("TrafficLight_%d"), TI));
    }

    // ── Initialize Day/Night cycle ──
    UCKDayNightComponent* DNC = FindComponentByClass<UCKDayNightComponent>();
    if (!DNC)
    {
        DNC = NewObject<UCKDayNightComponent>(this);
        DNC->RegisterComponent();
    }

    UE_LOG(LogTemp, Warning, TEXT("Concrete Kingdom level generated: 6x6 block city with roads, buildings, lighting, player start"));
}