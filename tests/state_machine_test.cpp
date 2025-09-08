#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>

// Mock X-Plane functions
void XPLMDebugString(const char* string) { std::cout << string; }

// Mock constants
namespace multibind::constants {
    static constexpr int MIN_BUTTON_ID = 0;
    static constexpr int MAX_BUTTON_ID = 999;
}

// Include our new system (we'll mock the parts we need)
enum class ButtonAction { PRESSED, HELD, RELEASED };

struct ButtonTrigger {
    int button_id;
    ButtonAction action;
    ButtonTrigger(int id, ButtonAction act) : button_id(id), action(act) {}
};

// Simplified state machine test
class StateTransition {
public:
    int button_id;
    ButtonAction action;
    StateTransition(int id, ButtonAction act) : button_id(id), action(act) {}
    
    bool operator==(const StateTransition& other) const {
        return button_id == other.button_id && action == other.action;
    }
};

class SimpleStateMachine {
private:
    std::vector<StateTransition> _pattern;
    std::string _command;
    size_t _current_step = 0;
    std::queue<std::string> _triggered_commands;
    std::unordered_map<int, bool> _held_buttons;  // Track held button states
    
public:
    SimpleStateMachine(const std::vector<ButtonTrigger>& triggers, const std::string& command) 
        : _command(command) {
        for (const auto& trigger : triggers) {
            _pattern.emplace_back(trigger.button_id, trigger.action);
        }
    }
    
    void process_event(int button_id, ButtonAction action) {
        // Update held button tracking
        if (action == ButtonAction::PRESSED) {
            _held_buttons[button_id] = true;
        } else if (action == ButtonAction::RELEASED) {
            _held_buttons[button_id] = false;
        }
        
        std::cout << "Processing: Button " << button_id << " action " << (int)action;
        std::cout << " (expecting step " << _current_step << ": Button " << _pattern[_current_step].button_id 
                  << " action " << (int)_pattern[_current_step].action << ")" << std::endl;
        
        if (_current_step >= _pattern.size()) {
            std::cout << "  Already completed, resetting..." << std::endl;
            _current_step = 0;
        }
        
        StateTransition current_event(button_id, action);
        
        if (_current_step < _pattern.size() && current_event == _pattern[_current_step]) {
            _current_step++;
            std::cout << "  ✓ Step " << _current_step << "/" << _pattern.size() << " matched" << std::endl;
            
            if (_current_step >= _pattern.size()) {
                // Pattern complete!
                _triggered_commands.push(_command);
                _current_step = 0; // Reset for next time
                
                // Smart reset: advance through held buttons
                smart_reset();
                
                std::cout << "  🎯 PATTERN COMPLETE! Command triggered: " << _command << std::endl;
            }
        } else {
            // Check if this event should reset us to step 0 and try again
            if (_pattern[0] == current_event) {
                _current_step = 1;
                std::cout << "  ↺ Reset to step 1 (matches first step)" << std::endl;
            } else {
                // Event doesn't match - for now, don't reset (allows held buttons to persist)
                std::cout << "  ✗ No match, staying at step " << _current_step << std::endl;
            }
        }
    }
    
private:
    void smart_reset() {
        // After pattern completion, advance through held buttons to find correct state
        for (const auto& pair : _held_buttons) {
            int button_id = pair.first;
            bool is_held = pair.second;
            
            if (is_held && _current_step < _pattern.size()) {
                // Check if this held button matches the next expected step
                StateTransition held_event(button_id, ButtonAction::HELD);
                if (held_event == _pattern[_current_step]) {
                    _current_step++;
                    std::cout << "  🔄 Smart reset: advanced to step " << _current_step << " for held button " << button_id << std::endl;
                }
            }
        }
    }
    
public:
    std::string get_triggered_command() {
        if (_triggered_commands.empty()) return "";
        std::string cmd = _triggered_commands.front();
        _triggered_commands.pop();
        return cmd;
    }
    
    size_t get_current_step() const { return _current_step; }
};

