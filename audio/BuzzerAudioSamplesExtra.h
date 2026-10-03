#pragma once

#include "BuzzerAudioSamples.h"

#if defined(WLED_BUZZER_ENABLE_AUDIO) && defined(WLED_BUZZER_AUDIO_SAMPLES)
namespace BuzzerAudioSamplesExtra {
const BuzzerAudioSample* find(const char* soundId);
size_t count();
}
#endif
