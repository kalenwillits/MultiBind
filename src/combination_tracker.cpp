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

    // Set up the axis callback
    _state_machine_manager.set_axis_callback(
        [this](const std::string& axis_id, const std::string& target_dataref) {
            this->on_axis_triggered(axis_id, target_dataref);
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
            if (binding.is_axis_binding) {
                // Add axis binding state machine
                _state_machine_manager.add_state_machine_axis(
                    binding.button_triggers,
                    binding.axis_id,
                    binding.target_dataref,
                    binding.description
                );
            } else {
                // Add regular command binding state machine
                _state_machine_manager.add_state_machine(
                    binding.button_triggers,
                    binding.target_command,
                    binding.description
                );
            }
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
    // Check if this is a continuous axis command (should be handled by continuous system)
    if (command.length() >= 16 && command.substr(0, 16) == "AXIS_CONTINUOUS:") {
        // This should not happen - continuous axis commands should go through continuous system
        std::string log_msg = "StateMachine: WARNING - Continuous axis command triggered as one-time: " + command + "\n";
        XPLMDebugString(log_msg.c_str());
        return;
    }

    // Check if this is a one-time axis command (encoded as "AXIS:axis_id:target_dataref")
    if (command.length() >= 5 && command.substr(0, 5) == "AXIS:") {
        // Parse axis command
        size_t first_colon = command.find(':', 5);
        if (first_colon != std::string::npos && first_colon + 1 < command.length()) {
            std::string axis_id = command.substr(5, first_colon - 5);
            std::string target_dataref = command.substr(first_colon + 1);

            // Validate parsed components
            if (!axis_id.empty() && !target_dataref.empty()) {
                // Trigger axis callback
                on_axis_triggered(axis_id, target_dataref);
                return;
            }
        }
    }

    // Regular command - queue for retrieval by the flight loop
    _triggered_commands.push(command);

    std::string log_msg = "StateMachine: Command queued: " + command + "\n";
    XPLMDebugString(log_msg.c_str());
}

void CombinationTracker::on_continuous_command_triggered(const std::string& command, bool start) {
    // Check if this is a continuous axis command
    if (command.length() >= 16 && command.substr(0, 16) == "AXIS_CONTINUOUS:") {
        // Parse continuous axis command
        size_t first_colon = command.find(':', 16);
        if (first_colon != std::string::npos && first_colon + 1 < command.length()) {
            std::string axis_id = command.substr(16, first_colon - 16);
            std::string target_dataref = command.substr(first_colon + 1);

            // Validate parsed components
            if (!axis_id.empty() && !target_dataref.empty()) {
                if (start) {
                    // Start continuous axis binding
                    _active_axis_bindings[axis_id] = target_dataref;
                    std::string log_msg = "StateMachine: Axis binding STARTED: " + axis_id + " -> " + target_dataref + "\n";
                    XPLMDebugString(log_msg.c_str());
                } else {
                    // Stop continuous axis binding
                    auto it = _active_axis_bindings.find(axis_id);
                    if (it != _active_axis_bindings.end()) {
                        std::string log_msg = "StateMachine: Axis binding STOPPED: " + axis_id + " -> " + it->second + "\n";
                        XPLMDebugString(log_msg.c_str());
                        _active_axis_bindings.erase(it);
                    }
                }
                return;
            }
        }
    }

    // Regular continuous command - queue for retrieval by the flight loop
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

std::pair<std::string, std::string> CombinationTracker::get_axis_action() {
    if (_axis_action_queue.empty()) {
        return std::make_pair("", "");
    }

    auto action = _axis_action_queue.front();
    _axis_action_queue.pop();
    return action;
}

void CombinationTracker::on_axis_triggered(const std::string& axis_id, const std::string& target_dataref) {
    // For axis bindings, we need to track them as active bindings rather than one-time actions
    // This callback currently gets called for one-time triggers, which is wrong for axis bindings
    // We'll handle this in the state machine integration phase
    std::string log_msg = "StateMachine: Axis trigger (needs continuous implementation): " + axis_id + " -> " + target_dataref + "\n";
    XPLMDebugString(log_msg.c_str());
}

const std::unordered_map<std::string, std::string>& CombinationTracker::get_active_axis_bindings() const {
    return _active_axis_bindings;
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

    // Stop all active axis bindings
    if (!_active_axis_bindings.empty()) {
        std::string log_msg = "CombinationTracker: Force stopped " +
                             std::to_string(_active_axis_bindings.size()) + " axis bindings\n";
        XPLMDebugString(log_msg.c_str());
        _active_axis_bindings.clear();
    }

    // Clear any pending continuous command actions
    while (!_continuous_command_queue.empty()) {
        _continuous_command_queue.pop();
    }

    // Clear any pending axis actions
    while (!_axis_action_queue.empty()) {
        _axis_action_queue.pop();
    }
}