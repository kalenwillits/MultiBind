#pragma once

#include "XPLMUtilities.h"
#include <string>

namespace multibind {
namespace error {

// Error severity levels
enum class Severity {
    INFO,
    WARNING,
    ERROR
};

// Standardized error logging
inline void log_error(Severity severity, const std::string& message) {
    const char* prefix = "";
    switch (severity) {
        case Severity::INFO:
            prefix = "Multibind: INFO - ";
            break;
        case Severity::WARNING:
            prefix = "Multibind: WARNING - ";
            break;
        case Severity::ERROR:
            prefix = "Multibind: ERROR - ";
            break;
    }
    
    std::string full_message = prefix + message + "\n";
    XPLMDebugString(full_message.c_str());
}

// Convenience functions
inline void log_info(const std::string& message) {
    log_error(Severity::INFO, message);
}

inline void log_warning(const std::string& message) {
    log_error(Severity::WARNING, message);
}

inline void log_error(const std::string& message) {
    log_error(Severity::ERROR, message);
}

// Result type for operations that can fail
template<typename T>
class Result {
private:
    bool _success;
    T _value;
    std::string _error_message;

public:
    // Success constructor
    explicit Result(const T& value) : _success(true), _value(value) {}
    
    // Error constructor
    explicit Result(const std::string& error) : _success(false), _error_message(error) {}
    
    bool is_success() const { return _success; }
    bool is_error() const { return !_success; }
    
    const T& value() const { 
        if (!_success) {
            log_error("Attempted to access value of failed Result: " + _error_message);
        }
        return _value; 
    }
    
    const std::string& error_message() const { return _error_message; }
};

// Specialization for void operations
template<>
class Result<void> {
private:
    bool _success;
    std::string _error_message;

public:
    // Success constructor
    explicit Result() : _success(true) {}
    
    // Error constructor
    explicit Result(const std::string& error) : _success(false), _error_message(error) {}
    
    bool is_success() const { return _success; }
    bool is_error() const { return !_success; }
    
    const std::string& error_message() const { return _error_message; }
};

} // namespace error
} // namespace multibind