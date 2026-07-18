#include "CKGunShopInteriorActor.h"
#include "Components/StaticMeshComponent.h"

ACKGunShopInteriorActor::ACKGunShopInteriorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunShopMesh"));
    RootComponent = Mesh;
    Mesh->SetWorldScale3D(FVector(2, 2, 2));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionProfileName(FName("BlockAll"));
}

void ACKGunShopInteriorActor::Enter()
{
    SetActorHiddenInGame(false);
    UE_LOG(LogTemp, Warning, TEXT("[SHOP] Entered gun shop"));
}

void ACKGunShopInteriorActor::Exit()
{
    SetActorHiddenInGame(true);
    UE_LOG(LogTemp, Warning, TEXT("[SHOP] Exited gun shop"));
}

bool ACKGunShopInteriorActor::BuyWeapon(FString WeaponName, int32 Price, int32& PlayerMoney)
{
    if (PlayerMoney < Price) return false;
    PlayerMoney -= Price;
    UE_LOG(LogTemp, Warning, TEXT("[SHOP] Bought %s for $%d"), *WeaponName, Price);
    return true;
}

TArray<FString> ACKGunShopInteriorActor::GetStock()
{
    return {TEXT("pistol"), TEXT("rifle")};
}
