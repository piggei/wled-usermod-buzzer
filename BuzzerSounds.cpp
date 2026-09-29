#include "BuzzerSounds.h"

#include <cstring>

namespace {
constexpr BuzzerNote SOUND_BEEP[] = {
  {1000, 120, 0},
};

constexpr BuzzerNote SOUND_DOUBLE_BEEP[] = {
  {1000, 80, 80},
  {1000, 110, 0},
};

// Original iDotMatrix three-pulse trill: 2 kHz passive tone,
// 90 ms ON with 70 ms gaps between the three pulses.
constexpr BuzzerNote SOUND_TRIPLE_BEEP[] = {
  {2000, 90, 70},
  {2000, 90, 70},
  {2000, 90, 0},
};

constexpr BuzzerNote SOUND_NOTIFICATION[] = {
  {880, 80, 45},
  {1175, 150, 0},
};

constexpr BuzzerNote SOUND_SUCCESS[] = {
  {659, 80, 30},
  {784, 90, 30},
  {988, 190, 0},
};

// Monophonic reduction of the supplied victory reference. The source builds
// an F-major arpeggio with overlapping tones; these measured onset pitches
// and intervals keep its contour recognizable on a single passive buzzer.
constexpr BuzzerNote SOUND_VICTORY[] = {
  {175, 145, 0},   // F3
  {220, 140, 0},   // A3
  {262, 145, 0},   // C4
  {349, 285, 0},   // F4
  {262, 140, 0},   // C4
  {349, 445, 0},   // F4
  {698, 700, 0},   // F5
};

// Chromatic "sad cartoon" descent inspired by the supplied failure reference.
constexpr BuzzerNote SOUND_FAIL[] = {
  {392, 170, 20},  // G4
  {370, 175, 20},  // F#4
  {349, 185, 20},  // F4
  {330, 205, 22},  // E4
  {311, 235, 24},  // Eb4
  {294, 520, 0},   // D4
};

constexpr BuzzerNote SOUND_WARNING[] = {
  {1175, 110, 70},
  {880, 210, 130},
  {1175, 110, 70},
  {880, 260, 0},
};

constexpr BuzzerNote SOUND_ERROR[] = {
  {294, 150, 150},
  {294, 190, 0},
};

constexpr BuzzerNote SOUND_CONNECT[] = {
  {784, 65, 25},
  {1047, 130, 0},
};

constexpr BuzzerNote SOUND_DISCONNECT[] = {
  {1047, 65, 25},
  {784, 150, 0},
};

constexpr BuzzerNote SOUND_ATTENTION[] = {
  {1319, 70, 65},
  {1319, 70, 65},
  {1319, 150, 0},
};

constexpr BuzzerNote SOUND_ALARM[] = {
  {880, 160, 45},
  {1175, 160, 45},
  {880, 160, 45},
  {1175, 160, 45},
  {880, 160, 45},
  {1175, 240, 180},
};

constexpr BuzzerSound SOUNDS[] = {
  {"beep", "Beep", SOUND_BEEP, sizeof(SOUND_BEEP) / sizeof(SOUND_BEEP[0])},
  {"double_beep", "Double Beep", SOUND_DOUBLE_BEEP, sizeof(SOUND_DOUBLE_BEEP) / sizeof(SOUND_DOUBLE_BEEP[0])},
  {"triple_beep", "Triple Beep", SOUND_TRIPLE_BEEP, sizeof(SOUND_TRIPLE_BEEP) / sizeof(SOUND_TRIPLE_BEEP[0])},
  {"notification", "Notification", SOUND_NOTIFICATION, sizeof(SOUND_NOTIFICATION) / sizeof(SOUND_NOTIFICATION[0])},
  {"success", "Success", SOUND_SUCCESS, sizeof(SOUND_SUCCESS) / sizeof(SOUND_SUCCESS[0])},
  {"victory", "Victory", SOUND_VICTORY, sizeof(SOUND_VICTORY) / sizeof(SOUND_VICTORY[0])},
  {"fail", "Fail", SOUND_FAIL, sizeof(SOUND_FAIL) / sizeof(SOUND_FAIL[0])},
  {"warning", "Warning", SOUND_WARNING, sizeof(SOUND_WARNING) / sizeof(SOUND_WARNING[0])},
  {"error", "Error", SOUND_ERROR, sizeof(SOUND_ERROR) / sizeof(SOUND_ERROR[0])},
  {"connect", "Connect", SOUND_CONNECT, sizeof(SOUND_CONNECT) / sizeof(SOUND_CONNECT[0])},
  {"disconnect", "Disconnect", SOUND_DISCONNECT, sizeof(SOUND_DISCONNECT) / sizeof(SOUND_DISCONNECT[0])},
  {"attention", "Attention", SOUND_ATTENTION, sizeof(SOUND_ATTENTION) / sizeof(SOUND_ATTENTION[0])},
  {"alarm", "Alarm", SOUND_ALARM, sizeof(SOUND_ALARM) / sizeof(SOUND_ALARM[0])},
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
