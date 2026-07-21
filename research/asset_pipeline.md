# Asset Pipeline Guide

## Audio
Files in `Content/Audio/` (raw .mp3):
- `city_ambient.mp3` — background ambient loop
- `engine_hum.mp3` — vehicle engine loop
- `footstep.mp3` — player footstep sound
- `gunshot.mp3` — weapon fire
- `police_siren.mp3` — police chase siren

**Import in UE5:**
1. Open Content Browser → Audio folder
2. Right-click → Import to /Game/Audio/
3. Select all 5 .mp3 files
4. After import, open each SoundWave and check: Looping (for ambient + engine), Attenuation settings

**Wire to `CKGameAudioComponent`:**
- The component has UPROPERTY SoundWave pointers for SirenSound, GunshotSound, CashSound, FootstepSound
- In the Level Blueprint or GameMode, reference the component and assign each SoundWave from the Content Browser

## Models
Folders in `Content/Models/`:
- `low_poly_buildings/` — building meshes
- `passenger_car_pack/` — traffic car meshes
- `street_lamp/` — street light meshes

**Import:** Same process — Content Browser → Models folder → Import all.

**Expected asset paths after import:**
- `/Game/Audio/city_ambient`
- `/Game/Audio/engine_hum`
- `/Game/Audio/footstep`
- `/Game/Audio/gunshot`
- `/Game/Audio/police_siren`
- `/Game/Models/low_poly_buildings/...`
- `/Game/Models/passenger_car_pack/...`
- `/Game/Models/street_lamp/...`

## Textures
`Content/Textures/commercial_facade.jpg` — building texture.
Import to `/Game/Textures/commercial_facade`.
