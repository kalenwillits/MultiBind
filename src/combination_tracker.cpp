#include "combination_tracker.h"
#include "XPLMUtilities.h"
#include "constants.h"

void CombinationTracker::set_button_pressed(int button_id, bool pressed)
{
    using namespace multibind::constants;
    
    if (button_id < MIN_BUTTON_ID || button_id > MAX_BUTTON_ID) {
        return;
    }
    
    bool was_pressed = _button_states[button_id];
    _button_states[button_id] = pressed;
    
    if (pressed && !was_pressed) {
        _currently_pressed.insert(button_id);
        if (_recording) {
            _recorded_combination.insert(button_id);
        }
    } else if (!pressed && was_pressed) {
        _currently_pressed.erase(button_id);
    }
    
    process_combination_change();
}

void CombinationTracker::set_button_state_transition(int button_id, ButtonAction action)
{
    using namespace multibind::constants;
    
    if (button_id < MIN_BUTTON_ID || button_id > MAX_BUTTON_ID) {
        return;
    }
    
    // Record the transition for frame-based processing
    _button_transitions[button_id] = action;
    _last_transition_time = std::chrono::steady_clock::now();
    
    // Pattern processing now happens in update() cycle - no immediate processing
}

void CombinationTracker::set_bindings(const std::vector<MultibindBinding>& bindings)
{
    _bindings = bindings;
}

void CombinationTracker::update()
{
    // Process enhanced trigger patterns with accumulated transitions from this frame
    process_enhanced_triggers();
    
    // Update continuous commands
    update_continuous_commands();
    
    // Clear frame transitions for next frame
    clear_frame_transitions();
    
    // Note: _triggered_command is cleared by get_triggered_command() 
    // after being retrieved, not here. This allows commands triggered
    // during update to be properly retrieved.
}

std::string CombinationTracker::get_triggered_command()
{
    std::string command = _triggered_command;
    _triggered_command.clear(); // Only trigger once
    return command;
}

void CombinationTracker::process_combination_change()
{
    using namespace multibind::constants;
    
    auto now = std::chrono::steady_clock::now();
    
    // Only process if we have buttons pressed
    if (_currently_pressed.empty()) {
        return;
    }
    
    // Check if current combination matches any binding
    for (const auto& binding : _bindings) {
        if (check_combination_match(_currently_pressed, binding.button_combination)) {
            // Found a match - but wait a bit to see if more buttons are coming
            _last_combination_time = now;
            
            // If this is an exact match and we've waited long enough, trigger it
            if (_currently_pressed == binding.button_combination) {
                // Small delay to prevent accidental double-triggering
                if (now - _last_trigger_time > std::chrono::milliseconds(TRIGGER_DEBOUNCE_MS)) {
                    _triggered_command = binding.target_command;
                    _last_trigger_time = now;
                    
                    std::string log_msg = "Multibind: Combination matched, triggering: " + binding.target_command + "\n";
                    XPLMDebugString(log_msg.c_str());
                }
            }
            return;
        }
    }
}

bool CombinationTracker::check_combination_match(const std::set<int>& pressed, const std::set<int>& target) const
{
    // Check if all target buttons are pressed
    for (int button : target) {
        if (pressed.find(button) == pressed.end()) {
            return false;
        }
    }
    
    // For now, allow extra buttons to be pressed (superset match)
    // This could be made configurable later
    return true;
}

void CombinationTracker::process_enhanced_triggers()
{
    using namespace multibind::constants;
    
    auto now = std::chrono::steady_clock::now();
    
    // Check enhanced trigger bindings
    for (const auto& binding : _bindings) {
        if (binding.uses_enhanced_triggers() && check_trigger_sequence_match(binding.button_triggers)) {
            
            // Determine if this should run continuously
            if (should_run_continuously(binding.button_triggers)) {
                // Start continuous command execution
                start_continuous_command(binding.target_command, binding.button_triggers);
            } else {
                // One-time trigger - check if this pattern contains HELD actions for different debounce logic
                bool has_held_actions = false;
                for (const auto& trigger : binding.button_triggers) {
                    if (trigger.action == ButtonAction::HELD) {
                        has_held_actions = true;
                        break;
                    }
                }
                
                // For patterns with HELD actions, use shorter debounce to allow rapid re-triggering
                auto debounce_time = has_held_actions ? 
                    std::chrono::milliseconds(50) :  // Short debounce for ambiguous patterns
                    std::chrono::milliseconds(TRIGGER_DEBOUNCE_MS); // Normal debounce for discrete patterns
                
                if (now - _last_trigger_time > debounce_time) {
                    _triggered_command = binding.target_command;
                    _last_trigger_time = now;
                    
                    std::string log_msg = "Multibind: Enhanced trigger sequence matched, triggering: " + binding.target_command + "\n";
                    XPLMDebugString(log_msg.c_str());
                }
            }
            return;
        }
    }
}

