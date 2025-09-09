#include "combination_tracker.h"
#include "XPLMUtilities.h"
#include "constants.h"

CombinationTracker::CombinationTracker() {
    // Set up the command callback for the state machine manager
    _state_machine_manager.set_command_callback(
        [this](const std::string& command) {
            this->on_command_triggered(command);
        }
    );
    
    // Set up the continuous command callback
    _state_machine_manager.set_continuous_command_callback(
        [this](const std::string& command, bool start) {
            this->on_continuous_command_triggered(command, start);
        }
    );
}

void CombinationTracker::set_button_state_transition(int button_id, ButtonAction action) {
    using namespace multibind::constants;
   
    if (button_id < MIN_BUTTON_ID || button_id > MAX_BUTTON_ID) {
        return;
    }
    
    // Handle recording if active
    if (_recording && action == ButtonAction::PRESSED) {
        // Check if this was a new press (not already in the combination)
        const auto& current_states = _state_machine_manager.get_current_button_states();
        auto state_it = current_states.find(button_id);
        bool was_pressed = (state_it != current_states.end() && state_it->second);
        
        if (!was_pressed) {
            _recorded_combination.insert(button_id);
        }
    }
    
    // Send to state machine manager
    _state_machine_manager.process_button_transition(button_id, action);
}

void CombinationTracker::set_bindings(const std::vector<MultibindBinding>& bindings) {
    // Clear existing state machines
    _state_machine_manager.clear();
    
    // Build state machines for each binding
    for (const auto& binding : bindings) {
        if (!binding.button_triggers.empty()) {
            _state_machine_manager.add_state_machine(
                binding.button_triggers, 
                binding.target_command, 
                binding.description
            );
        }
    }
}

void CombinationTracker::update() {
    // Update continuous command states based on current button states
    _state_machine_manager.update_continuous_commands();
    
    // In the state machine system, processing happens immediately when events occur
    // The update() method is now mainly for compatibility with the existing interface
    // Commands are queued by the state machines and retrieved via get_triggered_command()
}

std::string CombinationTracker::get_triggered_command() {
    if (_triggered_commands.empty()) {
        return "";
    }
    
    std::string command = _triggered_commands.front();
    _triggered_commands.pop();
    return command;
}

const std::unordered_map<int, bool>& CombinationTracker::get_current_button_states() const {
    return _state_machine_manager.get_current_button_states();
}

void CombinationTracker::clear_current_button_states() {
    _state_machine_manager.clear();
    _recorded_combination.clear();
}

void CombinationTracker::on_command_triggered(const std::string& command) {
    // Queue the command for retrieval by the flight loop
    _triggered_commands.push(command);
    
    std::string log_msg = "StateMachine: Command queued: " + command + "\n";
    XPLMDebugString(log_msg.c_str());
}

void CombinationTracker::on_continuous_command_triggered(const std::string& command, bool start) {
    // Queue the continuous command action for retrieval by the flight loop
    _continuous_command_queue.push(std::make_pair(command, start));
    
    std::string log_msg = "StateMachine: Continuous command queued: " + command + 
                         (start ? " (START)" : " (STOP)") + "\n";
    XPLMDebugString(log_msg.c_str());
}

std::pair<std::string, bool> CombinationTracker::get_continuous_command_action() {
    if (_continuous_command_queue.empty()) {
        return std::make_pair("", false);
    }
    
    auto action = _continuous_command_queue.front();
    _continuous_command_queue.pop();
    return action;
}

void CombinationTracker::stop_all_continuous_commands_real() {
    // Stop all active continuous commands
    for (const auto& pair : _active_continuous_commands) {
        XPLMCommandEnd(pair.second);
        
        std::string log_msg = "CombinationTracker: Force stopped continuous command: " + 
                             pair.first + "\n";
        XPLMDebugString(log_msg.c_str());
    }
    _active_continuous_commands.clear();
    
    // Clear any pending continuous command actions
    while (!_continuous_command_queue.empty()) {
        _continuous_command_queue.pop();
    }
}