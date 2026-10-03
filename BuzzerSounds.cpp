#include "BuzzerSounds.h"
#include "audio/BuzzerAudioConfig.h"

#include <cstring>

namespace {
constexpr BuzzerNote SOUND_BEEP[] = {
  {1000, 120, 200},
};

constexpr BuzzerNote SOUND_DOUBLE_BEEP[] = {
  {1000, 80, 80},
  {1000, 110, 250},
};

// High three-pulse trill: 2 kHz passive tone,
// 90 ms ON with 70 ms gaps between the three pulses. The final 550 ms gap
// is consumed only when another execution follows, so one-shot playback keeps
// the validated three-pulse cadence while repeat/loop playback gets a clean pause.
constexpr BuzzerNote SOUND_TRIPLE_BEEP[] = {
  {2000, 90, 70},
  {2000, 90, 70},
  {2000, 90, 550},
};

constexpr BuzzerNote SOUND_NOTIFICATION[] = {
  {1175, 150, 250},
};

constexpr BuzzerNote SOUND_SUCCESS[] = {
  {659, 80, 30},
  {784, 90, 30},
  {988, 190, 300},
};

// Victory pattern from the supplied BeepBox v9 URL, transposed +12 semitones.
// Hardware-validated timing: double-speed phrase with an extended final note.
constexpr BuzzerNote SOUND_VICTORY[] = {
  {1047, 100, 50}, // C6
  {1047, 100, 50}, // C6
  {1319, 100, 50}, // E6
  {1568, 200, 100}, // G6
  {1319, 100, 50}, // E6
  {1568, 400, 100}, // G6 - extended final note
};

// Yankee Doodle: first two phrases, revised ending after hardware test.
// Keep the former penultimate B4 as the held ending and drop only the final G4.
constexpr BuzzerNote SOUND_YANKEE_DOODLE[] = {
  {523, 280, 10}, // C5
  {523, 280, 10}, // C5
  {587, 280, 10}, // D5
  {659, 280, 10}, // E5
  {523, 280, 10}, // C5
  {659, 280, 10}, // E5
  {587, 560, 80}, // D5
  {523, 280, 10}, // C5
  {523, 280, 10}, // C5
  {587, 280, 10}, // D5
  {659, 280, 10}, // E5
  {523, 560, 10}, // C5 - held penultimate note
  {494, 560, 400},  // B4 - held ending
};

// Exact monophonic Fail pattern decoded from the supplied BeepBox v9 URL.
// Transposed +12 semitones after hardware validation of the original octave.
// 150 BPM timing is unchanged: 300 ms notes with 100 ms gaps, followed by
// two longer descending notes (600 ms / 800 ms).
constexpr BuzzerNote SOUND_FAIL[] = {
  {262, 300, 100}, // C4
  {247, 300, 100}, // B3
  {233, 300, 100}, // A#3
  {220, 300, 100}, // A3
  {208, 600, 200}, // G#3
  {196, 800, 350},   // G3
};

// Star Wars cue measured directly from the supplied reference MP3.
// Timing follows the 150 BPM BeepBox export: three C4 staccato cuts,
// then F4/C5 sustained notes, a 400 ms rest, and the complete closing phrase.
constexpr BuzzerNote SOUND_STAR_WARS[] = {
  {262, 100, 100}, // C4 cut 1/3
  {262, 100, 100}, // C4 cut 2/3
  {262, 100, 100}, // C4 cut 3/3
  {349,1200,   0}, // F4 long
  {523,1200, 400}, // C5 long + phrase rest
  {466, 400,   0}, // A#4 / Bb4
  {440, 400,   0}, // A4
  {392, 400,   0}, // G4
  {698,1200,   0}, // F5 long
  {523,1200, 400}, // C5 long ending
};

constexpr BuzzerNote SOUND_WARNING[] = {
  {1175, 110, 70},
  {880, 210, 130},
  {1175, 110, 70},
  {880, 260, 250},
};

constexpr BuzzerNote SOUND_ERROR[] = {
  {294, 150, 150},
  {294, 190, 300},
};

constexpr BuzzerNote SOUND_CONNECT[] = {
  {880, 65, 25},
  {1175, 130, 250},
};

constexpr BuzzerNote SOUND_DISCONNECT[] = {
  {1175, 65, 25},
  {880, 150, 250},
};

constexpr BuzzerNote SOUND_ATTENTION[] = {
  {1319, 70, 65},
  {1319, 70, 65},
  {1319, 150, 300},
};

constexpr BuzzerNote SOUND_ALARM[] = {
  {880, 320, 45},
  {1175, 320, 45},
  {880, 320, 45},
  {1175, 320, 45},
  {880, 320, 45},
  {1175, 480, 180},
};

// The final gap of every built-in sound is an inter-execution pause.
// BuzzerEngine consumes it only when repeat/loop playback has another execution,
// so one-shot playback remains audibly unchanged.
constexpr BuzzerSound SOUNDS[] = {
  {"alarm", "Alarm", SOUND_ALARM, sizeof(SOUND_ALARM) / sizeof(SOUND_ALARM[0]), BUZZER_BACKEND_ALL},
  {"attention", "Attention", SOUND_ATTENTION, sizeof(SOUND_ATTENTION) / sizeof(SOUND_ATTENTION[0]), BUZZER_BACKEND_ALL},
  {"beep", "Beep", SOUND_BEEP, sizeof(SOUND_BEEP) / sizeof(SOUND_BEEP[0]), BUZZER_BACKEND_ALL},
  {"connect", "Connect", SOUND_CONNECT, sizeof(SOUND_CONNECT) / sizeof(SOUND_CONNECT[0]), BUZZER_BACKEND_ALL},
  {"disconnect", "Disconnect", SOUND_DISCONNECT, sizeof(SOUND_DISCONNECT) / sizeof(SOUND_DISCONNECT[0]), BUZZER_BACKEND_ALL},
  {"double_beep", "Double Beep", SOUND_DOUBLE_BEEP, sizeof(SOUND_DOUBLE_BEEP) / sizeof(SOUND_DOUBLE_BEEP[0]), BUZZER_BACKEND_ALL},
  {"error", "Error", SOUND_ERROR, sizeof(SOUND_ERROR) / sizeof(SOUND_ERROR[0]), BUZZER_BACKEND_ALL},
  {"fail", "Fail", SOUND_FAIL, sizeof(SOUND_FAIL) / sizeof(SOUND_FAIL[0]), BUZZER_BACKEND_ALL},
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
  {"imperial_march", "Imperial March", nullptr, 0u, BUZZER_BACKEND_AUDIO},
#endif
  {"notification", "Notification", SOUND_NOTIFICATION, sizeof(SOUND_NOTIFICATION) / sizeof(SOUND_NOTIFICATION[0]), BUZZER_BACKEND_ALL},
  {"star_wars", "Star Wars", SOUND_STAR_WARS, sizeof(SOUND_STAR_WARS) / sizeof(SOUND_STAR_WARS[0]), BUZZER_BACKEND_ALL},
  {"success", "Success", SOUND_SUCCESS, sizeof(SOUND_SUCCESS) / sizeof(SOUND_SUCCESS[0]), BUZZER_BACKEND_ALL},
  {"triple_beep", "Triple Beep", SOUND_TRIPLE_BEEP, sizeof(SOUND_TRIPLE_BEEP) / sizeof(SOUND_TRIPLE_BEEP[0]), BUZZER_BACKEND_ALL},
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
  {"trumpet", "Trumpet", nullptr, 0u, BUZZER_BACKEND_AUDIO},
#endif
  {"victory", "Victory", SOUND_VICTORY, sizeof(SOUND_VICTORY) / sizeof(SOUND_VICTORY[0]), BUZZER_BACKEND_ALL},
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
  {"wakeup", "Wake Up", nullptr, 0u, BUZZER_BACKEND_AUDIO},
#endif
  {"warning", "Warning", SOUND_WARNING, sizeof(SOUND_WARNING) / sizeof(SOUND_WARNING[0]), BUZZER_BACKEND_ALL},
  {"yankee_doodle", "Yankee Doodle", SOUND_YANKEE_DOODLE, sizeof(SOUND_YANKEE_DOODLE) / sizeof(SOUND_YANKEE_DOODLE[0]), BUZZER_BACKEND_ALL},
// Audio-only entries are compiled only when bundled samples are enabled. This
// keeps their IDs and labels out of Active/Passive-only firmware completely.
};
}

const BuzzerSound* BuzzerSounds::find(const char* id) {
  if (id == nullptr || *id == '\0') return nullptr;
  for (const BuzzerSound& sound : SOUNDS) {
    if (std::strcmp(sound.id, id) == 0) return &sound;
  }
  return nullptr;
}

const BuzzerSound& BuzzerSounds::at(size_t index) {
  if (index >= count()) index = 0;
  return SOUNDS[index];
}

size_t BuzzerSounds::count() {
  return sizeof(SOUNDS) / sizeof(SOUNDS[0]);
}


bool BuzzerSounds::supportsBackend(const BuzzerSound& sound, uint8_t backendMask) {
  return (sound.backendMask & backendMask) != 0u;
}
