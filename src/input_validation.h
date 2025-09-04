#pragma once

#include <string>
#include <optional>
#include <stdexcept>
#include <cstring>

namespace multibind {
namespace validation {

// Safe integer parsing that returns std::nullopt on invalid input
inline std::optional<int> safe_parse_int(const char* str) {
    if (!str || str[0] == '\0') {
        return std::nullopt;
    }
    
    try {
        size_t pos = 0;
        int value = std::stoi(std::string(str), &pos);
        
        // Check if entire string was consumed (no trailing characters)
        if (pos != std::strlen(str)) {
            return std::nullopt;
        }
        
        return value;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

// Validate X-Plane command name format
inline bool is_valid_xplane_command(const std::string& command) {
    if (command.empty()) {
        return false;
    }
    
    // Basic validation: commands should contain forward slashes and no spaces
    // This is a simple heuristic, not exhaustive validation
    return command.find('/') != std::string::npos && 
           command.find(' ') == std::string::npos &&
           command.length() < 256;  // Reasonable length limit
}

// Validate binding description
inline bool is_valid_description(const std::string& description) {
    // Description can be empty, but if present, should be reasonable length
    return description.length() <= 256;
}

// Validate selection number input
inline std::optional<int> parse_selection_number(const char* text, int min_value, int max_value) {
    auto parsed = safe_parse_int(text);
    if (!parsed || *parsed < min_value || *parsed > max_value) {
        return std::nullopt;
    }
    return *parsed;
}

} // namespace validation
} // namespace multibind