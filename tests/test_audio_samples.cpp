#include "audio/BuzzerAudioSamples.h"

#include <cassert>
#include <cstring>

int main() {
#if defined(WLED_BUZZER_ENABLE_AUDIO) && defined(WLED_BUZZER_AUDIO_SAMPLES)
  assert(BuzzerAudioSamples::count() == 15u);
  const char* ids[] = {
    "fail", "star_wars", "alarm", "connect", "disconnect", "notification",
    "yankee_doodle", "success", "warning", "attention", "error",
    "victory", "imperial_march", "trumpet", "wakeup"
  };
  for (const char* id : ids) {
    const BuzzerAudioSample* sample = BuzzerAudioSamples::find(id);
    assert(sample != nullptr);
    assert(std::strcmp(sample->soundId, id) == 0);
    assert(sample->pcm != nullptr);
    assert(sample->frameCount > 1000u);
    assert(sample->sampleRate == 16000u);
    assert(sample->proxySound != nullptr);
    assert(std::strcmp(sample->proxySound->id, id) == 0);
    assert(sample->proxySound->noteCount == 1u);
    assert(sample->proxySound->notes[0].durationMs > 100u);
    assert(sample->proxySound->notes[0].gapMs > 0u);
  }

  const BuzzerAudioSample* victory = BuzzerAudioSamples::find("victory");
  assert(victory->frameCount == 28003u);
  int victoryPeak = 0;
  for (size_t i = 0; i < victory->frameCount; ++i) {
    int value = victory->pcm[i];
    if (value < 0) value = -value;
    if (value > victoryPeak) victoryPeak = value;
  }
  assert(victoryPeak <= 11500); // v0.2.0 level match: 35% of the original full-scale sample.
  assert(BuzzerAudioSamples::find("imperial_march")->frameCount == 67677u);
  assert(BuzzerAudioSamples::find("trumpet")->frameCount == 104908u);
  assert(BuzzerAudioSamples::find("wakeup")->frameCount == 96039u);
  assert(BuzzerAudioSamples::find("missing") == nullptr);
#endif
  return 0;
}
