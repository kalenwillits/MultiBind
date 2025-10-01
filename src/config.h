#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

enum class ButtonAction {
    PRESSED,    // Button just pressed (+ prefix)
    HELD,       // Button held down (* prefix)
    RELEASED,   // Button just released (- prefix)
    NOT_HELD    // Button NOT held down (~ prefix)
};

struct ButtonTrigger {
    int button_id;
    ButtonAction action;
    bool is_negated;  // true if ~ prefix used (button must NOT be held)

    ButtonTrigger() : button_id(0), action(ButtonAction::PRESSED), is_negated(false) {}
    ButtonTrigger(int id, ButtonAction act, bool negated = false)
        : button_id(id), action(act), is_negated(negated) {}

    // For compatibility with existing code that expects comparison
    bool operator<(const ButtonTrigger& other) const {
        if (button_id != other.button_id) return button_id < other.button_id;
        if (is_negated != other.is_negated) return is_negated < other.is_negated;
        return static_cast<int>(action) < static_cast<int>(other.action);
    }

    bool operator==(const ButtonTrigger& other) const {
        return button_id == other.button_id && action == other.action && is_negated == other.is_negated;
    }
};

struct MultibindBinding {
    std::vector<ButtonTrigger> button_triggers;  // Button triggers with actions
    std::string target_command;                  // XPlane command to execute
    std::string description;                     // User-friendly description
    
    MultibindBinding() = default;
    
    // Constructor for button triggers
    MultibindBinding(const std::vector<ButtonTrigger>& triggers, const std::string& command, const std::string& desc)
        : button_triggers(triggers), target_command(command), description(desc) {}
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
    
    const std::vector<MultibindBinding>& get_bindings() const { return _bindings; }
    std::vector<MultibindBinding>& get_bindings() { return _bindings; }
    
    const std::string& get_aircraft_id() const { return _aircraft_id; }
    
    bool create_multibind_directory();
    std::string get_config_file_path() const;
    
private:
    std::string _aircraft_id;
    std::vector<MultibindBinding> _bindings;
    
    // Trigger parsing functions
    std::string triggers_to_string(const std::vector<ButtonTrigger>& triggers) const;
    std::vector<ButtonTrigger> string_to_triggers(const std::string& str) const;
    
    std::string get_multibind_directory() const;
};