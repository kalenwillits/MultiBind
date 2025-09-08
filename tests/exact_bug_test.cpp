#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <unordered_map>

// Mock X-Plane
typedef void* XPLMCommandRef;
void XPLMDebugString(const char* string) { std::cout << "LOG: " << string; }
XPLMCommandRef XPLMFindCommand(const char* name) { return (XPLMCommandRef)0x12345678; }
void XPLMCommandOnce(XPLMCommandRef command) { std::cout << "COMMAND EXECUTED!" << std::endl; }

namespace multibind::constants {
    static constexpr int MIN_BUTTON_ID = 0;
    static constexpr int MAX_BUTTON_ID = 999;
    static constexpr int TRIGGER_DEBOUNCE_MS = 100;
}

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

// EXACT copy of the real implementation logic
class ExactCombinationTracker {
private:
    std::unordered_map<int, bool> _current_button_states;
    std::vector<MultibindBinding> _bindings;
    std::unordered_map<int, ButtonAction> _button_transitions;
    std::chrono::steady_clock::time_point _last_transition_time{};
    std::string _triggered_command;
    std::chrono::steady_clock::time_point _last_trigger_time{};

public:
    void set_button_state_transition(int button_id, ButtonAction action) {
        using namespace multibind::constants;
       
        if (button_id < MIN_BUTTON_ID || button_id > MAX_BUTTON_ID) {
            return;
        }
        
        // Record the transition for frame-based processing
        _button_transitions[button_id] = action;
        _last_transition_time = std::chrono::steady_clock::now();
        
        // Update current button states based on the action
        if (action == ButtonAction::PRESSED) {
            _current_button_states[button_id] = true;
        } else if (action == ButtonAction::RELEASED) {
            _current_button_states[button_id] = false;
        }
        // For HELD actions, state doesn't change (button remains pressed)
        
        std::cout << "  Button " << button_id << " -> " << (int)action 
                  << " (state now: " << _current_button_states[button_id] << ")" << std::endl;
    }

    void set_bindings(const std::vector<MultibindBinding>& bindings) {
        _bindings = bindings;
    }

    void update() {
        std::cout << "--- UPDATE() ---" << std::endl;
        process_triggers();
        clear_frame_transitions();
    }

    std::string get_triggered_command() {
        std::string command = _triggered_command;
        _triggered_command.clear();
        return command;
    }

    void print_state() {
        std::cout << "Button states: ";
        for (const auto& [button, pressed] : _current_button_states) {
            std::cout << button << "=" << pressed << " ";
        }
        std::cout << std::endl;
        
        std::cout << "Transitions: ";
        for (const auto& [button, action] : _button_transitions) {
            std::cout << button << "=" << (int)action << " ";
        }
        std::cout << std::endl;
    }

private:
    void process_triggers() {
        using namespace multibind::constants;
        
        auto now = std::chrono::steady_clock::now();
        
        for (const auto& binding : _bindings) {
            std::cout << "Checking binding: " << binding.target_command << std::endl;
            if (check_trigger_sequence_match(binding.button_triggers)) {
                std::cout << "PATTERN MATCHED!" << std::endl;
                
                // Check debounce (simplified - always use 0ms for testing)
                _triggered_command = binding.target_command;
                _last_trigger_time = now;
                
                std::cout << "COMMAND SET: " << binding.target_command << std::endl;
                return;
            } else {
                std::cout << "Pattern did not match" << std::endl;
            }
        }
    }

