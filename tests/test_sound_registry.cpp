#include "../audio/BuzzerAudioConfig.h"
#include "../BuzzerSounds.h"

#include <cassert>
#include <cstring>
#include <initializer_list>

int main() {
  static const char* EXPECTED_IDS[] = {
    "alarm", "attention", "beep", "connect", "disconnect",
    "double_beep", "error", "fail",
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
    "imperial_march",
#endif
    "notification", "star_wars", "success", "triple_beep",
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
    "trumpet",
#endif
    "victory",
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
    "wakeup",
#endif
    "warning", "yankee_doodle",
  };
  assert(BuzzerSounds::count() == (sizeof(EXPECTED_IDS) / sizeof(EXPECTED_IDS[0])));
  for (const char* id : EXPECTED_IDS) assert(BuzzerSounds::find(id) != nullptr);
  assert(BuzzerSounds::find("missing") == nullptr);

  for (size_t i = 1; i < BuzzerSounds::count(); ++i) {
    assert(std::strcmp(BuzzerSounds::at(i - 1u).label, BuzzerSounds::at(i).label) < 0);
  }
  for (size_t i = 0; i < BuzzerSounds::count(); ++i) {
    const BuzzerSound& sound = BuzzerSounds::at(i);
    const bool audioOnly = sound.backendMask == BUZZER_BACKEND_AUDIO;
    if (audioOnly) {
      assert(!BuzzerSounds::supportsBackend(sound, BUZZER_BACKEND_ACTIVE));
      assert(!BuzzerSounds::supportsBackend(sound, BUZZER_BACKEND_PASSIVE));
      assert(BuzzerSounds::supportsBackend(sound, BUZZER_BACKEND_AUDIO));
      assert(sound.notes == nullptr);
      assert(sound.noteCount == 0u);
    } else {
      assert(sound.backendMask == BUZZER_BACKEND_ALL);
      assert(BuzzerSounds::supportsBackend(sound, BUZZER_BACKEND_ACTIVE));
      assert(BuzzerSounds::supportsBackend(sound, BUZZER_BACKEND_PASSIVE));
      assert(BuzzerSounds::supportsBackend(sound, BUZZER_BACKEND_AUDIO));
      assert(sound.notes != nullptr);
      assert(sound.noteCount > 0u);
      assert(sound.notes[sound.noteCount - 1u].gapMs > 0u);
      for (size_t n = 0; n < sound.noteCount; ++n) {
        assert(sound.notes[n].durationMs > 0);
        assert(sound.notes[n].frequencyHz > 0);
      }
    }
    assert(sound.id != nullptr && *sound.id != '\0');
    assert(sound.label != nullptr && *sound.label != '\0');
    for (size_t j = i + 1; j < BuzzerSounds::count(); ++j) {
      assert(std::strcmp(sound.id, BuzzerSounds::at(j).id) != 0);
    }
  }

#if defined(WLED_BUZZER_AUDIO_SAMPLES)
  for (const char* id : {"imperial_march", "trumpet", "wakeup"}) {
    const BuzzerSound* sound = BuzzerSounds::find(id);
    assert(sound != nullptr);
    assert(sound->backendMask == BUZZER_BACKEND_AUDIO);
    assert(sound->notes == nullptr && sound->noteCount == 0u);
  }
#else
  assert(BuzzerSounds::find("imperial_march") == nullptr);
  assert(BuzzerSounds::find("trumpet") == nullptr);
  assert(BuzzerSounds::find("wakeup") == nullptr);
#endif

  const BuzzerSound* triple = BuzzerSounds::find("triple_beep");
  assert(triple != nullptr);
  assert(triple->noteCount == 3u);
  assert(triple->notes[0].durationMs == 90u && triple->notes[0].gapMs == 70u);
  assert(triple->notes[1].durationMs == 90u && triple->notes[1].gapMs == 70u);
  assert(triple->notes[2].durationMs == 90u && triple->notes[2].gapMs == 550u);
  return 0;
}
