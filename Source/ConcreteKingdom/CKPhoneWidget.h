#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CKPhoneWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKPhoneWidget : public UUserWidget {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void OpenPhone();
    UFUNCTION(BlueprintCallable) void ClosePhone();
    UFUNCTION(BlueprintCallable) bool IsPhoneOpen() { return bPhoneOpen; }

    UPROPERTY(meta=(BindWidget)) class UTextBlock* ContactList;
    UPROPERTY(meta=(BindWidget)) class UImage* PhoneBackground;

    UFUNCTION(BlueprintCallable) void AddContact(FString Name, FString Status);
    UFUNCTION(BlueprintCallable) void ShowMissionBrief(FString Title, FString Desc);

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> Contacts;
private:
    bool bPhoneOpen;
};
