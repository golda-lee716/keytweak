#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Build script for KeyTweak keyboard remapping application

.DESCRIPTION
    Automatically configures and builds the KeyTweak application with CMake.
    Supports multiple generators (Visual Studio, MinGW, Ninja, etc.)

.PARAMETER Generator
    CMake generator to use. Default: "Visual Studio 17 2022"
    Other options: "MinGW Makefiles", "Ninja", "Unix Makefiles"

.PARAMETER Config
    Build configuration. Default: "Release"
    Other options: "Debug"

.PARAMETER Clean
    Clean build directory before building

.PARAMETER Help
    Show this help message

.EXAMPLE
    .\build.ps1                          # Build with default settings
    .\build.ps1 -Clean                   # Clean and rebuild
    .\build.ps1 -Generator "MinGW Makefiles"  # Use MinGW
    .\build.ps1 -Config Debug            # Debug build
#>

param(
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Config = "Release",
    [switch]$Clean,
    [switch]$Help
)

if ($Help) {
    Get-Help $PSCommandPath -Detailed
    exit 0
}

$ErrorActionPreference = "Stop"
$BuildDir = "build"
$SourceDir = "."

Write-Host "KeyTweak Build Script" -ForegroundColor Cyan
Write-Host "=====================" -ForegroundColor Cyan
Write-Host ""

# Check if CMake is installed
Write-Host "Checking for CMake..." -ForegroundColor Yellow
$cmakePath = (Get-Command cmake -ErrorAction SilentlyContinue)
if (-not $cmakePath) {
    Write-Host "ERROR: CMake not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install CMake from: https://cmake.org/download/" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "After installation, add CMake to your PATH or specify its full path." -ForegroundColor Yellow
    exit 1
}

$cmakeVersion = & cmake --version | Select-Object -First 1
Write-Host "✓ Found: $cmakeVersion" -ForegroundColor Green

Write-Host ""
Write-Host "Build Configuration:" -ForegroundColor Cyan
Write-Host "  Generator: $Generator"
Write-Host "  Config:    $Config"
Write-Host "  Source:    $SourceDir"
Write-Host "  Build Dir: $BuildDir"
Write-Host ""

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }
}

# Create build directory if it doesn't exist
if (-not (Test-Path $BuildDir)) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configure
Write-Host ""
Write-Host "Configuring project with CMake..." -ForegroundColor Cyan
$configCmd = "cmake -S `"$SourceDir`" -B `"$BuildDir`" -G `"$Generator`""
Write-Host "  Command: $configCmd" -ForegroundColor Gray
Write-Host ""

& cmake -S $SourceDir -B $BuildDir -G $Generator
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host ""
Write-Host "Building project..." -ForegroundColor Cyan
$buildCmd = "cmake --build `"$BuildDir`" --config $Config"
Write-Host "  Command: $buildCmd" -ForegroundColor Gray
Write-Host ""

& cmake --build $BuildDir --config $Config
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    exit 1
}

# Success
Write-Host ""
Write-Host "✓ Build completed successfully!" -ForegroundColor Green
Write-Host ""

# Find executable
$exePath = Get-ChildItem -Path $BuildDir -Name "keytweak.exe" -Recurse | Select-Object -First 1
if ($exePath) {
    $exeFullPath = Join-Path $BuildDir $exePath
    Write-Host "Executable location:" -ForegroundColor Cyan
    Write-Host "  $exeFullPath" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "To run the application:" -ForegroundColor Cyan
    Write-Host "  .\$exeFullPath" -ForegroundColor Yellow
} else {
    Write-Host "Warning: Could not locate keytweak.exe" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
