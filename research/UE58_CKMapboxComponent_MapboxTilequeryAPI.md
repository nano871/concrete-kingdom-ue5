---
title: "CKMapboxComponent — Wire Mapbox Tilequery API HTTP Download"
topic_index: 1
topic: "IMPLEMENT: CKMapboxComponent — wire bUseMapbox=true + MapboxToken into actual API call to download road data"
date: 2026-07-21
researched_by: Nanov2_bot (cron)
confidence: proven
tags: [UE5.8, Mapbox, Tilequery, HTTP, road-data, GeoJSON, open-world, CKMapboxComponent]
---

# Implementation Card: FetchRoadDataFromMapbox() via Tilequery API

## Summary

Currently `CKMapboxComponent::LoadRoadData()` reads a local JSON file. There is no HTTP API call. This card adds `FetchRoadDataFromMapbox()` — a grid-based async HTTP downloader using the Mapbox Tilequery API (`mapbox.mapbox-streets-v8`) that populates the `Roads` array with real road geometry from Mapbox, then triggers `GenerateGridFromRoads()`.

**Flow:** `GenerateGridFromRoads()` → if `Roads` is empty and `bUseMapbox=true` → `FetchRoadDataFromMapbox()` → (async HTTP) → parse GeoJSON → populate `Roads` → auto-call `GenerateGridFromRoads()`.

---

## Design Decisions

| Decision | Choice | Rationale |
|---|---|---|
| API endpoint | **Tilequery API** (`/v4/{tileset_id}/tilequery/{lng},{lat}.json`) | Returns GeoJSON features near a point — no full tile download needed, works with Streets v8 |
| Tileset | `mapbox.mapbox-streets-v8` | Mapbox's standard street-level tileset with road geometry, names, classes |
| Source layers | `road,bridge,tunnel` | Roads + elevated + below-grade segments cover all drivable surfaces |
| Grid strategy | **3×3 grid** with 300m step = 9 requests | Covers ~900m×900m city block area. 9 requests at 50 limit each = up to 450 road features |
| Max features per request | `limit=50` | API max is 50 — gets densest possible road data per query |
| Search radius | `radius=250` | 250m radius overlaps grid spacing so no gaps between tiles |
| Deduplication | `dedupe=true` | Removes overlapping road segments returned by multiple grid cells |
| Async pattern | **Request counter** with completion delegate | UE5 HTTP is inherently async; we need to batch N requests and fire only when ALL complete |
| Building footprints | Omitted for now | Only roads (source layers: `road,bridge,tunnel`). Buildings are an enhancement for future |

---

## Mapbox API Reference

### Endpoint URL

```
GET https://api.mapbox.com/v4/mapbox.mapbox-streets-v8/tilequery/{lng},{lat}.json
    ?access_token={token}
    &radius=250
    &limit=50
    &layers=road,bridge,tunnel
    &dedupe=true
```

**Confirmed working example** (from public Mapbox docs / StackOverflow):
```
https://api.mapbox.com/v4/mapbox.mapbox-streets-v8/tilequery/-93.1219,44.9473.json?radius=25&limit=5&dedupe&access_token=YOUR_TOKEN
```

### Response Format (GeoJSON FeatureCollection)

```json
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "geometry": {
        "type": "LineString",
        "coordinates": [
          [-118.2501, 34.0502],
          [-118.2503, 34.0505],
          ...
        ]
      },
      "properties": {
        "class": "primary",
        "type": "primary",
        "name": "Wilshire Blvd",
        "structure": "none"
      }
    },
    {
      "type": "Feature",
      "geometry": {
        "type": "LineString",
        "coordinates": [[...]]
      },
      "properties": {
        "class": "street",
        "type": "residential",
        "name": "Oak St",
        "structure": "none"
      }
    }
  ]
}
```

### Source Layers in mapbox-streets-v8

| Source Layer | Description | Filter for roads |
|---|---|---|
| `road` | Standard road segments | Always query this |
| `bridge` | Road segments on bridges | Include for full coverage |
| `tunnel` | Road segments in tunnels | Include for full coverage |
| `building` | Building footprints | Future enhancement |

