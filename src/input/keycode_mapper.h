/**
* @file keycode_mapper.h
 * @brief Maps human-readable key names to system key codes
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

/**
 * @class KeyCodeMapper
 * @brief Provides mapping between human-readable key names and system key codes
 */
class KeyCodeMapper {
public:
 /**
  * @brief Get the singleton instance of the mapper
  * @return Reference to the singleton instance
  */
 static const KeyCodeMapper& instance();

 /**
  * @brief Get system key code for a key name
  * @param keyName The human-readable key name
  * @return The system key code or -1 if not found
  */
 int getKeyCode(std::string_view keyName) const;

 /**
  * @brief Get key name for a system key code
  * @param keyCode The system key code
  * @return The human-readable key name or empty string if not found
  */
 std::string_view getKeyName(int keyCode) const;

 /**
  * @brief Check if a key name is valid
  * @param keyName The key name to check
  * @return true if valid, false otherwise
  */
 bool isValidKeyName(std::string_view keyName) const;

public:
 KeyCodeMapper(const KeyCodeMapper&) = delete;
 KeyCodeMapper& operator=(const KeyCodeMapper&) = delete;

private:
 KeyCodeMapper();

 // Maps for key lookups
 std::unordered_map<std::string, int> nameToCode;
 std::unordered_map<int, std::string> codeToName;
};