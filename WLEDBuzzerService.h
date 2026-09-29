#pragma once

#include <cstdint>

class WLEDBuzzerService {
public:
  virtual ~WLEDBuzzerService() = default;

  static WLEDBuzzerService* instance();

  virtual bool play(const char* soundId, bool loop = false) = 0;
  virtual bool beep(uint16_t durationMs = 120, uint16_t frequencyHz = 1000) = 0;
  virtual bool tone(uint16_t frequencyHz, uint16_t durationMs) = 0;
  virtual void stop() = 0;
  virtual bool isReady() const = 0;
  virtual bool isPlaying() const = 0;
  virtual const char* currentSoundId() const = 0;

protected:
  static void setInstance(WLEDBuzzerService* instance);
};
