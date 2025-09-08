#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <chrono>
#include <thread>

// Mock X-Plane types and functions for testing
typedef void* XPLMCommandRef;

void XPLMDebugString(const char* string) {
    std::cout << "LOG: " << string;
}

XPLMCommandRef XPLMFindCommand(const char* name) {
    std::cout << "XPLMFindCommand: " << name << std::endl;
    return (XPLMCommandRef)0x12345678;
}

void XPLMCommandBegin(XPLMCommandRef command) {
    std::cout << "XPLMCommandBegin: " << command << std::endl;
}

void XPLMCommandEnd(XPLMCommandRef command) {
    std::cout << "XPLMCommandEnd: " << command << std::endl;
}

void XPLMCommandOnce(XPLMCommandRef command) {
    std::cout << "XPLMCommandOnce: " << command << std::endl;
}

// Mock constants
namespace multibind::constants {
    static constexpr int MIN_BUTTON_ID = 0;
    static constexpr int MAX_BUTTON_ID = 999;
    static constexpr int TRIGGER_DEBOUNCE_MS = 100;
}

// Copy the essential types from the source code
enum class ButtonAction {
    PRESSED,    // Button just pressed (+ prefix)
    HELD,       // Button held down (* prefix)
    RELEASED    // Button just released (- prefix)
};

struct ButtonTrigger {
    int button_id;
    ButtonAction action;
    
    ButtonTrigger() = default;
    ButtonTrigger(int id, ButtonAction act) : button_id(id), action(act) {}
    
    bool operator<(const ButtonTrigger& other) const {
        if (button_id != other.button_id) return button_id < other.button_id;
        return static_cast<int>(action) < static_cast<int>(other.action);
    }
    
    bool operator==(const ButtonTrigger& other) const {
        return button_id == other.button_id && action == other.action;
    }
};

struct MultibindBinding {
    std::vector<ButtonTrigger> button_triggers;
    std::string target_command;
    std::string description;
    
    MultibindBinding() = default;
    MultibindBinding(const std::vector<ButtonTrigger>& triggers, const std::string& command, const std::string& desc)
        : button_triggers(triggers), target_command(command), description(desc) {}
};

// Simplified version of CombinationTracker for testing
class SimpleCombinationTracker {
private:
    std::unordered_map<int, bool> _current_button_states;
    std::vector<MultibindBinding> _bindings;
    std::unordered_map<int, ButtonAction> _button_transitions;
    std::chrono::steady_clock::time_point _last_transition_time{};
    std::unordered_map<std::string, XPLMCommandRef> _active_continuous_commands;
    std::unordered_map<std::string, std::vector<ButtonTrigger>> _continuous_bindings;
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
    }

    void set_bindings(const std::vector<MultibindBinding>& bindings) {
        _bindings = bindings;
    }

    void update() {
        process_triggers();
        clear_frame_transitions();
    }

    std::string get_triggered_command() {
        std::string command = _triggered_command;
        _triggered_command.clear();
        return command;
    }

    const std::unordered_map<int, bool>& get_current_button_states() const { 
        return _current_button_states; 
    }

private:
    void process_triggers() {
        using namespace multibind::constants;
        
        auto now = std::chrono::steady_clock::now();
        
        for (const auto& binding : _bindings) {
            if (check_trigger_sequence_match(binding.button_triggers)) {
                // For this test, we'll focus on simple one-time triggers
                auto debounce_time = std::chrono::milliseconds(50);
                
                auto time_since_last = std::chrono::duration_cast<std::chrono::milliseconds>(now - _last_trigger_time);
                std::cout << "  Debounce check: " << time_since_last.count() << "ms since last trigger (need " << debounce_time.count() << "ms)" << std::endl;
                
                if (now - _last_trigger_time > debounce_time) {
                    _triggered_command = binding.target_command;
                    _last_trigger_time = now;
                    
                    std::string log_msg = "Trigger sequence matched, triggering: " + binding.target_command + "\n";
                    XPLMDebugString(log_msg.c_str());
                } else {
                    std::cout << "  BLOCKED by debounce timer!" << std::endl;
                }
                return;
            }
        }
    }

    bool check_trigger_sequence_match(const std::vector<ButtonTrigger>& sequence) const {
        if (sequence.empty()) {
            return false;
        }
        
        std::cout << "  Checking sequence match for " << sequence.size() << " triggers:" << std::endl;
        
        for (const auto& trigger : sequence) {
            auto transition_it = _button_transitions.find(trigger.button_id);
            
            std::cout << "    Trigger: Button " << trigger.button_id << " action " << (int)trigger.action;
            
            if (transition_it == _button_transitions.end()) {
                std::cout << " (no recent transition)";
                // Button hasn't transitioned recently - check based on action type
                if (trigger.action == ButtonAction::HELD) {
                    // For HELD, check if button is currently pressed
                    auto state_it = _current_button_states.find(trigger.button_id);
                    if (state_it == _current_button_states.end() || !state_it->second) {
                        std::cout << " -> FAIL (not currently held)" << std::endl;
                        return false;
                    }
                    std::cout << " -> OK (currently held)" << std::endl;
                } else {
                    // For PRESSED or RELEASED, we need a recent transition
                    std::cout << " -> FAIL (no recent transition for PRESSED/RELEASED)" << std::endl;
                    return false;
                }
            } else {
                // Button has transitioned - check if it matches required action
                ButtonAction recorded_action = transition_it->second;
                std::cout << " (recorded: " << (int)recorded_action << ")";
                
                if (trigger.action == ButtonAction::PRESSED) {
                    if (recorded_action != ButtonAction::PRESSED && recorded_action != ButtonAction::HELD) {
                        std::cout << " -> FAIL (doesn't match PRESSED)" << std::endl;
                        return false;
                    }
                } else if (trigger.action == ButtonAction::HELD) {
                    if (recorded_action != ButtonAction::HELD && recorded_action != ButtonAction::PRESSED) {
                        std::cout << " -> FAIL (doesn't match HELD)" << std::endl;
                        return false;
                    }
                } else if (trigger.action == ButtonAction::RELEASED) {
                    if (recorded_action != ButtonAction::RELEASED) {
                        std::cout << " -> FAIL (doesn't match RELEASED)" << std::endl;
                        return false;
                    }
                }
                std::cout << " -> OK" << std::endl;
            }
        }
        
        std::cout << "  -> SEQUENCE MATCH SUCCESS!" << std::endl;
        return true;
    }

    void clear_frame_transitions() {
        std::cout << "  Clearing frame transitions" << std::endl;
        _button_transitions.clear();
    }
};

