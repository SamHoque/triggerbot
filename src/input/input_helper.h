/**
* @file input_helper.h
 * @brief Input handling utilities
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

#include <windows.h>
#include <random>

/**
 * @class InputHelper
 * @brief Provides keyboard and mouse input utilities
 */
class InputHelper {
public:
    /**
     * @brief Check if a key is currently pressed
     * @param keyCode Virtual key code to check
     * @return true if the key is pressed, false otherwise
     */
    static bool isKeyPressed(int keyCode);

    /**
     * @brief Simulate a left mouse button click with random timing
     */
    static void leftClick();
};