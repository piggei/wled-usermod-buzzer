#pragma once

#include <cstddef>
#include <cstdint>

struct BuzzerNote {
  uint16_t frequencyHz;
  uint16_t durationMs;
  uint16_t gapMs;
};

enum BuzzerBackendMask : uint8_t {
  BUZZER_BACKEND_ACTIVE  = 0x01u,
  BUZZER_BACKEND_PASSIVE = 0x02u,
  BUZZER_BACKEND_AUDIO   = 0x04u,
  BUZZER_BACKEND_BUZZER  = BUZZER_BACKEND_ACTIVE | BUZZER_BACKEND_PASSIVE,
  BUZZER_BACKEND_ALL     = BUZZER_BACKEND_BUZZER | BUZZER_BACKEND_AUDIO,
};

struct BuzzerSound {
  const char* id;
  const char* label;
  const BuzzerNote* notes;
  size_t noteCount;
  uint8_t backendMask = BUZZER_BACKEND_ALL;
};

namespace BuzzerSounds {
const BuzzerSound* find(const char* id);
const BuzzerSound& at(size_t index);
size_t count();
bool supportsBackend(const BuzzerSound& sound, uint8_t backendMask);
}
