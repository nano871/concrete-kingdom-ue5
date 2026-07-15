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
    UE_LOG(LogTemp, Warning, TEXT("Generating city from Mapbox road data"));
}
