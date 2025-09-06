#pragma once

#include "config.h"
#include <unordered_map>
#include <set>
#include <string>
#include <chrono>

class CombinationTracker {
public:
    CombinationTracker() = default;
    ~CombinationTracker() = default;
    
    void set_button_pressed(int button_id, bool pressed);
    void set_button_state_transition(int button_id, ButtonAction action);
    void set_bindings(const std::vector<MultibindBinding>& bindings);
    
    void update();
    std::string get_triggered_command();
    
    const std::set<int>& get_currently_pressed_buttons() const { return _currently_pressed; }
    void clear_currently_pressed() { _currently_pressed.clear(); }
    
    bool is_recording() const { return _recording; }
    void start_recording() { _recording = true; _recorded_combination.clear(); }
    void stop_recording() { _recording = false; }
    const std::set<int>& get_recorded_combination() const { return _recorded_combination; }
    
private:
    std::unordered_map<int, bool> _button_states;  // button_id -> pressed
    std::set<int> _currently_pressed;              // Currently pressed buttons
    std::vector<MultibindBinding> _bindings;       // Current bindings to check against
    
    // Enhanced state tracking for new trigger system
    std::unordered_map<int, ButtonAction> _button_transitions;  // Latest button state transitions
    std::chrono::steady_clock::time_point _last_transition_time{};
    
    std::string _triggered_command;                // Command to execute this frame
    
    bool _recording = false;                       // Whether we're recording a new combination
    std::set<int> _recorded_combination;           // Buttons recorded during recording mode
    
    std::chrono::steady_clock::time_point _last_combination_time;
    std::chrono::steady_clock::time_point _last_trigger_time{};
    static constexpr auto COMBINATION_TIMEOUT = std::chrono::milliseconds(100);
    
    bool check_combination_match(const std::set<int>& pressed, const std::set<int>& target) const;
    void process_combination_change();
    void process_enhanced_triggers();
    bool check_trigger_sequence_match(const std::vector<ButtonTrigger>& sequence) const;
};