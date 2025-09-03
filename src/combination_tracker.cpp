#include "combination_tracker.h"
#include "XPLMUtilities.h"

void CombinationTracker::set_button_pressed(int button_id, bool pressed)
{
    if (button_id < 0 || button_id >= 1000) {
        return;
    }
    
    bool was_pressed = button_states_[button_id];
    button_states_[button_id] = pressed;
    
    if (pressed && !was_pressed) {
        currently_pressed_.insert(button_id);
        if (recording_) {
            recorded_combination_.insert(button_id);
        }
    } else if (!pressed && was_pressed) {
        currently_pressed_.erase(button_id);
    }
    
    process_combination_change();
}

void CombinationTracker::set_bindings(const std::vector<MultibindBinding>& bindings)
{
    bindings_ = bindings;
}

void CombinationTracker::update()
{
    // Clear any triggered command from previous frame
    triggered_command_.clear();
}

std::string CombinationTracker::get_triggered_command()
{
    std::string command = triggered_command_;
    triggered_command_.clear(); // Only trigger once
    return command;
}

void CombinationTracker::process_combination_change()
{
    auto now = std::chrono::steady_clock::now();
    
    // Only process if we have buttons pressed
    if (currently_pressed_.empty()) {
        return;
    }
    
    // Check if current combination matches any binding
    for (const auto& binding : bindings_) {
        if (check_combination_match(currently_pressed_, binding.button_combination)) {
            // Found a match - but wait a bit to see if more buttons are coming
            last_combination_time_ = now;
            
            // If this is an exact match and we've waited long enough, trigger it
            if (currently_pressed_ == binding.button_combination) {
                // Small delay to prevent accidental double-triggering
                static auto last_trigger = std::chrono::steady_clock::time_point{};
                if (now - last_trigger > std::chrono::milliseconds(200)) {
                    triggered_command_ = binding.target_command;
                    last_trigger = now;
                    
                    char log_msg[256];
                    snprintf(log_msg, sizeof(log_msg), "Multibind: Combination matched, triggering: %s\n", 
                            binding.target_command.c_str());
                    XPLMDebugString(log_msg);
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