// UE5.8 - GTA V style lane-based traffic AI
#include "CKTrafficManager.h"
#include "CKTrafficVehicleComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

UCKTrafficManager::UCKTrafficManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxVehicles = 15;
    SpawnRadius = 8000.0f;
    SpawnTimer = 0.0f;
    DesiredDensity = 1.0f;
}

void UCKTrafficManager::BuildRoadNetwork()
{
    RoadLanes.Empty();
    const float WorldSize = 10000.0f;
    const float RoadWidth = 400.0f;
    const float BlockSize = (WorldSize - RoadWidth * 3) / 4.0f;

    // Build N-S lanes
    for (int32 i = 0; i < 4; i++)
    {
        float X = -WorldSize / 2 + i * (BlockSize + RoadWidth) + RoadWidth + BlockSize / 2;
        FRoadLane LaneNS, LaneSN;
        LaneNS.SpeedLimit = 600.0f;
        LaneSN.SpeedLimit = 600.0f;
        LaneNS.Direction = FVector(0, 1, 0);
        LaneSN.Direction = FVector(0, -1, 0);

        float Offset = RoadWidth * 0.3f;
        for (float Y = -WorldSize / 2; Y <= WorldSize / 2; Y += 200.0f)
        {
            LaneNS.Waypoints.Add(FVector(X - Offset, Y, -4.7f));
            LaneSN.Waypoints.Add(FVector(X + Offset, Y, -4.7f));
        }
        // Reverse SN waypoints so they go in correct order
        TArray<FVector> Reversed;
        for (int32 j = LaneSN.Waypoints.Num() - 1; j >= 0; j--)
            Reversed.Add(LaneSN.Waypoints[j]);
        LaneSN.Waypoints = Reversed;

        RoadLanes.Add(LaneNS);
        RoadLanes.Add(LaneSN);
    }

    // Build E-W lanes
    for (int32 i = 0; i < 4; i++)
    {
        float Y = -WorldSize / 2 + i * (BlockSize + RoadWidth) + RoadWidth + BlockSize / 2;
        FRoadLane LaneEW, LaneWE;
        LaneEW.SpeedLimit = 600.0f;
        LaneWE.SpeedLimit = 600.0f;
        LaneEW.Direction = FVector(1, 0, 0);
        LaneWE.Direction = FVector(-1, 0, 0);

        float Offset = RoadWidth * 0.3f;
        for (float X = -WorldSize / 2; X <= WorldSize / 2; X += 200.0f)
        {
            LaneEW.Waypoints.Add(FVector(X, Y - Offset, -4.7f));
            LaneWE.Waypoints.Add(FVector(X, Y + Offset, -4.7f));
        }
        TArray<FVector> Reversed;
        for (int32 j = LaneWE.Waypoints.Num() - 1; j >= 0; j--)
            Reversed.Add(LaneWE.Waypoints[j]);
        LaneWE.Waypoints = Reversed;

        RoadLanes.Add(LaneEW);
        RoadLanes.Add(LaneWE);
    }

    UE_LOG(LogTemp, Warning, TEXT("Traffic network built: %d lanes"), RoadLanes.Num());
}

void UCKTrafficManager::SetDesiredDensity(float Density)
{
    DesiredDensity = FMath::Clamp(Density, 0.0f, 1.0f);
}

void UCKTrafficManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (RoadLanes.Num() == 0) return;

    int32 TargetCount = FMath::FloorToInt(MaxVehicles * DesiredDensity);
    
    // Clean up destroyed vehicles
    ActiveVehicles.RemoveAll([](AActor* A) { return !IsValid(A); });

    // Spawn if below target
    SpawnTimer += DeltaTime;
    if (SpawnTimer > 1.5f && ActiveVehicles.Num() < TargetCount)
    {
        SpawnTimer = 0.0f;
        SpawnVehicle();
    }

    // Update all active vehicles
    UpdateVehicles(DeltaTime);
}

