#include "config.h"
#include "XPLMUtilities.h"
#include "XPLMPlanes.h"
#include "constants.h"
#include "input_validation.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cstring>
#include <iomanip>

bool Config::load_config()
{
    _bindings.clear();
    
    std::string config_file = get_config_file_path();
    std::ifstream file(config_file);
    
    if (!file.is_open()) {
        std::string log_msg = "MultiBind: Config file not found: " + config_file + ", creating default config\n";
        XPLMDebugString(log_msg.c_str());
        
        if (!create_default_config()) {
            return false; // Failed to create default config
        }
        
        // Try to open the newly created file
        file.open(config_file);
        if (!file.is_open()) {
            std::string error_msg = "MultiBind: ERROR - Could not open newly created config file: " + config_file + "\n";
            XPLMDebugString(error_msg.c_str());
            return false;
        }
    }
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        // Bounds checking: prevent extremely long lines from causing issues
        constexpr size_t MAX_LINE_LENGTH = 2048;
        if (line.length() > MAX_LINE_LENGTH) {
            std::string error_msg = "MultiBind: Line " + std::to_string(line_number) + " exceeds maximum length (" + 
                                  std::to_string(MAX_LINE_LENGTH) + " characters), skipping\n";
            XPLMDebugString(error_msg.c_str());
            continue;
        }
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Format: "button1+button2=command" or "*button1+button2-button3=command"
        size_t equals_pos = line.find('=');
        
        if (equals_pos == std::string::npos) {
            std::string truncated_line = line.length() > 100 ? line.substr(0, 100) + "..." : line;
            std::string error_msg = "MultiBind: Invalid line format at line " + std::to_string(line_number) + 
                                  ": " + truncated_line + "\n";
            XPLMDebugString(error_msg.c_str());
            continue;
        }
        
        std::string combination_str = line.substr(0, equals_pos);
        std::string command = line.substr(equals_pos + 1);
        std::string description = ""; // No description in new format
        
        // Validate input components
        if (!multibind::validation::is_valid_xplane_command(command)) {
            std::string error_msg = "MultiBind: Invalid command at line " + std::to_string(line_number) + 
                                  ": " + command + "\n";
            XPLMDebugString(error_msg.c_str());
            continue;
        }
        
        if (!multibind::validation::is_valid_description(description)) {
            std::string error_msg = "MultiBind: Invalid description at line " + std::to_string(line_number) + 
                                  " (too long)\n";
            XPLMDebugString(error_msg.c_str());
            continue;
        }
        
        // Detect format: check for proper trigger pattern (*000, +001, -002)
        bool has_trigger_format = false;
        for (size_t i = 0; i < combination_str.length(); ++i) {
            char c = combination_str[i];
            if ((c == '*' || c == '+' || c == '-') && (i + 3 < combination_str.length())) {
                // Check if followed by 3 digits
                bool valid_digits = true;
                for (int j = 1; j <= 3; ++j) {
                    if (!std::isdigit(combination_str[i + j])) {
                        valid_digits = false;
                        break;
                    }
                }
                if (valid_digits) {
                    has_trigger_format = true;
                    break;
                }
            }
        }
        
        if (has_trigger_format) {
            // Parse trigger format (*005+018-020)
            std::vector<ButtonTrigger> triggers = string_to_triggers(combination_str);
            if (!triggers.empty() && !command.empty()) {
                // Prevent excessive bindings (DOS protection)
                constexpr size_t MAX_BINDINGS = 1000;
                if (_bindings.size() >= MAX_BINDINGS) {
                    std::string error_msg = "MultiBind: Maximum number of bindings (" + 
                                          std::to_string(MAX_BINDINGS) + ") reached at line " + 
                                          std::to_string(line_number) + ", ignoring remaining entries\n";
                    XPLMDebugString(error_msg.c_str());
                    break;
                }
                _bindings.emplace_back(triggers, command, description);
            } else if (triggers.empty()) {
                std::string error_msg = "MultiBind: No valid button triggers at line " + std::to_string(line_number) + "\n";
                XPLMDebugString(error_msg.c_str());
            }
        } else {
            // Invalid format
            std::string error_msg = "MultiBind: Invalid format at line " + std::to_string(line_number) + ". Use format like *000+001=command\n";
            XPLMDebugString(error_msg.c_str());
        }
    }
    
    std::string log_msg = "MultiBind: Loaded " + std::to_string(_bindings.size()) + " bindings from " + config_file + "\n";
    XPLMDebugString(log_msg.c_str());
    
    return true;
}

bool Config::save_config()
{
    // Configuration saving disabled - users must edit config files directly
    XPLMDebugString("MultiBind: Configuration saving disabled - users must edit config files manually\n");
    return false;
}