bool CombinationTracker::check_trigger_sequence_match(const std::vector<ButtonTrigger>& sequence) const
{
    if (sequence.empty()) {
        return false;
    }
    
    // For now, implement a simple "all triggers must match current state" approach
    // This can be enhanced later for more complex sequential matching
    for (const auto& trigger : sequence) {
        auto transition_it = _button_transitions.find(trigger.button_id);
        
        // Check if this button has the required transition
        if (transition_it == _button_transitions.end()) {
            // Button hasn't transitioned recently - check based on action type
            if (trigger.action == ButtonAction::HELD) {
                // For HELD, check if button is currently pressed
                if (_currently_pressed.find(trigger.button_id) == _currently_pressed.end()) {
                    return false;
                }
            } else {
                // For PRESSED or RELEASED, we need a recent transition - pattern doesn't match
                return false;
            }
        } else {
            // Button has transitioned - check if it matches required action
            ButtonAction recorded_action = transition_it->second;
            
            if (trigger.action == ButtonAction::PRESSED) {
                // PRESSED matches either PRESSED or HELD transitions
                if (recorded_action != ButtonAction::PRESSED && recorded_action != ButtonAction::HELD) {
                    return false;
                }
            } else if (trigger.action == ButtonAction::HELD) {
                // HELD matches HELD or PRESSED transitions (since HELD follows PRESSED)
                if (recorded_action != ButtonAction::HELD && recorded_action != ButtonAction::PRESSED) {
                    return false;
                }
            } else if (trigger.action == ButtonAction::RELEASED) {
                // RELEASED only matches RELEASED transitions
                if (recorded_action != ButtonAction::RELEASED) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

void CombinationTracker::start_continuous_command(const std::string& command, const std::vector<ButtonTrigger>& triggers)
{
    if (_active_continuous_commands.find(command) != _active_continuous_commands.end()) {
        return; // Already running
    }
    
    XPLMCommandRef command_ref = XPLMFindCommand(command.c_str());
    if (command_ref) {
        XPLMCommandBegin(command_ref);
        _active_continuous_commands[command] = command_ref;
        _continuous_bindings[command] = triggers;
        
        std::string log_msg = "Multibind: Started continuous command: " + command + "\n";
        XPLMDebugString(log_msg.c_str());
    }
}

void CombinationTracker::stop_continuous_command(const std::string& command)
{
    auto it = _active_continuous_commands.find(command);
    if (it != _active_continuous_commands.end()) {
        XPLMCommandEnd(it->second);
        _active_continuous_commands.erase(it);
        _continuous_bindings.erase(command);
        
        std::string log_msg = "Multibind: Stopped continuous command: " + command + "\n";
        XPLMDebugString(log_msg.c_str());
    }
}

void CombinationTracker::update_continuous_commands()
{
    // Check if any continuous commands should be stopped
    std::vector<std::string> commands_to_stop;
    
    for (const auto& pair : _continuous_bindings) {
        const std::string& command = pair.first;
        const std::vector<ButtonTrigger>& triggers = pair.second;
        
        if (!is_continuous_pattern_active(triggers)) {
            commands_to_stop.push_back(command);
        }
    }
    
    // Stop commands that are no longer active
    for (const std::string& command : commands_to_stop) {
        stop_continuous_command(command);
    }
}

void CombinationTracker::stop_all_continuous_commands()
{
    // Stop all currently running continuous commands
    std::vector<std::string> all_commands;
    for (const auto& pair : _active_continuous_commands) {
        all_commands.push_back(pair.first);
    }
    
    for (const std::string& command : all_commands) {
        stop_continuous_command(command);
    }
    
    XPLMDebugString("Multibind: All continuous commands stopped\n");
}

void CombinationTracker::clear_frame_transitions()
{
    // Clear all transitions for next frame - this gives each frame fresh pattern detection
    // HELD states are maintained through _currently_pressed, so continuous patterns still work
    _button_transitions.clear();
}


bool CombinationTracker::should_run_continuously(const std::vector<ButtonTrigger>& triggers) const
{
    // A pattern should run continuously if it contains HELD actions
    for (const auto& trigger : triggers) {
        if (trigger.action == ButtonAction::HELD) {
            return true;
        }
    }
    return false;
}

bool CombinationTracker::is_continuous_pattern_active(const std::vector<ButtonTrigger>& triggers) const
{
    // Check if the current button states match the continuous pattern requirements
    for (const auto& trigger : triggers) {
        switch (trigger.action) {
            case ButtonAction::HELD:
                // For HELD, button must be currently pressed
                if (_currently_pressed.find(trigger.button_id) == _currently_pressed.end()) {
                    return false;
                }
                break;
            case ButtonAction::PRESSED:
            case ButtonAction::RELEASED:
                // For discrete actions, these are not continuous requirements
                // The pattern was already triggered, so these don't affect continuous state
                break;
        }
    }
    return true;
}
