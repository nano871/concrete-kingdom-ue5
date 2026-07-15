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
    UFUNCTION(BlueprintCallable) void LoadRoadData(FString FilePath);
    UFUNCTION(BlueprintCallable) void GenerateGridFromRoads();
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseMapbox;
};