#include "CKPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "CKCharacter.h"
ACKPlayerController::ACKPlayerController() { bShowMouseCursor = false; }
void ACKPlayerController::OnMove(const FInputActionValue& V) {
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->Move(V);
}
void ACKPlayerController::OnLook(const FInputActionValue& V) {
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->Look(V);
}
void ACKPlayerController::OnJump() {
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->StartJump();
}
void ACKPlayerController::OnInteract() {
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->Interact();
}
void ACKPlayerController::OnShoot() {
    if (auto* C = Cast<ACKCharacter>(GetPawn())) C->Shoot();
}
