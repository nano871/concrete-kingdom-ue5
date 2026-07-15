#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKSupabaseComponent.generated.h"

USTRUCT(BlueprintType)
struct FEconomyValue {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Key;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Value;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Category;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKSupabaseComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKSupabaseComponent();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supabase")
    FString SupabaseURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supabase")
    FString AnonKey;

    UFUNCTION(BlueprintCallable, Category = "Supabase")
    void FetchEconomy();

    UFUNCTION(BlueprintCallable, Category = "Supabase")
    int32 GetEconomyValue(FString Key);

    UFUNCTION(BlueprintCallable, Category = "Supabase")
    void SavePlayerState(int32 Money, int32 Wanted, const TArray<FString>& OwnedMissions);

    UPROPERTY(BlueprintReadOnly, Category = "Supabase")
    TMap<FString, int32> EconomyCache;

    UFUNCTION(BlueprintCallable, Category = "Supabase")
    bool IsConnected() { return bConnected; }

private:
    bool bConnected;
    void HandleResponse(FString Response);
};