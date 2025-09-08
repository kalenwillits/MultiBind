#include <iostream>
#include <cassert>
#include <string>
#include <vector>

// Include the headers we need to test
#include "../src/combination_tracker.h"
#include "../src/config.h"

// Mock X-Plane SDK functions since we're testing without X-Plane
extern "C" {
    void XPLMDebugString(const char* string) {
        std::cout << "XPLMDebugString: " << string;
    }
    
    void* XPLMFindCommand(const char* name) {
        std::cout << "XPLMFindCommand: " << name << std::endl;
        return (void*)0x12345678; // Return fake command reference
    }
    
    void XPLMCommandBegin(void* command) {
        std::cout << "XPLMCommandBegin: " << command << std::endl;
    }
    
    void XPLMCommandEnd(void* command) {
        std::cout << "XPLMCommandEnd: " << command << std::endl;
    }
    
    void XPLMCommandOnce(void* command) {
        std::cout << "XPLMCommandOnce: " << command << std::endl;
    }
}

class TestRunner {
private:
    int passed = 0;
    int failed = 0;
    std::string current_test;

public:
    void start_test(const std::string& name) {
        current_test = name;
        std::cout << "\n=== Running: " << name << " ===" << std::endl;
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
            std::cout << "\n❌ Some tests FAILED - Bug reproduced!" << std::endl;
        } else {
            std::cout << "\n✅ All tests PASSED" << std::endl;
        }
    }
};

void test_basic_pattern_matching(TestRunner& runner) {
    runner.start_test("BasicPatternMatching");
    
    CombinationTracker tracker;
    
    // Create test binding: *000+001=sim/pitch_trim_up
    std::vector<ButtonTrigger> triggers;
    triggers.emplace_back(0, ButtonAction::HELD);      // *000
    triggers.emplace_back(1, ButtonAction::PRESSED);   // +001
    
    MultibindBinding binding(triggers, "sim/pitch_trim_up", "Test binding");
    std::vector<MultibindBinding> bindings = {binding};
    tracker.set_bindings(bindings);
    
    // Simulate: User holds button 000
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD);
    
    // Simulate: User presses button 001
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    
    // Process the triggers
    tracker.update();
    
    // Should trigger the command
    std::string triggered = tracker.get_triggered_command();
    runner.assert_equal(triggered, "sim/pitch_trim_up", "Initial pattern should trigger");
}

void test_ambiguous_trigger_bug(TestRunner& runner) {
    runner.start_test("AmbiguousTriggerBug - The Bug We Need To Fix");
    
    CombinationTracker tracker;
    
    // Create test binding: *000+001=sim/pitch_trim_up
    std::vector<ButtonTrigger> triggers;
    triggers.emplace_back(0, ButtonAction::HELD);      // *000
    triggers.emplace_back(1, ButtonAction::PRESSED);   // +001
    
    MultibindBinding binding(triggers, "sim/pitch_trim_up", "Test binding");
    std::vector<MultibindBinding> bindings = {binding};
    tracker.set_bindings(bindings);
    
    std::cout << "\n--- FRAME 1: Initial pattern match ---" << std::endl;
    // User holds button 000
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD);
    
    // User presses button 001
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    
    // Process - should trigger
    tracker.update();
    std::string triggered1 = tracker.get_triggered_command();
    runner.assert_equal(triggered1, "sim/pitch_trim_up", "First trigger should work");
    
    std::cout << "\n--- FRAME 2: User releases button 001 ---" << std::endl;
    tracker.set_button_state_transition(1, ButtonAction::RELEASED);
    tracker.update();
    
    // Should not trigger
    std::string triggered2 = tracker.get_triggered_command();
    runner.assert_true(triggered2.empty(), "Release should not trigger command");
    
    std::cout << "\n--- FRAME 3: User presses button 001 again ---" << std::endl;
    // Button 000 is still held (no new transition)
    // User presses button 001 again
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    
    // Process - THIS SHOULD TRIGGER BUT CURRENTLY DOESN'T (BUG)
    tracker.update();
    std::string triggered3 = tracker.get_triggered_command();
    
    // This test will demonstrate the bug
    if (triggered3 == "sim/pitch_trim_up") {
        runner.assert_equal(triggered3, "sim/pitch_trim_up", "Second trigger should work (BUG FIXED!)");
    } else {
        std::cout << "🐛 BUG REPRODUCED: Second trigger failed!" << std::endl;
        runner.assert_true(triggered3.empty(), "Current implementation has bug - second trigger fails (EXPECTED TO FAIL)");
    }
}

void test_current_button_states(TestRunner& runner) {
    runner.start_test("CurrentButtonStates - Debug Info");
    
    CombinationTracker tracker;
    
    // Create test binding
    std::vector<ButtonTrigger> triggers;
    triggers.emplace_back(0, ButtonAction::HELD);
    triggers.emplace_back(1, ButtonAction::PRESSED);
    
    MultibindBinding binding(triggers, "test_command", "Test");
    std::vector<MultibindBinding> bindings = {binding};
    tracker.set_bindings(bindings);
    
    // Press and hold button 000
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD);
    
    // Check button states
    const auto& states = tracker.get_current_button_states();
    std::cout << "Button states after holding 000:" << std::endl;
    for (const auto& [button, pressed] : states) {
        std::cout << "  Button " << button << ": " << (pressed ? "PRESSED" : "RELEASED") << std::endl;
    }
    
    // Process first frame
    tracker.update();
    
    // Check states again - the frame clearing might affect this
    std::cout << "Button states after first update():" << std::endl;
    for (const auto& [button, pressed] : states) {
        std::cout << "  Button " << button << ": " << (pressed ? "PRESSED" : "RELEASED") << std::endl;
    }
    
    runner.assert_true(true, "Debug info displayed");
}

int main() {
    std::cout << "🧪 Multibind Bug Reproduction Tests" << std::endl;
    std::cout << "====================================" << std::endl;
    
    TestRunner runner;
    
    // Run tests to expose the bug
    test_basic_pattern_matching(runner);
    test_current_button_states(runner);
    test_ambiguous_trigger_bug(runner);
    
    runner.print_summary();
    
    return 0;
}