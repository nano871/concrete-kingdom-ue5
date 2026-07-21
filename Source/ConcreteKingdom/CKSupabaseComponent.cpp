// Supabase integration - game economy, missions, player state
#include "CKSupabaseComponent.h"
#include "Engine/World.h"
#include "Http.h"
#include "Json.h"

UCKSupabaseComponent::UCKSupabaseComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bConnected = false;
    SupabaseURL = "https://psjeegjoumdorbygxkfd.supabase.co";
    AnonKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InBzamVlZ2pvdW1kb3JieWd4a2ZkIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODQxMzg1NjgsImV4cCI6MjA5OTcxNDU2OH0.7-LUy-MF4JQn59ury6rnwpvKk4IHFjCG5rTLMJp9Qto";
}

void UCKSupabaseComponent::BeginPlay()
{
    Super::BeginPlay();
    FetchEconomy();
}

void UCKSupabaseComponent::FetchEconomy()
{
    if (!GetWorld()) return;
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest> Request = Http->CreateRequest();
    Request->SetURL(SupabaseURL + "/rest/v1/economy?select=key,value,category");
    Request->SetVerb("GET");
    Request->SetHeader("apikey", AnonKey);
    Request->SetHeader("Authorization", "Bearer " + AnonKey);
    Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
    {
        if (bSuccess && Resp->GetResponseCode() == 200)
        {
            HandleResponse(Resp->GetContentAsString());
            bConnected = true;
            UE_LOG(LogTemp, Warning, TEXT("Supabase connected! Economy loaded."));
        }
        else
        {
            bConnected = false;
            UE_LOG(LogTemp, Warning, TEXT("Supabase connection failed, using defaults"));
            // Fallback defaults
            EconomyCache.Add("pistol_price", 200);
            EconomyCache.Add("rifle_price", 800);
            EconomyCache.Add("robbery_min", 50);
            EconomyCache.Add("robbery_max", 200);
            EconomyCache.Add("passive_income_rate", 2);
            EconomyCache.Add("vault_min", 500);
            EconomyCache.Add("vault_max", 1500);
        }
    });
    Request->ProcessRequest();
}

void UCKSupabaseComponent::HandleResponse(FString Response)
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
    TArray<TSharedPtr<FJsonValue>> Array;
    if (FJsonSerializer::Deserialize(Reader, Array))
    {
        for (auto& Val : Array)
        {
            TSharedPtr<FJsonObject> Obj = Val->AsObject();
            if (!Obj) continue;
            FString Key = Obj->GetStringField("key");
            int32 Value = Obj->GetIntegerField("value");
            EconomyCache.Add(Key, Value);
        }
        UE_LOG(LogTemp, Warning, TEXT("Loaded %d economy values from Supabase"), EconomyCache.Num());
    }
}

int32 UCKSupabaseComponent::GetEconomyValue(FString Key)
{
    if (EconomyCache.Contains(Key))
        return EconomyCache[Key];
    return 0;
}

void UCKSupabaseComponent::SavePlayerState(int32 Money, int32 Wanted, const TArray<FString>& OwnedMissions)
{
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest> Request = Http->CreateRequest();
    Request->SetURL(SupabaseURL + "/rest/v1/player_state?player_id=eq.default");
    Request->SetVerb("PATCH");
    Request->SetHeader("apikey", AnonKey);
    Request->SetHeader("Authorization", "Bearer " + AnonKey);
    Request->SetHeader("Content-Type", "application/json");

    TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
    JsonObj->SetNumberField("money", Money);
    JsonObj->SetNumberField("wanted_level", Wanted);
    JsonObj->SetStringField("updated_at", FDateTime::Now().ToString());
    FString Body;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
    Request->SetContentAsString(Body);
    Request->ProcessRequest();
}