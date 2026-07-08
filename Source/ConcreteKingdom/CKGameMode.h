#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CKGameMode.generated.h"
UCLASS()
class CONCRETEKINGDOM_API ACKGameMode : public AGameModeBase {
    GENERATED_BODY()
public:
    ACKGameMode();
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 WantedLevel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Money;
    UFUNCTION(BlueprintCallable) void AddWantedLevel(int32 A);
    UFUNCTION(BlueprintCallable) void AddMoney(int32 A);
};