void UCKTrafficManager::SpawnVehicle()
{
    if (RoadLanes.Num() == 0) return;

    int32 LaneIdx = FMath::RandRange(0, RoadLanes.Num() - 1);
    FRoadLane& Lane = RoadLanes[LaneIdx];
    if (Lane.Waypoints.Num() < 2) return;

    int32 WpIdx = FMath::RandRange(0, Lane.Waypoints.Num() - 3);
    FVector StartPos = Lane.Waypoints[WpIdx];

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!CubeMesh) return;

    AActor* Vehicle = GetWorld()->SpawnActor<AActor>();
    if (!Vehicle) return;

    Vehicle->SetActorLocation(StartPos);
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Vehicle);
    MeshComp->SetStaticMesh(CubeMesh);
    MeshComp->SetWorldScale3D(FVector(0.6f, 1.0f, 0.25f));
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionProfileName(FName("BlockAll"));
    MeshComp->RegisterComponent();
    Vehicle->SetRootComponent(MeshComp);

    // Store waypoint index and lane ref on the actor
    Vehicle->Tags.Add(FName("TrafficVehicle"));
    
    // IDM parameters for car-following
    UCKTrafficVehicleComponent* Follower = NewObject<UCKTrafficVehicleComponent>(Vehicle);
    Follower->CurrentLaneIndex = LaneIdx;
    Follower->CurrentWaypoint = WpIdx;
    Follower->Speed = 0.0f; // start stopped
    Follower->DesiredSpeed = Lane.SpeedLimit * (0.5f + FMath::FRand() * 0.3f);
    Follower->TimeHeadway = 1.8f; // 1.8 second following distance (GTA V typical)
    Follower->MaxAcceleration = 200.0f;
    Follower->ComfortableBraking = 150.0f;
    Follower->MinimumGap = 200.0f; // minimum bumper-to-bumper gap in cm
    Follower->bInitialized = true;
    Follower->RegisterComponent();

    ActiveVehicles.Add(Vehicle);
}

void UCKTrafficManager::UpdateVehicles(float DeltaTime)
{
    for (AActor* V : ActiveVehicles)
    {
        if (!IsValid(V)) continue;
        UCKTrafficVehicleComponent* Follower = V->FindComponentByClass<UCKTrafficVehicleComponent>();
        if (!Follower || !Follower->bInitialized || Follower->CurrentLaneIndex >= RoadLanes.Num()) continue;

        FRoadLane& Lane = RoadLanes[Follower->CurrentLaneIndex];
        if (!Lane.Waypoints.IsValidIndex(Follower->CurrentWaypoint)) continue;

        // Find distance to lead vehicle (same lane, ahead)
        float LeadDist = 100000.0f;
        float LeadSpeed = 0.0f;
        for (AActor* Other : ActiveVehicles)
        {
            if (Other == V || !IsValid(Other)) continue;
            UCKTrafficVehicleComponent* OtherF = Other->FindComponentByClass<UCKTrafficVehicleComponent>();
            if (!OtherF || OtherF->CurrentLaneIndex != Follower->CurrentLaneIndex) continue;
            if (OtherF->CurrentWaypoint <= Follower->CurrentWaypoint) continue;
            float D = FVector::Dist(V->GetActorLocation(), Other->GetActorLocation());
            if (D < LeadDist) { LeadDist = D; LeadSpeed = OtherF->Speed; }
        }

        // IDM acceleration
        float Accel = Follower->IDMAcceleration(Follower->Speed, LeadDist, LeadSpeed);
        Follower->Speed += Accel * DeltaTime;
        Follower->Speed = FMath::Clamp(Follower->Speed, 0.0f, Follower->DesiredSpeed * 1.1f);

        FVector Target = Lane.Waypoints[Follower->CurrentWaypoint];
        FVector Current = V->GetActorLocation();
        FVector Dir = (Target - Current).GetSafeNormal();

        float Dist = FVector::Dist(Current, Target);
        if (Dist < 50.0f)
        {
            Follower->CurrentWaypoint++;
            if (Follower->CurrentWaypoint >= Lane.Waypoints.Num())
            {
                V->Destroy();
                continue;
            }
            Target = Lane.Waypoints[Follower->CurrentWaypoint];
            Dir = (Target - Current).GetSafeNormal();
        }

        FVector NewPos = Current + Dir * Follower->Speed * DeltaTime;
        V->SetActorLocation(NewPos);
        V->SetActorRotation(Dir.Rotation());
    }
}