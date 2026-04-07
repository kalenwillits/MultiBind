# Multibind - X-Plane 12 Multi-Button Command Plugin

An X-Plane 12 plugin that enables multi-button joystick command binding. Create complex button combinations on your joystick/HOTAS and map them to any X-Plane command through simple configuration files.

## Features

- **1000 Custom Commands**: Creates `multibind/000` through `multibind/999` for use in X-Plane's joystick settings
- **Multi-Button Combinations**: Press multiple buttons simultaneously to trigger X-Plane commands
- **Aircraft-Specific Profiles**: Automatically loads/saves configurations per aircraft
- **Configuration File Based**: Simple text files for easy editing and sharing
- **Cross-Platform**: Supports Windows, Mac, and Linux

## How It Works

1. **Setup**: Plugin creates 1000 custom commands (`multibind/000` to `multibind/999`) that appear in X-Plane's joystick settings
2. **Assignment**: Assign these `multibind/XXX` commands to your joystick buttons using X-Plane's standard joystick configuration
3. **Configuration**: Define combinations in simple text files (e.g., `*005*010+015=sim/starters/engage_starter_1`)
4. **Execution**: When you press the button combination, the plugin triggers the target X-Plane command

## Installation

1. **Download** the appropriate package for your platform:
   - **Windows**: `multibind-windows.zip`
   - **macOS**: `multibind-macos.zip` (Universal binary)
   - **Linux**: `multibind-linux.zip`

2. **Extract** the downloaded file

3. **Install** by copying the `.xpl` file to:
   ```
   X-Plane 12/Resources/plugins/
   ```

4. **Restart** X-Plane

The final plugin path should be one of:
- Linux: `X-Plane 12/Resources/plugins/lin.xpl`
- macOS: `X-Plane 12/Resources/plugins/mac.xpl`
- Windows: `X-Plane 12/Resources/plugins/win.xpl`

## Usage

### Step 1: Assign Joystick Buttons

1. **Open** X-Plane's Settings → Joystick & Equipment
2. **Go to** the Buttons: Basic tab
3. **Press** your desired joystick button
4. **Find and assign** a `multibind/XXX` command (e.g., `multibind/001`)
5. **Repeat** for all buttons you want to use in combinations

**Example Button Assignments:**
- Hat Switch Up → `multibind/010`
- Hat Switch Down → `multibind/011`  
- Trigger → `multibind/001`
- Button 1 → `multibind/005`
- Button 2 → `multibind/006`

### Step 2: Create Configuration Files

The plugin automatically creates a `multibind/` directory in your X-Plane folder. Configuration files are stored here with the aircraft ICAO code as the filename.

**Configuration File Location:**
```
X-Plane 12/multibind/{AIRCRAFT-ICAO}.txt
```

**File Format:**
```
<triggers>=<command>
```

Each trigger is a prefix character followed by a 3-digit zero-padded button ID:

| Prefix | Meaning | Description |
|--------|---------|-------------|
| `*` | HELD | Button must be held down |
| `+` | PRESSED | Button was just pressed (fires on press) |
| `-` | RELEASED | Button was just released (fires on release) |
| `~` | NOT HELD | Button must NOT be held (negation) |

**Example Configuration File** (`X-Plane 12/multibind/B58T.txt`):
```
# Multibind configuration for Baron 58
# Format: <triggers>=<command>

# Engine controls - hold button 001, then press 005 to start engine 1
*001+005=sim/starters/engage_starter_1
*002+006=sim/starters/engage_starter_2
*001*002+010=sim/engines/mixture_max

# Landing gear and flaps
*010+011=sim/flight_controls/landing_gear_toggle
*010+012=sim/flight_controls/flaps_down
*011+012=sim/flight_controls/flaps_up

# Emergency procedures - hold 001+005+010, press 015
*001*005*010+015=sim/operation/pause_toggle
```

### Step 3: Using Your Combinations

1. **Load your aircraft** in X-Plane
2. **Press the button combinations** you defined
3. **The corresponding X-Plane commands will execute**

**Example Usage:**
- Hold button assigned to `multibind/001`, then press `multibind/005` → Engine 1 starts
- Hold button assigned to `multibind/010`, then press `multibind/011` → Landing gear toggles

## Configuration File Examples

### Combat Aircraft (F/A-18)
```
# Weapons systems
*001+002=sim/weapons/master_arm_toggle
*001+003=sim/weapons/gun_trigger
*002+004=sim/weapons/missile_launch

# Emergency procedures
*001*002*003+004=sim/operation/quit
```

### Airliner (737)
```
# Engine management  
*001+010=sim/engines/engage_start_run_1
*002+010=sim/engines/engage_start_run_2
*001*002+015=sim/engines/thrust_reverse_toggle

# Autopilot combinations
*005+006=sim/autopilot/heading_select
*005+007=sim/autopilot/altitude_select
*006+007=sim/autopilot/speed_select
```

