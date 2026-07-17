#include "CKCoverIndicatorWidget.h"
#include "Components/Image.h"

void UCKCoverIndicatorWidget::SetInCover(bool bCovered) { bInCover = bCovered; SetVisibility(bCovered ? ESlateVisibility::Visible : ESlateVisibility::Hidden); }
void UCKCoverIndicatorWidget::SetBlindFire(bool bFiring) { bBlindFiring = bFiring; }
