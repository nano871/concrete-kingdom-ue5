#include "CKFactionWidget.h"
void UCKFactionWidget::ShowFactions() { bVisible = true; SetVisibility(ESlateVisibility::Visible); }
void UCKFactionWidget::HideFactions() { bVisible = false; SetVisibility(ESlateVisibility::Hidden); }
