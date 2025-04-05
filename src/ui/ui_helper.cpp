/**
* @file ui_helper.cpp
 * @brief UI helper implementation
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include "ui_helper.h"
#include <cstdlib> // for system()

void UIHelper::disableQuickEdit() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;

    GetConsoleMode(hInput, &prevMode);
    SetConsoleMode(hInput, prevMode & ~ENABLE_QUICK_EDIT_MODE);
}

void UIHelper::printLogo() {
    system("cls");
    disableQuickEdit();
    std::cout << "==================================================\n";
    std::cout << " _              _                \n";
    std::cout << "( \\   |\\     /|( (    /||\\     /|\n";
    std::cout << "| (   ( \\   / )|  \\  ( |( \\   / )\n";
    std::cout << "| |    \\ (_) / |   \\ | | \\ (_) / \n";
    std::cout << "| |     \\   /  | (\\ \\) |  ) _ (  \n";
    std::cout << "| |      ) (   | | \\   | / ( ) \\ \n";
    std::cout << "| (____/\\| |   | )  \\  |( /   \\ )\n";
    std::cout << "(_______/\\_/   |/    )_)|/     \\|\n";
    std::cout << "                                 \n";
    std::cout << "==================================================\n";
    std::cout << "Version 1.0\n";
    std::cout << "Triggerbot is running successfully!\n";
    std::cout << "==================================================\n";
}