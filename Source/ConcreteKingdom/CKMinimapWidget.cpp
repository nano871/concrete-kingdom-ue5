#include "CKMinimapWidget.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UCKMinimapWidget::UpdateMinimap(const FVector& PlayerLocation, float PlayerYaw)
{
    if (!PlayerArrow) return;
    float NormX = (PlayerLocation.X / WorldSize + 0.5f) * MinimapScale;
    float NormY = (PlayerLocation.Y / WorldSize + 0.5f) * MinimapScale;
    FVector2D Pos(NormX, NormY);

    // Update arrow rotation
    PlayerArrow->SetRenderTranslation(Pos);
    PlayerArrow->SetRenderTransformAngle(PlayerYaw);
}
