#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMPlanes.h"

#include "config.h"
#include "combination_tracker.h"
#include "constants.h"

#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>

static XPLMMenuID g_menu_id = nullptr;
static int g_menu_item_id = -1;
static std::vector<XPLMCommandRef> g_multibind_commands;
static Config g_config;
static CombinationTracker g_tracker;

static std::string g_last_aircraft_id;

// Axis override system
static bool g_axis_override_active = false;
static XPLMDataRef g_joystick_axis_values_ref = nullptr;
static XPLMDataRef g_joystick_axis_assignments_ref = nullptr;
static XPLMDataRef g_override_throttles_ref = nullptr;
static XPLMDataRef g_override_control_surfaces_ref = nullptr;
static XPLMDataRef g_override_flightcontrol_ref = nullptr;

// Cache for axis assignment → physical index mapping
static std::unordered_map<int, int> g_axis_assignment_to_index;

// Dataref cache for performance
static std::unordered_map<std::string, XPLMDataRef> g_dataref_cache;

static float flight_loop_callback(float, float, int, void*);

static void menu_handler(void*, void* item_ref);
static int multibind_command_handler(XPLMCommandRef, XPLMCommandPhase phase, void* refcon);

void create_multibind_commands();
void load_aircraft_config();
void build_axis_assignment_cache();
void initialize_axis_system();
void apply_override_state(bool enable);
float get_axis_value(const std::string& axis_id);
XPLMDataRef get_cached_dataref(const std::string& dataref_name);

PLUGIN_API int XPluginStart(char* out_name, char* out_sig, char* out_desc)
{
    using namespace multibind::constants;
    
    std::string name = "Multibind";
    std::string sig = "multibind.plugin";
    std::string desc = "Multi-button joystick command binding plugin";
    
    // Safely copy strings with proper null termination
    std::strncpy(out_name, name.c_str(), XPLANE_STRING_BUFFER_SIZE - 1);
    std::strncpy(out_sig, sig.c_str(), XPLANE_STRING_BUFFER_SIZE - 1);
    std::strncpy(out_desc, desc.c_str(), XPLANE_STRING_BUFFER_SIZE - 1);
    out_name[XPLANE_STRING_BUFFER_SIZE - 1] = '\0';
    out_sig[XPLANE_STRING_BUFFER_SIZE - 1] = '\0';
    out_desc[XPLANE_STRING_BUFFER_SIZE - 1] = '\0';

    g_menu_id = XPLMFindPluginsMenu();
    if (g_menu_id) {
        g_menu_item_id = XPLMAppendMenuItem(g_menu_id, "Multibind", nullptr, 1);
        XPLMMenuID submenu = XPLMCreateMenu("Multibind", g_menu_id, g_menu_item_id, menu_handler, nullptr);
        XPLMAppendMenuItem(submenu, "Reload Configuration", (void*)2, 1);
    }

    create_multibind_commands();
    initialize_axis_system();

    XPLMRegisterFlightLoopCallback(flight_loop_callback, -1.0f, nullptr);

    XPLMDebugString("Multibind: Plugin started successfully\n");
    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    XPLMUnregisterFlightLoopCallback(flight_loop_callback, nullptr);
    
    for (auto command : g_multibind_commands) {
        if (command) {
            XPLMUnregisterCommandHandler(command, multibind_command_handler, 0, nullptr);
        }
    }
    g_multibind_commands.clear();

    XPLMDebugString("Multibind: Plugin stopped\n");
}

PLUGIN_API int XPluginEnable(void)
{
    load_aircraft_config();
    return 1;
}

PLUGIN_API void XPluginDisable(void)
{
    // Stop any running continuous commands
    g_tracker.stop_all_continuous_commands_real();
    // Note: Configuration saving disabled - users must edit files directly
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, int msg, void*)
{
    if (msg == XPLM_MSG_PLANE_LOADED) {
        load_aircraft_config();
    }
}

