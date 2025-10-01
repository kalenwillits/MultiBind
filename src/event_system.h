#pragma once

#include "config.h"  // For ButtonTrigger and ButtonAction types
#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

// Generic Event System for Button Input Processing
// This replaces the frame-based transition clearing approach with persistent state machines

enum class EventType {
    BUTTON_PRESSED,   // Button just pressed (+ prefix)
    BUTTON_HELD,      // Button held down (* prefix) 
    BUTTON_RELEASED   // Button just released (- prefix)
};

struct ButtonEvent {
    EventType type;
    int button_id;
    std::chrono::steady_clock::time_point timestamp;
    
    ButtonEvent(EventType t, int id) 
        : type(t), button_id(id), timestamp(std::chrono::steady_clock::now()) {}
        
    // Events are equal if they have the same type and button
    bool operator==(const ButtonEvent& other) const {
        return type == other.type && button_id == other.button_id;
    }
};

// Hash function for ButtonEvent to use as map keys
struct ButtonEventHash {
    std::size_t operator()(const ButtonEvent& event) const {
        return std::hash<int>()(event.button_id) ^ 
               (std::hash<int>()(static_cast<int>(event.type)) << 1);
    }
};

// Forward declaration
class StateNode;

// State machine node that represents a single state in the pattern matching
class StateNode {
public:
    using StatePtr = std::shared_ptr<StateNode>;
    using TransitionMap = std::unordered_map<ButtonEvent, StatePtr, ButtonEventHash>;
    using CommandCallback = std::function<void(const std::string&)>;
    
private:
    TransitionMap _transitions;           // Event -> Next State
    std::string _command;                 // Command to execute (if terminal)
    bool _is_terminal = false;            // True if this state executes a command
    std::string _debug_name;              // For debugging
    
public:
    StateNode(const std::string& debug_name = "") : _debug_name(debug_name) {}
    
    // Add a transition: event -> next_state
    void add_transition(const ButtonEvent& event, StatePtr next_state) {
        _transitions[event] = next_state;
    }
    
    // Make this a terminal state that executes a command
    void set_terminal(const std::string& command) {
        _command = command;
        _is_terminal = true;
    }
    