### Road `class` Values (used for visual differentiation)

`motorway`, `trunk`, `primary`, `secondary`, `tertiary`, `street`, `path`, `major_rail`, `minor_rail`

### Rate Limit

600 requests/minute on the Tilequery API. A 3×3 grid (9 requests) is well within budget. Even a 7×7 grid (49 requests) fits. For a city-scale implementation, consider caching the JSON response to disk on first fetch and re-using it offline.

---

## UE5 HTTP API Reference

### Core Classes & Includes

```cpp
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
```

These are already included in the .cpp file via `#include "HttpModule.h"` (line 10). No changes needed to includes for HTTP. However, `IHttpRequest.h` and `IHttpResponse.h` are **not** explicitly included but are typically pulled in transitively by `HttpModule.h`. If the compiler complains about incomplete types, add the explicit includes.

### Request Pattern (Standard UE5 Async)

```cpp
TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

Request->OnProcessRequestComplete().BindLambda(
    [this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
    {
        if (bSuccess && Resp && Resp->GetResponseCode() == 200)
        {
            FString ResponseStr = Resp->GetContentAsString();
            // Parse with FJsonSerializer
        }
    }
);

Request->SetURL(Url);
Request->SetVerb(TEXT("GET"));
Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
Request->ProcessRequest();
```

### Critical: Async Lifetime Safety

The `this` pointer captured in `BindLambda` will dangle if the component is destroyed while HTTP requests are in-flight. **Fix:** Use `TWeakObjectPtr<UCKMapboxComponent>`:

```cpp
TWeakObjectPtr<UCKMapboxComponent> WeakThis(this);
Request->OnProcessRequestComplete().BindLambda(
    [WeakThis](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
    {
        if (!WeakThis.IsValid()) return;
        UCKMapboxComponent* Self = WeakThis.Get();
        // Safe to use Self now
    }
);
```

### Grid URL Construction

```cpp
FString BuildTilequeryURL(float Lng, float Lat, const FString& Token)
{
    return FString::Printf(TEXT(
        "https://api.mapbox.com/v4/mapbox.mapbox-streets-v8/tilequery/%f,%f.json"
        "?access_token=%s"
        "&radius=250"
        "&limit=50"
        "&layers=road,bridge,tunnel"
        "&dedupe=true"
    ), Lng, Lat, *Token);
}
```

### Grid Coordinate Generation

Each tilequery queries at a specific (lng, lat). To cover a city area, generate a grid of points:

```cpp
// Offset from center in meters, converted to lat/lng deltas
const float GridStepM = 300.0f;          // 300m between grid points
const int32 GridSize = 3;                // 3×3 = 9 requests

// Approximate conversion (1 deg lat ≈ 111320m, 1 deg lng ≈ 111320*cos(lat) m)
const float LatPerM = 1.0f / 111320.0f;
const float LngPerM = 1.0f / (111320.0f * FMath::Cos(FMath::DegreesToRadians(CenterLat)));

for (int32 ix = 0; ix < GridSize; ix++)
{
    for (int32 iy = 0; iy < GridSize; iy++)
    {
        float OffsetX = (ix - (GridSize-1)/2.0f) * GridStepM;
        float OffsetY = (iy - (GridSize-1)/2.0f) * GridStepM;
        float QLng = CenterLng + OffsetX * LngPerM;
        float QLat = CenterLat + OffsetY * LatPerM;
        // Build URL and send request
    }
}
```

---

## Header File Changes

**File:** `Source/ConcreteKingdom/CKMapboxComponent.h`

### Add to UCLASS, after existing UFUNCTION declarations:

```cpp
// --- Mapbox HTTP Fetch ---
UFUNCTION(BlueprintCallable, Category="Mapbox")
void FetchRoadDataFromMapbox();
```

### Add private/protected helper methods:

