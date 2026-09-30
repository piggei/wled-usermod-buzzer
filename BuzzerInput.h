#pragma once

#include <cstdint>

namespace BuzzerInput {

inline bool parseUnsignedDecimal(const char* value, uint32_t minimum, uint32_t maximum, uint32_t& output) {
  if (value == nullptr || *value == '\0' || minimum > maximum) return false;

  uint32_t parsed = 0;
  for (const char* p = value; *p != '\0'; ++p) {
    if (*p < '0' || *p > '9') return false;
    const uint32_t digit = static_cast<uint32_t>(*p - '0');
    if (digit > maximum || parsed > (maximum - digit) / 10u) return false;
    parsed = parsed * 10u + digit;
  }

  if (parsed < minimum || parsed > maximum) return false;
  output = parsed;
  return true;
}

inline char asciiLower(char value) {
  return value >= 'A' && value <= 'Z' ? static_cast<char>(value + ('a' - 'A')) : value;
}

inline bool equalsIgnoreCase(const char* left, const char* right) {
  if (left == nullptr || right == nullptr) return false;
  while (*left != '\0' && *right != '\0') {
    if (asciiLower(*left) != asciiLower(*right)) return false;
    ++left;
    ++right;
  }
  return *left == '\0' && *right == '\0';
}

inline bool parseBooleanText(const char* value, bool& output) {
  if (value == nullptr || *value == '\0') return false;
  if (equalsIgnoreCase(value, "1") || equalsIgnoreCase(value, "true") || equalsIgnoreCase(value, "on")) {
    output = true;
    return true;
  }
  if (equalsIgnoreCase(value, "0") || equalsIgnoreCase(value, "false") || equalsIgnoreCase(value, "off")) {
    output = false;
    return true;
  }
  return false;
}

} // namespace BuzzerInput
