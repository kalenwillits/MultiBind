# C++17 Style Guide

This style guide outlines the C++17 coding standards used in this project, inspired by Godot game engine's coding principles while adapted for X-Plane plugin development.

## Naming Conventions

### Variables and Functions
- Use `snake_case` for variables, functions, and methods
- Use descriptive names that clearly indicate purpose

```cpp
// Good
int button_count = 0;
std::string aircraft_name;
void update_aircraft_display(const std::string& name);

// Bad
int bc = 0;
std::string aircraftName;
void updateAircraftDisplay(const std::string& name);
```

### Private Member Variables
- Use leading underscore for private member variables
- This distinguishes them from local variables and parameters

```cpp
class Config {
private:
    std::string _aircraft_id;           // Good
    std::vector<MultibindBinding> _bindings;  // Good
    
    // Bad:
    // std::string aircraft_id_;
    // std::string aircraftId;
};
```

### Constants
- Use `SCREAMING_SNAKE_CASE` for compile-time constants
- Use `static constexpr` for class constants

```cpp
// Good
static constexpr int WINDOW_WIDTH = 600;
static constexpr auto COMBINATION_TIMEOUT = std::chrono::milliseconds(100);

// Bad
static constexpr int windowWidth = 600;
static constexpr int window_width = 600;  // lowercase for constants
```

### Classes and Types
- Use `PascalCase` for class names, types, and enums

```cpp
// Good
class CombinationTracker {
    // ...
};

enum class ButtonState {
    PRESSED,
    RELEASED
};

// Bad
class combination_tracker {
    // ...
};
```

## String Handling

### Prefer std::string Over C-style Strings
- Always use `std::string` for string operations
- Only use `c_str()` when interfacing with C APIs
- Use string concatenation and `std::ostringstream` instead of `snprintf`

```cpp
// Good
std::string log_msg = "Multibind: Loading config for aircraft: " + aircraft_id + "\n";
XPLMDebugString(log_msg.c_str());

std::ostringstream command_name_ss;
command_name_ss << "multibind/" << std::setfill('0') << std::setw(3) << i;
std::string command_name = command_name_ss.str();

// Bad
char log_msg[512];
snprintf(log_msg, sizeof(log_msg), "Multibind: Loading config for aircraft: %s\n", aircraft_id.c_str());
XPLMDebugString(log_msg);
```

### C API Interfacing
- When interfacing with C APIs that require char arrays, use safe copying
- Always null-terminate and bounds-check

```cpp
// Good
std::string name = "Multibind";
std::strncpy(out_name, name.c_str(), 255);
out_name[255] = '\0';

// For reading from C APIs
std::string aircraft_filename(512, '\0');
XPLMGetDatab(g_aircraft_filename_ref, &aircraft_filename[0], 0, aircraft_filename.size());
aircraft_filename.resize(std::strlen(aircraft_filename.c_str())); // Trim to actual length
```

## Type Clarity

### Explicit Types Over auto
- Use explicit types when it improves code readability
- `auto` can be used for complex iterators or when the type is obvious from context

```cpp
// Good - explicit types are clearer
std::string aircraft_id = extract_aircraft_id(current_aircraft);
std::vector<MultibindBinding> bindings = config.get_bindings();

// Acceptable uses of auto
for (const auto& binding : _bindings) {  // Type is clear from context
    // ...
}

auto now = std::chrono::steady_clock::now();  // Type is obvious
```

### Const Correctness
- Use `const` wherever possible
- Make member functions `const` when they don't modify state
- Use `const` references for parameters that won't be modified

```cpp
// Good
bool is_window_visible() const;
void process_binding(const MultibindBinding& binding);
const std::vector<MultibindBinding>& get_bindings() const { return _bindings; }
```

## Memory Management and RAII

### Avoid Static Local Variables
- Move static local variables to be member variables when possible
- This improves testability and prevents hidden state

