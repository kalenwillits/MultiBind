#include "event_system.h"
#include "config.h"
#include "XPLMUtilities.h"
#include <sstream>
#include <iostream>
#include <iomanip>

// StateMachine implementation

void StateMachine::build_from_pattern(const std::vector<ButtonTrigger>& triggers, const std::string& command) {
    if (triggers.empty()) {
        return;
    }

    // Store the triggers and command for later reference
    _command = command;

    // Separate negated and non-negated triggers
    std::vector<ButtonTrigger> positive_triggers;
    for (const auto& trigger : triggers) {
        if (trigger.is_negated) {
            _negated_triggers.push_back(trigger);
        } else {
            positive_triggers.push_back(trigger);
        }
    }

    // Store all triggers for reference
    _triggers = triggers;

    // Build a linear state machine for the positive trigger sequence only
    // Negated triggers are checked separately during validation
    // Example: ~000*010+011 creates: ROOT -> HELD(010) -> PRESSED(011) -> TERMINAL(command)
    //          with ~000 stored in _negated_triggers

    if (positive_triggers.empty()) {
        // All triggers are negated - this is unusual but technically valid
        // We'll create a trivial state machine that always validates
        XPLMDebugString("StateMachine: WARNING - Pattern has only negated triggers\n");
        return;
    }

    StatePtr current_state = _root_state;

    std::ostringstream debug_stream;
    debug_stream << "Pattern: ";

    // Track index of positive triggers separately for terminal state determination
    size_t positive_index = 0;

    for (size_t i = 0; i < triggers.size(); ++i) {
        const auto& trigger = triggers[i];

        // Add to debug description
        char prefix;
        if (trigger.is_negated) {
            prefix = '~';
        } else {
            prefix = (trigger.action == ButtonAction::HELD) ? '*' :
                     (trigger.action == ButtonAction::PRESSED) ? '+' : '-';
        }
        debug_stream << prefix << std::setfill('0') << std::setw(3) << trigger.button_id;

        // Only build state machine for positive triggers
        if (!trigger.is_negated) {
            ButtonEvent event = button_trigger_to_event(trigger);

            // Create next state
            std::ostringstream state_name;
            state_name << "State_" << positive_index + 1 << "_" << prefix << trigger.button_id;
            StatePtr next_state = std::make_shared<StateNode>(state_name.str());

            // Add transition from current to next state
            current_state->add_transition(event, next_state);

            // If this is the LAST POSITIVE trigger, make it terminal
            if (positive_index == positive_triggers.size() - 1) {
                next_state->set_terminal(command);
            }

            // Move to next state
            current_state = next_state;
            positive_index++;
        }
    }
    
    debug_stream << " = " << command;
    _pattern_description = debug_stream.str();

    // Determine if this is a continuous pattern (all POSITIVE triggers are HELD)
    // Negated triggers don't affect continuous behavior - they're just conditions
    _is_continuous_pattern = true;
    for (const auto& trigger : triggers) {
        // Skip negated triggers - they don't determine continuous vs one-time behavior
        if (trigger.is_negated) {
            continue;
        }

        if (trigger.action != ButtonAction::HELD) {
            _is_continuous_pattern = false;
            break;
        }
    }
    
    std::string log_msg = "StateMachine: Built pattern: " + _pattern_description + 
                         (_is_continuous_pattern ? " (CONTINUOUS)" : " (ONE-TIME)") + "\n";
    XPLMDebugString(log_msg.c_str());
}

void StateMachine::process_event(const ButtonEvent& event, const std::unordered_map<int, bool>& current_button_states) {
    // Try to transition from current state
    StatePtr next_state = _current_state->process_event(event);

    if (next_state) {
        // Before transitioning, validate negated conditions
        // This ensures negated buttons aren't pressed during the entire sequence
        if (!validate_negated_conditions(current_button_states)) {
            // Negation check failed - reset to root
            if (_current_state != _root_state) {
                std::string log_msg = "StateMachine: Negation check failed during sequence for pattern: " +
                                     _pattern_description + ", resetting\n";
                XPLMDebugString(log_msg.c_str());
                _current_state = _root_state;
            }
            return;
        }

        // Valid transition found and negation validated
        _current_state = next_state;

        std::ostringstream log;
        log << "StateMachine: " << _pattern_description
            << " -> " << _current_state->get_debug_name() << "\n";
        XPLMDebugString(log.str().c_str());

        // Check if we reached a terminal state
        if (_current_state->is_terminal()) {
            // Negation already validated above, so we can execute
            if (_command_callback) {
                _command_callback(_current_state->get_command());
            }

            // Smart reset: advance through held buttons to find correct state
            _current_state = _root_state;
            smart_reset_for_held_buttons(current_button_states);

            std::string trigger_log = "StateMachine: TRIGGERED " + _current_state->get_command() + " -> SMART RESET\n";
            XPLMDebugString(trigger_log.c_str());
        }
    } else {
        // No valid transition - check if we should reset or stay
        // For held buttons, we might want to stay in current state
        // For discrete events, we might want to reset to root
        
        if (event.type == EventType::BUTTON_RELEASED) {
            // Button released - this might invalidate current state
            // For now, reset to root (can be optimized later)
            if (_current_state != _root_state) {
                _current_state = _root_state;
                std::string reset_log = "StateMachine: " + _pattern_description + " -> RESET (release)\n";
                XPLMDebugString(reset_log.c_str());
            }
        }
        // For PRESSED/HELD events with no transition, we stay in current state
        // This allows held buttons to maintain their state
    }
}

