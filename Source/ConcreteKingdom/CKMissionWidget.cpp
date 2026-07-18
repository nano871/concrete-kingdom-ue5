#include "CKMissionWidget.h"
#include "Components/CanvasPanelSlot.h"

void UCKMissionWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    
    // Build UI from code — no Blueprint needed
    if (!RootPanel)
    {
        RootPanel = NewObject<UCanvasPanel>(this);
        if (RootPanel)
        {
            WidgetTree->RootWidget = RootPanel;
            
            TitleText = NewObject<UTextBlock>(this);
            TitleText->SetText(FText::FromString(TEXT("== MISSIONS ==")));
            TitleText->Font.Size = 24;
            TitleText->SetColorAndOpacity(FLinearColor::White);
            if (UPanelSlot* Slot = RootPanel->AddChild(TitleText))
                if (UCanvasPanelSlot* CSlot = Cast<UCanvasPanelSlot>(Slot))
                    CSlot->SetPosition(FVector2D(40, 40));
            
            MissionListText = NewObject<UTextBlock>(this);
            MissionListText->SetText(FText::FromString(TEXT("No missions available.")));
            MissionListText->Font.Size = 16;
            if (UPanelSlot* Slot2 = RootPanel->AddChild(MissionListText))
                if (UCanvasPanelSlot* CSlot2 = Cast<UCanvasPanelSlot>(Slot2))
                    CSlot2->SetPosition(FVector2D(40, 80));
        }
    }
    bVisible = false;
    SetVisibility(ESlateVisibility::Hidden);
}

void UCKMissionWidget::Show()
{
    bVisible = true;
    SetVisibility(ESlateVisibility::Visible);
    if (APlayerController* PC = GetOwningPlayer())
        PC->SetShowMouseCursor(true);
}

void UCKMissionWidget::Hide()
{
    bVisible = false;
    SetVisibility(ESlateVisibility::Hidden);
    if (APlayerController* PC = GetOwningPlayer())
        PC->SetShowMouseCursor(false);
}

void UCKMissionWidget::UpdateMissionList(const TArray<FString>& Available, const TArray<FString>& Completed)
{
    FString Text;
    Text += TEXT("AVAILABLE:\n");
    for (const FString& M : Available) Text += FString::Printf(TEXT("  [E] %s\n"), *M);
    Text += TEXT("\nCOMPLETED:\n");
    for (const FString& M : Completed) Text += FString::Printf(TEXT("  ✓ %s\n"), *M);
    if (MissionListText)
        MissionListText->SetText(FText::FromString(Text));
}
