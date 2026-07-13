#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "CKLevelGenerator.generated.h"

UCLASS()
class CONCRETEKINGDOM_API ACKLevelGenerator : public ALevelScriptActor
{
    GENERATED_BODY()
public:
    ACKLevelGenerator();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
    TSubclassOf<class AActor> CityGeneratorClass;

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void GenerateCompleteLevel();
};