    bool check_trigger_sequence_match(const std::vector<ButtonTrigger>& sequence) const {
        if (sequence.empty()) {
            return false;
        }
        
        for (const auto& trigger : sequence) {
            std::cout << "  Checking trigger: Button " << trigger.button_id << " action " << (int)trigger.action << std::endl;
            
            auto transition_it = _button_transitions.find(trigger.button_id);
            
            if (transition_it == _button_transitions.end()) {
                std::cout << "    No recent transition for button " << trigger.button_id << std::endl;
                if (trigger.action == ButtonAction::HELD) {
                    auto state_it = _current_button_states.find(trigger.button_id);
                    if (state_it == _current_button_states.end() || !state_it->second) {
                        std::cout << "    HELD check FAILED: button not currently pressed" << std::endl;
                        return false;
                    }
                    std::cout << "    HELD check OK: button is currently pressed" << std::endl;
                } else {
                    std::cout << "    FAILED: PRESSED/RELEASED needs recent transition" << std::endl;
                    return false;
                }
            } else {
                ButtonAction recorded_action = transition_it->second;
                std::cout << "    Has transition: " << (int)recorded_action << std::endl;
                
                if (trigger.action == ButtonAction::PRESSED) {
                    if (recorded_action != ButtonAction::PRESSED && recorded_action != ButtonAction::HELD) {
                        std::cout << "    FAILED: doesn't match PRESSED" << std::endl;
                        return false;
                    }
                } else if (trigger.action == ButtonAction::HELD) {
                    if (recorded_action != ButtonAction::HELD && recorded_action != ButtonAction::PRESSED) {
                        std::cout << "    FAILED: doesn't match HELD" << std::endl;
                        return false;
                    }
                } else if (trigger.action == ButtonAction::RELEASED) {
                    if (recorded_action != ButtonAction::RELEASED) {
                        std::cout << "    FAILED: doesn't match RELEASED" << std::endl;
                        return false;
                    }
                }
                std::cout << "    OK: transition matches" << std::endl;
            }
        }
        
        return true;
    }

    void clear_frame_transitions() {
        std::cout << "Clearing frame transitions" << std::endl;
        _button_transitions.clear();
    }
};

int main() {
    std::cout << "🔍 EXACT Bug Reproduction Test" << std::endl;
    std::cout << "Tracing through the exact scenario from TODO-2.md" << std::endl;
    std::cout << "Pattern: *000+001=sim/pitch_trim_up" << std::endl;
    std::cout << std::endl;

    ExactCombinationTracker tracker;
    
    // Set up the binding
    std::vector<ButtonTrigger> triggers;
    triggers.emplace_back(0, ButtonAction::HELD);      // *000
    triggers.emplace_back(1, ButtonAction::PRESSED);   // +001
    MultibindBinding binding(triggers, "sim/pitch_trim_up", "Test");
    tracker.set_bindings({binding});

    std::cout << "=== SCENARIO 1: Initial hold + press ===" << std::endl;
    std::cout << "User holds button 000..." << std::endl;
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD);
    
    std::cout << "User presses button 001..." << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    
    tracker.print_state();
    tracker.update();
    
    std::string result1 = tracker.get_triggered_command();
    std::cout << "Result: '" << result1 << "'" << std::endl;
    std::cout << std::endl;

    std::cout << "=== SCENARIO 2: Release button 001 ===" << std::endl;
    std::cout << "User releases button 001..." << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::RELEASED);
    
    tracker.print_state();
    tracker.update();
    
    std::string result2 = tracker.get_triggered_command();
    std::cout << "Result: '" << result2 << "'" << std::endl;
    std::cout << std::endl;

    std::cout << "=== SCENARIO 3: Press button 001 again (THE BUG) ===" << std::endl;
    std::cout << "User presses button 001 again..." << std::endl;
    std::cout << "(Button 000 is still held from before)" << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    
    tracker.print_state();
    tracker.update();
    
    std::string result3 = tracker.get_triggered_command();
    std::cout << "Result: '" << result3 << "'" << std::endl;
    
    if (result3 == "sim/pitch_trim_up") {
        std::cout << "\n✅ Second press worked - no bug reproduced" << std::endl;
    } else {
        std::cout << "\n🐛 BUG REPRODUCED: Second press failed!" << std::endl;
    }

    return 0;
}