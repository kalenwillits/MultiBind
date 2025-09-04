#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"

#include "config.h"
#include "combination_tracker.h"
#include "ui.h"
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
static UI g_ui;

static XPLMDataRef g_aircraft_filename_ref = nullptr;
static std::string g_last_aircraft_filename;

static float flight_loop_callback(float elapsed_since_last_call, 
                                 float elapsed_time_since_last_flightloop,
                                 int counter, 
                                 void* refcon);

static void menu_handler(void* menu_ref, void* item_ref);
static int multibind_command_handler(XPLMCommandRef command, 
                                   XPLMCommandPhase phase, 
                                   void* refcon);

void create_multibind_commands();
void load_aircraft_config();
std::string extract_aircraft_id(const std::string& filename);

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

    g_aircraft_filename_ref = XPLMFindDataRef("sim/aircraft/view/acf_filename");
    if (!g_aircraft_filename_ref) {
        XPLMDebugString("Multibind: ERROR - Could not find aircraft filename dataref\n");
        return 0;
    }

    g_menu_id = XPLMFindPluginsMenu();
    if (g_menu_id) {
        g_menu_item_id = XPLMAppendMenuItem(g_menu_id, "Multibind", nullptr, 1);
        XPLMMenuID submenu = XPLMCreateMenu("Multibind", g_menu_id, g_menu_item_id, menu_handler, nullptr);
        XPLMAppendMenuItem(submenu, "Open Multibind Window", (void*)1, 1);
    }

    create_multibind_commands();
    
    g_ui.initialize(&g_config, &g_tracker);
    
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
    g_ui.hide_window();
    g_config.save_config();
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void* param)
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
    
    std::string aircraft_filename(XPLANE_PATH_BUFFER_SIZE, '\0');
    XPLMGetDatab(g_aircraft_filename_ref, &aircraft_filename[0], 0, aircraft_filename.size());
    aircraft_filename.resize(std::strlen(aircraft_filename.c_str())); // Trim to actual length
    
    if (aircraft_filename != g_last_aircraft_filename) {
        g_last_aircraft_filename = aircraft_filename;
        
        std::string aircraft_id = extract_aircraft_id(aircraft_filename);
        
        std::string log_msg = "Multibind: Loading config for aircraft: " + aircraft_id + "\n";
        XPLMDebugString(log_msg.c_str());
        
        g_config.load_config(aircraft_id);
        g_tracker.set_bindings(g_config.get_bindings());
        
        // Update UI with current aircraft name
        g_ui.update_aircraft_display(aircraft_id);
    }
}

std::string extract_aircraft_id(const std::string& filename)
{
    size_t last_slash = filename.find_last_of("/\\");
    std::string basename = (last_slash != std::string::npos) ? 
                          filename.substr(last_slash + 1) : filename;
    
    size_t dot_pos = basename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        basename = basename.substr(0, dot_pos);
    }
    
    return basename;
}

static float flight_loop_callback(float elapsed_since_last_call, 
                                 float elapsed_time_since_last_flightloop,
                                 int counter, 
                                 void* refcon)
{
    g_tracker.update();
    g_ui.update();
    
    std::string triggered_command = g_tracker.get_triggered_command();
    if (!triggered_command.empty()) {
        XPLMCommandRef command_ref = XPLMFindCommand(triggered_command.c_str());
        if (command_ref) {
            XPLMCommandOnce(command_ref);
            
            std::string log_msg = "Multibind: Triggered command: " + triggered_command + "\n";
            XPLMDebugString(log_msg.c_str());
        }
    }
    
    return -1.0f;
}

static void menu_handler(void* menu_ref, void* item_ref)
{
    intptr_t item = (intptr_t)item_ref;
    
    switch (item) {
        case 1: // Open Multibind Window
            g_ui.show_window();
            break;
    }
}

static int multibind_command_handler(XPLMCommandRef command, 
                                   XPLMCommandPhase phase, 
                                   void* refcon)
{
    int command_id = (int)(intptr_t)refcon;
    
    if (phase == xplm_CommandBegin) {
        g_tracker.set_button_pressed(command_id, true);
    } else if (phase == xplm_CommandEnd) {
        g_tracker.set_button_pressed(command_id, false);
    }
    
    return 0;
}