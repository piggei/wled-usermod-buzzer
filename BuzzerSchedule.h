#pragma once

#include <cstdint>

namespace BuzzerSchedule {

inline bool parseClockHHMM(const char* value, uint16_t& minutesOfDay) {
  if (value == nullptr) return false;
  if (value[0] < '0' || value[0] > '9' ||
      value[1] < '0' || value[1] > '9' ||
      value[2] != ':' ||
      value[3] < '0' || value[3] > '9' ||
      value[4] < '0' || value[4] > '9' ||
      value[5] != '\0') return false;

  const uint8_t hour = static_cast<uint8_t>((value[0] - '0') * 10 + (value[1] - '0'));
  const uint8_t minute = static_cast<uint8_t>((value[3] - '0') * 10 + (value[4] - '0'));
  if (hour > 23 || minute > 59) return false;

  minutesOfDay = static_cast<uint16_t>(hour) * 60u + minute;
  return true;
}

inline bool isMutedAtMinute(bool enabled, bool timeValid, uint16_t currentMinute,
                            uint16_t startMinute, uint16_t endMinute) {
  if (!enabled || !timeValid || currentMinute >= 1440u ||
      startMinute >= 1440u || endMinute >= 1440u) return false;

  // Equal boundaries intentionally mean a full-day quiet period.
  if (startMinute == endMinute) return true;
  if (startMinute < endMinute) {
    return currentMinute >= startMinute && currentMinute < endMinute;
  }
  return currentMinute >= startMinute || currentMinute < endMinute;
}

} // namespace BuzzerSchedule
