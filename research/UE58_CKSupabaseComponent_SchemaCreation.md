---
id: CARD-024
version: 1
status: active
created: 2026-07-21
last_verified: 2026-07-21
confidence: proven
half_life: slow
tags: [ue5.8, supabase, schema, economy, player_state, http, rpc]
links: []
uses: 0
---

# CKSupabaseComponent — Schema Creation & Verification via Supabase RPC

## Problem

The current `CKSupabaseComponent` assumes the `economy` and `player_state` tables already exist in the Supabase project. On first run (or a fresh Supabase project), `FetchEconomy()` GET-requests `/rest/v1/economy` and gets a 404 or 400 error, causing the component to fall back to hardcoded default values. There is no mechanism to **create the tables if they don't exist** or **verify the schema**.

## Solution Architecture

Three-layer approach:

1. **Supabase side** (one-time setup in SQL Editor): Create a PostgreSQL function `create_concrete_kingdom_tables()` with `SECURITY DEFINER` so it runs with the owner's privileges (can create tables). Grant `EXECUTE` to the `anon` role so the game client can call it over HTTP.

2. **UE5 C++ side** (in `CKSupabaseComponent.cpp`): In `BeginPlay()`, before calling `FetchEconomy()`, call the new `VerifySchema()` method. This hits `POST /rest/v1/rpc/create_concrete_kingdom_tables` and then queries the `economy` table to confirm the schema exists.

3. **Verification**: After the RPC call, check if the `economy` table responds with a valid 200. If yes, proceed to `FetchEconomy()`. If the RPC itself fails, log the error and fall back to defaults.

## UE5 API Reference

### HTTP Module
- **Include**: `#include "Http.h"` (already present)
- **Module dependency**: `"HTTP"` in `ConcreteKingdom.Build.cs` (already present)
- **Key classes**:
  - `FHttpModule` — singleton via `FHttpModule::Get()`
  - `IHttpRequest` — interface via `FHttpModule::Get().CreateRequest()`
  - `IHttpResponse` — in the completion callback
  - `FHttpRequestPtr` / `FHttpResponsePtr` — shared pointers

### JSON Module
- **Include**: `#include "Json.h"` (already present)
- **Key classes**:
  - `FJsonObject` — for building JSON body
  - `FJsonSerializer` — for serializing to string
  - `TSharedRef<TJsonWriter<>>` / `TJsonWriterFactory`
  - `TSharedRef<TJsonReader<>>` / `TJsonReaderFactory` — for parsing response

### FDateTime
- **Include**: `#include "Misc/DateTime.h"` (already available via Core)
- `FDateTime::Now().ToString()` — for timestamps

## Supabase SQL — Run Once in Dashboard

Execute this SQL in the Supabase SQL Editor (project → SQL Editor → New Query):

```sql
-- Create both tables if they don't exist
CREATE OR REPLACE FUNCTION public.create_concrete_kingdom_tables()
RETURNS void
LANGUAGE plpgsql
SECURITY DEFINER
SET search_path = public
AS $$
BEGIN
    -- economy table: key-value store for game balance tuning
    CREATE TABLE IF NOT EXISTS public.economy (
        id SERIAL PRIMARY KEY,
        key TEXT NOT NULL UNIQUE,
        value INTEGER NOT NULL DEFAULT 0,
        category TEXT DEFAULT ''
    );

    -- player_state table: single-row player persistence
    CREATE TABLE IF NOT EXISTS public.player_state (
        id SERIAL PRIMARY KEY,
        player_id TEXT NOT NULL DEFAULT 'default' UNIQUE,
        money INTEGER NOT NULL DEFAULT 0,
        wanted_level INTEGER NOT NULL DEFAULT 0,
        updated_at TIMESTAMPTZ DEFAULT NOW()
    );

    -- Insert default economy values if table was just created
    INSERT INTO public.economy (key, value, category)
    SELECT * FROM (VALUES
        ('pistol_price', 200, 'weapons'),
        ('rifle_price', 800, 'weapons'),
        ('robbery_min', 50, 'crimes'),
        ('robbery_max', 200, 'crimes'),
        ('passive_income_rate', 2, 'income'),
        ('vault_min', 500, 'heists'),
        ('vault_max', 1500, 'heists')
    ) AS defaults(k, v, c)
    WHERE NOT EXISTS (
        SELECT 1 FROM public.economy WHERE economy.key = defaults.k
    );

    -- Insert default player state row if not exists
    INSERT INTO public.player_state (player_id, money, wanted_level, updated_at)
    SELECT 'default', 0, 0, NOW()
    WHERE NOT EXISTS (
        SELECT 1 FROM public.player_state WHERE player_id = 'default'
    );
END;
$$;

-- Grant EXECUTE to the anon role so the game client can call it
GRANT EXECUTE ON FUNCTION public.create_concrete_kingdom_tables() TO anon;
```

