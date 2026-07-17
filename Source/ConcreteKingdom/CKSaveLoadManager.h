#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKSaveLoadManager.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKSaveLoadManager : public UActorComponent {
    GENERATED_BODY()
public:
    UCKSaveLoadManager();
    UFUNCTION(BlueprintCallable) void AutoSave();
    UFUNCTION(BlueprintCallable) void QuickSave();
    UFUNCTION(BlueprintCallable) void QuickLoad();
};
