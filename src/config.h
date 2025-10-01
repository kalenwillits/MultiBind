#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

enum class ButtonAction {
    PRESSED,    // Button just pressed (+ prefix)
    HELD,       // Button held down (* prefix)
    RELEASED    // Button just released (- prefix)
};

struct ButtonTrigger {
    int button_id;
    ButtonAction action;
    
    ButtonTrigger() = default;
    ButtonTrigger(int id, ButtonAction act) : button_id(id), action(act) {}
    
    // For compatibility with existing code that expects comparison
    bool operator<(const ButtonTrigger& other) const {
        if (button_id != other.button_id) return button_id < other.button_id;
        return static_cast<int>(action) < static_cast<int>(other.action);
    }
    
    bool operator==(const ButtonTrigger& other) const {
        return button_id == other.button_id && action == other.action;
    }
};

struct AxisBinding {
    std::vector<ButtonTrigger> button_triggers;  // Button triggers with actions
    std::string axis_id;                         // Axis ID (A00-A66)
    std::string target_dataref;                  // XPlane dataref to write to
    std::string description;                     // User-friendly description

    AxisBinding() = default;

    // Constructor for axis bindings
    AxisBinding(const std::vector<ButtonTrigger>& triggers, const std::string& axis, const std::string& dataref, const std::string& desc)
        : button_triggers(triggers), axis_id(axis), target_dataref(dataref), description(desc) {}
};

struct MultibindBinding {
    std::vector<ButtonTrigger> button_triggers;  // Button triggers with actions
    std::string target_command;                  // XPlane command to execute
    std::string target_dataref;                  // XPlane dataref to write to (for axis bindings)
    std::string axis_id;                         // Axis ID (A00-A66) for axis bindings
    std::string description;                     // User-friendly description
    bool is_axis_binding;                        // True if this is an axis binding

    MultibindBinding() : is_axis_binding(false) {}

    // Constructor for button triggers (command bindings)
    MultibindBinding(const std::vector<ButtonTrigger>& triggers, const std::string& command, const std::string& desc)
        : button_triggers(triggers), target_command(command), description(desc), is_axis_binding(false) {}

    // Constructor for axis bindings
    MultibindBinding(const std::vector<ButtonTrigger>& triggers, const std::string& axis, const std::string& dataref, const std::string& desc)
        : button_triggers(triggers), target_dataref(dataref), axis_id(axis), description(desc), is_axis_binding(true) {}
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

    // Check if any axis bindings exist
    bool has_axis_bindings() const;
    
    const std::string& get_aircraft_id() const { return _aircraft_id; }
    
    bool create_multibind_directory();
    std::string get_config_file_path() const;
    
private:
    std::string _aircraft_id;
    std::vector<MultibindBinding> _bindings;
    
    // Trigger parsing functions
    std::string triggers_to_string(const std::vector<ButtonTrigger>& triggers) const;
    std::vector<ButtonTrigger> string_to_triggers(const std::string& str) const;

    // Axis binding parsing functions
    std::pair<std::vector<ButtonTrigger>, std::string> string_to_axis_binding(const std::string& str) const;
    
    std::string get_multibind_directory() const;
};