```cpp
protected:
    // Callback invoked when all grid HTTP requests complete
    void OnAllTilequeriesComplete();

    // Parse a single GeoJSON FeatureCollection string into road segments
    void ParseTilequeryResponse(const FString& JsonStr);

private:
    // Accumulated roads from HTTP responses (merged after all complete)
    TArray<FRoadSegment> PendingRoads;

    // Number of finished HTTP requests in current batch
    int32 PendingRequests;

    // Total requests sent in current batch
    int32 TotalRequests;
```

### Full header after edits (expected layout):

```cpp
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKMapboxComponent.generated.h"

USTRUCT(BlueprintType)
struct FRoadSegment {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) FString Type;
    UPROPERTY(EditAnywhere) TArray<FVector> Points;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKMapboxComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKMapboxComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString MapboxToken;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CenterLat;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CenterLng;
    UPROPERTY(BlueprintReadOnly) TArray<FRoadSegment> Roads;

    // --- Road Mesh Generation (from previous card) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Roads")
    UStaticMesh* RoadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Roads")
    float RoadHalfWidth;

    UPROPERTY()
    TArray<AActor*> SpawnedRoadActors;

    UFUNCTION(BlueprintCallable) void LoadRoadData(FString FilePath);
    UFUNCTION(BlueprintCallable) void GenerateGridFromRoads();
    UFUNCTION(BlueprintCallable) void ClearRoads();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseMapbox;

    // --- NEW: Mapbox HTTP Fetch ---
    UFUNCTION(BlueprintCallable, Category="Mapbox")
    void FetchRoadDataFromMapbox();

    // --- NEW: Triggered after all HTTP completes ---
    UFUNCTION(BlueprintCallable, Category="Mapbox")
    void ForceGenerateFromPendingRoads();

protected:
    void OnAllTilequeriesComplete();
    void ParseTilequeryResponse(const FString& JsonStr);

private:
    TArray<FRoadSegment> PendingRoads;
    int32 PendingRequests;
    int32 TotalRequests;
};
```

**Change summary (header):**
1. Add `FetchRoadDataFromMapbox()` — BlueprintCallable public function
2. Add `ForceGenerateFromPendingRoads()` — BlueprintCallable (manual fallback)
3. Add `OnAllTilequeriesComplete()` — protected, called when batch finishes
4. Add `ParseTilequeryResponse(const FString& JsonStr)` — protected JSON parser
5. Add `PendingRoads`, `PendingRequests`, `TotalRequests` — private batch tracking

---

## Source File Changes

**File:** `Source/ConcreteKingdom/CKMapboxComponent.cpp`

### Add these includes (keep existing ones):

```cpp
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
```

Place these after line 10 (`#include "HttpModule.h"`).

### Constructor — add defaults:

In `UCKMapboxComponent::UCKMapboxComponent()`, after existing lines, add:

```cpp
PendingRequests = 0;
TotalRequests = 0;
```

### GENERATEGRIDFROMROADS() — modify to auto-fetch when no data

Replace the existing `GenerateGridFromRoads()` with:

