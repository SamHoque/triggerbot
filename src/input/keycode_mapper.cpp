/**
 * @file keycode_mapper.cpp
 * @brief Implementation of KeyCodeMapper class
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include "keycode_mapper.h"

// Static key mappings array
static const std::pair<std::string, int> KEY_MAPPINGS[] = {
    {"left_mouse_button", 0x01},
    {"right_mouse_button", 0x02},
    {"x1", 0x05},
    {"x2", 0x06},
    {"num_0", 0x30},
    {"num_1", 0x31},
    {"num_2", 0x32},
    {"num_3", 0x33},
    {"num_4", 0x34},
    {"num_5", 0x35},
    {"num_6", 0x36},
    {"num_7", 0x37},
    {"num_8", 0x38},
    {"num_9", 0x39},
    {"a", 0x41},
    {"b", 0x42},
    {"c", 0x43},
    {"d", 0x44},
    {"e", 0x45},
    {"f", 0x46},
    {"g", 0x47},
    {"h", 0x48},
    {"i", 0x49},
    {"j", 0x4A},
    {"k", 0x4B},
    {"l", 0x4C},
    {"m", 0x4D},
    {"n", 0x4E},
    {"o", 0x4F},
    {"p", 0x50},
    {"q", 0x51},
    {"r", 0x52},
    {"s", 0x53},
    {"t", 0x54},
    {"u", 0x55},
    {"v", 0x56},
    {"w", 0x57},
    {"x", 0x58},
    {"y", 0x59},
    {"z", 0x5A},
    {"backspace", 0x08},
    {"down_arrow", 0x28},
    {"enter", 0x0D},
    {"esc", 0x1B},
    {"home", 0x24},
    {"insert", 0x2D},
    {"left_alt", 0xA4},
    {"left_ctrl", 0xA2},
    {"left_shift", 0xA0},
    {"page_down", 0x22},
    {"page_up", 0x21},
    {"right_alt", 0xA5},
    {"right_ctrl", 0xA3},
    {"right_shift", 0xA1},
    {"space", 0x20},
    {"tab", 0x09},
    {"up_arrow", 0x26},
    {"f1", 0x70},
    {"f2", 0x71},
    {"f3", 0x72},
    {"f4", 0x73},
    {"f5", 0x74},
    {"f6", 0x75},
    {"f7", 0x76},
    {"f8", 0x77},
    {"f9", 0x78},
    {"f10", 0x79},
    {"f11", 0x7A},
    {"f12", 0x7B}
};

const KeyCodeMapper& KeyCodeMapper::instance() {
    // Thread-safe in C++11 and later
    static const KeyCodeMapper instance;
    return instance;
}

KeyCodeMapper::KeyCodeMapper() {
    // Reserve space for the maps to avoid rehashing
    nameToCode.reserve(std::size(KEY_MAPPINGS));
    codeToName.reserve(std::size(KEY_MAPPINGS));

    // Fill both maps for bidirectional lookup
    for (const auto&[fst, snd] : KEY_MAPPINGS) {
        nameToCode[fst] = snd;
        codeToName[snd] = fst;
    }
}

int KeyCodeMapper::getKeyCode(const std::string_view keyName) const {
    // Convert string_view to string for lookup
    // This is necessary since we're using std::unordered_map with std::string keys
    const std::string key(keyName);
    const auto it = nameToCode.find(key);
    return (it != nameToCode.end()) ? it->second : -1;
}

std::string_view KeyCodeMapper::getKeyName(const int keyCode) const {
    auto it = codeToName.find(keyCode);
    return (it != codeToName.end()) ? std::string_view(it->second) : std::string_view();
}

bool KeyCodeMapper::isValidKeyName(const std::string_view keyName) const {
    return nameToCode.find(std::string(keyName)) != nameToCode.end();
}