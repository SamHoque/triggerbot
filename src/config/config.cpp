/**
 * @file config.cpp
 * @brief Configuration manager implementation
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include "config.h"
#include <fstream>
#include <utility>

Config::Config(std::string  file) : filename(std::move(file)) {
    load(filename);
}

bool Config::load(const std::string& file) {
    if (!file.empty()) filename = file;

    std::ifstream infile(filename);
    if (!infile) {
        createDefault();
        infile.open(filename);
        if (!infile) return false;
    }

    values.clear();
    std::string line;
    while (std::getline(infile, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        // Parse key-value pairs
        if (const size_t pos = line.find('='); pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            const std::string value = line.substr(pos + 1);
            values[key] = value;
        }
    }
    return true;
}

std::string Config::get(const std::string& key, const std::string& defaultValue) const {
    auto it = values.find(key);
    return (it != values.end()) ? it->second : defaultValue;
}

bool Config::get(const std::string& key, bool defaultValue) const {
    auto it = values.find(key);
    if (it == values.end()) return defaultValue;

    return (it->second == "true" || it->second == "1" ||
            it->second == "yes" || it->second == "on");
}

bool Config::save(const std::string& file) {
    if (!file.empty()) filename = file;

    std::ofstream outfile(filename);
    if (!outfile) return false;

    outfile << "# TriggerBot Configuration\n\n";
    for (const auto& pair : values) {
        outfile << pair.first << "=" << pair.second << "\n";
    }
    return true;
}

void Config::createDefault() const {
    if (std::ofstream file(filename); file) {
        file << "# TriggerBot Configuration\n";
        file << "hold_mode=1\n";
        file << "hold_key=left_alt\n";
        file << "tap_time=100\n";
        file << "scan_area_x=8\n";
        file << "scan_area_y=8\n";
        file << "change_sensitivity=40.0\n";
        file << "use_universal_mode=true\n";
        file << "pixel_change_threshold=15\n";
        file << "adaptive_mode=true\n";
        file << "debug_mode=true\n";
        file << "show_overlay=true\n";
        file << "reaction_time_min=0\n";
        file << "reaction_time_max=0\n";
        file << "\n# Notes:\n";
        file << "# scan_area_x/y: Size of detection area at screen center\n";
        file << "# change_sensitivity: How much a pixel must change to count (0-255)\n";
        file << "# pixel_change_threshold: Number of changed pixels needed to trigger\n";
        file << "# adaptive_mode: Dynamically adjust to game conditions\n";
        file << "# debug_mode: Show debug information in console\n";
        file << "# show_overlay: Toggle visual overlay of scan area\n";
        file << "# reaction_time_min/max: Random delay in milliseconds before firing (0 = no delay)\n";
    }
}