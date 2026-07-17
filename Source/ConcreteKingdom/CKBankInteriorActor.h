#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKBankInteriorActor.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKBankInteriorActor : public AActor {
    GENERATED_BODY()
public:
    ACKBankInteriorActor();
    UFUNCTION(BlueprintCallable) void Enter();
    UFUNCTION(BlueprintCallable) void Exit();
    UFUNCTION(BlueprintCallable) int32 LootVault(); // returns money amount
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bVaultOpen;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 VaultMoney;
};
