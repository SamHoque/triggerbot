/**
 * @file config.h
 * @brief Configuration manager header
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

#include <string>
#include <unordered_map>
#include <sstream>

/**
 * @class Config
 * @brief Handles application configuration loading, accessing, and saving
 */
class Config {
public:
    /**
     * @brief Constructor with optional filename
     * @param file Path to configuration file
     */
    explicit Config(std::string  file = "config.txt");

    /**
     * @brief Load configuration from file
     * @param file Optional new file path
     * @return true if successful, false otherwise
     */
    bool load(const std::string& file = "");

    /**
     * @brief Save configuration to file
     * @param file Optional new file path
     * @return true if successful, false otherwise
     */
    bool save(const std::string& file = "");

    /**
     * @brief Get value with type conversion and default
     * @param key Configuration key
     * @param defaultValue Value to return if key not found
     * @return Configuration value or default
     */
    template <typename T>
    T get(const std::string& key, const T& defaultValue) const {
     const auto it = values.find(key);
        if (it == values.end()) return defaultValue;

        T value;
        std::istringstream(it->second) >> value;
        return value;
    }

    /**
     * @brief Specialized get for strings
     */
    std::string get(const std::string& key, const std::string& defaultValue) const;

    /**
     * @brief Specialized get for booleans
     */
    bool get(const std::string& key, bool defaultValue) const;

    /**
     * @brief Set configuration value
     * @param key Configuration key
     * @param value Value to set
     */
    template <typename T>
    void set(const std::string& key, const T& value) {
        std::ostringstream ss;
        ss << value;
        values[key] = ss.str();
    }

private:
    std::unordered_map<std::string, std::string> values;
    std::string filename;

    /**
     * @brief Create default configuration file
     */
    void createDefault() const;
};