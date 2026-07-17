#include "CKGunShopInteriorActor.h"

ACKGunShopInteriorActor::ACKGunShopInteriorActor() { PrimaryActorTick.bCanEverTick = false; }
void ACKGunShopInteriorActor::Enter() { UE_LOG(LogTemp, Warning, TEXT("Entered gun shop")); }
void ACKGunShopInteriorActor::Exit() { UE_LOG(LogTemp, Warning, TEXT("Exited gun shop")); }
bool ACKGunShopInteriorActor::BuyWeapon(FString WeaponName, int32 Price, int32& PlayerMoney)
{
    if (PlayerMoney < Price) return false;
    PlayerMoney -= Price;
    UE_LOG(LogTemp, Warning, TEXT("Bought %s for $%d"), *WeaponName, Price);
    return true;
}
TArray<FString> ACKGunShopInteriorActor::GetStock() { return {TEXT("pistol"), TEXT("rifle")}; }
