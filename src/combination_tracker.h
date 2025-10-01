#pragma once

#include "config.h"
#include "event_system.h"
#include "XPLMUtilities.h"
#include <unordered_map>
#include <set>
#include <string>
#include <chrono>
#include <queue>

class CombinationTracker {
public:
    CombinationTracker();
    ~CombinationTracker() = default;
    
    void set_button_state_transition(int button_id, ButtonAction action);
    void set_bindings(const std::vector<MultibindBinding>& bindings);
    
    void update();
    std::string get_triggered_command();
    
    // Get next continuous command action (command, start/stop)
    std::pair<std::string, bool> get_continuous_command_action();

    // Get next axis action (axis_id, target_dataref)
    std::pair<std::string, std::string> get_axis_action();

    // Get currently active axis bindings for continuous processing
    const std::unordered_map<std::string, std::string>& get_active_axis_bindings() const;
    
    // Stop all active continuous commands
    void stop_all_continuous_commands_real();
    
    const std::unordered_map<int, bool>& get_current_button_states() const;
    void clear_current_button_states();
    
    bool is_recording() const { return _recording; }
    void start_recording() { _recording = true; _recorded_combination.clear(); }
    void stop_recording() { _recording = false; }
    const std::set<int>& get_recorded_combination() const { return _recorded_combination; }
    
    // Legacy method for compatibility (no longer does anything meaningful)
    void stop_all_continuous_commands() { /* No-op in state machine system */ }
    
private:
    // State machine system replaces all the old trigger logic
    StateMachineManager _state_machine_manager;
    
    // Command queue for triggered commands
    std::queue<std::string> _triggered_commands;
    
    // Continuous command tracking
    std::unordered_map<std::string, XPLMCommandRef> _active_continuous_commands;
    std::queue<std::pair<std::string, bool>> _continuous_command_queue;  // command, start/stop

    // Axis binding tracking
    std::queue<std::pair<std::string, std::string>> _axis_action_queue;  // axis_id, target_dataref
    std::unordered_map<std::string, std::string> _active_axis_bindings;  // axis_id -> target_dataref (currently active)
    
    // UI recording support
    bool _recording = false;
    std::set<int> _recorded_combination;
    
    // Command callback for state machines
    void on_command_triggered(const std::string& command);
    void on_continuous_command_triggered(const std::string& command, bool start);
    void on_axis_triggered(const std::string& axis_id, const std::string& target_dataref);
};