```cpp
// Bad
void process_combination_change() {
    static auto last_trigger = std::chrono::steady_clock::time_point{};
    // ...
}

// Good
class CombinationTracker {
private:
    std::chrono::steady_clock::time_point _last_trigger_time{};
    
public:
    void process_combination_change() {
        // Use _last_trigger_time instead
    }
};
```

### RAII Principles
- Use stack allocation where possible
- Let destructors handle cleanup automatically
- Prefer standard containers over raw arrays

```cpp
// Good
std::vector<XPLMCommandRef> commands;  // Automatically managed
std::string buffer;                     // Automatically managed

// Bad
XPLMCommandRef* commands = new XPLMCommandRef[1000];  // Manual memory management
char* buffer = malloc(512);                           // Manual memory management
```

## Code Organization

### Header Files
- Use `#pragma once` instead of include guards
- Include what you use, avoid transitive includes
- Order includes: standard library, third-party, then local headers

```cpp
#pragma once

// Standard library
#include <string>
#include <vector>
#include <chrono>

// Third-party (X-Plane SDK)
#include "XPLMUtilities.h"

// Local headers
#include "config.h"
```

### Class Structure
- Group public members before private
- Group methods logically (constructors, getters, setters, operations)

```cpp
class UI {
public:
    // Constructors and destructor
    UI() = default;
    ~UI() = default;
    
    // Public interface
    void initialize(Config* config, CombinationTracker* tracker);
    void show_window();
    bool is_window_visible() const;
    
private:
    // Private members
    Config* _config = nullptr;
    XPLMWidgetID _main_window = nullptr;
    
    // Private methods
    void create_widgets();
    void update_binding_list();
};
```

## Error Handling

### Return Values Over Exceptions
- For X-Plane plugins, prefer return codes over exceptions
- Use boolean returns for simple success/failure
- Log errors appropriately

```cpp
// Good
bool Config::save_config() {
    if (!create_multibind_directory()) {
        XPLMDebugString("Multibind: ERROR - Could not create multibind directory\n");
        return false;
    }
    
    std::ofstream file(config_file);
    if (!file.is_open()) {
        std::string error_msg = "Multibind: ERROR - Could not open config file: " + config_file + "\n";
        XPLMDebugString(error_msg.c_str());
        return false;
    }
    
    return true;
}
```

## C++17 Features

### Use Modern C++17 Features When Appropriate
- `std::filesystem` for path operations
- `constexpr` for compile-time constants
- `std::string_view` for read-only string parameters (when available)
- Structured bindings for multiple return values

```cpp
// std::filesystem
#include <filesystem>
std::filesystem::create_directories(multibind_dir);

// constexpr
static constexpr int WINDOW_WIDTH = 600;
static constexpr auto COMBINATION_TIMEOUT = std::chrono::milliseconds(100);
```

## Comments and Documentation

### Self-Documenting Code
- Write code that explains itself through clear naming
- Use comments for complex logic or API requirements
- Document unusual or non-obvious behavior

```cpp
// Good - explains the "why"
// Small delay to prevent accidental double-triggering
if (now - _last_trigger_time > std::chrono::milliseconds(200)) {
    _triggered_command = binding.target_command;
    _last_trigger_time = now;
}

// Update UI with current aircraft name
g_ui.update_aircraft_display(aircraft_id);
```

## Platform Considerations

### X-Plane Plugin Specific
- Global static variables are acceptable for plugin state (required by X-Plane C API)
- Use PLUGIN_API macro for exported functions
- Handle C callback signatures appropriately

```cpp
// Plugin globals are necessary for X-Plane callbacks
static Config g_config;
static CombinationTracker g_tracker;
static UI g_ui;

// Required X-Plane plugin entry points
PLUGIN_API int XPluginStart(char* out_name, char* out_sig, char* out_desc) {
    // Plugin initialization
    return 1;
}
```

This style guide ensures consistent, maintainable, and readable C++17 code while working within the constraints of X-Plane plugin development.