void StateMachine::smart_reset_for_held_buttons(const std::unordered_map<int, bool>& current_button_states) {
    // Start from root and advance through any held button states
    // This allows patterns like *000+001 to work repeatedly
    
    for (const auto& button_pair : current_button_states) {
        int button_id = button_pair.first;
        bool is_held = button_pair.second;
        
        if (is_held) {
            // Try to advance state machine with HELD event for this button
            ButtonEvent held_event(EventType::BUTTON_HELD, button_id);
            StatePtr next_state = _current_state->process_event(held_event);
            
            if (next_state) {
                _current_state = next_state;
                
                std::ostringstream log;
                log << "StateMachine: Smart reset advanced to " << _current_state->get_debug_name() 
                    << " for held button " << button_id << "\n";
                XPLMDebugString(log.str().c_str());
            }
        }
    }
}

// StateMachineManager implementation

void StateMachineManager::add_state_machine(const std::vector<ButtonTrigger>& triggers, 
                                           const std::string& command, 
                                           const std::string& description) {
    auto machine = std::make_unique<StateMachine>(description);
    machine->set_command_callback(_command_callback);
    machine->build_from_pattern(triggers, command);
    _state_machines.push_back(std::move(machine));
}

void StateMachineManager::process_button_transition(int button_id, ButtonAction action) {
    // Update button state tracking
    if (action == ButtonAction::PRESSED) {
        _current_button_states[button_id] = true;
    } else if (action == ButtonAction::RELEASED) {
        _current_button_states[button_id] = false;
    }
    // HELD doesn't change the pressed state
    
    // Create button event
    ButtonEvent event(button_action_to_event_type(action), button_id);
    
    // Send event to all state machines
    for (auto& machine : _state_machines) {
        machine->process_event(event, _current_button_states);
    }
}

void StateMachineManager::clear() {
    _state_machines.clear();
    _current_button_states.clear();
    _active_continuous_commands.clear();
}

void StateMachineManager::update_continuous_commands() {
    if (!_continuous_command_callback) {
        return; // No continuous command callback set
    }
    
    // Check each state machine for continuous patterns
    for (const auto& machine : _state_machines) {
        if (!machine->is_continuous_pattern()) {
            continue; // Skip one-time patterns
        }
        
        // Check if this continuous pattern should be active
        bool should_be_active = is_continuous_pattern_currently_active(*machine);
        
        // Get the command from the state machine (we need to enhance this)
        std::string command = get_command_from_machine(*machine);
        if (command.empty()) {
            continue;
        }
        
        auto it = _active_continuous_commands.find(command);
        bool is_currently_active = (it != _active_continuous_commands.end() && it->second);
        
        if (should_be_active && !is_currently_active) {
            // Start continuous command
            _continuous_command_callback(command, true);
            _active_continuous_commands[command] = true;
            
            std::string log_msg = "StateMachineManager: Started continuous command: " + command + "\n";
            XPLMDebugString(log_msg.c_str());
        } else if (!should_be_active && is_currently_active) {
            // Stop continuous command
            _continuous_command_callback(command, false);
            _active_continuous_commands[command] = false;
            
            std::string log_msg = "StateMachineManager: Stopped continuous command: " + command + "\n";
            XPLMDebugString(log_msg.c_str());
        }
    }
}

// Helper method to check if a continuous pattern should be active
bool StateMachineManager::is_continuous_pattern_currently_active(const StateMachine& machine) {
    if (!machine.is_continuous_pattern()) {
        return false;
    }

    // A continuous pattern is active if:
    // 1. All its HELD button triggers (positive) are currently pressed
    // 2. All its negated button triggers are currently NOT pressed

    // Check positive conditions - all HELD buttons must be pressed
    const auto& triggers = machine.get_triggers();
    for (const auto& trigger : triggers) {
        if (trigger.action == ButtonAction::HELD && !trigger.is_negated) {
            auto it = _current_button_states.find(trigger.button_id);
            if (it == _current_button_states.end() || !it->second) {
                return false; // Button not pressed
            }
        }
    }

    // Check negated conditions - all negated buttons must NOT be pressed
    const auto& negated_triggers = machine.get_negated_triggers();
    for (const auto& trigger : negated_triggers) {
        auto it = _current_button_states.find(trigger.button_id);
        if (it != _current_button_states.end() && it->second) {
            return false; // Negated button is pressed - pattern should not be active
        }
    }

    return true;
}

// Helper method to get command from state machine
std::string StateMachineManager::get_command_from_machine(const StateMachine& machine) {
    return machine.get_command();
}

// Implementation of StateMachine::get_command
std::string StateMachine::get_command() const {
    return _command;
}

bool StateMachine::validate_negated_conditions(const std::unordered_map<int, bool>& current_button_states) const {
    // Check all negated triggers - they must NOT be pressed
    for (const auto& negated_trigger : _negated_triggers) {
        auto it = current_button_states.find(negated_trigger.button_id);

        // If button is in state map and is pressed, negation fails
        if (it != current_button_states.end() && it->second) {
            std::ostringstream log;
            log << "StateMachine: Negation failed - button " << negated_trigger.button_id << " is pressed\n";
            XPLMDebugString(log.str().c_str());
            return false;
        }
    }

    // All negated buttons are either not pressed or not in state map - validation succeeds
    return true;
}