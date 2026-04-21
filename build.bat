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

REM Detect available compilers
set COMPILER_FOUND=0
set BUILD_GENERATOR=""
set CMAKE_EXTRA_FLAGS=""

echo Detecting available compilers...

REM Use vswhere to detect Visual Studio installations (IDE and Build Tools)
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist %VSWHERE% (
    REM Try VS 2022 first
    for /f "tokens=*" %%i in ('%VSWHERE% -version "[17.0,18.0)" -latest -property installationPath 2^>nul') do (
        if !COMPILER_FOUND! == 0 (
            set BUILD_GENERATOR="Visual Studio 17 2022"
            set CMAKE_EXTRA_FLAGS=-A x64
            set COMPILER_FOUND=1
            echo ✅ Found Visual Studio 2022 at %%i
        )
    )
    REM Try VS 2019
    if !COMPILER_FOUND! == 0 (
        for /f "tokens=*" %%i in ('%VSWHERE% -version "[16.0,17.0)" -latest -property installationPath 2^>nul') do (
            if !COMPILER_FOUND! == 0 (
                set BUILD_GENERATOR="Visual Studio 16 2019"
                set CMAKE_EXTRA_FLAGS=-A x64
                set COMPILER_FOUND=1
                echo ✅ Found Visual Studio 2019 at %%i
            )
        )
    )
)

REM 4. Try MinGW-w64 as fallback
if !COMPILER_FOUND! == 0 (
    where /q gcc 2>nul
    if !errorlevel! == 0 (
        where /q mingw32-make 2>nul
        if !errorlevel! == 0 (
            echo ✅ Found MinGW-w64 (GCC compiler)
            set BUILD_GENERATOR="MinGW Makefiles"
            set CMAKE_EXTRA_FLAGS=-DCMAKE_MAKE_PROGRAM=mingw32-make
            set COMPILER_FOUND=1
        ) else (
            where /q make 2>nul
            if !errorlevel! == 0 (
                echo ✅ Found MinGW-w64 (GCC compiler)
                set BUILD_GENERATOR="MinGW Makefiles"
                set CMAKE_EXTRA_FLAGS=""
                set COMPILER_FOUND=1
            )
        )
    )
)

REM Handle case where no compiler was found
if !COMPILER_FOUND! == 0 (
    echo.
    echo ❌ No suitable C++ compiler found!
    echo.
    echo You have Visual Studio Code, but you need a C++ compiler to build this project.
    echo Visual Studio Code is a code editor, not a compiler.
    echo.
    echo Please install ONE of the following:
    echo.
    echo 1. Visual Studio 2019/2022 Community ^(free^):
    echo    https://visualstudio.microsoft.com/downloads/
    echo    - Install "Desktop development with C++" workload
    echo.
    echo 2. Build Tools for Visual Studio ^(smaller download^):
    echo    https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
    echo    - Install "C++ build tools" workload
    echo.
    echo 3. MinGW-w64 via winget ^(command line^):
    echo    winget install mingw-w64
    echo.
    echo 4. MinGW-w64 via MSYS2 ^(manual^):
    echo    https://www.msys2.org/
    echo.
    pause
    exit /b 1
)

echo.
echo Building Multibind Plugin...
echo.

cd build

REM Configure with CMake using detected compiler
if "!BUILD_GENERATOR!" == """" (
    echo Configuring with auto-detected generator...
    cmake .. !CMAKE_EXTRA_FLAGS! -DCMAKE_BUILD_TYPE=Release
) else (
    echo Configuring with !BUILD_GENERATOR!...
    cmake .. -G !BUILD_GENERATOR! !CMAKE_EXTRA_FLAGS! -DCMAKE_BUILD_TYPE=Release
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
if "!BUILD_GENERATOR!" == """MinGW Makefiles""" (
    REM MinGW uses make instead of cmake --build
    if "!CMAKE_EXTRA_FLAGS!" == "-DCMAKE_MAKE_PROGRAM=mingw32-make" (
        mingw32-make
    ) else (
        make
    )
) else (
    cmake --build . --config Release
)

if !errorlevel! neq 0 (
    echo.
    echo ❌ Build failed!
    echo.
    echo Troubleshooting tips:
    if "!BUILD_GENERATOR!" == """MinGW Makefiles""" (
        echo - Make sure MinGW-w64 is properly installed and in PATH
        echo - Try installing via: winget install mingw-w64
        echo - Or reinstall via MSYS2: https://www.msys2.org/
    ) else (
        echo - Make sure Visual Studio C++ tools are properly installed
        echo - Try running from "Developer Command Prompt for VS"
        echo - Reinstall Visual Studio with "Desktop development with C++" workload
    )
    echo.
    pause
    cd ..
    exit /b 1
)

REM Create the plugin directory structure
echo.
echo Creating plugin directory structure...
if "!BUILD_GENERATOR!" == """MinGW Makefiles""" (
    if "!CMAKE_EXTRA_FLAGS!" == "-DCMAKE_MAKE_PROGRAM=mingw32-make" (
        mingw32-make plugin
    ) else (
        make plugin
    )
) else (
    cmake --build . --target plugin
)

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
    if "!BUILD_GENERATOR!" == """MinGW Makefiles""" (
        echo Compiled with: MinGW-w64 ^(GCC^)
    ) else if "!BUILD_GENERATOR!" == """" (
        echo Compiled with: Visual Studio Build Tools ^(MSVC^)
    ) else (
        echo Compiled with: !BUILD_GENERATOR! ^(MSVC^)
    )
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
    echo This might happen if:
    if "!BUILD_GENERATOR!" == """MinGW Makefiles""" (
        echo - MinGW-w64 installation is incomplete
        echo - PATH environment variable doesn't include MinGW-w64
        echo - Try reinstalling MinGW-w64: winget install mingw-w64
    ) else (
        echo - Visual Studio C++ tools are not properly configured
        echo - Try running from "Developer Command Prompt for VS"
        echo - Reinstall Visual Studio with C++ development tools
    )
    echo.
    pause
    exit /b 1
)

endlocal