void create_multibind_commands()
{
    using namespace multibind::constants;
    
    g_multibind_commands.reserve(MAX_MULTIBIND_COMMANDS);
    
    for (int i = 0; i < MAX_MULTIBIND_COMMANDS; i++) {
        std::ostringstream command_name_ss;
        command_name_ss << "multibind/" << std::setfill('0') << std::setw(3) << i;
        std::string command_name = command_name_ss.str();
        
        std::ostringstream command_desc_ss;
        command_desc_ss << "Multibind command " << std::setfill('0') << std::setw(3) << i;
        std::string command_desc = command_desc_ss.str();
        
        XPLMCommandRef command = XPLMCreateCommand(command_name.c_str(), command_desc.c_str());
        if (command) {
            XPLMRegisterCommandHandler(command, multibind_command_handler, 1, (void*)(intptr_t)i);
            g_multibind_commands.push_back(command);
        } else {
            std::string error_msg = "Multibind: ERROR - Failed to create command " + command_name + "\n";
            XPLMDebugString(error_msg.c_str());
        }
    }
    
    std::ostringstream log_msg_ss;
    log_msg_ss << "Multibind: Created " << g_multibind_commands.size() << " multibind commands (000-999)\n";
    std::string log_msg = log_msg_ss.str();
    XPLMDebugString(log_msg.c_str());
}

void load_aircraft_config()
{
    using namespace multibind::constants;

    // Use SDK function to get aircraft filename - always reliable
    char filename[256];
    char path[512];
    XPLMGetNthAircraftModel(0, filename, path);  // 0 = user's aircraft

    // Extract aircraft ID from filename (e.g., "Cessna_172SP.acf" → "Cessna_172SP")
    std::string aircraft_file(filename);
    std::string aircraft_id = aircraft_file.substr(0, aircraft_file.find_last_of('.'));

    if (aircraft_id != g_last_aircraft_id) {
        g_last_aircraft_id = aircraft_id;

        std::string log_msg = "Multibind: Loading config for aircraft: " + aircraft_id +
                            " (from file: " + aircraft_file + ")\n";
        XPLMDebugString(log_msg.c_str());

        // Stop any continuous commands from previous aircraft
        g_tracker.stop_all_continuous_commands();

        g_config.load_config(aircraft_id);
        g_tracker.set_bindings(g_config.get_bindings());

        // Rebuild axis assignment cache for new aircraft
        build_axis_assignment_cache();

        // Check for axis bindings and apply override state automatically
        bool has_axis_bindings = g_config.has_axis_bindings();
        apply_override_state(has_axis_bindings);

    }
}


static float flight_loop_callback(float, float, int, void*)
{
    g_tracker.update();
    
    // Handle one-time commands
    std::string triggered_command = g_tracker.get_triggered_command();
    if (!triggered_command.empty()) {
        XPLMCommandRef command_ref = XPLMFindCommand(triggered_command.c_str());
        if (command_ref) {
            XPLMCommandOnce(command_ref);
            
            std::string log_msg = "Multibind: Triggered command: " + triggered_command + "\n";
            XPLMDebugString(log_msg.c_str());
        }
    }
    
    // Handle continuous commands (start/stop)
    auto continuous_action = g_tracker.get_continuous_command_action();
    if (!continuous_action.first.empty()) {
        const std::string& command = continuous_action.first;
        bool start = continuous_action.second;

        XPLMCommandRef command_ref = XPLMFindCommand(command.c_str());
        if (command_ref) {
            if (start) {
                XPLMCommandBegin(command_ref);
                std::string log_msg = "Multibind: Started continuous command: " + command + "\n";
                XPLMDebugString(log_msg.c_str());
            } else {
                XPLMCommandEnd(command_ref);
                std::string log_msg = "Multibind: Stopped continuous command: " + command + "\n";
                XPLMDebugString(log_msg.c_str());
            }
        }
    }

    // Handle continuous axis processing (only when override is active)
    if (g_axis_override_active) {
        // Process all currently active axis bindings every frame
        const auto& active_axis_bindings = g_tracker.get_active_axis_bindings();
        for (const auto& binding : active_axis_bindings) {
            const std::string& axis_id = binding.first;
            const std::string& target_dataref = binding.second;

            // Get current axis value
            float axis_value = get_axis_value(axis_id);

            // Find target dataref using cache and write axis value
            XPLMDataRef dataref = get_cached_dataref(target_dataref);
            if (dataref) {
                XPLMSetDataf(dataref, axis_value);
                // Note: Removed per-frame logging to reduce spam
            }
            // Note: Error message already printed by get_cached_dataref if needed
        }

        // Also handle any one-time axis actions (for compatibility)
        auto axis_action = g_tracker.get_axis_action();
        if (!axis_action.first.empty() && !axis_action.second.empty()) {
            // This is now primarily for logging/debugging
            std::string log_msg = "Multibind: One-time axis action: " + axis_action.first + " -> " + axis_action.second + "\n";
            XPLMDebugString(log_msg.c_str());
        }
    }
    
    return -1.0f;
}

