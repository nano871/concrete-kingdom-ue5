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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable) void InitBusinesses();
    UFUNCTION(BlueprintCallable) void DefineBusinesses();
    UFUNCTION(BlueprintCallable) void StartCapture(FName BusinessID);
    UFUNCTION(BlueprintCallable) bool CaptureBusiness(FString BusinessID);
    UFUNCTION(BlueprintCallable) int32 GetTotalIncome();
    UFUNCTION(BlueprintCallable) float GetPassiveIncome();
    UFUNCTION(BlueprintCallable) TArray<FString> GetOwnedBusinesses();

    UPROPERTY(BlueprintReadOnly) TMap<FName, FBusinessDef> Businesses;
    UPROPERTY(BlueprintReadOnly) FName NearbyBusiness;
    UPROPERTY() TArray<FString> OwnedBusinesses;
    UPROPERTY() int32 PassiveIncomeRate;
};
