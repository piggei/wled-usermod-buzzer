#include "../BuzzerSchedule.h"

#include <cassert>
#include <cstdint>
#include <initializer_list>

int main() {
  uint16_t value = 0;
  assert(BuzzerSchedule::parseClockHHMM("00:00", value) && value == 0);
  assert(BuzzerSchedule::parseClockHHMM("07:05", value) && value == 425);
  assert(BuzzerSchedule::parseClockHHMM("23:59", value) && value == 1439);
  for (const char* bad : {"", "7:05", "07:5", "24:00", "23:60", "-1:00", "07.05", "0705"}) {
    assert(!BuzzerSchedule::parseClockHHMM(bad, value));
  }

  // Overnight interval: 23:00 -> 07:00.
  assert(!BuzzerSchedule::isMutedAtMinute(true, true, 22u * 60u + 59u, 23u * 60u, 7u * 60u));
  assert(BuzzerSchedule::isMutedAtMinute(true, true, 23u * 60u, 23u * 60u, 7u * 60u));
  assert(BuzzerSchedule::isMutedAtMinute(true, true, 2u * 60u, 23u * 60u, 7u * 60u));
  assert(!BuzzerSchedule::isMutedAtMinute(true, true, 7u * 60u, 23u * 60u, 7u * 60u));

  // Same-day interval: 13:30 -> 15:00.
  assert(!BuzzerSchedule::isMutedAtMinute(true, true, 13u * 60u + 29u, 810u, 900u));
  assert(BuzzerSchedule::isMutedAtMinute(true, true, 810u, 810u, 900u));
  assert(BuzzerSchedule::isMutedAtMinute(true, true, 899u, 810u, 900u));
  assert(!BuzzerSchedule::isMutedAtMinute(true, true, 900u, 810u, 900u));

  // Disabled or invalid time never mutes; equal limits mean full-day quiet.
  assert(!BuzzerSchedule::isMutedAtMinute(false, true, 100u, 0u, 0u));
  assert(!BuzzerSchedule::isMutedAtMinute(true, false, 100u, 0u, 0u));
  assert(BuzzerSchedule::isMutedAtMinute(true, true, 100u, 300u, 300u));
  return 0;
}
