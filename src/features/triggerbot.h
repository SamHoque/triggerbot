/**
 * @file features/triggerbot.h
 * @brief Main TriggerBot class header
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

#include <windows.h>
#include <string>

#include "config/config.h"
#include "performance/performance_tracker.h"
#include "graphics/desktop_duplicator.h"
#include "graphics/overlay.h"

/**
 * @class TriggerBot
 * @brief Main application class that coordinates pixel detection and mouse triggering
 */
class TriggerBot {
public:
    /**
     * @brief Constructor
     */
    TriggerBot();

    /**
     * @brief Initialize the TriggerBot
     * @return true if initialization is successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Run the main application loop
     */
    [[noreturn]] void run();

private:
    /**
     * @brief Validate configuration parameters
     * @return true if configuration is valid, false otherwise
     */
    bool validateConfig() const;
    
    /**
     * @brief Generate a random reaction delay between min and max
     * @return Delay in milliseconds
     */
    DWORD getReactionDelay() const;

    Config config;
    PerformanceTracker performance_tracker;
    DesktopDuplicator dupl;
    Overlay overlay;
    int key_code;
    DWORD last_trigger_time; // For cooldown
    DWORD cooldown_period;  // Cooldown in milliseconds
    bool was_key_pressed;    // Track previous key state
};