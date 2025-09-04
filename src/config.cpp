#include "config.h"
#include "XPLMUtilities.h"
#include "constants.h"
#include "input_validation.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cstring>

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
        std::string log_msg = "Multibind: Config file not found, creating new one: " + config_file + "\n";
        XPLMDebugString(log_msg.c_str());
        return save_config(); // Create empty config file
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
        
        // Simple format: "button1+button2+button3|command|description"
        size_t first_pipe = line.find('|');
        size_t second_pipe = line.find('|', first_pipe + 1);
        
        if (first_pipe == std::string::npos || second_pipe == std::string::npos) {
            std::string truncated_line = line.length() > 100 ? line.substr(0, 100) + "..." : line;
            std::string error_msg = "Multibind: Invalid line format at line " + std::to_string(line_number) + 
                                  ": " + truncated_line + "\n";
            XPLMDebugString(error_msg.c_str());
            continue;
        }
        
        std::string combination_str = line.substr(0, first_pipe);
        std::string command = line.substr(first_pipe + 1, second_pipe - first_pipe - 1);
        std::string description = line.substr(second_pipe + 1);
        
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
    
    std::string log_msg = "Multibind: Loaded " + std::to_string(_bindings.size()) + " bindings from " + config_file + "\n";
    XPLMDebugString(log_msg.c_str());
    
    return true;
}

bool Config::save_config()
{
    if (_aircraft_id.empty()) {
        XPLMDebugString("Multibind: ERROR - Cannot save config, no aircraft ID set\n");
        return false;
    }
    
    if (!create_multibind_directory()) {
        XPLMDebugString("Multibind: ERROR - Could not create multibind directory\n");
        return false;
    }
    
    std::string config_file = get_config_file_path();
    std::ofstream file(config_file);
    
    if (!file.is_open()) {
        std::string error_msg = "Multibind: ERROR - Could not open config file for writing: " + config_file + "\n";
        XPLMDebugString(error_msg.c_str());
        return false;
    }
    
    file << "# Multibind configuration for " << _aircraft_id << "\n";
    file << "# Format: button1+button2+button3|command|description\n";
    file << "# Example: 1+5+10|sim/starters/engage_starter_1|Start Engine 1\n";
    file << "\n";
    
    for (const auto& binding : _bindings) {
        file << combination_to_string(binding.button_combination) << "|" 
             << binding.target_command << "|" 
             << binding.description << "\n";
    }
    
    std::string log_msg = "Multibind: Saved " + std::to_string(_bindings.size()) + " bindings to " + config_file + "\n";
    XPLMDebugString(log_msg.c_str());
    
    return true;
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
        ss << button;
        first = false;
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
