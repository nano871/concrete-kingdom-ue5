#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKDayNightComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONCRETEKINGDOM_API UCKDayNightComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UCKDayNightComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DaySpeed; // multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunHeight;

    UFUNCTION(BlueprintCallable, Category = "Time")
    bool IsNight();

    UFUNCTION(BlueprintCallable, Category = "Time")
    float GetSunAltitude();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
    float SunAltitude;

private:
    class ADirectionalLight* SunLight;
    FLinearColor DayColor;
    FLinearColor NightColor;
    FLinearColor CurrentColor;
};