void Config::add_binding(const MultibindBinding& binding)
{
    _bindings.push_back(binding);
}

void Config::remove_binding(size_t index)
{
    if (index < _bindings.size()) {
        _bindings.erase(_bindings.begin() + index);
    }
}

void Config::update_binding(size_t index, const MultibindBinding& binding)
{
    if (index < _bindings.size()) {
        _bindings[index] = binding;
    }
}


std::string Config::get_config_file_path() const
{
    std::string aircraft_dir = get_aircraft_directory();
    return aircraft_dir + "/MultiBind.cfg";
}

std::string Config::get_aircraft_directory() const
{
    using namespace multibind::constants;
    
    char filename[XPLANE_PATH_BUFFER_SIZE];
    char path[XPLANE_PATH_BUFFER_SIZE];
    
    XPLMGetNthAircraftModel(0, filename, path);
    
    std::string aircraft_path(path);
    if (!aircraft_path.empty()) {
        // The path includes the .acf filename, extract just the directory
        size_t last_slash = aircraft_path.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            aircraft_path = aircraft_path.substr(0, last_slash);
        }
    }
    
    return aircraft_path;
}

bool Config::create_default_config() const
{
    std::string config_file = get_config_file_path();
    
    // Create directory if it doesn't exist
    std::filesystem::path config_path(config_file);
    std::filesystem::path config_dir = config_path.parent_path();
    
    if (!config_dir.empty()) {
        try {
            std::filesystem::create_directories(config_dir);
        } catch (const std::exception& e) {
            std::string error_msg = "MultiBind: ERROR - Failed to create directory " + config_dir.string() + ": " + e.what() + "\n";
            XPLMDebugString(error_msg.c_str());
            return false;
        }
    }
    
    // Create default config file with instructions
    std::ofstream file(config_file);
    if (!file.is_open()) {
        std::string error_msg = "MultiBind: ERROR - Could not create config file: " + config_file + "\n";
        XPLMDebugString(error_msg.c_str());
        return false;
    }
    
    // Write default config with comments
    file << "# MultiBind Configuration File\n";
    file << "# \n";
    file << "# Format: *button1+button2=command_name\n";
    file << "# Example: *000+001=sim/pitch_trim_up\n";
    file << "# \n";
    file << "# Button numbers correspond to joystick buttons (000-999)\n";
    file << "# Multiple buttons can be combined with +\n";
    file << "# Lines starting with # are comments\n";
    file << "# \n";
    file << "# Add your button combinations below:\n";
    file << "\n";
    
    file.close();
    
    std::string log_msg = "MultiBind: Created default config file: " + config_file + "\n";
    XPLMDebugString(log_msg.c_str());
    
    return true;
}


std::vector<ButtonTrigger> Config::string_to_triggers(const std::string& str) const
{
    using namespace multibind::constants;
    
    std::vector<ButtonTrigger> triggers;
    std::string current_token;
    
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        
        if (c == '*' || c == '+' || c == '-') {
            // Found a trigger prefix - extract the button ID that follows
            ButtonAction action;
            switch (c) {
                case '*': action = ButtonAction::HELD; break;
                case '+': action = ButtonAction::PRESSED; break; 
                case '-': action = ButtonAction::RELEASED; break;
            }
            
            // Extract button number (next 3 digits)
            if (i + 3 < str.length()) {
                // Validate that next 3 characters are digits
                bool valid_digits = true;
                for (int j = 1; j <= 3; ++j) {
                    if (!std::isdigit(str[i + j])) {
                        valid_digits = false;
                        break;
                    }
                }
                
                if (valid_digits) {
                    std::string button_str = str.substr(i + 1, 3);
                    try {
                        int button = std::stoi(button_str);
                        if (button >= MIN_BUTTON_ID && button <= MAX_BUTTON_ID) {
                            triggers.emplace_back(button, action);
                        }
                    } catch (const std::exception&) {
                        // Invalid button number, skip
                    }
                }
                i += 3; // Skip the button digits we just processed
            }
        }
    }
    
    return triggers;
}

std::string Config::triggers_to_string(const std::vector<ButtonTrigger>& triggers) const
{
    std::stringstream ss;
    
    for (size_t i = 0; i < triggers.size(); ++i) {
        const auto& trigger = triggers[i];
        
        // Add prefix based on action
        switch (trigger.action) {
            case ButtonAction::HELD: ss << "*"; break;
            case ButtonAction::PRESSED: ss << "+"; break;
            case ButtonAction::RELEASED: ss << "-"; break;
        }
        
        // Add zero-padded button ID
        ss << std::setfill('0') << std::setw(3) << trigger.button_id;
    }
    
    return ss.str();
}

