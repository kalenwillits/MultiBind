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
    
    // Record the transition and timestamp
    _button_transitions[button_id] = action;
    _last_transition_time = std::chrono::steady_clock::now();
    
    // Process enhanced trigger patterns
    process_enhanced_triggers();
}

void CombinationTracker::set_bindings(const std::vector<MultibindBinding>& bindings)
{
    _bindings = bindings;
}

void CombinationTracker::update()
{
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
            // Small delay to prevent accidental double-triggering
            if (now - _last_trigger_time > std::chrono::milliseconds(TRIGGER_DEBOUNCE_MS)) {
                _triggered_command = binding.target_command;
                _last_trigger_time = now;
                
                std::string log_msg = "Multibind: Enhanced trigger sequence matched, triggering: " + binding.target_command + "\n";
                XPLMDebugString(log_msg.c_str());
                
                // Clear transitions after successful match to prevent re-triggering
                _button_transitions.clear();
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
            // Button hasn't transitioned - check if it's in the required state
            if (trigger.action == ButtonAction::HELD) {
                // For HELD, check if button is currently pressed
                if (_currently_pressed.find(trigger.button_id) == _currently_pressed.end()) {
                    return false;
                }
            } else {
                // For PRESSED or RELEASED, we need an actual transition
                return false;
            }
        } else {
            // Button has transitioned - check if it matches required action
            if (transition_it->second != trigger.action) {
                return false;
            }
        }
    }
    
    return true;
}