static void menu_handler(void*, void* item_ref)
{
    intptr_t item = (intptr_t)item_ref;
    
    switch (item) {
        case 2: // Reload Configuration
            {
                std::string log_msg = "Multibind: Reloading configuration for current aircraft\n";
                XPLMDebugString(log_msg.c_str());
                
                // Force reload by clearing the last aircraft ICAO
                g_last_aircraft_id.clear();
                load_aircraft_config();
                
                XPLMDebugString("Multibind: Configuration reloaded successfully\n");
            }
            break;
    }
}

static int multibind_command_handler(XPLMCommandRef, XPLMCommandPhase phase, void* refcon)
{
    int command_id = (int)(intptr_t)refcon;
    
    // Button state tracking for trigger system
    if (phase == xplm_CommandBegin) {
        g_tracker.set_button_state_transition(command_id, ButtonAction::PRESSED);
    } else if (phase == xplm_CommandContinue) {
        g_tracker.set_button_state_transition(command_id, ButtonAction::HELD);
    } else if (phase == xplm_CommandEnd) {
        g_tracker.set_button_state_transition(command_id, ButtonAction::RELEASED);
    }
    
    return 0;
}

void build_axis_assignment_cache()
{
    using namespace multibind::constants;

    g_axis_assignment_to_index.clear();

    if (!g_joystick_axis_assignments_ref) {
        return;
    }

    // Read all 500 axis assignments
    int assignments[500];
    int count = XPLMGetDatavi(g_joystick_axis_assignments_ref, assignments, 0, 500);

    // Build reverse map: assignment_enum → physical_axis_index
    for (int i = 0; i < count; ++i) {
        int assignment_enum = assignments[i];
        if (assignment_enum >= MIN_AXIS_ID && assignment_enum <= MAX_AXIS_ID) {
            g_axis_assignment_to_index[assignment_enum] = i;
        }
    }

    std::string log_msg = "Multibind: Built axis assignment cache with " +
                         std::to_string(g_axis_assignment_to_index.size()) + " mappings\n";
    XPLMDebugString(log_msg.c_str());
}

void initialize_axis_system()
{
    // Find axis-related datarefs
    g_joystick_axis_values_ref = XPLMFindDataRef("sim/joystick/joystick_axis_values");
    g_joystick_axis_assignments_ref = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    g_override_throttles_ref = XPLMFindDataRef("sim/operation/override/override_throttles");
    g_override_control_surfaces_ref = XPLMFindDataRef("sim/operation/override/override_control_surfaces");
    g_override_flightcontrol_ref = XPLMFindDataRef("sim/operation/override/override_flightcontrol");

    if (!g_joystick_axis_values_ref) {
        XPLMDebugString("Multibind: WARNING - Could not find joystick axis values dataref\n");
    }

    if (!g_joystick_axis_assignments_ref) {
        XPLMDebugString("Multibind: WARNING - Could not find joystick axis assignments dataref\n");
    }

    if (!g_override_throttles_ref) {
        XPLMDebugString("Multibind: WARNING - Could not find throttle override dataref\n");
    }

    if (!g_override_control_surfaces_ref) {
        XPLMDebugString("Multibind: WARNING - Could not find control surfaces override dataref\n");
    }

    if (!g_override_flightcontrol_ref) {
        XPLMDebugString("Multibind: WARNING - Could not find flight control override dataref\n");
    }

    // Build initial cache
    build_axis_assignment_cache();

    XPLMDebugString("Multibind: Axis system initialized\n");
}