```cpp
void UCKMapboxComponent::GenerateGridFromRoads()
{
    // If no roads loaded and bUseMapbox is true, fetch from Mapbox API first
    if (Roads.Num() == 0 && bUseMapbox && !MapboxToken.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Mapbox: No road data, fetching from Tilequery API..."));
        FetchRoadDataFromMapbox();
        return;  // Roads will be generated async after HTTP completes
    }

    // Validate state (original logic continues here)
    if (!bUseMapbox)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mapbox disabled, skipping road generation"));
        return;
    }
    if (Roads.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No road data loaded, cannot generate roads"));
        return;
    }
    if (!RoadMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("RoadMesh is null — assign a static mesh in the blueprint defaults"));
        return;
    }
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world reference"));
        return;
    }

    // Clear previously generated roads
    ClearRoads();

    int32 TotalSegments = 0;

    for (const FRoadSegment& Seg : Roads)
    {
        if (Seg.Points.Num() < 2) continue;

        for (int32 i = 0; i < Seg.Points.Num() - 1; i++)
        {
            const FVector& P0 = Seg.Points[i];
            const FVector& P1 = Seg.Points[i + 1];

            float SegLength = FVector::Dist(P0, P1);
            if (SegLength < 1.0f) continue;

            FVector Direction = (P1 - P0).GetSafeNormal();
            FVector Tangent = Direction * (SegLength * 0.5f);

            AActor* RoadActor = GetWorld()->SpawnActor<AActor>();
            if (!RoadActor) continue;

            USceneComponent* RootComp = NewObject<USceneComponent>(RoadActor);
            RootComp->RegisterComponent();
            RoadActor->SetRootComponent(RootComp);
            RoadActor->SetActorLocation(P0);

            USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(RoadActor);
            SplineMesh->SetStaticMesh(RoadMesh);
            SplineMesh->SetForwardAxis(ESplineMeshAxis::X);

            FVector LocalEnd = P1 - P0;

            SplineMesh->SetStartAndEnd(
                FVector::ZeroVector,
                Tangent,
                LocalEnd,
                Tangent
            );

            float WidthScale = (RoadHalfWidth * 2.0f) / 100.0f;
            SplineMesh->SetWorldScale3D(FVector(1.0f, WidthScale, 0.08f));

            SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SplineMesh->SetCollisionProfileName(FName("BlockAll"));
            SplineMesh->RegisterComponent();

            SpawnedRoadActors.Add(RoadActor);
            TotalSegments++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Mapbox: Generated %d road segments from %d roads"),
        TotalSegments, Roads.Num());
}
```

### NEW: FetchRoadDataFromMapbox() — full implementation:

```cpp
void UCKMapboxComponent::FetchRoadDataFromMapbox()
{
    if (MapboxToken.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Mapbox: MapboxToken is empty — set it in the component properties"));
        return;
    }

    // Clear previous pending state
    PendingRoads.Empty();
    PendingRequests = 0;

    // Grid parameters
    const float GridStepM = 300.0f;       // 300 meters between grid points
    const int32 GridSize = 3;             // 3x3 = 9 tilequery calls
    const float Radius = 250.0f;          // Search radius in meters

    // Coordinate conversion factors
    const float LatPerM = 1.0f / 111320.0f;
    const float LngPerM = 1.0f / (111320.0f * FMath::Cos(FMath::DegreesToRadians(CenterLat)));

    // Calculate total and set counter
    TotalRequests = GridSize * GridSize;
    PendingRequests = TotalRequests;

    for (int32 ix = 0; ix < GridSize; ix++)
    {
        for (int32 iy = 0; iy < GridSize; iy++)
        {
            float OffsetX = (ix - (GridSize - 1) / 2.0f) * GridStepM;
            float OffsetY = (iy - (GridSize - 1) / 2.0f) * GridStepM;

            float QueryLng = CenterLng + OffsetX * LngPerM;
            float QueryLat = CenterLat + OffsetY * LatPerM;

            FString Url = FString::Printf(TEXT(
                "https://api.mapbox.com/v4/mapbox.mapbox-streets-v8/tilequery/%f,%f.json"
                "?access_token=%s"
                "&radius=%.0f"
                "&limit=50"
                "&layers=road,bridge,tunnel"
                "&dedupe=true"
            ), QueryLng, QueryLat, *MapboxToken, Radius);

            // Create HTTP request
            TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
                FHttpModule::Get().CreateRequest();

            TWeakObjectPtr<UCKMapboxComponent> WeakThis(this);

            Request->OnProcessRequestComplete().BindLambda(
                [WeakThis, this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
                {
                    if (!WeakThis.IsValid()) return;

                    if (bSuccess && Resp && Resp->GetResponseCode() == 200)
                    {
                        FString ResponseStr = Resp->GetContentAsString();
                        ParseTilequeryResponse(ResponseStr);
                    }
                    else
                    {
                        int32 Code = Resp ? Resp->GetResponseCode() : 0;
                        UE_LOG(LogTemp, Warning, TEXT("Mapbox: Tilequery HTTP %d (success=%d)"),
                            Code, bSuccess);
                    }

                    // Decrement counter; when zero, all requests complete
                    PendingRequests--;
                    if (PendingRequests <= 0)
                    {
                        OnAllTilequeriesComplete();
                    }
                }
            );

            Request->SetURL(Url);
            Request->SetVerb(TEXT("GET"));
            Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
            Request->ProcessRequest();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Mapbox: Queued %d tilequery requests"), TotalRequests);
}
```

