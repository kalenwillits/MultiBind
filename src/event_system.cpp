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
    
    // Build a linear state machine for the trigger sequence
    // Example: *000+001 creates: ROOT -> HELD(000) -> PRESSED(001) -> TERMINAL(command)
    
    StatePtr current_state = _root_state;
    
    std::ostringstream debug_stream;
    debug_stream << "Pattern: ";
    
    for (size_t i = 0; i < triggers.size(); ++i) {
        const auto& trigger = triggers[i];
        ButtonEvent event = button_trigger_to_event(trigger);
        
        // Add to debug description
        char prefix = (trigger.action == ButtonAction::HELD) ? '*' : 
                     (trigger.action == ButtonAction::PRESSED) ? '+' : '-';
        debug_stream << prefix << std::setfill('0') << std::setw(3) << trigger.button_id;
        
        // Create next state
        std::ostringstream state_name;
        state_name << "State_" << i + 1 << "_" << prefix << trigger.button_id;
        StatePtr next_state = std::make_shared<StateNode>(state_name.str());
        
        // Add transition from current to next state
        current_state->add_transition(event, next_state);
        
        // If this is the last trigger, make it terminal
        if (i == triggers.size() - 1) {
            next_state->set_terminal(command);
        }
        
        // Move to next state
        current_state = next_state;
    }
    
    debug_stream << " = " << command;
    _pattern_description = debug_stream.str();
    
    std::string log_msg = "StateMachine: Built pattern: " + _pattern_description + "\n";
    XPLMDebugString(log_msg.c_str());
}

void StateMachine::process_event(const ButtonEvent& event, const std::unordered_map<int, bool>& current_button_states) {
    // Try to transition from current state
    StatePtr next_state = _current_state->process_event(event);
    
    if (next_state) {
        // Valid transition found
        _current_state = next_state;
        
        std::ostringstream log;
        log << "StateMachine: " << _pattern_description 
            << " -> " << _current_state->get_debug_name() << "\n";
        XPLMDebugString(log.str().c_str());
        
        // Check if we reached a terminal state
        if (_current_state->is_terminal()) {
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
}