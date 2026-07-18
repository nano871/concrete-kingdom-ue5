#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "CKMissionWidget.generated.h"

UCLASS()
class CONCRETEKINGDOM_API UCKMissionWidget : public UUserWidget {
    GENERATED_BODY()
public:
    void NativeOnInitialized() override;
    void UpdateMissionList(const TArray<FString>& Available, const TArray<FString>& Completed);
    void Show();
    void Hide();
    bool bVisible;
    UPROPERTY() class UTextBlock* TitleText;
    UPROPERTY() class UTextBlock* MissionListText;
    UPROPERTY() class UCanvasPanel* RootPanel;
};
