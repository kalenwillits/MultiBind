#pragma once

#include "config.h"
#include "XPLMUtilities.h"
#include <unordered_map>
#include <set>
#include <string>
#include <chrono>

class CombinationTracker {
public:
    CombinationTracker() = default;
    ~CombinationTracker() = default;
    
    void set_button_state_transition(int button_id, ButtonAction action);
    void set_bindings(const std::vector<MultibindBinding>& bindings);
    
    void update();
    std::string get_triggered_command();
    
    const std::unordered_map<int, bool>& get_current_button_states() const { return _current_button_states; }
    void clear_current_button_states() { _current_button_states.clear(); }
    
    bool is_recording() const { return _recording; }
    void start_recording() { _recording = true; _recorded_combination.clear(); }
    void stop_recording() { _recording = false; }
    const std::set<int>& get_recorded_combination() const { return _recorded_combination; }
    
    // Public cleanup method
    void stop_all_continuous_commands();
    
private:
    std::unordered_map<int, bool> _current_button_states;  // button_id -> currently pressed
    std::vector<MultibindBinding> _bindings;               // Current bindings to check against
    
    // Button state tracking for trigger system
    std::unordered_map<int, ButtonAction> _button_transitions;  // Latest button state transitions
    std::chrono::steady_clock::time_point _last_transition_time{};
    
    // Continuous command execution tracking
    std::unordered_map<std::string, XPLMCommandRef> _active_continuous_commands;  // Commands currently running
    std::unordered_map<std::string, std::vector<ButtonTrigger>> _continuous_bindings; // Commands that should run continuously
    
    std::string _triggered_command;                // Command to execute this frame
    
    bool _recording = false;                       // Whether we're recording a new combination
    std::set<int> _recorded_combination;           // Buttons recorded during recording mode
    
    std::chrono::steady_clock::time_point _last_trigger_time{};
    
    void process_triggers();
    bool check_trigger_sequence_match(const std::vector<ButtonTrigger>& sequence) const;
    
    // Continuous command management
    void start_continuous_command(const std::string& command, const std::vector<ButtonTrigger>& triggers);
    void stop_continuous_command(const std::string& command);
    void update_continuous_commands();
    void clear_frame_transitions();
    bool should_run_continuously(const std::vector<ButtonTrigger>& triggers) const;
    bool is_continuous_pattern_active(const std::vector<ButtonTrigger>& triggers) const;
};