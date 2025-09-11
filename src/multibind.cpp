#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"

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


static float flight_loop_callback(float, float, int, void*);

static void menu_handler(void*, void* item_ref);
static int multibind_command_handler(XPLMCommandRef, XPLMCommandPhase phase, void* refcon);

void create_multibind_commands();
void load_aircraft_config();

PLUGIN_API int XPluginStart(char* out_name, char* out_sig, char* out_desc)
{
    using namespace multibind::constants;
    
    std::string name = "MultiBind";
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
        g_menu_item_id = XPLMAppendMenuItem(g_menu_id, "MultiBind", nullptr, 1);
        XPLMMenuID submenu = XPLMCreateMenu("MultiBind", g_menu_id, g_menu_item_id, menu_handler, nullptr);
        XPLMAppendMenuItem(submenu, "Reload Configuration", (void*)2, 1);
    }

    create_multibind_commands();
    
    
    XPLMRegisterFlightLoopCallback(flight_loop_callback, -1.0f, nullptr);

    XPLMDebugString("MultiBind: Plugin started successfully\n");
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

    XPLMDebugString("MultiBind: Plugin stopped\n");
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
            std::string error_msg = "MultiBind: ERROR - Failed to create command " + command_name + "\n";
            XPLMDebugString(error_msg.c_str());
        }
    }
    
    std::ostringstream log_msg_ss;
    log_msg_ss << "MultiBind: Created " << g_multibind_commands.size() << " multibind commands (000-999)\n";
    std::string log_msg = log_msg_ss.str();
    XPLMDebugString(log_msg.c_str());
}

void load_aircraft_config()
{
    XPLMDebugString("MultiBind: Loading aircraft configuration\n");
    
    // Stop any continuous commands from previous aircraft
    g_tracker.stop_all_continuous_commands();
    
    g_config.load_config();
    g_tracker.set_bindings(g_config.get_bindings());
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
            
            std::string log_msg = "MultiBind: Triggered command: " + triggered_command + "\n";
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
                std::string log_msg = "MultiBind: Started continuous command: " + command + "\n";
                XPLMDebugString(log_msg.c_str());
            } else {
                XPLMCommandEnd(command_ref);
                std::string log_msg = "MultiBind: Stopped continuous command: " + command + "\n";
                XPLMDebugString(log_msg.c_str());
            }
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
                std::string log_msg = "MultiBind: Reloading configuration for current aircraft\n";
                XPLMDebugString(log_msg.c_str());
                
                // Force reload configuration
                load_aircraft_config();
                
                XPLMDebugString("MultiBind: Configuration reloaded successfully\n");
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