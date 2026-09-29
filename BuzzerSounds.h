#pragma once

#include <cstddef>
#include <cstdint>

struct BuzzerNote {
  uint16_t frequencyHz;
  uint16_t durationMs;
  uint16_t gapMs;
};

struct BuzzerSound {
  const char* id;
  const char* label;
  const BuzzerNote* notes;
  size_t noteCount;
};

namespace BuzzerSounds {
const BuzzerSound* find(const char* id);
const BuzzerSound& at(size_t index);
size_t count();
}
