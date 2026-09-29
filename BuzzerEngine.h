#pragma once

#include "BuzzerSounds.h"

#include <cstddef>
#include <cstdint>

#if defined(ARDUINO_ARCH_ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif

class BuzzerEngine {
public:
  using OutputCallback = void (*)(void* context, uint16_t frequencyHz, bool on);

  void attach(OutputCallback callback, void* context) {
    outputCallback_ = callback;
    outputContext_ = context;
  }

  bool beginThreadSafe();

  bool play(const BuzzerSound& sound, uint64_t nowUs, bool loop = false);
  bool playTone(uint16_t frequencyHz, uint16_t durationMs, uint64_t nowUs, const char* id = "tone");
  bool stop();
  void service(uint64_t nowUs);

  bool isPlaying() const;
  bool isLooping() const;
  bool outputOn() const;
  const char* currentSoundId() const;
  size_t currentNoteIndex() const;
  size_t currentNoteCount() const;
  uint16_t currentFrequencyHz() const;
  uint32_t lastLatenessUs() const;
  uint32_t maxLatenessUs() const;

private:
  enum class Phase : uint8_t {
    Idle = 0,
    Tone,
    Gap,
  };

  static constexpr uint32_t COMMAND_LOCK_TIMEOUT_MS = 25u;

  bool lock(bool wait) const;
  void unlock() const;
  void setOutputLocked(uint16_t frequencyHz, bool on);
  bool startLocked(const BuzzerNote* notes, size_t noteCount, const char* id, uint64_t nowUs, bool loop);
  void startCurrentNoteLocked(uint64_t nowUs, bool preservePhase);
  void advanceNoteLocked(uint64_t nowUs);
  void stopLocked();
  void scheduleNextLocked(uint64_t nowUs, uint64_t intervalUs);
  bool hasFollowingNoteLocked() const;

  OutputCallback outputCallback_ = nullptr;
  void* outputContext_ = nullptr;

  const BuzzerNote* notes_ = nullptr;
  size_t noteCount_ = 0;
  const char* currentSoundId_ = nullptr;
  BuzzerNote customNote_{1000, 100, 0};

  Phase phase_ = Phase::Idle;
  size_t noteIndex_ = 0;
  bool loop_ = false;
  bool outputOn_ = false;
  uint16_t outputFrequencyHz_ = 0;
  uint64_t nextChangeAtUs_ = 0;
  uint32_t lastLatenessUs_ = 0;
  uint32_t maxLatenessUs_ = 0;

#if defined(ARDUINO_ARCH_ESP32)
  mutable SemaphoreHandle_t mutex_ = nullptr;
#endif
};
