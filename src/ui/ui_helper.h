/**
* @file ui_helper.h
 * @brief UI helper utilities
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

#include <windows.h>
#include <iostream>

/**
 * @class UIHelper
 * @brief Provides UI-related utilities
 */
class UIHelper {
public:
    /**
     * @brief Disable console quick edit mode
     */
    static void disableQuickEdit();

    /**
     * @brief Print application logo and info
     */
    static void printLogo();
};