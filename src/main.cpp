/**
 * @file main.cpp
 * @brief Main entry point for TriggerBot application
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include <iostream>
#include "features/triggerbot.h"

int main() {
    try {
        if (TriggerBot bot; bot.initialize()) bot.run();
        std::cerr << "\nInitialization failed! Press Enter to exit: ";
        std::cin.get();
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        std::cerr << "Press Enter to exit: ";
        std::cin.get();
        return 0;
    }
}
