cmake_minimum_required(VERSION 3.10)
project(TriggerBot VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define source files by category
set(MAIN_SOURCE
    src/main.cpp
)

# Define features
set(FEATURES_SOURCE
    src/features/triggerbot.cpp
    src/features/triggerbot.h
)

set(CONFIG_SOURCE
    src/config/config.cpp
    src/config/config.h
)

set(INPUT_SOURCE
    src/input/input_helper.cpp
    src/input/input_helper.h
    src/input/keycode_mapper.cpp
    src/input/keycode_mapper.h
)

set(UI_SOURCE
    src/ui/ui_helper.cpp
    src/ui/ui_helper.h
)

set(PERFORMANCE_SOURCE
    src/performance/performance_tracker.cpp
    src/performance/performance_tracker.h
)

set(GRAPHICS_SOURCE
    src/graphics/desktop_duplicator.cpp
    src/graphics/desktop_duplicator.h
    src/graphics/overlay.cpp
    src/graphics/overlay.h
)

set(COMMON_SOURCE
    src/common/com_resource.h
)

# Combine all sources
set(ALL_SOURCES
    ${MAIN_SOURCE}
    ${FEATURES_SOURCE}
    ${CONFIG_SOURCE}
    ${INPUT_SOURCE}
    ${UI_SOURCE}
    ${PERFORMANCE_SOURCE}
    ${GRAPHICS_SOURCE}
    ${COMMON_SOURCE}
)

# Create executable
add_executable(TriggerBot ${ALL_SOURCES})

# Include directories
target_include_directories(TriggerBot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add libraries
target_link_libraries(TriggerBot PRIVATE d3d11 dxgi gdi32)

# Set output directory
set_target_properties(TriggerBot PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)

# Debug and release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(TriggerBot PRIVATE DEBUG)
else()
    target_compile_definitions(TriggerBot PRIVATE NDEBUG)
endif()

# Optional: Install instructions
install(TARGETS TriggerBot
    RUNTIME DESTINATION bin
)