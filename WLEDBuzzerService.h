#pragma once

#include <cstdint>

class WLEDBuzzerService {
public:
  virtual ~WLEDBuzzerService() = default;

  static WLEDBuzzerService* instance();

  virtual bool play(const char* soundId, bool loop = false) = 0;
  virtual bool playRepeat(const char* soundId, uint16_t repeatCount) = 0;
  virtual bool beep(uint16_t durationMs = 120, uint16_t frequencyHz = 1000) = 0;
  virtual bool tone(uint16_t frequencyHz, uint16_t durationMs) = 0;
  virtual void stop() = 0;
  virtual bool isReady() const = 0;
  virtual bool isPlaying() const = 0;
  virtual const char* currentSoundId() const = 0;

protected:
  static void setInstance(WLEDBuzzerService* instance);
};

// Stable optional C ABI for consumers that must remain linkable when this
// usermod is absent. Consumers may weak-link these functions without including
// this header or creating a PlatformIO library dependency.
extern "C" {
bool wledBuzzerServiceReady();
bool wledBuzzerServicePlaying();
bool wledBuzzerServicePlay(const char* soundId, bool loop);
bool wledBuzzerServicePlayRepeat(const char* soundId, uint16_t repeatCount);
bool wledBuzzerServiceBeep(uint16_t durationMs, uint16_t frequencyHz);
bool wledBuzzerServiceTone(uint16_t frequencyHz, uint16_t durationMs);
void wledBuzzerServiceStop();
const char* wledBuzzerServiceCurrentSoundId();
}
