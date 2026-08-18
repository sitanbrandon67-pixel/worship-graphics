param([string]$Configuration = "RelWithDebInfo")
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Build = Join-Path $Root "build_x64"
cmake --build $Build --config $Configuration --target package
Write-Host "Installer/package generated in $Build"