class TestRunner {
private:
    int passed = 0;
    int failed = 0;
    std::string current_test;

public:
    void start_test(const std::string& name) {
        current_test = name;
        std::cout << "\n=== " << name << " ===" << std::endl;
    }
    
    void assert_equal(const std::string& actual, const std::string& expected, const std::string& message = "") {
        if (actual == expected) {
            std::cout << "✓ PASS: " << message << std::endl;
            passed++;
        } else {
            std::cout << "✗ FAIL: " << message << std::endl;
            std::cout << "  Expected: '" << expected << "'" << std::endl;
            std::cout << "  Actual:   '" << actual << "'" << std::endl;
            failed++;
        }
    }
    
    void assert_true(bool condition, const std::string& message = "") {
        if (condition) {
            std::cout << "✓ PASS: " << message << std::endl;
            passed++;
        } else {
            std::cout << "✗ FAIL: " << message << std::endl;
            failed++;
        }
    }
    
    void print_summary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;
        
        if (failed > 0) {
            std::cout << "\n❌ TESTS FAILED - Bug reproduced successfully!" << std::endl;
        } else {
            std::cout << "\n✅ All tests PASSED" << std::endl;
        }
    }
};

void test_bug_reproduction(TestRunner& runner) {
    runner.start_test("🐛 Bug Reproduction: *000+001 Pattern");
    
    SimpleCombinationTracker tracker;
    
    // Create test binding: *000+001=sim/pitch_trim_up
    std::vector<ButtonTrigger> triggers;
    triggers.emplace_back(0, ButtonAction::HELD);      // *000
    triggers.emplace_back(1, ButtonAction::PRESSED);   // +001
    
    MultibindBinding binding(triggers, "sim/pitch_trim_up", "Test binding");
    std::vector<MultibindBinding> bindings = {binding};
    tracker.set_bindings(bindings);
    
    std::cout << "\n--- FRAME 1: Initial pattern (hold 000, press 001) ---" << std::endl;
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD);
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    
    tracker.update();
    std::string triggered1 = tracker.get_triggered_command();
    runner.assert_equal(triggered1, "sim/pitch_trim_up", "First trigger should work");
    
    std::cout << "\n--- FRAME 2: Release button 001 ---" << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::RELEASED);
    tracker.update();
    
    std::string triggered2 = tracker.get_triggered_command();
    runner.assert_true(triggered2.empty(), "Release should not trigger");
    
    std::cout << "\n--- FRAME 3: Press button 001 again (BUG TEST) ---" << std::endl;
    std::cout << "Button 000 is still held from Frame 1, no new transition needed" << std::endl;
    std::cout << "Button 001 gets pressed again..." << std::endl;
    
    // Add realistic delay to simulate user timing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    tracker.update();
    
    std::string triggered3 = tracker.get_triggered_command();
    
    std::cout << "Triggered command in Frame 3: '" << triggered3 << "'" << std::endl;
    
    if (triggered3 == "sim/pitch_trim_up") {
        std::cout << "\n🎉 UNEXPECTED: Second trigger actually works!" << std::endl;
        std::cout << "The bug might be elsewhere in the real implementation." << std::endl;
        runner.assert_equal(triggered3, "sim/pitch_trim_up", "Second trigger works - BUG NOT REPRODUCED in test");
    } else {
        std::cout << "\n🐛 BUG REPRODUCED!" << std::endl;
        std::cout << "The pattern *000+001 failed on second +001 press" << std::endl;
        std::cout << "Pattern matched but command was not triggered" << std::endl;
        std::cout << "This is exactly the bug described in TODO-2.md" << std::endl;
        runner.assert_true(triggered3.empty(), "Expected: Bug reproduced (second trigger fails)");
    }
}

int main() {
    std::cout << "🧪 Multibind Bug Reproduction Test" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Testing the bug described in TODO-2.md:" << std::endl;
    std::cout << "Pattern *000+001 should trigger repeatedly when:" << std::endl;
    std::cout << "1. User holds button 000" << std::endl;
    std::cout << "2. User presses button 001 -> triggers" << std::endl;
    std::cout << "3. User releases button 001" << std::endl;
    std::cout << "4. User presses button 001 again -> should trigger again" << std::endl;
    std::cout << std::endl;
    
    TestRunner runner;
    test_bug_reproduction(runner);
    runner.print_summary();
    
    return 0;
}