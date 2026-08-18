param(
  [Parameter(Mandatory=$true)][string]$ObsPrefix,
  [string]$Configuration = "RelWithDebInfo"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Build = Join-Path $Root "build_x64"
$Release = Join-Path $Root "release\WorshipGraphics"

cmake -S $Root -B $Build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="$ObsPrefix" `
  -DCMAKE_INSTALL_PREFIX="$Release"
cmake --build $Build --config $Configuration
cmake --install $Build --config $Configuration

Write-Host "Build completed: $Release"
