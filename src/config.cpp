#include "config.h"
#include "XPLMUtilities.h"
#include "constants.h"
#include "input_validation.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cstring>
#include <iomanip>

bool Config::load_config(const std::string& aircraft_id)
{
    _aircraft_id = aircraft_id;
    _bindings.clear();
    
    if (!create_multibind_directory()) {
        XPLMDebugString("Multibind: WARNING - Could not create multibind directory\n");
    }
    
    std::string config_file = get_config_file_path();
    std::ifstream file(config_file);
    
    if (!file.is_open()) {
        std::string log_msg = "Multibind: Config file not found: " + config_file + " (users must create manually)\n";
        XPLMDebugString(log_msg.c_str());
        return true; // Don't auto-create - users must create files manually
    }
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        // Bounds checking: prevent extremely long lines from causing issues
        constexpr size_t MAX_LINE_LENGTH = 2048;
        if (line.length() > MAX_LINE_LENGTH) {
            std::string error_msg = "Multibind: Line " + std::to_string(line_number) + " exceeds maximum length (" + 
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
            std::string error_msg = "Multibind: Invalid line format at line " + std::to_string(line_number) + 
                                  ": " + truncated_line + "\n";
            XPLMDebugString(error_msg.c_str());
            continue;
        }
        
        std::string combination_str = line.substr(0, equals_pos);
        std::string command = line.substr(equals_pos + 1);
        std::string description = ""; // No description in new format
        
        // Validate input components
        if (!multibind::validation::is_valid_xplane_command(command)) {
            std::string error_msg = "Multibind: Invalid command at line " + std::to_string(line_number) + 
                                  ": " + command + "\n";
            XPLMDebugString(error_msg.c_str());
            continue;
        }
        
        if (!multibind::validation::is_valid_description(description)) {
            std::string error_msg = "Multibind: Invalid description at line " + std::to_string(line_number) + 
                                  " (too long)\n";
            XPLMDebugString(error_msg.c_str());
            continue;
        }
        
        // Detect format: enhanced (*+-) vs legacy (numeric+numeric)
        bool has_enhanced_format = combination_str.find_first_of("*+-") != std::string::npos;
        
        if (has_enhanced_format) {
            // Parse enhanced trigger format (*005+018-020)
            std::vector<ButtonTrigger> triggers = string_to_triggers(combination_str);
            if (!triggers.empty() && !command.empty()) {
                // Prevent excessive bindings (DOS protection)
                constexpr size_t MAX_BINDINGS = 1000;
                if (_bindings.size() >= MAX_BINDINGS) {
                    std::string error_msg = "Multibind: Maximum number of bindings (" + 
                                          std::to_string(MAX_BINDINGS) + ") reached at line " + 
                                          std::to_string(line_number) + ", ignoring remaining entries\n";
                    XPLMDebugString(error_msg.c_str());
                    break;
                }
                _bindings.emplace_back(triggers, command, description);
            } else if (triggers.empty()) {
                std::string error_msg = "Multibind: No valid button triggers at line " + std::to_string(line_number) + "\n";
                XPLMDebugString(error_msg.c_str());
            }
        } else {
            // Parse legacy combination format (000+018)
            std::set<int> combination = string_to_combination(combination_str);
            if (!combination.empty() && !command.empty()) {
                // Prevent excessive bindings (DOS protection)
                constexpr size_t MAX_BINDINGS = 1000;
                if (_bindings.size() >= MAX_BINDINGS) {
                    std::string error_msg = "Multibind: Maximum number of bindings (" + 
                                          std::to_string(MAX_BINDINGS) + ") reached at line " + 
                                          std::to_string(line_number) + ", ignoring remaining entries\n";
                    XPLMDebugString(error_msg.c_str());
                    break;
                }
                _bindings.emplace_back(combination, command, description);
            } else if (combination.empty()) {
                std::string error_msg = "Multibind: No valid button combination at line " + std::to_string(line_number) + "\n";
                XPLMDebugString(error_msg.c_str());
            }
        }
    }
    
    std::string log_msg = "Multibind: Loaded " + std::to_string(_bindings.size()) + " bindings from " + config_file + "\n";
    XPLMDebugString(log_msg.c_str());
    
    return true;
}

bool Config::save_config()
{
    // Configuration saving disabled - users must edit config files directly
    XPLMDebugString("Multibind: Configuration saving disabled - users must edit config files manually\n");
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

bool Config::create_multibind_directory()
{
    std::string multibind_dir = get_multibind_directory();
    
    try {
        std::filesystem::create_directories(multibind_dir);
        return true;
    } catch (const std::exception& e) {
        std::string error_msg = "Multibind: ERROR - Failed to create directory " + multibind_dir + ": " + e.what() + "\n";
        XPLMDebugString(error_msg.c_str());
        return false;
    }
}

std::string Config::get_config_file_path() const
{
    return get_multibind_directory() + "/" + _aircraft_id + ".txt";
}

std::string Config::get_multibind_directory() const
{
    using namespace multibind::constants;
    
    std::string xplane_path(XPLANE_PATH_BUFFER_SIZE, '\0');
    XPLMGetSystemPath(&xplane_path[0]);
    xplane_path.resize(std::strlen(xplane_path.c_str())); // Trim to actual length
    return xplane_path + "multibind";
}

std::string Config::combination_to_string(const std::set<int>& combination) const
{
    std::stringstream ss;
    bool first = true;
    
    for (int button : combination) {
        if (!first) ss << "+";
        ss << std::setfill('0') << std::setw(3) << button;
        first = false;
    }
    
    return ss.str();
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
                std::string button_str = str.substr(i + 1, 3);
                try {
                    int button = std::stoi(button_str);
                    if (button >= MIN_BUTTON_ID && button <= MAX_BUTTON_ID) {
                        triggers.emplace_back(button, action);
                    }
                } catch (const std::exception&) {
                    // Invalid button number, skip
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

std::set<int> Config::string_to_combination(const std::string& str) const
{
    using namespace multibind::constants;
    
    std::set<int> combination;
    std::stringstream ss(str);
    std::string button_str;
    
    while (std::getline(ss, button_str, '+')) {
        try {
            int button = std::stoi(button_str);
            if (button >= MIN_BUTTON_ID && button <= MAX_BUTTON_ID) {
                combination.insert(button);
            }
        } catch (const std::exception&) {
            // Invalid button number, skip
        }
    }
    
    return combination;
}
