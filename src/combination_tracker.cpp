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