### General Aviation
```
# Pre-flight checks
*001+002=sim/engines/mixture_max
*001+003=sim/engines/prop_advance
*002+003=sim/electrical/battery_1_on

# Landing pattern
*010+011=sim/flight_controls/landing_gear_down
*010+012=sim/flight_controls/flaps_down
*011+012=sim/lights/landing_lights_toggle
```

### Using Negation (~) for Exclusive Modes
```
# Safety guard - gear toggle only works when safety button (050) is NOT held
~050*051+052=sim/gear/landing_gear_toggle

# Mode-exclusive controls
~100*110+111=sim/autopilot/mode_a   # Mode A only when 100 NOT held
*100*110+111=sim/autopilot/mode_b   # Mode B only when 100 IS held
```

## Finding X-Plane Commands

To find available X-Plane commands for your configurations:

1. **In X-Plane**: Open Developer → Command Search
2. **Search** for commands by category (e.g., "engine", "gear", "autopilot")
3. **Use the exact command name** in your configuration file

**Common Command Categories:**
- `sim/engines/` - Engine controls
- `sim/flight_controls/` - Flight controls (gear, flaps, trim)  
- `sim/autopilot/` - Autopilot functions
- `sim/lights/` - Aircraft lighting
- `sim/electrical/` - Electrical systems
- `sim/fuel/` - Fuel system controls

## Aircraft-Specific Configurations

The plugin automatically loads the correct configuration file based on the aircraft's ICAO code:

- **Cessna 172**: `C172.txt`
- **Boeing 737**: `B737.txt`
- **Airbus A320**: `A320.txt`
- **Baron 58**: `B58T.txt`

This allows you to have different button combinations for different aircraft types.

## Troubleshooting

### Plugin Not Loading
- **Check** X-Plane's Log.txt for error messages
- **Verify** the plugin file is in the correct location
- **Ensure** you have the correct architecture (64-bit)

### Combinations Not Working
- **Verify** all buttons are assigned to `multibind/XXX` commands in X-Plane's joystick settings
- **Check** your configuration file syntax (use `=` to separate triggers from command)
- **Ensure** trigger prefixes are correct (`*` held, `+` pressed, `-` released, `~` not held)
- **Ensure** button IDs are 3-digit zero-padded numbers (e.g., `*005` not `*5`)
- **Ensure** the X-Plane command exists (use Command Search in X-Plane)

### Configuration File Issues
- **Location**: Files must be in `X-Plane 12/multibind/`
- **Filename**: Must match aircraft ICAO code (check X-Plane's aircraft data)
- **Format**: Use format `<triggers>=<command>` (e.g., `*001+005=sim/some/command`)
- **Comments**: Lines starting with `#` are ignored

### Button Numbers
- Button numbers correspond to the `multibind/XXX` number you assigned
- If you assigned `multibind/005` to a button, use `005` in your configuration file (always 3 digits, zero-padded)
- Numbers can range from 000-999

## Menu Options

The plugin adds a "Multibind" menu to X-Plane's Plugins menu with:

- **Reload Configuration**: Reloads the current aircraft's configuration file without restarting X-Plane

Use this when you edit configuration files and want to test changes immediately.

## Configuration File Sharing

Configuration files are simple text files that can be easily shared:

1. **Export**: Copy your aircraft's configuration file from `X-Plane 12/multibind/`
2. **Share**: Send the `.txt` file to other users
3. **Import**: Other users place the file in their `multibind/` directory

This makes it easy to share button configurations within communities or flight simulation groups.

## Tips and Best Practices

### Button Assignment Strategy
- **Use logical groupings**: Assign related buttons to sequential multibind numbers
- **Reserve ranges**: Use 1-10 for primary controls, 11-20 for secondary, etc.
- **Document your assignments**: Keep notes on which physical buttons map to which multibind numbers

### Configuration File Organization
- **Use comments**: Add `#` comments to explain button combinations
- **Group by system**: Organize commands by aircraft systems (engines, flight controls, etc.)
- **Include descriptions**: Use meaningful descriptions for easier identification

### Testing New Configurations
1. **Start simple**: Test single button combinations first
2. **Use reload**: Use the Plugins → Multibind → Reload Configuration menu option
3. **Check logs**: Monitor X-Plane's Log.txt for error messages
4. **Test in safe conditions**: Test new combinations on the ground first

## Support

For issues and questions:
1. **Check** the troubleshooting section above
2. **Review** X-Plane's Log.txt for error messages  
3. **Verify** your configuration file syntax and X-Plane command names
4. **Test** with simple combinations first before creating complex ones

This plugin provides a powerful way to create custom button combinations for any X-Plane aircraft, enhancing your simulation experience through personalized control schemes.
