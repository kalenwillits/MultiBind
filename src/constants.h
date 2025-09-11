#pragma once

// MultiBind Plugin Constants

namespace multibind {
namespace constants {

// Command system limits
constexpr int MAX_MULTIBIND_COMMANDS = 1000;
constexpr int MIN_BUTTON_ID = 0;
constexpr int MAX_BUTTON_ID = MAX_MULTIBIND_COMMANDS - 1;

// Timing constants
constexpr int TRIGGER_DEBOUNCE_MS = 200;  // Milliseconds to prevent double-triggering

// Buffer sizes
constexpr size_t XPLANE_PATH_BUFFER_SIZE = 512;  // X-Plane system path buffer size
constexpr size_t XPLANE_STRING_BUFFER_SIZE = 256; // X-Plane plugin string buffers

// UI constants
constexpr int SELECTION_TEXT_BUFFER_SIZE = 32;  // Buffer for selection input text

} // namespace constants
} // namespace multibind