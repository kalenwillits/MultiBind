# Multibind Plugin - Decision Log

This document records key architectural and implementation decisions made during the development of the Multibind X-Plane plugin.

## Core Architecture Decisions

### Decision 1: 1000 Custom Commands (000-999)
**Date**: Initial Implementation  
**Decision**: Create exactly 1000 custom X-Plane commands named `multibind/000` through `multibind/999`  
**Rationale**: 
- Provides sufficient coverage for complex HOTAS setups (most users won't need more than 50-100 combinations)
- Three-digit numbering ensures consistent sorting in X-Plane's command list
- Large number allows for future expansion without breaking existing configs
- Commands are created at plugin startup and appear in X-Plane's standard joystick assignment interface

**Alternatives Considered**: 
- Dynamic command creation (rejected due to X-Plane SDK limitations)
- Fewer commands like 100 (rejected as too limiting for power users)

### Decision 2: Target X-Plane Commands (Not Datarefs)
**Date**: Initial Implementation  
**Decision**: Multibind combinations trigger X-Plane commands rather than manipulate datarefs directly  
**Rationale**:
- Commands are the standard way to trigger actions in X-Plane
- Commands handle complex logic internally (safety checks, animations, sounds)
- Matches user expectations from standard joystick button assignments
- Avoids complexity of dataref manipulation and potential safety issues

**Alternatives Considered**:
- Dataref manipulation (rejected as out of scope and potentially unsafe)
- Mixed command/dataref system (rejected as overly complex)

### Decision 3: Simple Text File Configuration Format
**Date**: Initial Implementation  
**Decision**: Use simple pipe-separated text format: `button1+button2|command|description`  
**Rationale**:
- Human readable and editable
- Easy to parse without external dependencies
- Simple backup and sharing of configurations
- Clear separation of concerns (buttons | command | description)

**Example Format**:
```
1+5+10|sim/starters/engage_starter_1|Start Engine 1
2+6|sim/starters/engage_starter_2|Start Engine 2
```

**Alternatives Considered**:
- CSV format (rejected due to escaping complexity with command names)
- JSON (rejected as overkill for simple key-value storage)
- Binary format (rejected due to lack of human readability)

### Decision 4: Aircraft-Specific Configuration Files
**Date**: Initial Implementation  
**Decision**: Store configurations per aircraft in `multibind/{aircraft-name}.txt`  
**Rationale**:
- Different aircraft have different systems and commands
- Users typically want different bindings for different aircraft types
- Automatic loading when switching aircraft
- Follows X-Plane's aircraft-specific customization pattern

**Implementation**: Extract aircraft identifier from `sim/aircraft/view/acf_filename` dataref

## UI Architecture Decisions

### Decision 5: XPlane Widget System (Not ImGui)
**Date**: Initial Implementation  
**Decision**: Use X-Plane's native widget system for the configuration UI  
**Rationale**:
- No external dependencies required
- Consistent with X-Plane's native UI style
- Lightweight and sufficient for the simple UI requirements
- Easier deployment (no additional libraries to bundle)

**Alternatives Considered**:
- ImGui (rejected due to added complexity and library dependencies)
- Custom OpenGL rendering (rejected as unnecessary for simple forms)

### Decision 6: Live Combination Display
**Date**: Initial Implementation  
**Decision**: Show currently pressed button combinations in real-time in the UI  
**Rationale**:
- Provides immediate feedback during configuration
- Helps users verify their button assignments are working
- Essential for the recording workflow
- Aids in debugging combination detection issues

## Technical Architecture Decisions

### Decision 7: Component Separation
**Date**: Initial Implementation  
**Decision**: Split functionality into discrete classes:
- `Config`: File I/O and data management
- `CombinationTracker`: Button state and combination detection
- `UI`: User interface and interaction
- `multibind.cpp`: Plugin lifecycle and X-Plane integration

**Rationale**:
- Clear separation of concerns
- Easier testing and debugging
- Modular design allows for future enhancements
- Follows standard software engineering practices

### Decision 8: Set-Based Combination Storage
**Date**: Initial Implementation  
**Decision**: Use `std::set<int>` to represent button combinations  
**Rationale**:
- Automatically handles duplicate buttons
- Order-independent comparison (1+5 equals 5+1)
- Efficient lookup and comparison operations
- Natural representation for mathematical set operations

### Decision 9: Superset Matching for Combinations
**Date**: Initial Implementation  
**Decision**: Allow combinations to trigger even when additional buttons are pressed  
**Rationale**:
- More forgiving for users with complex control setups
- Prevents accidental blocking by unrelated button presses
- Can be refined in future versions if needed
- Matches intuitive user expectations

**Example**: If combination is defined as `1+5`, it will trigger when pressing `1+5+7`

### Decision 10: Command Handler Phase Processing
**Date**: Initial Implementation  
**Decision**: Track button press/release using X-Plane command phases (Begin/End)  
**Rationale**:
- Leverages X-Plane's built-in command system architecture
- Handles edge cases like rapid button presses
- Provides clean press/release semantics
- Integrates naturally with existing X-Plane command flow

## Build System Decisions

### Decision 11: CMake Build System
**Date**: Initial Implementation  
**Decision**: Use CMake for cross-platform building  
**Rationale**:
- Industry standard for C++ cross-platform projects
- Good X-Plane SDK integration examples available
- Handles platform-specific compilation flags automatically
- Familiar to most C++ developers

### Decision 12: C++17 Standard
**Date**: Initial Implementation  
**Decision**: Target C++17 as the minimum standard  
**Rationale**:
- `std::filesystem` support for cross-platform directory operations
- Modern C++ features for cleaner code
- Widely supported by compilers on all target platforms
- Balances modernity with compatibility

## Future Enhancement Considerations

### Potential Future Decisions

1. **Exact vs. Superset Matching**: Could be made user-configurable per combination
2. **Timing Sensitivity**: Could add timing-based combinations (e.g., double-press, hold duration)
3. **Combination Conflicts**: Could add conflict detection and resolution
4. **Export/Import**: Could add profile sharing functionality
5. **Advanced UI**: Could upgrade to ImGui for richer interface options

## Lessons Learned

1. **X-Plane SDK Integration**: The command system provides a clean abstraction for plugin functionality
2. **Configuration Management**: Simple text formats are often better than complex structured formats for user-editable configs
3. **Real-time Feedback**: Live UI updates are essential for configuration tools
4. **Cross-platform Considerations**: Early decisions about file paths and build systems pay dividends later

---

This decision log should be updated as the plugin evolves and new architectural decisions are made.