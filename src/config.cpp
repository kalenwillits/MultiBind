#include "config.h"
#include "XPLMUtilities.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

bool Config::load_config(const std::string& aircraft_id)
{
    aircraft_id_ = aircraft_id;
    bindings_.clear();
    
    if (!create_multibind_directory()) {
        XPLMDebugString("Multibind: WARNING - Could not create multibind directory\n");
    }
    
    std::string config_file = get_config_file_path();
    std::ifstream file(config_file);
    
    if (!file.is_open()) {
        char log_msg[512];
        snprintf(log_msg, sizeof(log_msg), "Multibind: Config file not found, creating new one: %s\n", config_file.c_str());
        XPLMDebugString(log_msg);
        return save_config(); // Create empty config file
    }
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Simple format: "button1+button2+button3|command|description"
        size_t first_pipe = line.find('|');
        size_t second_pipe = line.find('|', first_pipe + 1);
        
        if (first_pipe == std::string::npos || second_pipe == std::string::npos) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Multibind: Invalid line format at line %d: %s\n", line_number, line.c_str());
            XPLMDebugString(error_msg);
            continue;
        }
        
        std::string combination_str = line.substr(0, first_pipe);
        std::string command = line.substr(first_pipe + 1, second_pipe - first_pipe - 1);
        std::string description = line.substr(second_pipe + 1);
        
        std::set<int> combination = string_to_combination(combination_str);
        if (!combination.empty() && !command.empty()) {
            bindings_.emplace_back(combination, command, description);
        }
    }
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Multibind: Loaded %zu bindings from %s\n", bindings_.size(), config_file.c_str());
    XPLMDebugString(log_msg);
    
    return true;
}

bool Config::save_config()
{
    if (aircraft_id_.empty()) {
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
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), "Multibind: ERROR - Could not open config file for writing: %s\n", config_file.c_str());
        XPLMDebugString(error_msg);
        return false;
    }
    
    file << "# Multibind configuration for " << aircraft_id_ << "\n";
    file << "# Format: button1+button2+button3|command|description\n";
    file << "# Example: 1+5+10|sim/starters/engage_starter_1|Start Engine 1\n";
    file << "\n";
    
    for (const auto& binding : bindings_) {
        file << combination_to_string(binding.button_combination) << "|" 
             << binding.target_command << "|" 
             << binding.description << "\n";
    }
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Multibind: Saved %zu bindings to %s\n", bindings_.size(), config_file.c_str());
    XPLMDebugString(log_msg);
    
    return true;
}

void Config::add_binding(const MultibindBinding& binding)
{
    bindings_.push_back(binding);
}

void Config::remove_binding(size_t index)
{
    if (index < bindings_.size()) {
        bindings_.erase(bindings_.begin() + index);
    }
}

void Config::update_binding(size_t index, const MultibindBinding& binding)
{
    if (index < bindings_.size()) {
        bindings_[index] = binding;
    }
}

bool Config::create_multibind_directory()
{
    std::string multibind_dir = get_multibind_directory();
    
    try {
        std::filesystem::create_directories(multibind_dir);
        return true;
    } catch (const std::exception& e) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), "Multibind: ERROR - Failed to create directory %s: %s\n", 
                multibind_dir.c_str(), e.what());
        XPLMDebugString(error_msg);
        return false;
    }
}

std::string Config::get_config_file_path() const
{
    return get_multibind_directory() + "/" + aircraft_id_ + ".txt";
}

std::string Config::get_multibind_directory() const
{
    char xplane_path[512];
    XPLMGetSystemPath(xplane_path);
    return std::string(xplane_path) + "multibind";
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
    std::set<int> combination;
    std::stringstream ss(str);
    std::string button_str;
    
    while (std::getline(ss, button_str, '+')) {
        try {
            int button = std::stoi(button_str);
            if (button >= 0 && button < 1000) {
                combination.insert(button);
            }
        } catch (const std::exception&) {
            // Invalid button number, skip
        }
    }
    
    return combination;
}