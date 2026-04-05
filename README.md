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
3. **Configuration**: Define combinations in simple text files (e.g., "buttons 5+10+15 = start engine 1")
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
button1+button2+button3|command|description
```

**Example Configuration File** (`X-Plane 12/multibind/B58T.txt`):
```
# Multibind configuration for Baron 58
# Format: button1+button2+button3|command|description

# Engine controls
1+5|sim/starters/engage_starter_1|Start Engine 1
2+6|sim/starters/engage_starter_2|Start Engine 2
1+2+10|sim/engines/mixture_max|Emergency Full Rich

# Landing gear and flaps
10+11|sim/flight_controls/landing_gear_toggle|Gear Toggle
10+12|sim/flight_controls/flaps_down|Flaps Down
11+12|sim/flight_controls/flaps_up|Flaps Up

# Emergency procedures
1+5+10+15|sim/operation/pause_toggle|Emergency Pause
```

### Step 3: Using Your Combinations

1. **Load your aircraft** in X-Plane
2. **Press the button combinations** you defined
3. **The corresponding X-Plane commands will execute**

**Example Usage:**
- Press buttons assigned to `multibind/001` and `multibind/005` simultaneously → Engine 1 starts
- Press buttons assigned to `multibind/010` and `multibind/011` simultaneously → Landing gear toggles

## Configuration File Examples

### Combat Aircraft (F/A-18)
```
# Weapons systems
1+2|sim/weapons/master_arm_toggle|Master Arm Toggle
1+3|sim/weapons/gun_trigger|Gun Fire
2+4|sim/weapons/missile_launch|Missile Launch

# Emergency procedures
1+2+3+4|sim/operation/quit|Emergency Quit
```

### Airliner (737)
```
# Engine management  
1+10|sim/engines/engage_start_run_1|Engine 1 Start
2+10|sim/engines/engage_start_run_2|Engine 2 Start
1+2+15|sim/engines/thrust_reverse_toggle|Reverse Thrust

# Autopilot combinations
5+6|sim/autopilot/heading_select|HDG Select
5+7|sim/autopilot/altitude_select|ALT Select
6+7|sim/autopilot/speed_select|SPD Select
```

### General Aviation
```
# Pre-flight checks
1+2|sim/engines/mixture_max|Mixture Rich
1+3|sim/engines/prop_advance|Prop Full Forward  
2+3|sim/electrical/battery_1_on|Battery On

# Landing pattern
10+11|sim/flight_controls/landing_gear_down|Gear Down
10+12|sim/flight_controls/flaps_down|Flaps Down
11+12|sim/lights/landing_lights_toggle|Landing Lights
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
- **Check** your configuration file syntax (use pipes `|` as separators)
- **Ensure** the X-Plane command exists (use Command Search in X-Plane)
- **Test** timing - all buttons must be pressed within a short time window

### Configuration File Issues
- **Location**: Files must be in `X-Plane 12/multibind/`
- **Filename**: Must match aircraft ICAO code (check X-Plane's aircraft data)
- **Format**: Use format `button+button|command|description`
- **Comments**: Lines starting with `#` are ignored

### Button Numbers
- Button numbers correspond to the `multibind/XXX` number you assigned
- If you assigned `multibind/005` to a button, use `5` in your configuration file
- Numbers can range from 0-999

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
