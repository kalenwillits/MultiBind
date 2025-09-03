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
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lin_x64)
    set(PLUGIN_EXTENSION ".xpl")
endif()

# Find X-Plane SDK
find_path(XPLANE_SDK_ROOT_DIR
    NAMES SDK/CHeaders/XPLM/XPLMPlugin.h
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/SDK
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK
        $ENV{XPLANE_SDK_PATH}
    DOC "Path to X-Plane SDK"
)

if(NOT XPLANE_SDK_ROOT_DIR)
    message(FATAL_ERROR "X-Plane SDK not found. Please set XPLANE_SDK_PATH environment variable or place SDK in project directory.")
endif()

set(XPLANE_SDK_INCLUDE_DIR ${XPLANE_SDK_ROOT_DIR}/SDK/CHeaders)

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
        ${XPLANE_SDK_ROOT_DIR}/SDK/Libraries/Win/XPLM_64.lib
        ${XPLANE_SDK_ROOT_DIR}/SDK/Libraries/Win/XPWidgets_64.lib
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
        ${XPLANE_SDK_ROOT_DIR}/SDK/Libraries/Mac/XPLM.framework
        ${XPLANE_SDK_ROOT_DIR}/SDK/Libraries/Mac/XPWidgets.framework
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