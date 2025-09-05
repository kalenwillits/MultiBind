# Multibind - Quick Start Guide

**Build an X-Plane 12 multi-button joystick plugin in 2 simple steps!**

## 🚀 Super Quick Build

### Windows
```batch
setup-sdk.bat
build.bat
```

### Linux/Mac
```bash
./setup-sdk.sh
./build.sh
```

That's it! The plugin will be built and ready to install.

---

## What is Multibind?

An X-Plane 12 plugin that creates 1000 custom commands (`multibind/000` through `multibind/999`) so you can:
- **Create complex button combinations** on your joystick/HOTAS
- **Map them to any X-Plane command** (engine start, gear, autopilot, etc.)
- **Save different profiles** for each aircraft automatically

## Features
- ✅ **1000 Custom Commands** ready to assign in X-Plane
- ✅ **Multi-Button Combinations** - press multiple buttons simultaneously
- ✅ **Aircraft-Specific Profiles** - automatically loads/saves per aircraft
- ✅ **Simple UI** - easy configuration window
- ✅ **Cross-Platform** - Windows, Mac, Linux

---

## Installation

After building:

1. **Copy** the generated `build/Multibind/` folder to X-Plane:
   ```
   X-Plane 12/Resources/plugins/Multibind/
   ```

2. **Final plugin paths will be:**
   ```
   X-Plane 12/Resources/plugins/Multibind/
   ├── win.xpl     (Windows)
   ├── mac.xpl     (macOS) 
   └── lin.xpl     (Linux)
   ```

3. **Restart X-Plane**

**Note**: The build process now creates the correct folder structure automatically!

---

## Quick Usage

### 1. Assign Joystick Buttons
- Open X-Plane **Settings → Joystick & Equipment**
- Go to **Buttons: Basic** tab
- Press your joystick button
- Assign it to a `multibind/XXX` command (e.g., `multibind/001`)

### 2. Create Button Combinations
- Open **Plugins → Multibind** in X-Plane
- Click **"Record"**
- Press your button combination (e.g., buttons assigned to `multibind/001` + `multibind/005`)
- Enter the target command (e.g., `sim/starters/engage_starter_1`)
- Click **"Save"**

### 3. Done!
Now when you press that button combination, the engine starter engages!

---

## Example Combinations

- **Engine Start**: `multibind/001` + `multibind/005` → `sim/starters/engage_starter_1`
- **Landing Gear**: `multibind/010` + `multibind/020` → `sim/flight_controls/landing_gear_toggle`  
- **Emergency Pause**: `multibind/050` + `multibind/060` + `multibind/070` → `sim/operation/pause_toggle`

---

## Troubleshooting

### Build Issues
- **SDK not found**: Run the setup script first (`setup-sdk.bat` or `./setup-sdk.sh`)
- **Build fails**: Make sure you have Visual Studio (Windows) or development tools installed
- **Permission errors**: Run as administrator (Windows) or check file permissions (Linux/Mac)

### Plugin Issues
- **Plugin not loading**: Check X-Plane's `Log.txt` for error messages
- **Combinations not working**: Ensure all buttons are assigned to multibind commands in X-Plane settings
- **Missing commands**: Verify target X-Plane commands exist (use X-Plane's command search)

---

## Requirements

- **X-Plane 12**
- **Windows**: Visual Studio 2019/2022 or MinGW-w64
- **macOS**: Xcode Command Line Tools
- **Linux**: GCC 7+ or Clang 5+
- **CMake 3.16+** (auto-installed by build scripts)

## Project Structure

- `src/` - Source code (4 files, ~1000 lines)
- `build.bat` / `build.sh` - Automated build scripts
- `setup-sdk.bat` / `setup-sdk.sh` - Automatic SDK download/setup
- `CMakeLists.txt` - Enhanced build configuration

---

**Need the full documentation?** See [README.md](README.md) for detailed information.