**What this does**:
- Creates `economy` table with columns `id`, `key` (unique), `value`, `category`
- Creates `player_state` table with columns `id`, `player_id` (unique), `money`, `wanted_level`, `updated_at`
- Seeds default economy values (matching the existing hardcoded fallback in `FetchEconomy`)
- Seeds a default player state row
- Grants `EXECUTE` to `anon` so the REST API call works with the anon key

## UE5 C++ Code Changes

### File: `CKSupabaseComponent.h`

**Add after line 19** (after constructor declaration):

```cpp
// Schema creation / verification
UFUNCTION(BlueprintCallable, Category = "Supabase")
void VerifySchema();
```

**Add after line 44** (before closing brace of class):

```cpp
void OnSchemaResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
```

### File: `CKSupabaseComponent.cpp`

**Change `BeginPlay()` (lines 15–19)** — insert schema verification before `FetchEconomy`:

```cpp
void UCKSupabaseComponent::BeginPlay()
{
    Super::BeginPlay();
    VerifySchema();  // was: FetchEconomy();
}
```

**Add new method after `BeginPlay()`** (insert after line 19, before `FetchEconomy`):

```cpp
void UCKSupabaseComponent::VerifySchema()
{
    if (!GetWorld()) return;

    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest> Request = Http->CreateRequest();

    // Call the RPC function we created in Supabase
    Request->SetURL(SupabaseURL + TEXT("/rest/v1/rpc/create_concrete_kingdom_tables"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("apikey"), AnonKey);
    Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + AnonKey);
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Empty JSON body for the RPC (no params needed)
    Request->SetContentAsString(TEXT("{}"));

    Request->OnProcessRequestComplete().BindUObject(this, &UCKSupabaseComponent::OnSchemaResponse);
    Request->ProcessRequest();
}
```

**Add new callback method** (insert anywhere, e.g. after `VerifySchema`):

```cpp
void UCKSupabaseComponent::OnSchemaResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        int32 ResponseCode = Response->GetResponseCode();
        if (ResponseCode == 200 || ResponseCode == 201 || ResponseCode == 204)
        {
            UE_LOG(LogTemp, Warning, TEXT("Supabase schema verified. Proceeding to load economy."));
            // Now fetch the actual economy data
            FetchEconomy();
            return;
        }
    }

    // Schema verification failed — log and fall back to defaults
    UE_LOG(LogTemp, Warning, TEXT("Supabase schema RPC failed (code: %d). Using fallback defaults."),
        Response.IsValid() ? Response->GetResponseCode() : -1);
    bConnected = false;
    EconomyCache.Empty();
    EconomyCache.Add(TEXT("pistol_price"), 200);
    EconomyCache.Add(TEXT("rifle_price"), 800);
    EconomyCache.Add(TEXT("robbery_min"), 50);
    EconomyCache.Add(TEXT("robbery_max"), 200);
    EconomyCache.Add(TEXT("passive_income_rate"), 2);
    EconomyCache.Add(TEXT("vault_min"), 500);
    EconomyCache.Add(TEXT("vault_max"), 1500);
}
```

**No changes needed to** `FetchEconomy()`, `HandleResponse()`, `GetEconomyValue()`, or `SavePlayerState()` — they already work correctly once the table exists.

## What Compile Errors to Watch For