void apply_override_state(bool enable)
{
    g_axis_override_active = enable;

    if (enable) {
        XPLMDebugString("Multibind: Axis bindings detected - enabling axis override mode\n");

        // Enable override datarefs
        if (g_override_throttles_ref) {
            XPLMSetDatai(g_override_throttles_ref, 1);
        }
        if (g_override_control_surfaces_ref) {
            XPLMSetDatai(g_override_control_surfaces_ref, 1);
        }
        if (g_override_flightcontrol_ref) {
            XPLMSetDatai(g_override_flightcontrol_ref, 1);
        }
    } else {
        XPLMDebugString("Multibind: No axis bindings - normal X-Plane axis control\n");

        // Stop all active axis bindings before disabling override
        g_tracker.stop_all_continuous_commands_real();

        // Disable override datarefs
        if (g_override_throttles_ref) {
            XPLMSetDatai(g_override_throttles_ref, 0);
        }
        if (g_override_control_surfaces_ref) {
            XPLMSetDatai(g_override_control_surfaces_ref, 0);
        }
        if (g_override_flightcontrol_ref) {
            XPLMSetDatai(g_override_flightcontrol_ref, 0);
        }

        // Clear dataref cache to avoid stale references for new aircraft
        g_dataref_cache.clear();
        XPLMDebugString("Multibind: Cleared dataref cache\n");
    }
}

float get_axis_value(const std::string& axis_id)
{
    using namespace multibind::constants;

    if (!g_joystick_axis_values_ref) {
        return 0.0f;  // Return neutral if no axis dataref available
    }

    // Parse axis ID (A00-A66) to get assignment enum
    if (axis_id.length() < 3 || axis_id[0] != 'A') {
        return 0.0f;  // Invalid axis ID format
    }

    std::string axis_number_str = axis_id.substr(1, 2);
    int assignment_enum;
    try {
        assignment_enum = std::stoi(axis_number_str);
    } catch (const std::exception&) {
        return 0.0f;  // Invalid axis number
    }

    if (assignment_enum < MIN_AXIS_ID || assignment_enum > MAX_AXIS_ID) {
        return 0.0f;  // Assignment enum out of range
    }

    // Look up physical axis index from assignment enum
    auto it = g_axis_assignment_to_index.find(assignment_enum);
    if (it == g_axis_assignment_to_index.end()) {
        // This assignment not found in joystick configuration
        // This is normal - user may not have this axis mapped
        return 0.0f;
    }

    int physical_axis_index = it->second;

    // Read the axis values array
    float axis_values[500];
    int values_read = XPLMGetDatavf(g_joystick_axis_values_ref, axis_values, 0, 500);

    if (physical_axis_index < values_read) {
        return axis_values[physical_axis_index];
    }

    return 0.0f;  // Physical index beyond available values
}

XPLMDataRef get_cached_dataref(const std::string& dataref_name)
{
    // Check cache first
    auto it = g_dataref_cache.find(dataref_name);
    if (it != g_dataref_cache.end()) {
        return it->second;
    }

    // Not in cache, look it up and cache it
    XPLMDataRef dataref = XPLMFindDataRef(dataref_name.c_str());
    g_dataref_cache[dataref_name] = dataref;  // Cache even if nullptr for failed lookups

    if (!dataref) {
        std::string error_msg = "Multibind: WARNING - Dataref not found (cached): " + dataref_name + "\n";
        XPLMDebugString(error_msg.c_str());
    }

    return dataref;
}