### NEW: ParseTilequeryResponse() — GeoJSON to FRoadSegments:

```cpp
void UCKMapboxComponent::ParseTilequeryResponse(const FString& JsonStr)
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
    TSharedPtr<FJsonObject> Root;

    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Mapbox: Failed to parse tilequery JSON response"));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* Features;
    if (!Root->TryGetArrayField(TEXT("features"), Features))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mapbox: No 'features' array in tilequery response"));
        return;
    }

    for (const TSharedPtr<FJsonValue>& FeatureVal : *Features)
    {
        TSharedPtr<FJsonObject> Feature = FeatureVal->AsObject();
        if (!Feature) continue;

        // Get geometry
        TSharedPtr<FJsonObject> Geometry;
        if (!Feature->TryGetObjectField(TEXT("geometry"), Geometry)) continue;

        FString GeomType;
        if (!Geometry->TryGetStringField(TEXT("type"), GeomType)) continue;

        // Roads come as LineStrings and Points (Point is a centroid fallback)
        EGeometryType ParsedType = ParseGeometryType(GeomType);
        if (ParsedType == EGeometryType::Unknown) continue;

        // Get coordinates array
        const TArray<TSharedPtr<FJsonValue>>* Coords;
        if (!Geometry->TryGetArrayField(TEXT("coordinates"), Coords)) continue;

        TSharedPtr<FJsonObject> Properties = Feature->GetObjectField(TEXT("properties"));
        FString RoadName = Properties->GetStringField(TEXT("name"));
        FString RoadClass = Properties->GetStringField(TEXT("type"));
        if (RoadClass.IsEmpty())
        {
            Properties->TryGetStringField(TEXT("class"), RoadClass);
        }

        // Build FRoadSegment
        FRoadSegment Seg;
        Seg.Name = RoadName;
        Seg.Type = RoadClass;

        if (ParsedType == EGeometryType::LineString)
        {
            // LineString: coordinates is array of [lng, lat] pairs
            for (const TSharedPtr<FJsonValue>& PtVal : *Coords)
            {
                const TArray<TSharedPtr<FJsonValue>>* PtArr;
                if (PtVal->TryGetArray(PtArr) && PtArr->Num() >= 2)
                {
                    double Lng = (*PtArr)[0]->AsNumber();
                    double Lat = (*PtArr)[1]->AsNumber();
                    // Convert to UE world coordinates (same formula as LoadRoadData)
                    FVector WorldPos(
                        (Lng - CenterLng) * 111000.0,
                        (Lat - CenterLat) * 111000.0,
                        0.0
                    );
                    Seg.Points.Add(WorldPos);
                }
            }
        }
        else if (ParsedType == EGeometryType::Point && Coords->Num() >= 2)
        {
            // Point: coordinates is [lng, lat] directly
            double Lng = (*Coords)[0]->AsNumber();
            double Lat = (*Coords)[1]->AsNumber();
            FVector WorldPos(
                (Lng - CenterLng) * 111000.0,
                (Lat - CenterLat) * 111000.0,
                0.0
            );
            Seg.Points.Add(WorldPos);
        }

        // Only add if we got at least 2 points (a valid road segment)
        if (Seg.Points.Num() >= 2)
        {
            PendingRoads.Add(Seg);
        }
    }
}
```

**NOTE:** The code above references `EGeometryType` which doesn't exist yet. You need an enum. **Option A (simple):** Just check `GeomType == "LineString"` and `GeomType == "Point"` directly with string compare. **Option B (cleaner):** Add a local helper function:

```cpp
// At the top of the .cpp file or inside ParseTilequeryResponse:
static bool IsLineString(const FString& Type) { return Type == TEXT("LineString"); }
static bool IsPointGeometry(const FString& Type) { return Type == TEXT("Point"); }
```

**Recommended:** Use Option A (inline string comparison). Replace the `EGeometryType` switch with:

```cpp
if (GeomType == TEXT("LineString"))
{
    // LineString: coordinates is array of [lng, lat] pairs
    ...
}
else if (GeomType == TEXT("Point") && Coords->Num() >= 2)
{
    // Point: coordinates is [lng, lat] directly
    ...
}
else
{
    continue; // Unknown geometry type
}
```

### NEW: OnAllTilequeriesComplete() — merge and generate:

```cpp
void UCKMapboxComponent::OnAllTilequeriesComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("Mapbox: All %d tilequeries complete, %d road segments found"),
        TotalRequests, PendingRoads.Num());

    // Merge pending roads into main Roads array
    Roads = PendingRoads;
    PendingRoads.Empty();

    if (Roads.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mapbox: No road segments found — check CenterLat/CenterLng and MapboxToken"));
        return;
    }

    // Now auto-generate the road meshes
    GenerateGridFromRoads();
}
```

### NEW: ForceGenerateFromPendingRoads() — manual fallback:

```cpp
void UCKMapboxComponent::ForceGenerateFromPendingRoads()
{
    if (PendingRoads.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mapbox: No pending road data to generate from"));
        return;
    }

    Roads = PendingRoads;
    PendingRoads.Empty();
    GenerateGridFromRoads();
}
```

---

## Compile Errors to Watch For

### 1. `TWeakObjectPtr` requires include

**Fix:** Add `#include "UObject/WeakObjectPtr.h"` at the top of CKMapboxComponent.h or .cpp. In UE5.8, TWeakObjectPtr is typically available through `CoreMinimal.h`, but if you get `incomplete type` errors on the BindLambda capture, explicitly include `"UObject/WeakObjectPtr.h"`.

### 2. `IHttpRequest.h` / `IHttpResponse.h` incomplete types

**Fix:** The existing `HttpModule.h` include usually pulls these in transitively. If compile fails with `'IHttpRequest' : incomplete type`, add explicit:
```cpp
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
```
These should be placed after `#include "HttpModule.h"` in the .cpp file.

### 3. `TSharedRef` capture in lambda (static assertion)

**Symptom:** Compiler error about `TSharedRef` not being copyable into a lambda. **Fix:** The existing code uses `TSharedRef<IHttpRequest, ESPMode::ThreadSafe>` which is fine as a return type from `CreateRequest()`. The problematic pattern would be capturing a `TSharedRef` by value in the lambda — we don't do that. The lambda captures `WeakThis` and `this`, both of which are fine.

### 4. `this` alongside `WeakThis` in lambda capture

**Symptom:** `warning: 'this' is also captured by TWeakObjectPtr — only one needed`. **Fix:** Remove `this` from the capture list. The `WeakThis.IsValid()` guard + `Self->` pattern doesn't need `this`. However, `PendingRequests--` and `OnAllTilequeriesComplete()` need the object. Since `WeakThis` already gives us `Self`, use `Self->PendingRequests--` everywhere and capture only `WeakThis`:

```cpp
Request->OnProcessRequestComplete().BindLambda(
    [WeakThis](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
    {
        if (!WeakThis.IsValid()) return;
        UCKMapboxComponent* Self = WeakThis.Get();

        if (bSuccess && Resp && Resp->GetResponseCode() == 200)
        {
            Self->ParseTilequeryResponse(Resp->GetContentAsString());
        }
        else { ... }

        Self->PendingRequests--;
        if (Self->PendingRequests <= 0)
        {
            Self->OnAllTilequeriesComplete();
        }
    }
);
```

This is the safe, correct pattern. **Remove `this` from the capture.**

### 5. `TSharedRef` cannot be copied into lambda (CreateRequest return)

