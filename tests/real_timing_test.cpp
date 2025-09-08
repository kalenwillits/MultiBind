#include <iostream>
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

// Simulation of the REAL timing behavior
class RealisticTimingTracker {
private:
    std::unordered_map<int, bool> _current_button_states;
    std::vector<MultibindBinding> _bindings;
    std::unordered_map<int, ButtonAction> _button_transitions;  // OVERWRITES previous transitions!
    std::string _triggered_command;

public:
    void set_button_state_transition(int button_id, ButtonAction action) {
        // This is the CRITICAL bug: overwrites previous transitions
        _button_transitions[button_id] = action;
        
        if (action == ButtonAction::PRESSED) {
            _current_button_states[button_id] = true;
        } else if (action == ButtonAction::RELEASED) {
            _current_button_states[button_id] = false;
        }
        
        std::cout << "  Transition recorded: Button " << button_id << " -> " << (int)action << std::endl;
    }

    void set_bindings(const std::vector<MultibindBinding>& bindings) {
        _bindings = bindings;
    }

    void update() {
        std::cout << "--- FLIGHT LOOP UPDATE() ---" << std::endl;
        process_triggers();
        clear_frame_transitions();
    }

    std::string get_triggered_command() {
        std::string command = _triggered_command;
        _triggered_command.clear();
        return command;
    }

private:
    void process_triggers() {
        for (const auto& binding : _bindings) {
            if (check_trigger_sequence_match(binding.button_triggers)) {
                _triggered_command = binding.target_command;
                std::cout << "COMMAND SET: " << binding.target_command << std::endl;
                return;
            }
        }
    }

    bool check_trigger_sequence_match(const std::vector<ButtonTrigger>& sequence) const {
        for (const auto& trigger : sequence) {
            std::cout << "  Checking: Button " << trigger.button_id << " action " << (int)trigger.action << std::endl;
            
            auto transition_it = _button_transitions.find(trigger.button_id);
            
            if (transition_it == _button_transitions.end()) {
                if (trigger.action == ButtonAction::HELD) {
                    auto state_it = _current_button_states.find(trigger.button_id);
                    if (state_it == _current_button_states.end() || !state_it->second) {
                        std::cout << "    FAIL: HELD but not currently pressed" << std::endl;
                        return false;
                    }
                    std::cout << "    OK: HELD and currently pressed" << std::endl;
                } else {
                    std::cout << "    FAIL: Need recent transition for PRESSED/RELEASED" << std::endl;
                    return false;
                }
            } else {
                ButtonAction recorded_action = transition_it->second;
                std::cout << "    Recorded action: " << (int)recorded_action << std::endl;
                
                if (trigger.action == ButtonAction::PRESSED && 
                    recorded_action != ButtonAction::PRESSED && recorded_action != ButtonAction::HELD) {
                    std::cout << "    FAIL: doesn't match PRESSED" << std::endl;
                    return false;
                }
                std::cout << "    OK: action matches" << std::endl;
            }
        }
        return true;
    }

    void clear_frame_transitions() {
        std::cout << "Clearing transitions" << std::endl;
        _button_transitions.clear();
    }
};

void simulate_real_x_plane_timing() {
    std::cout << "🎮 Simulating REAL X-Plane Timing" << std::endl;
    std::cout << "=================================" << std::endl;
    
    RealisticTimingTracker tracker;
    
    // Set up binding: *000+001=sim/pitch_trim_up
    std::vector<ButtonTrigger> triggers;
    triggers.emplace_back(0, ButtonAction::HELD);
    triggers.emplace_back(1, ButtonAction::PRESSED);
    MultibindBinding binding(triggers, "sim/pitch_trim_up", "Test");
    tracker.set_bindings({binding});

    std::cout << "\n=== FIRST SEQUENCE: Works fine ===" << std::endl;
    std::cout << "X-Plane command handler calls:" << std::endl;
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD);
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    
    std::cout << "Flight loop runs..." << std::endl;
    tracker.update();
    std::string result1 = tracker.get_triggered_command();
    std::cout << "Result: " << (result1.empty() ? "NONE" : result1) << std::endl;

    std::cout << "\n=== SECOND SEQUENCE: The bug scenario ===" << std::endl;
    std::cout << "User releases button 001..." << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::RELEASED);
    tracker.update();
    tracker.get_triggered_command(); // consume
    
    std::cout << "\nNow user presses button 001 again..." << std::endl;
    std::cout << "BUT - what if multiple transitions happen before flight loop?" << std::endl;
    
    std::cout << "\nScenario A: Single transition (should work)" << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    tracker.update();
    std::string resultA = tracker.get_triggered_command();
    std::cout << "Result A: " << (resultA.empty() ? "NONE" : resultA) << std::endl;
    
    std::cout << "\nScenario B: Multiple rapid transitions (the real bug)" << std::endl;
    std::cout << "What if button gets pressed AND immediately held before flight loop runs?" << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::RELEASED);
    tracker.update(); // clear
    tracker.get_triggered_command();
    
    // This simulates rapid button events that happen between flight loop calls
    std::cout << "Rapid sequence (all before flight loop):" << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);  // This gets OVERWRITTEN
    tracker.set_button_state_transition(1, ButtonAction::HELD);     // This is what remains
    
    tracker.update();
    std::string resultB = tracker.get_triggered_command();
    std::cout << "Result B: " << (resultB.empty() ? "NONE" : resultB) << std::endl;
    
    if (resultB.empty()) {
        std::cout << "\n🐛 BUG FOUND: Pattern expects PRESSED but only HELD was recorded!" << std::endl;
        std::cout << "The PRESSED transition was overwritten by HELD transition" << std::endl;
    }
}

int main() {
    simulate_real_x_plane_timing();
    return 0;
}