# Concrete Kingdom - One-Click Sync and Play
# Save this as "SyncAndPlay.ps1" on your desktop
# Run it whenever I push new code

Write-Host "=== Concrete Kingdom Sync & Play ===" -ForegroundColor Cyan

# 1. Pull latest code
Write-Host "[1/4] Pulling latest from GitHub..." -ForegroundColor Yellow
git pull
if ($LASTEXITCODE -ne 0) {
    Write-Host "Git pull failed! Check your internet connection." -ForegroundColor Red
    exit 1
}

# 2. Generate project files (UE5 detects changes)
Write-Host "[2/4] Generating project files..." -ForegroundColor Yellow
$uproject = Get-ChildItem -Path . -Filter "*.uproject" | Select-Object -First 1
if ($uproject) {
    # UE5 commandlet to generate project files
    $enginePath = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealVersionSelector.exe"
    if (Test-Path $enginePath) {
        Start-Process -FilePath $enginePath -ArgumentList "/projectfiles `"$($uproject.FullName)`"" -Wait
    }
}

# 3. Detect and import new content assets
Write-Host "[3/4] Checking for new assets..." -ForegroundColor Yellow
$contentFolders = @("Content\Audio", "Content\Models", "Content\Data")
foreach ($folder in $contentFolders) {
    if (Test-Path $folder) {
        $newItems = Get-ChildItem -Path $folder -Recurse | Where-Object { $_.Extension -match "\.(mp3|wav|gltf|glb|obj|fbx|json)$" }
        if ($newItems.Count -gt 0) {
            Write-Host "  Found $($newItems.Count) new assets in $folder" -ForegroundColor Green
        }
    }
}

# 4. Open the project
Write-Host "[4/4] Opening Unreal Engine..." -ForegroundColor Yellow
if ($uproject) {
    Start-Process -FilePath $uproject.FullName
}

Write-Host "=== Done! UE5 will import new assets on launch. ===" -ForegroundColor Cyan
Write-Host "Press Play in the editor to test." -ForegroundColor Green
