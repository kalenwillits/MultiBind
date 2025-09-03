#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"

#include "config.h"
#include "combination_tracker.h"
#include "ui.h"

#include <string>
#include <vector>

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
    strcpy(out_name, "Multibind");
    strcpy(out_sig, "multibind.plugin");
    strcpy(out_desc, "Multi-button joystick command binding plugin");

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
    g_multibind_commands.reserve(1000);
    
    for (int i = 0; i < 1000; i++) {
        char command_name[64];
        char command_desc[128];
        snprintf(command_name, sizeof(command_name), "multibind/%03d", i);
        snprintf(command_desc, sizeof(command_desc), "Multibind command %03d", i);
        
        XPLMCommandRef command = XPLMCreateCommand(command_name, command_desc);
        if (command) {
            XPLMRegisterCommandHandler(command, multibind_command_handler, 1, (void*)(intptr_t)i);
            g_multibind_commands.push_back(command);
        } else {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Multibind: ERROR - Failed to create command %s\n", command_name);
            XPLMDebugString(error_msg);
        }
    }
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Multibind: Created %zu multibind commands (000-999)\n", g_multibind_commands.size());
    XPLMDebugString(log_msg);
}

void load_aircraft_config()
{
    char aircraft_filename[512];
    XPLMGetDatab(g_aircraft_filename_ref, aircraft_filename, 0, sizeof(aircraft_filename));
    
    std::string current_aircraft(aircraft_filename);
    if (current_aircraft != g_last_aircraft_filename) {
        g_last_aircraft_filename = current_aircraft;
        
        std::string aircraft_id = extract_aircraft_id(current_aircraft);
        
        char log_msg[512];
        snprintf(log_msg, sizeof(log_msg), "Multibind: Loading config for aircraft: %s\n", aircraft_id.c_str());
        XPLMDebugString(log_msg);
        
        g_config.load_config(aircraft_id);
        g_tracker.set_bindings(g_config.get_bindings());
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
            
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Multibind: Triggered command: %s\n", triggered_command.c_str());
            XPLMDebugString(log_msg);
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