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
    void set_bindings(const std::vector<MultibindBinding>& bindings);
    
    void update();
    std::string get_triggered_command();
    
    const std::set<int>& get_currently_pressed_buttons() const { return currently_pressed_; }
    void clear_currently_pressed() { currently_pressed_.clear(); }
    
    bool is_recording() const { return recording_; }
    void start_recording() { recording_ = true; recorded_combination_.clear(); }
    void stop_recording() { recording_ = false; }
    const std::set<int>& get_recorded_combination() const { return recorded_combination_; }
    
private:
    std::unordered_map<int, bool> button_states_;  // button_id -> pressed
    std::set<int> currently_pressed_;              // Currently pressed buttons
    std::vector<MultibindBinding> bindings_;       // Current bindings to check against
    
    std::string triggered_command_;                // Command to execute this frame
    
    bool recording_ = false;                       // Whether we're recording a new combination
    std::set<int> recorded_combination_;           // Buttons recorded during recording mode
    
    std::chrono::steady_clock::time_point last_combination_time_;
    static constexpr auto COMBINATION_TIMEOUT = std::chrono::milliseconds(100);
    
    bool check_combination_match(const std::set<int>& pressed, const std::set<int>& target) const;
    void process_combination_change();
};