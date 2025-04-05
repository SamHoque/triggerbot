/**
* @file input_helper.cpp
 * @brief Input handling implementation
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include "input_helper.h"

bool InputHelper::isKeyPressed(const int keyCode) {
    return (GetAsyncKeyState(keyCode) & 0x8000) != 0;
}

/**
 * TODO: Hook ntdll.
 */
void InputHelper::leftClick() {
    // Using SendInput with randomization for more human-like behavior
    INPUT input[2] = {};

    // Mouse down
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    // Mouse up
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    // Add randomization for more human-like click
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 5);
    const int variation = distrib(gen);

    SendInput(1, &input[0], sizeof(INPUT));
    Sleep(variation);  // Small random delay between down and up
    SendInput(1, &input[1], sizeof(INPUT));
}