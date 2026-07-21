// Mapbox-powered road network generation
#include "CKMapboxComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HttpModule.h"

UCKMapboxComponent::UCKMapboxComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bUseMapbox = false;
    CenterLat = 34.05f; CenterLng = -118.25f;
}

void UCKMapboxComponent::LoadRoadData(FString FilePath)
{
    FString JsonStr;
    if (!FFileHelper::LoadFileToString(JsonStr, *FilePath)) return;
    
    TSharedRef<TJsonReader<>> Rdr = TJsonReaderFactory<>::Create(JsonStr);
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(Rdr, Root)) return;
    
    const TArray<TSharedPtr<FJsonValue>>* RoadsArr;
    if (!Root->TryGetArrayField("roads", RoadsArr)) return;
    
    for (auto& V : *RoadsArr)
    {
        TSharedPtr<FJsonObject> O = V->AsObject();
        if (!O) continue;
        FRoadSegment Seg;
        Seg.Name = O->GetStringField("name");
        Seg.Type = O->GetStringField("type");
        const TArray<TSharedPtr<FJsonValue>>* Pts;
        if (O->TryGetArrayField("points", Pts))
            for (auto& Pt : *Pts)
            {
                const TArray<TSharedPtr<FJsonValue>>* Cs;
                if (Pt->AsArray(Cs) && Cs->Num() >= 2)
                    Seg.Points.Add(FVector(((*Cs)[0]->AsNumber() - CenterLng) * 111000,
                                           ((*Cs)[1]->AsNumber() - CenterLat) * 111000, 0));
            }
        Roads.Add(Seg);
    }
    UE_LOG(LogTemp, Warning, TEXT("Loaded %d Mapbox road segments"), Roads.Num());
}

void UCKMapboxComponent::GenerateGridFromRoads()
{
    if (!bUseMapbox || Roads.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mapbox data empty, falling back to grid"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Generating city from %d Mapbox road segments"), Roads.Num());

    UWorld* World = GetWorld();
    if (!World) return;

    // Load a cube mesh for placeholder buildings
    UStaticMesh* CubeMesh = nullptr;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded()) CubeMesh = CubeFinder.Object;

    int32 BuildingCount = 0;
    for (const FRoadSegment& Seg : Roads)
    {
        if (Seg.Points.Num() < 2) continue;

        // Walk each road segment and place buildings along both sides
        for (int32 i = 0; i < Seg.Points.Num() - 1; i++)
        {
            FVector A = Seg.Points[i];
            FVector B = Seg.Points[i + 1];
            FVector Dir = (B - A).GetSafeNormal();
            FVector Side = FVector(-Dir.Y, Dir.X, 0); // perpendicular
            float SegLength = FVector::Dist(A, B);
            int32 BuildingsPerSide = FMath::Max(1, FMath::FloorToInt(SegLength / 2000.0f));

            for (int32 b = 0; b < BuildingsPerSide; b++)
            {
                float T = (b + 0.5f) / BuildingsPerSide;
                FVector RoadPos = FMath::Lerp(A, B, T);

                // Place buildings on both sides, offset 800 units from road center
                for (int32 SideSign = -1; SideSign <= 1; SideSign += 2)
                {
                    FVector BuildPos = RoadPos + Side * SideSign * 800.0f;
                    BuildPos.Z += 200.0f; // half height

                    // Scale varies by road type
                    float Scale = (Seg.Type == TEXT("highway") || Seg.Type == TEXT("primary")) ? 2.0f : 1.0f;

                    // Spawn the building
                    FActorSpawnParameters Params;
                    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                    AActor* Building = World->SpawnActor<AActor>(AActor::StaticClass(), BuildPos, FRotator::ZeroRotator, Params);
                    if (Building)
                    {
                        // Attach a cube mesh as the visual
                        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Building);
                        if (MeshComp && CubeMesh)
                        {
                            MeshComp->SetStaticMesh(CubeMesh);
                            MeshComp->SetWorldScale3D(FVector(Scale * 3.0f, Scale * 3.0f, Scale * 1.5f + FMath::FRand() * 3.0f));
                            MeshComp->SetWorldLocation(BuildPos);
                            // Vary color by road type
                            FLinearColor Color = (Seg.Type == TEXT("highway")) ? FLinearColor(0.3f, 0.3f, 0.3f) :
                                                  (Seg.Type == TEXT("primary")) ? FLinearColor(0.5f, 0.2f, 0.1f) :
                                                  FLinearColor(0.6f, 0.6f, 0.7f);
                            MeshComp->SetVectorParameterValueOnMaterials(TEXT("Color"), Color);
                            MeshComp->SetupAttachment(Building->GetRootComponent());
                            MeshComp->RegisterComponent();
                        }
                        BuildingCount++;
                    }
                }
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Generated %d buildings from Mapbox road data"), BuildingCount);
}
