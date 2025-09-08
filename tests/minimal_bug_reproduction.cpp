#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

// Exact copy of the bug logic
typedef void* XPLMCommandRef;
void XPLMDebugString(const char* string) { std::cout << string; }

namespace multibind::constants {
    static constexpr int MIN_BUTTON_ID = 0;
    static constexpr int MAX_BUTTON_ID = 999;
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
    MultibindBinding(const std::vector<ButtonTrigger>& triggers, const std::string& command)
        : button_triggers(triggers), target_command(command) {}
};

class MinimalTracker {
private:
    std::unordered_map<int, bool> _current_button_states;
    std::vector<MultibindBinding> _bindings;
    std::unordered_map<int, ButtonAction> _button_transitions;
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
        for (const auto& binding : _bindings) {
            if (check_pattern(binding.button_triggers)) {
                _triggered_command = binding.target_command;
                break;
            }
        }
        _button_transitions.clear();
    }

    std::string get_triggered_command() {
        std::string cmd = _triggered_command;
        _triggered_command.clear();
        return cmd;
    }

private:
    bool check_pattern(const std::vector<ButtonTrigger>& pattern) const {
        for (const auto& trigger : pattern) {
            auto trans_it = _button_transitions.find(trigger.button_id);
            
            if (trans_it == _button_transitions.end()) {
                // No transition recorded for this button
                if (trigger.action == ButtonAction::HELD) {
                    // For HELD, check current state
                    auto state_it = _current_button_states.find(trigger.button_id);
                    if (state_it == _current_button_states.end() || !state_it->second) {
                        return false; // Button not currently pressed
                    }
                } else {
                    // For PRESSED/RELEASED, we need a transition
                    return false;
                }
            } else {
                // We have a transition - check if it matches
                ButtonAction recorded = trans_it->second;
                if (trigger.action == ButtonAction::PRESSED) {
                    if (recorded != ButtonAction::PRESSED && recorded != ButtonAction::HELD) {
                        return false;
                    }
                } else if (trigger.action == ButtonAction::HELD) {
                    if (recorded != ButtonAction::HELD && recorded != ButtonAction::PRESSED) {
                        return false;
                    }
                } else if (trigger.action == ButtonAction::RELEASED) {
                    if (recorded != ButtonAction::RELEASED) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};

void test_exact_scenario() {
    std::cout << "Testing exact scenario from TODO-2.md\n";
    std::cout << "Pattern: *000+001=sim/pitch_trim_up\n\n";
    
    MinimalTracker tracker;
    std::vector<ButtonTrigger> pattern = {
        ButtonTrigger(0, ButtonAction::HELD),
        ButtonTrigger(1, ButtonAction::PRESSED)
    };
    tracker.set_bindings({MultibindBinding(pattern, "sim/pitch_trim_up")});
    
    std::cout << "Step 1: User holds button 000 and presses button 001\n";
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD); 
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    tracker.update();
    
    std::string result1 = tracker.get_triggered_command();
    std::cout << "Result: " << (result1.empty() ? "NO TRIGGER" : result1) << "\n\n";
    
    std::cout << "Step 2: User releases button 001\n";
    tracker.set_button_state_transition(1, ButtonAction::RELEASED);
    tracker.update();
    
    std::string result2 = tracker.get_triggered_command();
    std::cout << "Result: " << (result2.empty() ? "NO TRIGGER" : result2) << "\n\n";
    
    std::cout << "Step 3: User presses button 001 again (keeps 000 held)\n";
    std::cout << "Key insight: Button 000 gets NO new transition!\n";
    // Only button 1 gets a transition:
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    tracker.update();
    
    std::string result3 = tracker.get_triggered_command();
    std::cout << "Result: " << (result3.empty() ? "NO TRIGGER" : result3) << "\n";
    
    if (result3.empty()) {
        std::cout << "\n🐛 BUG REPRODUCED!\n";
        std::cout << "Reason: Pattern *000+001 requires BOTH:\n";
        std::cout << "- Button 0 in HELD state (✓ satisfied by current state)\n"; 
        std::cout << "- Button 1 with PRESSED transition (✓ satisfied)\n";
        std::cout << "But the check_pattern logic fails somewhere!\n";
    } else {
        std::cout << "\n✅ Works correctly - bug not reproduced\n";
    }
}

int main() {
    test_exact_scenario();
    return 0;
}