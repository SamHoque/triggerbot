/**
 * @file features/triggerbot.cpp
 * @brief Main TriggerBot class implementation
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include "triggerbot.h"
#include <iostream>
#include <algorithm>
#include <random>
#include "input/keycode_mapper.h"
#include "input/input_helper.h"
#include "ui/ui_helper.h"

TriggerBot::TriggerBot()
    : key_code(0), last_trigger_time(0),
      cooldown_period(150),
      was_key_pressed(false) {
}

bool TriggerBot::initialize() {
    // Load configuration
    if (!validateConfig()) {
        std::cerr << "Error: Invalid configuration\n";
        return false;
    }

    // Validate key code
    const KeyCodeMapper& key_mapper = KeyCodeMapper::instance();
    const auto hold_key = config.get<std::string>("hold_key", "left_alt");
    key_code = key_mapper.getKeyCode(hold_key);
    if (key_code == -1) {
        std::cerr << "The hold_key you chose could not be found: " << hold_key << "\n";
        return false;
    }

    // Initialize Desktop Duplication API
    if (!dupl.initialize()) {
        std::cerr << "Error: Failed to initialize DXGI Desktop Duplication.\n";
        return false;
    }
    
    // Initialize overlay
    RECT desktop;
    GetWindowRect(GetDesktopWindow(), &desktop);
    if (!overlay.initialize(desktop.right, desktop.bottom)) {
        std::cerr << "Warning: Failed to initialize overlay. Continuing without visual feedback.\n";
        // Non-critical, continue anyway
    }
    
    // Show overlay if enabled in config
    overlay.setVisible(config.get("show_overlay", true));

    UIHelper::printLogo();

    // Print configuration summary
    std::cout << "Configuration:\n";
    std::cout << "  Scan area: " << config.get<int>("scan_area_x", 8) << "x"
              << config.get<int>("scan_area_y", 8) << " pixels\n";
    std::cout << "  Hold key: " << hold_key << "\n";
    std::cout << "  Hold mode: " << config.get<int>("hold_mode", 1) << "\n";
    std::cout << "  Tap time: " << config.get<double>("tap_time", 100.0) << " ms\n";
    std::cout << "  Pixel change sensitivity: " << config.get<double>("change_sensitivity", 40.0) << "\n";
    std::cout << "  Pixel change threshold: " << config.get<int>("pixel_change_threshold", 15) << "\n";
    std::cout << "  Adaptive mode: " << (config.get("adaptive_mode", true) ? "Enabled" : "Disabled") << "\n";
    std::cout << "  Universal mode: " << (config.get("use_universal_mode", true) ? "Enabled" : "Disabled") << "\n";
    std::cout << "  Debug mode: " << (config.get("debug_mode", true) ? "Enabled" : "Disabled") << "\n";
    std::cout << "  Overlay: " << (config.get("show_overlay", true) ? "Enabled" : "Disabled") << "\n";
    std::cout << "  Reaction time: " << config.get<int>("reaction_time_min", 0) << "-" 
              << config.get<int>("reaction_time_max", 0) << " ms\n";
    
    std::cout << "\nPress " << hold_key << " to activate triggerbot\n\n";

    // Create screenshots directory
    CreateDirectoryA("screenshots", nullptr);

    return true;
}

DWORD TriggerBot::getReactionDelay() const {
    int min_delay = config.get<int>("reaction_time_min", 0);
    int max_delay = config.get<int>("reaction_time_max", 0);
    
    // If both values are 0 or min > max, return 0 (no delay)
    if (min_delay == 0 && max_delay == 0 || min_delay > max_delay) {
        return 0;
    }
    
    // If min and max are the same, return that value
    if (min_delay == max_delay) {
        return static_cast<DWORD>(min_delay);
    }
    
    // Generate random delay between min and max
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min_delay, max_delay);
    
    return static_cast<DWORD>(distrib(gen));
}

[[noreturn]] void TriggerBot::run() {
    int current_change_threshold = config.get<int>("pixel_change_threshold", 15);
    const int scan_area_x = config.get<int>("scan_area_x", 8);
    const int scan_area_y = config.get<int>("scan_area_y", 8);
    const bool show_overlay = config.get("show_overlay", true);
    const bool save_screenshots = config.get("debug_mode", true);

    while (true) {
        // Check if key is pressed
        const bool is_key_pressed = InputHelper::isKeyPressed(key_code);

        // Reset logic: If key wasn't pressed before but is now pressed,
        // capture a new reference frame
        if (!was_key_pressed && is_key_pressed) {
            // Capture reference frame and save screenshot if enabled
            dupl.captureReferenceFrame(scan_area_x, scan_area_y, save_screenshots);
            if (config.get("debug_mode", true)) {
                std::cout << "\rReference frame reset                                        ";
            }
            Sleep(10); // Short delay to let next frame be different
        }

        // Remember current key state for next loop
        was_key_pressed = is_key_pressed;

        // If key is pressed (and hold mode is enabled), check for changes
        if (config.get<int>("hold_mode", 1) == 0 || is_key_pressed) {
            performance_tracker.startCounter();
            
            // Get mouse position for overlay - but limit updates for performance
            if (show_overlay) {
                static DWORD lastUpdate = 0;
                DWORD now = GetTickCount();
                
                // Only update cursor position at 30Hz (33ms) max to reduce overhead
                if (now - lastUpdate > 33) {
                    POINT mouse_pos;
                    GetCursorPos(&mouse_pos);
                    overlay.update(mouse_pos.x, mouse_pos.y, scan_area_x, scan_area_y);
                    lastUpdate = now;
                }
            }

            // Check for significant pixel changes using Desktop Duplication API
            if (dupl.checkForChanges(
                    scan_area_x,
                    scan_area_y,
                    config.get<double>("change_sensitivity", 40.0),
                    current_change_threshold,
                    config.get("debug_mode", true),
                    save_screenshots)) {

                // Get current time for cooldown check
                DWORD current_time = GetTickCount();

                // Only trigger if not in cooldown period
                if (current_time - last_trigger_time > cooldown_period) {
                    // Apply reaction delay if configured
                    DWORD reaction_delay = getReactionDelay();
                    if (reaction_delay > 0) {
                        if (config.get("debug_mode", true)) {
                            std::cout << "\rWaiting " << reaction_delay << "ms before firing...";
                        }
                        Sleep(reaction_delay);
                    }
                    
                    InputHelper::leftClick();
                    performance_tracker.stopCounter();
                    last_trigger_time = current_time;

                    // Take a new reference frame after firing
                    dupl.captureReferenceFrame(scan_area_x, scan_area_y, save_screenshots);

                    // Apply cooldown
                    Sleep(static_cast<DWORD>(config.get<double>("tap_time", 100.0)));

                    // Adaptive mode: adjust threshold dynamically to prevent over-triggering
                    if (config.get("adaptive_mode", true)) {
                        // Slightly increase threshold after each trigger
                        current_change_threshold = std::min(
                            current_change_threshold + 1,
                            config.get<int>("pixel_change_threshold", 15) * 2);
                    }
                }
            } else if (config.get("adaptive_mode", true)) {
                // Gradually reduce threshold to initial value when not triggering
                if (current_change_threshold > config.get<int>("pixel_change_threshold", 15)) {
                    current_change_threshold--;
                }
            }
        } else {
            Sleep(1); // Short sleep when not active to reduce CPU usage
        }
    }
}

bool TriggerBot::validateConfig() const {
    return config.get<int>("scan_area_x", 8) > 0 &&
           config.get<int>("scan_area_y", 8) > 0 &&
           config.get<double>("tap_time", 100.0) > 0 &&
           !config.get<std::string>("hold_key", "left_alt").empty() &&
           config.get<double>("change_sensitivity", 40.0) > 0 &&
           config.get<int>("pixel_change_threshold", 15) > 0 &&
           config.get<int>("reaction_time_min", 0) >= 0 &&
           config.get<int>("reaction_time_max", 0) >= 0;
}