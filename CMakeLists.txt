cmake_minimum_required(VERSION 3.16)
project(Multibind VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/win_x64)
    set(PLUGIN_EXTENSION ".xpl")
elseif(APPLE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/mac_x64)
    set(PLUGIN_EXTENSION ".xpl")
    # Support both Intel and Apple Silicon
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lin_x64)
    set(PLUGIN_EXTENSION ".xpl")
endif()

# Find X-Plane SDK with comprehensive search and helpful error messages
message(STATUS "Searching for X-Plane SDK...")

find_path(XPLANE_SDK_ROOT_DIR
    NAMES CHeaders/XPLM/XPLMPlugin.h
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/SDK
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK
        $ENV{XPLANE_SDK_PATH}
        # Common Windows paths
        "C:/XPSDK"
        "C:/X-Plane SDK"
        "C:/dev/XPSDK"
        # Common macOS paths
        "/Applications/X-Plane 12/Resources/SDK"
        "$ENV{HOME}/XPSDK"
        "$ENV{HOME}/Downloads/XPSDK"
        # Common Linux paths
        "/opt/XPSDK"
        "$ENV{HOME}/XPSDK"
        "$ENV{HOME}/Downloads/XPSDK"
    DOC "Path to X-Plane SDK"
)

if(XPLANE_SDK_ROOT_DIR)
    message(STATUS "✅ X-Plane SDK found at: ${XPLANE_SDK_ROOT_DIR}")
    
    # Verify required SDK components exist
    set(REQUIRED_SDK_FILES
        "CHeaders/XPLM/XPLMPlugin.h"
        "CHeaders/XPLM/XPLMUtilities.h" 
        "CHeaders/Widgets/XPWidgets.h"
    )
    
    foreach(file ${REQUIRED_SDK_FILES})
        if(NOT EXISTS "${XPLANE_SDK_ROOT_DIR}/${file}")
            message(FATAL_ERROR "❌ SDK file missing: ${file}\nSDK may be incomplete or corrupted.")
        endif()
    endforeach()
    
    # Verify platform-specific libraries exist
    if(WIN32)
        if(NOT EXISTS "${XPLANE_SDK_ROOT_DIR}/Libraries/Win/XPLM_64.lib")
            message(FATAL_ERROR "❌ Windows SDK library missing: Libraries/Win/XPLM_64.lib")
        endif()
        if(NOT EXISTS "${XPLANE_SDK_ROOT_DIR}/Libraries/Win/XPWidgets_64.lib")
            message(FATAL_ERROR "❌ Windows SDK library missing: Libraries/Win/XPWidgets_64.lib")
        endif()
    elseif(APPLE)
        if(NOT EXISTS "${XPLANE_SDK_ROOT_DIR}/Libraries/Mac/XPLM.framework")
            message(FATAL_ERROR "❌ macOS SDK framework missing: Libraries/Mac/XPLM.framework")
        endif()
        if(NOT EXISTS "${XPLANE_SDK_ROOT_DIR}/Libraries/Mac/XPWidgets.framework")
            message(FATAL_ERROR "❌ macOS SDK framework missing: Libraries/Mac/XPWidgets.framework")
        endif()
    endif()
    
    message(STATUS "✅ SDK verification complete")
else()
    message(FATAL_ERROR 
        "❌ X-Plane SDK not found!\n\n"
        "Solutions:\n"
        "1. Run the SDK setup script:\n"
        "   Windows: setup-sdk.bat\n"
        "   Linux/Mac: ./setup-sdk.sh\n\n"
        "2. Manual setup:\n"
        "   - Download SDK from: https://developer.x-plane.com/sdk/plugin-sdk-downloads/\n"
        "   - Extract to 'SDK' folder in project directory\n"
        "   - OR set XPLANE_SDK_PATH environment variable\n\n"
        "3. Check expected file exists: SDK/CHeaders/XPLM/XPLMPlugin.h\n"
    )
