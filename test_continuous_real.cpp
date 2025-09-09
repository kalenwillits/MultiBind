#include <iostream>
#include <vector>
#include <string>

// Mock X-Plane API to capture what actually gets called
std::vector<std::string> xplm_begin_calls;
std::vector<std::string> xplm_end_calls;
std::vector<std::string> xplm_once_calls;

typedef void* XPLMCommandRef;

void XPLMDebugString(const char* message) {
    std::cout << "DEBUG: " << message;
}

XPLMCommandRef XPLMFindCommand(const char* command) {
    return (XPLMCommandRef)command;
}

void XPLMCommandBegin(XPLMCommandRef command) {
    std::string cmd = (char*)command;
    xplm_begin_calls.push_back(cmd);
    std::cout << "CONTINUOUS START: " << cmd << std::endl;
}

void XPLMCommandEnd(XPLMCommandRef command) {
    std::string cmd = (char*)command;
    xplm_end_calls.push_back(cmd);
    std::cout << "CONTINUOUS STOP:  " << cmd << std::endl;
}

void XPLMCommandOnce(XPLMCommandRef command) {
    std::string cmd = (char*)command;
    xplm_once_calls.push_back(cmd);
    std::cout << "ONE-TIME TRIGGER: " << cmd << std::endl;
}

// Include the real implementation
#include "src/config.h"
#include "src/event_system.h"  
#include "src/combination_tracker.h"

void simulate_flight_loop(CombinationTracker& tracker) {
    // Simulate what the real flight loop does
    tracker.update();
    
    // Handle one-time commands
    std::string triggered = tracker.get_triggered_command();
    if (!triggered.empty()) {
        XPLMCommandRef ref = XPLMFindCommand(triggered.c_str());
        XPLMCommandOnce(ref);
    }
    
    // Handle continuous commands
    auto continuous = tracker.get_continuous_command_action();
    if (!continuous.first.empty()) {
        XPLMCommandRef ref = XPLMFindCommand(continuous.first.c_str());
        if (continuous.second) {
            XPLMCommandBegin(ref);
        } else {
            XPLMCommandEnd(ref);
        }
    }
}

int main() {
    std::cout << "TESTING REAL CONTINUOUS COMMANDS IMPLEMENTATION\n";
    std::cout << "===============================================\n\n";
    
    CombinationTracker tracker;
    
    // Create test bindings
    std::vector<MultibindBinding> bindings;
    
    // 1. Continuous pattern: *000*001 (all HELD)
    MultibindBinding continuous_binding;
    continuous_binding.command = "sim/engines/engage_starters";
    continuous_binding.description = "Continuous Engine Start";
    continuous_binding.triggers.push_back(ButtonTrigger(0, ButtonAction::HELD));
    continuous_binding.triggers.push_back(ButtonTrigger(1, ButtonAction::HELD));
    bindings.push_back(continuous_binding);
    
    // 2. One-time pattern: *002+003 (mixed)
    MultibindBinding onetime_binding;
    onetime_binding.command = "sim/flight_controls/gear_toggle";
    onetime_binding.description = "One-time Gear Toggle";
    onetime_binding.triggers.push_back(ButtonTrigger(2, ButtonAction::HELD));
    onetime_binding.triggers.push_back(ButtonTrigger(3, ButtonAction::PRESSED));
    bindings.push_back(onetime_binding);
    
    tracker.set_bindings(bindings);
    
    std::cout << "Test 1: Continuous Command (*000*001)\n";
    std::cout << "=====================================\n";
    
    // Press and hold button 000
    std::cout << "1. Press and hold button 000:\n";
    tracker.set_button_state_transition(0, ButtonAction::PRESSED);
    tracker.set_button_state_transition(0, ButtonAction::HELD);
    simulate_flight_loop(tracker);
    
    // Press and hold button 001  
    std::cout << "\n2. Press and hold button 001:\n";
    tracker.set_button_state_transition(1, ButtonAction::PRESSED);
    tracker.set_button_state_transition(1, ButtonAction::HELD);
    simulate_flight_loop(tracker);
    
    // Continue holding (multiple flight loop cycles)
    std::cout << "\n3. Continue holding (3 flight loop cycles):\n";
    for (int i = 0; i < 3; i++) {
        std::cout << "   Cycle " << (i+1) << ": ";
        simulate_flight_loop(tracker);
    }
    
    // Release button 000
    std::cout << "\n4. Release button 000:\n";
    tracker.set_button_state_transition(0, ButtonAction::RELEASED);
    simulate_flight_loop(tracker);
    
    std::cout << "\n\nTest 2: One-time Command (*002+003)\n";
    std::cout << "===================================\n";
    
    // Hold 002, press 003
    std::cout << "1. Hold 002, press 003:\n";
    tracker.set_button_state_transition(2, ButtonAction::PRESSED);
    tracker.set_button_state_transition(2, ButtonAction::HELD);
    tracker.set_button_state_transition(3, ButtonAction::PRESSED);
    simulate_flight_loop(tracker);
    
    std::cout << "\n\nRESULTS:\n";
    std::cout << "========\n";
    std::cout << "XPLMCommandBegin calls: " << xplm_begin_calls.size() << "\n";
    for (const auto& cmd : xplm_begin_calls) {
        std::cout << "  - " << cmd << "\n";
    }
    
    std::cout << "XPLMCommandEnd calls: " << xplm_end_calls.size() << "\n";
    for (const auto& cmd : xplm_end_calls) {
        std::cout << "  - " << cmd << "\n";
    }
    
    std::cout << "XPLMCommandOnce calls: " << xplm_once_calls.size() << "\n";
    for (const auto& cmd : xplm_once_calls) {
        std::cout << "  - " << cmd << "\n";
    }
    
    std::cout << "\nEXPECTED:\n";
    std::cout << "- 1 XPLMCommandBegin for sim/engines/engage_starters\n";
    std::cout << "- 1 XPLMCommandEnd for sim/engines/engage_starters\n";
    std::cout << "- 1 XPLMCommandOnce for sim/flight_controls/gear_toggle\n\n";
    
    bool test_passed = (xplm_begin_calls.size() == 1 && 
                       xplm_end_calls.size() == 1 &&
                       xplm_once_calls.size() == 1 &&
                       xplm_begin_calls[0] == "sim/engines/engage_starters" &&
                       xplm_end_calls[0] == "sim/engines/engage_starters" &&
                       xplm_once_calls[0] == "sim/flight_controls/gear_toggle");
    
    std::cout << "TEST RESULT: " << (test_passed ? "✅ PASS" : "❌ FAIL") << "\n";
    
    if (!test_passed) {
        std::cout << "\n❌ IMPLEMENTATION ISSUES FOUND!\n";
        std::cout << "The continuous commands feature is not working as expected.\n";
    } else {
        std::cout << "\n✅ CONTINUOUS COMMANDS WORKING!\n";
        std::cout << "Button held actions now properly use XPLMCommandBegin/End!\n";
    }
    
    return test_passed ? 0 : 1;
}