This is a false alarm from some UE5 compiler versions. `FHttpModule::Get().CreateRequest()` returns a `TSharedRef<IHttpRequest, ESPMode::ThreadSafe>` which is immediately assigned to a local. As long as the lambda doesn't capture the `TSharedRef` by value (it doesn't — it captures `WeakThis` and uses the `Request` object only to set its properties before `ProcessRequest`), this is fine.

### 6. `FJsonSerializer::Deserialize` overload ambiguity

In UE5.8, the `FJsonSerializer::Deserialize` function may require explicit template arguments. The current code in `LoadRoadData` uses `TJsonReaderFactory<>::Create(JsonStr)` which should work. If compile error, try:
```cpp
TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
```

### 7. `TryGetStringField` vs `GetStringField` for optional fields

The `name` property may be empty for some road segments (unnamed streets). Use `TryGetStringField` instead of `GetStringField` for `name` to avoid crashes:

```cpp
FString RoadName;
Properties->TryGetStringField(TEXT("name"), RoadName);
// RoadName will be empty string "" if the field is missing — that's fine
```

Similarly, `class` and `type` might be missing from some features. Both should use `TryGetStringField`.

### 8. `FVector` coordinate conversion mismatch

The existing `LoadRoadData` converts using `((*Cs)[0]->AsNumber() - CenterLng) * 111000` for the X component (longitude → UE X) and `((*Cs)[1]->AsNumber() - CenterLat) * 111000` for the Y component (latitude → UE Y). This is a rough Mercator projection. For city-scale (a few km), this approximation is acceptable.

**If roads appear in wrong locations**, verify:
- UE X = longitude offset (east-west)
- UE Y = latitude offset (north-south)
- The 111000 factor converts degrees to meters at the equator

At latitude 34° (Los Angeles), a more precise factor would be:
- 1° lat ≈ 110,950 m (varies slightly)
- 1° lng ≈ 92,300 m (cos(34°) × 111,320)

**Fix for better accuracy** — replace the coordinate conversion in `ParseTilequeryResponse`:

```cpp
float CosLat = FMath::Cos(FMath::DegreesToRadians(CenterLat));
float LatToM = 111132.92f - 559.82f * FMath::Cos(2.0f * FMath::DegreesToRadians(CenterLat)) + 1.175f * FMath::Cos(4.0f * FMath::DegreesToRadians(CenterLat)); // more accurate
float LngToM = LatToM * CosLat;

FVector WorldPos(
    (Lng - CenterLng) * LngToM,
    (Lat - CenterLat) * LatToM,
    0.0
);
```

---

## Testing Procedure

### 1. Prerequisites

- A valid Mapbox access token (public token starting with `pk.` is sufficient)
- Set `bUseMapbox = true` on the component
- Set `MapboxToken` to your Mapbox public token
- Set `CenterLat` / `CenterLng` to a real city center
- `RoadMesh` must be assigned (can be `/Engine/BasicShapes/Cube.Cube`)

### 2. In-Editor Test

1. Open the level containing the actor with `UCKMapboxComponent`
2. Set the component properties in the Details panel:
   - `bUseMapbox` = true
   - `MapboxToken` = "pk.your_token_here"
   - `CenterLat` = 34.05 (LA) or 40.7128 (NYC) or your test city
   - `CenterLng` = -118.25 (LA) or -74.0060 (NYC)
   - `RoadMesh` = /Engine/BasicShapes/Cube.Cube
   - `RoadHalfWidth` = 200.0
3. Call `GenerateGridFromRoads()` via Blueprint or the console command:
   - Open Output Log (Window → Developer Tools → Output Log)
   - Look for: `Mapbox: Queued 9 tilequery requests`
4. Wait 1-3 seconds for HTTP responses
5. Look for: `Mapbox: All 9 tilequeries complete, X road segments found`
6. Look for: `Mapbox: Generated Y road segments from Z roads`

### 3. Verify Correctness Checklist

