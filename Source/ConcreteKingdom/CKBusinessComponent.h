#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKBusinessComponent.generated.h"

USTRUCT(BlueprintType)
struct FBusinessDef {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FName BusinessID;
    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) FVector Location;
    UPROPERTY(EditAnywhere) FString Owner; // player, mafia, syndicate, neutral
    UPROPERTY(BlueprintReadWrite) float CaptureProgress;
    UPROPERTY(BlueprintReadWrite) bool bPlayerOwned;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKBusinessComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKBusinessComponent();
    UFUNCTION(BlueprintCallable) void DefineBusinesses();
    UFUNCTION(BlueprintCallable) void StartCapture(FName BusinessID);
    UFUNCTION(BlueprintCallable) float GetPassiveIncome();
    UPROPERTY(BlueprintReadOnly) TMap<FName, FBusinessDef> Businesses;
    UPROPERTY(BlueprintReadOnly) FName NearbyBusiness;
};
