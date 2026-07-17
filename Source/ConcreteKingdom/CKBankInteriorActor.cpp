#include "CKBankInteriorActor.h"
#include "Components/StaticMeshComponent.h"

ACKBankInteriorActor::ACKBankInteriorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bVaultOpen = false;
    VaultMoney = 1000;
    UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
}

void ACKBankInteriorActor::Enter() { SetActorHiddenInGame(false); bVaultOpen = false; UE_LOG(LogTemp, Warning, TEXT("Entered bank")); }
void ACKBankInteriorActor::Exit() { SetActorHiddenInGame(true); UE_LOG(LogTemp, Warning, TEXT("Exited bank")); }
int32 ACKBankInteriorActor::LootVault() { bVaultOpen = true; return VaultMoney; }
