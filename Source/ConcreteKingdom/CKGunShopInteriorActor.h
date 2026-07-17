#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKGunShopInteriorActor.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKGunShopInteriorActor : public AActor {
    GENERATED_BODY()
public:
    ACKGunShopInteriorActor();
    UFUNCTION(BlueprintCallable) void Enter();
    UFUNCTION(BlueprintCallable) void Exit();
    UFUNCTION(BlueprintCallable) bool BuyWeapon(FString WeaponName, int32 Price, int32& PlayerMoney);
    UFUNCTION(BlueprintCallable) TArray<FString> GetStock();
};
