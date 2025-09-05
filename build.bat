@echo off
setlocal enabledelayedexpansion

echo ========================================
echo  Multibind X-Plane Plugin Build Script
echo ========================================
echo.

REM Check if we're in the right directory
if not exist "src\multibind.cpp" (
    echo ERROR: This script must be run from the project root directory
    echo Current directory: %CD%
    pause
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build

REM Check for X-Plane SDK
set SDK_FOUND=0
if exist "SDK\CHeaders\XPLM\XPLMPlugin.h" (
    echo ✓ X-Plane SDK found in SDK directory
    set SDK_FOUND=1
) else if defined XPLANE_SDK_PATH (
    if exist "%XPLANE_SDK_PATH%\CHeaders\XPLM\XPLMPlugin.h" (
        echo ✓ X-Plane SDK found at XPLANE_SDK_PATH: %XPLANE_SDK_PATH%
        set SDK_FOUND=1
    )
)

if !SDK_FOUND! == 0 (
    echo.
    echo ❌ X-Plane SDK not found!
    echo.
    echo Please do one of the following:
    echo   1. Download SDK from https://developer.x-plane.com/sdk/plugin-sdk-downloads/
    echo   2. Extract it to the 'SDK' folder in this project directory
    echo   3. OR set XPLANE_SDK_PATH environment variable to SDK location
    echo.
    echo Expected file: SDK\CHeaders\XPLM\XPLMPlugin.h
    echo.
    pause
    exit /b 1
)

REM Detect Visual Studio version
set VS_GENERATOR=""
set VS_YEAR=""

REM Try VS 2022 first (most common)
where /q devenv 2>nul
if !errorlevel! == 0 (
    for /f "tokens=*" %%i in ('where devenv 2^>nul') do (
        if "!VS_GENERATOR!" == """" (
            echo %%i | findstr /i "2022" >nul
            if !errorlevel! == 0 (
                set VS_GENERATOR="Visual Studio 17 2022"
                set VS_YEAR="2022"
                echo ✓ Found Visual Studio 2022
            )
        )
    )
)

REM Try VS 2019 if 2022 not found
if !VS_GENERATOR! == """" (
    for /f "tokens=*" %%i in ('where devenv 2^>nul') do (
        echo %%i | findstr /i "2019" >nul
        if !errorlevel! == 0 (
            set VS_GENERATOR="Visual Studio 16 2019"
            set VS_YEAR="2019"
            echo ✓ Found Visual Studio 2019
        )
    )
)

REM Fallback: Let CMake auto-detect
if !VS_GENERATOR! == """" (
    echo ? Visual Studio version not detected, letting CMake auto-detect...
    set VS_GENERATOR=""
    set CMAKE_ARCH_FLAG=-A x64
) else (
    set CMAKE_ARCH_FLAG=-A x64
)

echo.
echo Building Multibind Plugin...
echo.

cd build

REM Configure with CMake
if !VS_GENERATOR! == """" (
    echo Configuring with auto-detected generator...
    cmake .. !CMAKE_ARCH_FLAG! -DCMAKE_BUILD_TYPE=Release
) else (
    echo Configuring with !VS_GENERATOR!...
    cmake .. -G !VS_GENERATOR! !CMAKE_ARCH_FLAG! -DCMAKE_BUILD_TYPE=Release
)

if !errorlevel! neq 0 (
    echo.
    echo ❌ CMake configuration failed!
    echo.
    pause
    cd ..
    exit /b 1
)

REM Build the project
echo.
echo Building plugin...
cmake --build . --config Release

if !errorlevel! neq 0 (
    echo.
    echo ❌ Build failed!
    echo.
    pause
    cd ..
    exit /b 1
)

REM Create the plugin directory structure
echo.
echo Creating plugin directory structure...
cmake --build . --target plugin

if !errorlevel! neq 0 (
    echo.
    echo ❌ Plugin structure creation failed!
    echo.
    pause
    cd ..
    exit /b 1
)

cd ..

REM Check if build was successful
if exist "build\Multibind\win.xpl" (
    echo.
    echo ========================================
    echo ✅ BUILD SUCCESSFUL!
    echo ========================================
    echo.
    echo Plugin built: build\Multibind\win.xpl
    echo.
    echo Installation:
    echo 1. Copy the entire 'Multibind' folder to:
    echo    X-Plane 12\Resources\plugins\
    echo.
    echo 2. Final path should be:
    echo    X-Plane 12\Resources\plugins\Multibind\win.xpl
    echo.
    echo The plugin is now ready for installation!
    echo.
    echo Press any key to open the plugin directory...
    pause >nul
    explorer build\Multibind
) else (
    echo.
    echo ❌ Build completed but plugin file not found!
    echo Expected: build\Multibind\win.xpl
    echo.
    pause
    exit /b 1
)

endlocal