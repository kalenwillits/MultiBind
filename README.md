# Multibind - XPlane-12 Multi-Button Command Plugin

[![Build Status](https://github.com/yourusername/multibind/workflows/Build%20Multibind%20Plugin/badge.svg)](https://github.com/yourusername/multibind/actions)
[![Release](https://img.shields.io/github/v/release/yourusername/multibind)](https://github.com/yourusername/multibind/releases)
[![License](https://img.shields.io/github/license/yourusername/multibind)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)](https://github.com/yourusername/multibind/releases)

An X-Plane 12 plugin that enables multi-button joystick command binding. Create complex button combinations on your joystick/HOTAS and map them to any X-Plane command.

## Features

- **1000 Custom Commands**: Creates `multibind/000` through `multibind/999` that can be assigned to joystick buttons in X-Plane's settings
- **Multi-Button Combinations**: Press multiple multibind buttons simultaneously to trigger X-Plane commands
- **Aircraft-Specific Profiles**: Automatically loads/saves configurations per aircraft
- **Simple Configuration UI**: Easy-to-use plugin window for creating and managing bindings
- **Cross-Platform**: Supports Windows, Mac, and Linux

## How It Works

1. **Setup Phase**: Plugin creates 1000 custom commands that appear in X-Plane's joystick settings
2. **Assignment Phase**: You assign these `multibind/XXX` commands to your joystick buttons using X-Plane's standard joystick configuration
3. **Combination Phase**: Define combinations in the plugin UI (e.g., "buttons 5+10+15 = start engine 1")
4. **Execution Phase**: When you press the button combination, the plugin triggers the target X-Plane command

## 🚀 Quick Start

**Want to build immediately?** See [QUICKSTART.md](QUICKSTART.md) for the 2-step build process!

## Download

### Pre-built Releases
Download the latest pre-built plugin from the [Releases page](https://github.com/yourusername/multibind/releases):

- **Windows**: `multibind-windows.zip` 
- **macOS**: `multibind-macos.zip` (Universal binary: Intel + Apple Silicon)
- **Linux**: `multibind-linux.zip`

### Installation from Release
1. Download the appropriate package for your platform
2. Extract the zip file  
3. Copy the `Multibind` folder to `X-Plane 12/Resources/plugins/`
4. Restart X-Plane

## Building from Source

### ⚡ Simple Method (Recommended)

**Windows:**
```cmd
setup-sdk.bat
build.bat
```

**Linux/Mac:**
```bash
./setup-sdk.sh
./build.sh
```

### 🔧 Manual Method

### Prerequisites
- **X-Plane 12**
- **X-Plane SDK** (for compilation) - See download instructions below
- **CMake 3.16+**
- **C++17 compatible compiler**:
  - **Windows**: Visual Studio 2019/2022 with C++ development tools, or MinGW-w64
  - **macOS**: Xcode Command Line Tools or full Xcode
  - **Linux**: GCC 7+ or Clang 5+

### Download X-Plane SDK
1. Download the X-Plane SDK from [developer.x-plane.com](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
2. Extract it to the project directory as `SDK/` or set the `XPLANE_SDK_PATH` environment variable

### Compilation

#### Windows

**Option 1: Visual Studio (Recommended)**
```cmd
mkdir build
cd build
# For Visual Studio 2022
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# For Visual Studio 2019
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release

# Or let CMake auto-detect your Visual Studio version
cmake .. -A x64
cmake --build . --config Release
```

**Option 2: MinGW-w64**
```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

#### Mac
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

#### Linux
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Plugin Installation
1. Copy the compiled plugin file to your X-Plane plugins directory:
   - **Windows**: `X-Plane 12/Resources/plugins/Multibind/win_x64/multibind.xpl`
   - **Mac**: `X-Plane 12/Resources/plugins/Multibind/mac_x64/multibind.xpl`
   - **Linux**: `X-Plane 12/Resources/plugins/Multibind/lin_x64/multibind.xpl`

## Usage

### Initial Setup
1. Start X-Plane 12 and load any aircraft
2. Go to **Plugins → Multibind** to open the configuration window
3. The plugin automatically creates a `multibind/` directory in your X-Plane folder

### Assigning Joystick Buttons
1. Open X-Plane's **Settings → Joystick & Equipment**
2. Go to the **Buttons: Basic** tab
3. Press your desired joystick button
4. In the command dropdown, find and assign a `multibind/XXX` command (e.g., `multibind/001`)
5. Repeat for all buttons you want to use in combinations

### Creating Combinations
1. Open the Multibind plugin window (**Plugins → Multibind**)
2. Click **"Record"** to start recording a combination
3. Press the joystick buttons you want in the combination (they should be assigned to multibind commands)
4. The plugin will show which buttons are currently pressed
5. Enter the target X-Plane command (e.g., `sim/starters/engage_starter_1`)
6. Add a description (optional, e.g., "Start Engine 1")
7. Click **"Save"** to save the combination
8. Click **"Record"** again to stop recording mode

### Example Combinations
- **Engine Start**: `multibind/001 + multibind/005` → `sim/starters/engage_starter_1`
- **Gear + Brakes**: `multibind/010 + multibind/020` → `sim/flight_controls/landing_gear_toggle`
- **Emergency**: `multibind/050 + multibind/060 + multibind/070` → `sim/operation/pause_toggle`

## Configuration Files

The plugin stores configurations in simple text files:
- **Location**: `X-Plane 12/multibind/{aircraft-name}.txt`
- **Format**: `button1+button2+button3|command|description`

Example file content:
```
# Multibind configuration for Baron_58
# Format: button1+button2+button3|command|description
# Example: 1+5+10|sim/starters/engage_starter_1|Start Engine 1

1+5|sim/starters/engage_starter_1|Start Engine 1
2+6|sim/starters/engage_starter_2|Start Engine 2
10+20+30|sim/operation/pause_toggle|Emergency Pause
```

## Troubleshooting

### Compilation Issues

#### Windows-Specific Issues
- **"Visual Studio not found"**: Install Visual Studio 2019/2022 with "Desktop development with C++" workload
- **CMake not found**: Install CMake via Visual Studio Installer or from [cmake.org](https://cmake.org/download/)
- **"SDK not found"**: Ensure the X-Plane SDK is extracted to the `SDK/` directory in your project folder
- **Path issues**: Use forward slashes or double backslashes in paths: `C:/path/to/sdk` or `C:\\path\\to\\sdk`
- **Antivirus interference**: Add your build directory to antivirus exclusions
- **"LNK2019 unresolved external symbol"**: Make sure you're building for x64 architecture with `-A x64`

#### General Issues
- **CMake version error**: Ensure CMake 3.16+ is installed
- **C++17 compiler error**: Update your compiler to support C++17 standard
- **SDK path issues**: Set `XPLANE_SDK_PATH` environment variable if SDK is not in project directory

### Plugin Runtime Issues

#### Plugin Not Loading
- Check X-Plane's **Log.txt** for error messages
- Ensure the plugin is in the correct directory structure
- Verify you have the correct architecture (64-bit)
- **Windows**: Make sure Visual C++ Redistributable is installed

#### Commands Not Working
- Ensure you've assigned multibind commands to your joystick buttons in X-Plane's settings
- Check that the target X-Plane command exists (use X-Plane's command search)
- Look for debug messages in X-Plane's developer console

#### Combinations Not Triggering
- Verify all buttons in the combination are assigned to multibind commands
- Check the timing - all buttons must be pressed within a short time window
- Ensure button combinations don't conflict with existing X-Plane assignments

## Continuous Integration

This project uses GitHub Actions for automated building and releasing:

### Build Workflow
- **Triggers**: Push to main/develop, pull requests, manual dispatch
- **Platforms**: Windows (VS2022), macOS (Xcode), Linux (GCC)
- **Artifacts**: Build outputs stored for 30 days
- **SDK Caching**: X-Plane SDK cached to speed up builds

### Release Workflow  
- **Triggers**: Git tags (`v*.*.*`) or manual dispatch
- **Automatic Releases**: Creates GitHub releases with downloadable binaries
- **Cross-Platform Packages**: Windows, macOS, and Linux builds packaged automatically

To trigger a release:
```bash
git tag v1.0.0
git push origin v1.0.0
```

## Development

### Code Structure
```
src/
├── multibind.cpp          # Main plugin entry point and lifecycle
├── config.h/cpp           # Configuration file management
├── combination_tracker.h/cpp  # Multi-button combination detection
└── ui.h/cpp              # Plugin window interface
```

### Key Classes
- **Config**: Manages loading/saving aircraft-specific binding configurations
- **CombinationTracker**: Detects button press combinations and triggers commands
- **UI**: Provides the plugin configuration window interface

### Building for Development
Use `CMAKE_BUILD_TYPE=Debug` for development builds with additional logging:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## License

This project is provided as-is for educational and personal use. See the code for implementation details.

## Support

For issues and questions:
1. Check the troubleshooting section above
2. Review X-Plane's Log.txt for error messages
3. Ensure you're using a supported X-Plane 12 version