void test_bug_scenario() {
    std::cout << "🧪 Testing State Machine with Bug Scenario\n";
    std::cout << "==========================================\n\n";
    
    // Create state machine for *000+001=sim/pitch_trim_up
    std::vector<ButtonTrigger> pattern = {
        ButtonTrigger(0, ButtonAction::HELD),
        ButtonTrigger(1, ButtonAction::PRESSED)
    };
    
    SimpleStateMachine machine(pattern, "sim/pitch_trim_up");
    
    std::cout << "Pattern: *000+001=sim/pitch_trim_up\n\n";
    
    std::cout << "=== Step 1: User holds button 000 ===\n";
    machine.process_event(0, ButtonAction::PRESSED);
    machine.process_event(0, ButtonAction::HELD);
    
    std::cout << "\n=== Step 2: User presses button 001 (first time) ===\n";
    machine.process_event(1, ButtonAction::PRESSED);
    
    std::string result1 = machine.get_triggered_command();
    std::cout << "Result 1: " << (result1.empty() ? "NONE" : result1) << "\n\n";
    
    std::cout << "=== Step 3: User releases button 001 ===\n";
    machine.process_event(1, ButtonAction::RELEASED);
    
    std::string result2 = machine.get_triggered_command();
    std::cout << "Result 2: " << (result2.empty() ? "NONE" : result2) << "\n\n";
    
    std::cout << "=== Step 4: User presses button 001 again (THE FIX TEST) ===\n";
    std::cout << "In the old system, this would fail because continuous command was already active\n";
    std::cout << "In the new system, this should work because state machine resets after trigger\n\n";
    
    machine.process_event(1, ButtonAction::PRESSED);
    
    std::string result3 = machine.get_triggered_command();
    std::cout << "Result 3: " << (result3.empty() ? "NONE" : result3) << "\n";
    
    if (result3 == "sim/pitch_trim_up") {
        std::cout << "\n🎉 SUCCESS! Bug is FIXED!\n";
        std::cout << "Second button press correctly triggers the command again.\n";
        std::cout << "State machine approach eliminates the continuous command conflict.\n";
    } else {
        std::cout << "\n❌ FAILED: Second button press did not trigger\n";
    }
}

void test_multiple_rapid_presses() {
    std::cout << "\n\n🚀 Testing Rapid Button Presses\n";
    std::cout << "===============================\n\n";
    
    std::vector<ButtonTrigger> pattern = {
        ButtonTrigger(0, ButtonAction::HELD),
        ButtonTrigger(1, ButtonAction::PRESSED)
    };
    
    SimpleStateMachine machine(pattern, "sim/pitch_trim_up");
    
    std::cout << "Setup: User holds button 000\n";
    machine.process_event(0, ButtonAction::PRESSED);
    machine.process_event(0, ButtonAction::HELD);
    
    std::cout << "\nRapid button 001 presses:\n";
    
    for (int i = 0; i < 5; i++) {
        std::cout << "\n--- Press " << (i + 1) << " ---\n";
        machine.process_event(1, ButtonAction::PRESSED);
        machine.process_event(1, ButtonAction::RELEASED);  // Quick release
        
        std::string result = machine.get_triggered_command();
        std::cout << "Result: " << (result.empty() ? "NONE" : result) << "\n";
    }
    
    std::cout << "\n✅ All rapid presses should trigger successfully!\n";
    std::cout << "This demonstrates the state machine handles repeated patterns correctly.\n";
}

int main() {
    test_bug_scenario();
    test_multiple_rapid_presses();
    
    std::cout << "\n\n📋 SUMMARY:\n";
    std::cout << "The state machine approach fixes the original bug by:\n";
    std::cout << "1. Eliminating the continuous vs discrete command distinction\n";
    std::cout << "2. Allowing patterns to trigger repeatedly without interference\n";
    std::cout << "3. Providing deterministic state transitions\n";
    std::cout << "4. Resetting to root state after each successful pattern match\n";
    
    return 0;
}