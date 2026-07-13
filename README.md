# Concrete Kingdom — UE5

Open-world crime sandbox. Built in Unreal Engine **5.8**.

## Setup

1. **Install UE5.8** from Epic Games Launcher
2. **Clone this repo** to your PC
3. Right-click `ConcreteKingdom.uproject` → **Generate Visual Studio project files**
4. Open `ConcreteKingdom.sln` → **Build** (Ctrl+Shift+B)
5. Press **Play** in the editor

## Controls

| Key | Action |
|---|---|
| W/S | Move forward/backward |
| A/D | Strafe/Steer |
| Mouse | Look around |
| Space | Jump |
| E | Interact |
| Left Click | Shoot |
| F | Enter/Exit vehicle |

## C++ Classes

- `ACKCharacter` — Third-person player movement, camera, combat
- `ACKVehiclePawn` — Chaos Vehicle with throttle/steering/handbrake
- `ACKPlayerController` — EnhancedInput bindings to all actions
- `ACKGameMode` — Wanted level and money tracking
- `ACKPoliceAIController` — Patrol/chase AI using NavMesh
- `UCKHUDWidget` — Money, wanted, ammo display (UMG)

## Project Structure

```
ConcreteKingdom/
├── Config/               # DefaultInput.ini
├── Content/              # Blueprints, meshes, materials (add in-editor)
├── Source/
│   ├── ConcreteKingdom/  # All C++ classes
│   └── *.Target.cs       # Build targets
└── ConcreteKingdom.uproject
```

## Next Steps After Compiling

1. Set `ACKGameMode` as default game mode in Project Settings
2. Create Blueprints inheriting from each C++ class
3. Place player start, buildings, police spawners in the level
4. Create UMG widget for HUD
5. Create Input Mapping Context with the actions