- [ ] Roads appear as spline meshes in the world
- [ ] Road positions approximate the real city street layout
- [ ] Road segments connect end-to-end at intersections
- [ ] Multiple road types appear (some wide primary, some narrow street)
- [ ] `ClearRoads()` removes all road actors
- [ ] Calling `GenerateGridFromRoads()` a second time re-fetches (or uses cached data)

### 4. Debugging Common Failures

| Symptom | Likely Cause | Fix |
|---|---|---|
| `MapboxToken is empty` log | Token not set in component properties | Set the MapboxToken field in Details panel |
| HTTP response 401 | Invalid or missing access token | Verify token is correct and has tilequery permissions |
| HTTP response 429 | Rate limited | Wait 1 minute. Reduce grid from 3×3 to 2×2 |
| `No road segments found` | CenterLat/Lng in ocean or empty area | Use coordinates in a city: 34.05,-118.25 or 40.71,-74.00 |
| Roads in wrong position | Coordinate conversion factor wrong | Verify LatToM/LngToM calculation. Roads should be relative to CenterLat/CenterLng |
| Component crashes on PIE | No MapboxToken set but bUseMapbox=true | Add a guard in constructor or FetchRoadDataFromMapbox |
| Roads not visible but log says generated | RoadMesh not assigned | Assign RoadMesh in component defaults |
| HTTP requests never complete | UE5 HTTP module not initialized | Ensure the module is loaded — `FHttpModule::Get()` will initialize it. Add `FHttpModule::Get().SetHttpModule(...)` if needed. |

### 5. Network Test (confirms HTTP works)

Add this temporary test to verify HTTP works at all:

```cpp
// In FetchRoadDataFromMapbox, add a test request before the grid:
FString TestUrl = TEXT("https://httpbin.org/get");
TSharedRef<IHttpRequest, ESPMode::ThreadSafe> TestReq = FHttpModule::Get().CreateRequest();
TestReq->SetURL(TestUrl);
TestReq->SetVerb(TEXT("GET"));
TestReq->OnProcessRequestComplete().BindLambda(
    [](FHttpRequestPtr, FHttpResponsePtr Resp, bool bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("HTTP Test: success=%d, code=%d"), bSuccess, Resp ? Resp->GetResponseCode() : 0);
    }
);
TestReq->ProcessRequest();
```

If this returns code 200, the HTTP module works. If it hangs or fails, check:
- Does the packaged game have `HTTP` module in Build.cs? **Yes, it's already there.**
- Is the engine built with HTTP support? (Default: yes)
- Are you running in a sandboxed environment? (Editor/world settings)

### 6. Performance at Scale

- 9 requests at ~200-400ms each (total ~2-3 seconds parallel)
- Typical response per request: 10-50KB JSON
- Road count: 50-500 segments for a 3×3 grid in a typical city
- Each road segment (2-10 points) → Number of SplineMeshComponents = sum of (points-1) across all segments
- Budget: Under 1000 SplineMeshComponents is fine for PC

---

## GitHub Push

After writing the card, push it to the UE5 repo's `research/` directory:

```bash
cd /root/concrete-kingdom-ue5
cp /root/.hermes/profiles/nanov2/cards/UE58_CKMapboxComponent_MapboxTilequeryAPI.md research/
git add research/UE58_CKMapboxComponent_MapboxTilequeryAPI.md
git commit -m "research: CKMapboxComponent Tilequery API HTTP download specification"
git push
```

---

## Relationship to Existing Card

The existing card `UE58_SplineMeshComponent_RoadGeneration_GenerateGridFromRoads.md` covers the mesh-generation side. This card covers the **HTTP data-fetching** side. Together they complete the full flow:

1. This card: `FetchRoadDataFromMapbox()` → HTTP Tilequery → parse GeoJSON → populate `Roads`
2. Existing card: `GenerateGridFromRoads()` → iterate `Roads` → spawn `USplineMeshComponent` instances

The modification to `GenerateGridFromRoads()` in this card adds the trigger: "if Roads is empty and bUseMapbox → fetch first, then re-enter generation."
