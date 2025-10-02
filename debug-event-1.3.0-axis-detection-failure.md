Given this config
---
# TODO - Trim Plugin, Brakes Plugin, Radio Selector Plugin, Autopilot Mode Selector Plugin


# Primary
*101^A00=sim/cockpit2/engine/actuators/throttle_ratio_all

# ENGINE
# Ignition
*000+011=sim/ignition/ignition_up_1
*000+012=sim/ignition/ignition_down_1
*000*017=sim/engines/engage_starters
#*000*013=afm/sr/cmd/panel/keyPositionIn
#*000*014=afm/sr/cmd/panel/keyPositionOut


# Parking Brake
*000+105=sim/flight_controls/brakes_toggle_min
*000+105=sim/flight_controls/park_brake_release
*000+105=sim/flight_controls/park_brake_valve_close
*000+106=sim/flight_controls/brakes_max
*000+106=sim/flight_controls/park_brake_set
*000+106=sim/flight_controls/park_brake_valve_open



*000+007=sim/electrical/battery_1_toggle
#*000+007=afm/sr/cmd/switches/bat2_Toggle
*000+008=sim/electrical/battery_2_toggle
#*000+008=afm/sr/cmd/switches/bat1_Toggle
*000+009=sim/electrical/generator_1_toggle
#*000+009=afm/sr/cmd/switches/alt1_Toggle
*000+010=sim/electrical/generator_2_toggle
#*000+010=afm/sr/cmd/switches/alt2_Toggle

*000-005-006-103=sim/fuel/fuel_selector_none
*000-005-006=sim/fuel/fuel_selector_all
#*000+005=afm/sr/cmd/fuel/selLeft
*000+005=sim/fuel/fuel_selector_lft
#*000+006=afm/sr/cmd/fuel/selRight
*001+006=sim/fuel/fuel_selector_rgt
*001-005-006=sim/fuel/fuel_selector_all
#*001+005=afm/sr/cmd/fuel/selLeft
*001+005=sim/fuel/fuel_selector_lft
#*001+006=afm/sr/cmd/fuel/selRight
*001+006=sim/fuel/fuel_selector_rgt
*002-005-006=sim/fuel/fuel_selector_all
#*002+005=afm/sr/cmd/fuel/selLeft
*002+005=sim/fuel/fuel_selector_lft
#*002+006=afm/sr/cmd/fuel/selRight
*002+006=sim/fuel/fuel_selector_rgt

*002+101=sim/flight_controls/left_brake
*002+102=sim/flight_controls/right_brake
*002+100=sim/flight_controls/brakes_max


*000+104=sim/engines/carb_heat_off
*000+103=sim/engines/carb_heat_on
*001+104=sim/engines/carb_heat_off
*001+103=sim/engines/carb_heat_on
*003+104=sim/engines/carb_heat_off
*003+103=sim/engines/carb_heat_on

# ELECTRICAL
*001+007=sim/systems/avionics_toggle
#*001+007=afm/sr/cmd/switches/avionics_Toggle
*001+008=sim/lights/nav_lights_toggle
#*001+008=afm/sr/cmd/switches/nav_Toggle
*001+009=sim/lights/strobe_lights_toggle
#*001+009=afm/sr/cmd/switches/strobe_Toggle
*001+010=sim/lights/landing_lights_toggle
#*001+010=afm/sr/cmd/switches/landing_Toggle
*001+101=sim/ice/pitot_heat0_tog
*001+101=sim/ice/pitot_heat1_tog
#*001+101=afm/sr/cmd/switches/pitot_Toggle

*001+011=sim/instruments/barometer_up
*001+012=sim/instruments/barometer_down

*002+105=trimgear/pitch_trim_down
*002+106=trimgear/pitch_trim_up

*002*011=trimgear/roll_trim_right
*002*012=trimgear/roll_trim_left
*002*015=trimgear/rudder_trim_right
*002*016=trimgear/rudder_trim_left
*002*013=trimgear/pitch_trim_down
*002*014=trimgear/pitch_trim_up


