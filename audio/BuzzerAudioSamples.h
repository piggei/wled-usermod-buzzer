#pragma once

#include "BuzzerAudioConfig.h"
#include "../BuzzerSounds.h"

#include <cstddef>
#include <cstdint>

#if defined(WLED_BUZZER_ENABLE_AUDIO)

struct BuzzerAudioSample {
  const char* soundId;
  const int16_t* pcm;
  uint32_t frameCount;
  uint32_t sampleRate;
  const BuzzerSound* proxySound;
};

namespace BuzzerAudioSamples {
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
const BuzzerAudioSample* find(const char* soundId);
size_t count();
#else
inline const BuzzerAudioSample* find(const char*) { return nullptr; }
inline size_t count() { return 0u; }
#endif
}

#else

struct BuzzerAudioSample;

namespace BuzzerAudioSamples {
inline const BuzzerAudioSample* find(const char*) { return nullptr; }
inline size_t count() { return 0u; }
}

#endif