| Error | Cause | Fix |
|---|---|---|
| `'OnSchemaResponse' is not a member of 'UCKSupabaseComponent'` | Forgot to add the method declaration in the `.h` file | Add `void OnSchemaResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);` to the header |
| `'BindUObject': no matching overloaded function found` | Signature mismatch in the lambda/bind | Ensure the callback signature is exactly `(FHttpRequestPtr, FHttpResponsePtr, bool)` — the third param is `bool bWasSuccessful` |
| `'IHttpRequest' does not provide a call operator` | Trying to call `Request()` instead of `Request->ProcessRequest()` | Use `Request->ProcessRequest()` |
| `'JsonUtilities' module not found` | Missing module dependency | Check `ConcreteKingdom.Build.cs` has `"JsonUtilities"` in `PublicDependencyModuleNames` (it already does) |
| `Error LNK2019: unresolved external symbol` | Missing `#include "Http.h"` or `"Json.h"` | Both are already included at the top of `CKSupabaseComponent.cpp` |
| `'BindUObject' requires a UObject-derived class` | Using `BindRaw` or `BindLambda` on a `UObject` | Use `BindUObject` for UObject-derived callbacks; `this` is a `UCKSupabaseComponent*` (UObject) so `BindUObject` is correct |
| `TEXT() macro required for FString` | Passing raw string literals to `FString` | Use `TEXT("...")` for all string literals passed to `FString` methods |

## How to Test

### Prerequisites
1. Create a fresh Supabase project (or use the existing one at `psjeegjoumdorbygxkfd.supabase.co`)
2. Run the SQL from the "Supabase SQL — Run Once in Dashboard" section above
3. Build the UE5 project with `Compile` in the editor

### Test Procedure

1. **First-run test (fresh project)**:
   - Delete (or comment out) any existing `economy` and `player_state` tables in the Supabase dashboard
   - Launch the game
   - **Expected**: `VerifySchema()` calls the RPC → tables are created → `FetchEconomy()` succeeds → `Server connected! Economy loaded.` log message
   - `bConnected` should be `true`
   - `EconomyCache` should contain all 7 default values

2. **Existing data test**:
   - Launch the game with existing tables and data
   - **Expected**: `VerifySchema()` calls the RPC → `CREATE TABLE IF NOT EXISTS` is a no-op → `FetchEconomy()` loads existing data
   - Existing economy values preserved (the `INSERT ... WHERE NOT EXISTS` prevents overwrites)

3. **Offline/network failure test**:
   - Disconnect network or use a bad URL
   - Launch the game
   - **Expected**: `OnSchemaResponse` fires with `bWasSuccessful == false` → fallback defaults loaded
   - `bConnected` should be `false`
   - `EconomyCache` should have the hardcoded defaults

4. **Verify Supabase SQL function**:
   ```sql
   -- Check the function exists
   SELECT * FROM pg_proc WHERE proname = 'create_concrete_kingdom_tables';
   
   -- Check tables exist
   SELECT table_name FROM information_schema.tables 
   WHERE table_schema = 'public' AND table_name IN ('economy', 'player_state');
   
   -- Check economy data
   SELECT * FROM public.economy;
   ```

### UE5 Log Verification
Look for these log lines in the Output Log:
- `LogTemp: Warning: Supabase schema verified. Proceeding to load economy.` — schema RPC succeeded
- `LogTemp: Warning: Supabase connected! Economy loaded.` — economy data loaded from server
- `LogTemp: Warning: Loaded 7 economy values from Supabase` — number of values loaded
- `LogTemp: Warning: Supabase schema RPC failed (code: XXX). Using fallback defaults.` — fallback

## Sequence Diagram

```
BeginPlay()
    |
    +-> VerifySchema()
    |       |
    |       +-> POST /rest/v1/rpc/create_concrete_kingdom_tables  (HTTP)
    |       |
    |       +-> OnSchemaResponse()
    |               |
    |               +-> [200 OK] -> FetchEconomy()
    |               |                 |
    |               |                 +-> GET /rest/v1/economy?select=key,value,category
    |               |                 |
    |               |                 +-> HandleResponse() -> fill EconomyCache
    |               |
    |               +-> [error]  -> fill fallback defaults, bConnected = false
    |
    +-> [gameplay continues with economy values available]
```

## Architecture Notes

- **Why RPC instead of direct SQL?** The Supabase REST API (PostgREST) does not allow raw SQL execution. The `SECURITY DEFINER` RPC pattern is the standard way to allow the anon key to perform privileged operations like table creation.
- **Why not use the service_role key?** Embedding the service_role key in the game client is a security risk — it has full database access. The anon key + RPC pattern limits what the client can do.
- **Idempotency**: `CREATE TABLE IF NOT EXISTS` and `INSERT ... WHERE NOT EXISTS` make the operation safe to call every time the game starts.
- **Fallback resilience**: If Supabase is unreachable (CI environment, offline, network issue), the game still works with hardcoded defaults.