*004+101=sim/GPS/g1000n1_ias
*004+100=sim/GPS/g1000n1_vs
*004+102=sim/GPS/g1000n1_alt
*004+102=sim/autopilot/altitude_hold
*004+007=sim/gps/g1000n1_ap
*004+008=sim/autopilot/fdir_toggle
*004+109=sim/autopilot/heading_hold

*004+109=sim/GPS/g1000n1_hdg
*004+110=sim/GPS/g1000n1_nav

*004+109+110=sim/GPS/g1000n1_apr

# TODO - Create new plugin 'autopilot selector'
*004+011=sim/autopilot/heading_up
*004+012=sim/autopilot/heading_down
*004+017=sim/autopilot/heading_sync
*004+104=sim/autopilot/vertical_speed_down
*004+103=sim/autopilot/vertical_speed_up
---
We then attempted to acutate the throttle movement here `*101^A00=sim/cockpit2/engine/actuators/throttle_ratio_all
We then changed the mapping from A04 to A00 and reloaded. (We set the throttle lever to `None`) 
` -> all with no result.
---
Below are the logs:
---
Search "Multibind" (31 hits in 1 file of 1 searched) [Normal: Case]
  C:\Program Files (x86)\Steam\steamapps\common\X-Plane 12\Log.txt (31 hits)
	Line  173: Multibind: Created 1000 multibind commands (000-999)
	Line  174: Multibind: Axis system initialized
	Line  175: Multibind: Plugin started successfully
	Line  176: Loaded: C:\Program Files (x86)/Steam/steamapps/common/X-Plane 12/Resources/plugins/Multibind/win.xpl (multibind.plugin).
	Line 1041: Multibind: Loading config for aircraft: C172
	Line 1042: Multibind: Config file not found: C:\Program Files (x86)\Steam\steamapps\common\X-Plane 12\multibind/C172.txt (users must create manually)
	Line 1043: Multibind: Axis detection check - found 0 axis bindings out of 0 total bindings
	Line 1044: Multibind: No axis bindings - normal X-Plane axis control
	Line 1045: Multibind: Cleared dataref cache
	Line 2064: Multibind: Reloading configuration for current aircraft
	Line 2065: Multibind: Loading config for aircraft: C172
	Line 2066: Multibind: Config file not found: C:\Program Files (x86)\Steam\steamapps\common\X-Plane 12\multibind/C172.txt (users must create manually)
	Line 2067: Multibind: Axis detection check - found 0 axis bindings out of 0 total bindings
	Line 2068: Multibind: No axis bindings - normal X-Plane axis control
	Line 2069: Multibind: Cleared dataref cache
	Line 2070: Multibind: Configuration reloaded successfully
	Line 2079: Multibind: Reloading configuration for current aircraft
	Line 2080: Multibind: Loading config for aircraft: C172
	Line 2081: Multibind: Config file not found: C:\Program Files (x86)\Steam\steamapps\common\X-Plane 12\multibind/C172.txt (users must create manually)
	Line 2082: Multibind: Axis detection check - found 0 axis bindings out of 0 total bindings
	Line 2083: Multibind: No axis bindings - normal X-Plane axis control
	Line 2084: Multibind: Cleared dataref cache
	Line 2085: Multibind: Configuration reloaded successfully
	Line 2087: Multibind: Reloading configuration for current aircraft
	Line 2088: Multibind: Loading config for aircraft: C172
	Line 2089: Multibind: Config file not found: C:\Program Files (x86)\Steam\steamapps\common\X-Plane 12\multibind/C172.txt (users must create manually)
	Line 2090: Multibind: Axis detection check - found 0 axis bindings out of 0 total bindings
	Line 2091: Multibind: No axis bindings - normal X-Plane axis control
	Line 2092: Multibind: Cleared dataref cache
	Line 2093: Multibind: Configuration reloaded successfully
	Line 2112: Multibind: Plugin stopped
