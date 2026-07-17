#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKPhoneScreenWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKPhoneScreenWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void OpenPhone();
    UFUNCTION(BlueprintCallable) void ClosePhone();
    UFUNCTION(BlueprintCallable) void SelectContact(int32 Index);
    UFUNCTION(BlueprintCallable) void ShowMessage(FString Sender, FString Message);
    UFUNCTION(BlueprintCallable) bool IsOpen() { return bIsOpen; }
private:
    bool bIsOpen;
};
