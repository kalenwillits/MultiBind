#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

struct MultibindBinding {
    std::set<int> button_combination;  // Set of multibind button IDs (0-999)
    std::string target_command;        // XPlane command to execute
    std::string description;           // User-friendly description
    
    MultibindBinding() = default;
    MultibindBinding(const std::set<int>& buttons, const std::string& command, const std::string& desc)
        : button_combination(buttons), target_command(command), description(desc) {}
};

class Config {
public:
    Config() = default;
    ~Config() = default;
    
    bool load_config(const std::string& aircraft_id);
    bool save_config();
    
    void add_binding(const MultibindBinding& binding);
    void remove_binding(size_t index);
    void update_binding(size_t index, const MultibindBinding& binding);
    
    const std::vector<MultibindBinding>& get_bindings() const { return bindings_; }
    std::vector<MultibindBinding>& get_bindings() { return bindings_; }
    
    const std::string& get_aircraft_id() const { return aircraft_id_; }
    
    bool create_multibind_directory();
    std::string get_config_file_path() const;
    
private:
    std::string aircraft_id_;
    std::vector<MultibindBinding> bindings_;
    
    std::string combination_to_string(const std::set<int>& combination) const;
    std::set<int> string_to_combination(const std::string& str) const;
    std::string get_multibind_directory() const;
};