    // Process an event and return the next state (or nullptr if no transition)
    StatePtr process_event(const ButtonEvent& event) const {
        auto it = _transitions.find(event);
        if (it != _transitions.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    // Check if this state should execute a command
    bool is_terminal() const { return _is_terminal; }
    const std::string& get_command() const { return _command; }
    const std::string& get_debug_name() const { return _debug_name; }
    
    // Get all possible transitions (for debugging)
    const TransitionMap& get_transitions() const { return _transitions; }
};

// State machine that processes button events for a specific pattern
class StateMachine {
public:
    using StatePtr = std::shared_ptr<StateNode>;
    using CommandCallback = std::function<void(const std::string&)>;
    using ContinuousCommandCallback = std::function<void(const std::string&, bool)>;  // command, start/stop
    using AxisCallback = std::function<void(const std::string&, const std::string&)>;  // axis_id, target_dataref
    
private:
    StatePtr _root_state;
    StatePtr _current_state;
    std::string _pattern_description;
    CommandCallback _command_callback;
    bool _is_continuous_pattern = false;  // True if all triggers are HELD
    std::vector<ButtonTrigger> _triggers;  // Store the original triggers
    std::vector<ButtonTrigger> _negated_triggers;  // Store negated triggers separately
    std::string _command;  // Store the command
    
public:
    StateMachine(const std::string& pattern_desc = "") 
        : _pattern_description(pattern_desc) {
        _root_state = std::make_shared<StateNode>("ROOT");
        _current_state = _root_state;
    }
    
    // Set the callback function for when commands are triggered
    void set_command_callback(CommandCallback callback) {
        _command_callback = callback;
    }
    
    // Build the state machine from a button trigger pattern
    void build_from_pattern(const std::vector<ButtonTrigger>& triggers, const std::string& command);
    
    // Check if this pattern should run continuously (all triggers are HELD)
    bool is_continuous_pattern() const { return _is_continuous_pattern; }
    
    // Get the command this state machine will execute
    std::string get_command() const;
    
    // Get the triggers for this state machine
    const std::vector<ButtonTrigger>& get_triggers() const { return _triggers; }

    // Get the negated triggers for this state machine
    const std::vector<ButtonTrigger>& get_negated_triggers() const { return _negated_triggers; }

    // Validate negated button conditions
    bool validate_negated_conditions(const std::unordered_map<int, bool>& current_button_states) const;
    
    // Process a button event through the state machine  
    void process_event(const ButtonEvent& event, const std::unordered_map<int, bool>& current_button_states);
    
    // Reset the state machine to the root state
    void reset() { _current_state = _root_state; }
    
    // Get current state for debugging
    StatePtr get_current_state() const { return _current_state; }
    
private:
    // Smart reset: advance state machine through currently held buttons
    void smart_reset_for_held_buttons(const std::unordered_map<int, bool>& current_button_states);
    StatePtr get_root_state() const { return _root_state; }
    
    const std::string& get_pattern_description() const { return _pattern_description; }
};

// Manager for multiple state machines (one per binding)
class StateMachineManager {
private:
    std::vector<std::unique_ptr<StateMachine>> _state_machines;
    std::unordered_map<int, bool> _current_button_states;  // Track button press states
    StateMachine::CommandCallback _command_callback;
    StateMachine::ContinuousCommandCallback _continuous_command_callback;
    StateMachine::AxisCallback _axis_callback;
    std::unordered_map<std::string, bool> _active_continuous_commands;  // Track running continuous commands
    
public:
    StateMachineManager() = default;
    
    // Set the callback for when any state machine triggers a command
    void set_command_callback(StateMachine::CommandCallback callback) {
        _command_callback = callback;
    }
    
    // Set the callback for continuous commands (command, start/stop)
    void set_continuous_command_callback(StateMachine::ContinuousCommandCallback callback) {
        _continuous_command_callback = callback;
    }

    // Set the callback for axis actions (axis_id, target_dataref)
    void set_axis_callback(StateMachine::AxisCallback callback) {
        _axis_callback = callback;
    }
    
    // Update continuous command states based on current button states
    void update_continuous_commands();
    
    // Add a state machine for a binding pattern
    void add_state_machine(const std::vector<ButtonTrigger>& triggers, const std::string& command, const std::string& description);

    // Add a state machine for an axis binding pattern
    void add_state_machine_axis(const std::vector<ButtonTrigger>& triggers, const std::string& axis_id, const std::string& target_dataref, const std::string& description);
    
    // Process a button state change and generate appropriate events
    void process_button_transition(int button_id, ButtonAction action);
    
    // Get current button states (for UI)
    const std::unordered_map<int, bool>& get_current_button_states() const {
        return _current_button_states;
    }
    
    // Clear all state machines and button states
    void clear();
    
    // Get all state machines (for debugging)
    const std::vector<std::unique_ptr<StateMachine>>& get_state_machines() const {
        return _state_machines;
    }
    
private:
    // Helper methods for continuous command management
    bool is_continuous_pattern_currently_active(const StateMachine& machine);
    std::string get_command_from_machine(const StateMachine& machine);
};

// Helper function to convert ButtonAction to EventType
inline EventType button_action_to_event_type(ButtonAction action) {
    switch (action) {
        case ButtonAction::PRESSED: return EventType::BUTTON_PRESSED;
        case ButtonAction::HELD: return EventType::BUTTON_HELD;
        case ButtonAction::RELEASED: return EventType::BUTTON_RELEASED;
        default: return EventType::BUTTON_PRESSED;
    }
}

// Helper function to convert ButtonTrigger to ButtonEvent  
inline ButtonEvent button_trigger_to_event(const ButtonTrigger& trigger) {
    return ButtonEvent(button_action_to_event_type(trigger.action), trigger.button_id);
}