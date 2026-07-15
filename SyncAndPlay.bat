@echo off
REM Concrete Kingdom - One-Click Sync (Batch version)
REM Save as SyncAndPlay.bat on your desktop

echo === Concrete Kingdom Sync ===
echo.

echo [1/3] Pulling latest code...
git pull
if %ERRORLEVEL% NEQ 0 (
    echo Git pull failed!
    pause
    exit /b 1
)

echo [2/3] Checking for new assets...
if exist Content\Audio\*.mp3 echo   Audio assets found - UE5 will import on launch
if exist Content\Models\*.gltf echo   3D models found - UE5 will import on launch

echo [3/3] Opening project...
for %%f in (*.uproject) do start "" "%%f"

echo.
echo === Done! Press Play in UE5 ===
pause
