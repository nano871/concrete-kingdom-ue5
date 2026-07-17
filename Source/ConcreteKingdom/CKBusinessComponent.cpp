#include "CKBusinessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

UCKBusinessComponent::UCKBusinessComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PassiveIncomeRate = 2;
}

void UCKBusinessComponent::InitBusinesses()
{
    DefineBusinesses();
    UE_LOG(LogTemp, Warning, TEXT("[BIZ] Initialized %d businesses"), Businesses.Num());
}

void UCKBusinessComponent::DefineBusinesses()
{
    TArray<FString> Names = {TEXT("Bar"), TEXT("Warehouse"), TEXT("Apartment"), TEXT("Office"), TEXT("Bodega")};
    TArray<FVector> Positions = {
        FVector(1000, -600, 0), FVector(-1800, -900, 0), FVector(-1400, 1600, 0),
        FVector(1600, 1600, 0), FVector(-2600, 2400, 0)
    };
    Businesses.Empty();
    for (int32 i = 0; i < Names.Num(); i++)
    {
        FBusinessDef B;
        B.BusinessID = FName(*FString::Printf(TEXT("biz_%d"), i));
        B.Name = Names[i];
        B.Location = Positions[i];
        B.Owner = TEXT("neutral");
        B.CaptureProgress = 0.0f;
        B.bPlayerOwned = false;
        Businesses.Add(B.BusinessID, B);
    }
    UE_LOG(LogTemp, Warning, TEXT("[BIZ] Defined %d businesses"), Businesses.Num());
}

void UCKBusinessComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    NearbyBusiness = FName("None");
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player) return;
    float BestDist = 500.0f;
    for (auto& Pair : Businesses)
    {
        float D = FVector::Dist(Player->GetActorLocation(), Pair.Value.Location);
        if (D < BestDist) { BestDist = D; NearbyBusiness = Pair.Key; }
    }
}

void UCKBusinessComponent::StartCapture(FName BusinessID)
{
    FBusinessDef* B = Businesses.Find(BusinessID);
    if (!B || B->bPlayerOwned) return;
    B->Owner = TEXT("contested");
}

float UCKBusinessComponent::GetPassiveIncome()
{
    float Total = 0;
    for (auto& Pair : Businesses)
        if (Pair.Value.bPlayerOwned) Total += 2.0f;
    return Total;
}

bool UCKBusinessComponent::CaptureBusiness(FString BusinessID)
{
    if (OwnedBusinesses.Contains(BusinessID)) return false;
    OwnedBusinesses.Add(BusinessID);
    UE_LOG(LogTemp, Warning, TEXT("[BIZ] Captured: %s"), *BusinessID);
    return true;
}

int32 UCKBusinessComponent::GetTotalIncome()
{
    return OwnedBusinesses.Num() * PassiveIncomeRate;
}

TArray<FString> UCKBusinessComponent::GetOwnedBusinesses()
{
    return OwnedBusinesses;
}