endif()

set(XPLANE_SDK_INCLUDE_DIR ${XPLANE_SDK_ROOT_DIR}/CHeaders)

# Plugin source files
set(PLUGIN_SOURCES
    src/multibind.cpp
    src/config.cpp
    src/combination_tracker.cpp
    src/ui.cpp
)

# Plugin headers
set(PLUGIN_HEADERS
    src/config.h
    src/combination_tracker.h
    src/ui.h
)

# Create the plugin target
add_library(multibind SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# Include directories
target_include_directories(multibind PRIVATE
    ${XPLANE_SDK_INCLUDE_DIR}
    ${XPLANE_SDK_INCLUDE_DIR}/XPLM
    ${XPLANE_SDK_INCLUDE_DIR}/Widgets
    src/
)

# Platform-specific compilation flags
if(WIN32)
    target_compile_definitions(multibind PRIVATE
        APL=0 IBM=1 LIN=0
        _CRT_SECURE_NO_WARNINGS
    )
    target_link_libraries(multibind 
        ${XPLANE_SDK_ROOT_DIR}/Libraries/Win/XPLM_64.lib
        ${XPLANE_SDK_ROOT_DIR}/Libraries/Win/XPWidgets_64.lib
    )
    set_target_properties(multibind PROPERTIES
        SUFFIX ${PLUGIN_EXTENSION}
        PREFIX ""
    )
elseif(APPLE)
    target_compile_definitions(multibind PRIVATE
        APL=1 IBM=0 LIN=0
    )
    target_link_libraries(multibind
        ${XPLANE_SDK_ROOT_DIR}/Libraries/Mac/XPLM.framework
        ${XPLANE_SDK_ROOT_DIR}/Libraries/Mac/XPWidgets.framework
    )
    set_target_properties(multibind PROPERTIES
        SUFFIX ${PLUGIN_EXTENSION}
        PREFIX ""
        BUNDLE TRUE
        BUNDLE_EXTENSION "xpl"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    )
else()
    target_compile_definitions(multibind PRIVATE
        APL=0 IBM=0 LIN=1
    )
    set_target_properties(multibind PROPERTIES
        SUFFIX ${PLUGIN_EXTENSION}
        PREFIX ""
    )
    # Link with -rdynamic for proper symbol export on Linux
    set_target_properties(multibind PROPERTIES
        LINK_FLAGS "-rdynamic"
    )
endif()

# X-Plane SDK defines
target_compile_definitions(multibind PRIVATE
    XPLM=1
    XPLM200=1
    XPLM210=1
    XPLM300=1
    XPLM301=1
    XPLM303=1
    XPLM400=1
    XPWIDGETS=1
)

# Compiler-specific flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(multibind PRIVATE
        -Wall -Wextra -Wshadow -Wfloat-equal -Wundef -Wcast-align
        -Wwrite-strings -Wlogical-op -Wmissing-declarations
        -Wredundant-decls -Wshadow -Woverloaded-virtual
    )
endif()

# Debug/Release configurations
target_compile_definitions(multibind PRIVATE
    $<$<CONFIG:Debug>:DEBUG=1>
    $<$<CONFIG:Release>:NDEBUG=1>
)

# Print build information
message(STATUS "=== Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
if(WIN32)
    message(STATUS "Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/multibind.xpl")
elseif(APPLE)
    message(STATUS "Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/multibind.xpl (Universal Binary)")
else()
    message(STATUS "Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/multibind.xpl")
endif()
message(STATUS "==========================")

# Add install target for easier deployment
install(TARGETS multibind
    RUNTIME DESTINATION "Multibind/${CMAKE_SYSTEM_NAME}_x64"
    LIBRARY DESTINATION "Multibind/${CMAKE_SYSTEM_NAME}_x64"
    BUNDLE DESTINATION "Multibind/mac_x64"
)

# Create convenience target for building release version
add_custom_target(release
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config Release
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building Release configuration"
)