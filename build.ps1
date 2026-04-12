# build.ps1 — Convenience script for MinGW/g++ without CMake
# Run from the project root: .\build.ps1

$root = Split-Path -Parent $MyInvocation.MyCommand.Definition
$include = Join-Path $root "include"
$output  = Join-Path $root "ams.exe"
$main    = Join-Path $root "main.cpp"

$sources = Get-ChildItem -Recurse -Filter "*.cpp" (Join-Path $root "src") |
           ForEach-Object { '"' + $_.FullName + '"' }

$allFiles = @('"' + $main + '"') + $sources

$cmd = "g++ -std=c++17 -Wall -Wextra -O2 -I`"$include`" -o `"$output`" $($allFiles -join ' ')"

Write-Host "`nBuilding Attendance Management System..." -ForegroundColor Cyan
Write-Host "Compiler: g++ (MinGW)"
Write-Host "Standard: C++17"
Write-Host ""

Invoke-Expression $cmd
if ($LASTEXITCODE -eq 0) {
    Write-Host "`n[BUILD SUCCESS] Binary: $output" -ForegroundColor Green
    Write-Host "Run with: .\ams.exe" -ForegroundColor Yellow
} else {
    Write-Host "`n[BUILD FAILED]" -ForegroundColor Red
    exit 1
}
