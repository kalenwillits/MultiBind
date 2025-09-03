# Multibind - XPlane-12 Multi-Button Command Plugin

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

## Installation

### Prerequisites
- X-Plane 12
- X-Plane SDK (for compilation)
- CMake 3.16+
- C++17 compatible compiler

### Download X-Plane SDK
1. Download the X-Plane SDK from [developer.x-plane.com](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
2. Extract it to the project directory as `SDK/` or set the `XPLANE_SDK_PATH` environment variable

### Compilation

#### Windows (Visual Studio)
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
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

### Plugin Not Loading
- Check X-Plane's **Log.txt** for error messages
- Ensure the plugin is in the correct directory structure
- Verify you have the correct architecture (64-bit)

### Commands Not Working
- Ensure you've assigned multibind commands to your joystick buttons in X-Plane's settings
- Check that the target X-Plane command exists (use X-Plane's command search)
- Look for debug messages in X-Plane's developer console

### Combinations Not Triggering
- Verify all buttons in the combination are assigned to multibind commands
- Check the timing - all buttons must be pressed within a short time window
- Ensure button combinations don't conflict with existing X-Plane assignments

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