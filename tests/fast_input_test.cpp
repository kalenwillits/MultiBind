#include <iostream>
#include <chrono>
#include <thread>

// This test simulates the exact scenario from TODO-2.md with fast input timing

// Mock X-Plane types
typedef void* XPLMCommandRef;
void XPLMDebugString(const char* string) { std::cout << "LOG: " << string; }
XPLMCommandRef XPLMFindCommand(const char* name) { return (XPLMCommandRef)0x12345678; }
void XPLMCommandBegin(XPLMCommandRef command) { }
void XPLMCommandEnd(XPLMCommandRef command) { }
void XPLMCommandOnce(XPLMCommandRef command) { }

// Mock constants
namespace multibind::constants {
    static constexpr int MIN_BUTTON_ID = 0;
    static constexpr int MAX_BUTTON_ID = 999;
    static constexpr int TRIGGER_DEBOUNCE_MS = 100;  // Real implementation uses 100ms
}

// Copy essential types
#include <vector>
#include <string>
#include <unordered_map>

enum class ButtonAction { PRESSED, HELD, RELEASED };

struct ButtonTrigger {
    int button_id;
    ButtonAction action;
    ButtonTrigger(int id, ButtonAction act) : button_id(id), action(act) {}
};

struct MultibindBinding {
    std::vector<ButtonTrigger> button_triggers;
    std::string target_command;
    std::string description;
    MultibindBinding(const std::vector<ButtonTrigger>& triggers, const std::string& command, const std::string& desc)
        : button_triggers(triggers), target_command(command), description(desc) {}
};

class RealisticCombinationTracker {
private:
    std::unordered_map<int, bool> _current_button_states;
    std::vector<MultibindBinding> _bindings;
    std::unordered_map<int, ButtonAction> _button_transitions;
    std::chrono::steady_clock::time_point _last_trigger_time{};
    std::string _triggered_command;

public:
    void set_button_state_transition(int button_id, ButtonAction action) {
        _button_transitions[button_id] = action;
        if (action == ButtonAction::PRESSED) {
            _current_button_states[button_id] = true;
        } else if (action == ButtonAction::RELEASED) {
            _current_button_states[button_id] = false;
        }
    }

    void set_bindings(const std::vector<MultibindBinding>& bindings) {
        _bindings = bindings;
    }

    void update() {
        auto now = std::chrono::steady_clock::now();
        
        for (const auto& binding : _bindings) {
            if (check_trigger_sequence_match(binding.button_triggers)) {
                // Use realistic debounce from the actual implementation
                auto debounce_time = std::chrono::milliseconds(50);  // has_held_actions = true
                
                if (now - _last_trigger_time > debounce_time) {
                    _triggered_command = binding.target_command;
                    _last_trigger_time = now;
                    std::cout << "✓ TRIGGERED: " << binding.target_command << std::endl;
                } else {
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _last_trigger_time);
                    std::cout << "✗ BLOCKED: debounce (" << elapsed.count() << "ms < " << debounce_time.count() << "ms)" << std::endl;
                }
                return;
            }
        }
        
        // Clear transitions after processing (frame-based)
        _button_transitions.clear();
    }

    std::string get_triggered_command() {
        std::string command = _triggered_command;
        _triggered_command.clear();
        return command;
    }

private:
    bool check_trigger_sequence_match(const std::vector<ButtonTrigger>& sequence) const {
        for (const auto& trigger : sequence) {
            auto transition_it = _button_transitions.find(trigger.button_id);
            
            if (transition_it == _button_transitions.end()) {
                if (trigger.action == ButtonAction::HELD) {
                    auto state_it = _current_button_states.find(trigger.button_id);
                    if (state_it == _current_button_states.end() || !state_it->second) {
                        return false;
                    }
                } else {
                    return false;
                }
            } else {
                ButtonAction recorded_action = transition_it->second;
                if (trigger.action == ButtonAction::PRESSED) {
                    if (recorded_action != ButtonAction::PRESSED && recorded_action != ButtonAction::HELD) {
                        return false;
                    }
                } else if (trigger.action == ButtonAction::HELD) {
                    if (recorded_action != ButtonAction::HELD && recorded_action != ButtonAction::PRESSED) {
                        return false;
                    }
                } else if (trigger.action == ButtonAction::RELEASED) {
                    if (recorded_action != ButtonAction::RELEASED) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};

void simulate_fast_user_input() {
    std::cout << "🎮 Simulating Fast User Input Scenario" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "User holds button 000 and rapidly taps button 001" << std::endl;
    std::cout << "Expected: Each tap should trigger the command" << std::endl;
    std::cout << "Actual: Only first tap works due to debounce timing" << std::endl;
    std::cout << std::endl;

    RealisticCombinationTracker tracker;
    
    // Create binding: *000+001=sim/pitch_trim_up
    std::vector<ButtonTrigger> triggers;
    triggers.emplace_back(0, ButtonAction::HELD);
    triggers.emplace_back(1, ButtonAction::PRESSED);
    MultibindBinding binding(triggers, "sim/pitch_trim_up", "Test");
    tracker.set_bindings({binding});

    std::cout << "⏱️  T=0ms: User holds button 000" << std::endl;
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD);
    tracker.update();
    
    std::cout << "⏱️  T=10ms: User presses button 001 (first time)" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    tracker.update();
    tracker.get_triggered_command(); // consume
    
    std::cout << "⏱️  T=20ms: User releases button 001" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tracker.set_button_state_transition(1, ButtonAction::RELEASED);
    tracker.update();
    
    std::cout << "⏱️  T=30ms: User presses button 001 (second time) - TOO FAST!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    tracker.update();
    
    std::cout << "\n🐛 BUG REPRODUCED: Second press blocked by debounce!" << std::endl;
    std::cout << "Real users would experience: trim doesn't actuate on rapid button presses" << std::endl;
    
    std::cout << "\nNow testing with realistic user timing (100ms between presses):" << std::endl;
    
    std::cout << "⏱️  T=150ms: User releases button 001 again" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    tracker.set_button_state_transition(1, ButtonAction::RELEASED);
    tracker.update();
    
    std::cout << "⏱️  T=250ms: User presses button 001 (third time) - GOOD TIMING" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    tracker.update();
    
    std::cout << "\n✅ Third press works because sufficient time elapsed" << std::endl;
}

int main() {
    simulate_fast_user_input();
    
    std::cout << "\n📋 CONCLUSION:" << std::endl;
    std::cout << "The bug is in debounce timing for ambiguous patterns." << std::endl;
    std::cout << "Current debounce = 50ms for patterns with HELD actions" << std::endl;
    std::cout << "Fast users can press buttons faster than 50ms intervals" << std::endl;
    std::cout << "State machine approach will eliminate this timing dependency" << std::endl;
    
    return 0;
}