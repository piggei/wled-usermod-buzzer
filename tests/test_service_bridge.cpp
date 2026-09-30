#include "WLEDBuzzerService.h"

#include <cassert>
#include <cstring>

class MockService final : public WLEDBuzzerService {
public:
  void publish() { setInstance(this); }
  void unpublish() { setInstance(nullptr); }

  bool play(const char* soundId, bool loop = false) override {
    if (!ready_ || soundId == nullptr) return false;
    current_ = soundId;
    playing_ = true;
    loop_ = loop;
    repeat_ = 0;
    return true;
  }
  bool playRepeat(const char* soundId, uint16_t repeatCount) override {
    if (!ready_ || soundId == nullptr || repeatCount == 0) return false;
    current_ = soundId;
    playing_ = true;
    loop_ = false;
    repeat_ = repeatCount;
    return true;
  }
  bool beep(uint16_t durationMs, uint16_t frequencyHz) override {
    if (!ready_ || durationMs == 0 || frequencyHz == 0) return false;
    current_ = "beep";
    playing_ = true;
    duration_ = durationMs;
    frequency_ = frequencyHz;
    return true;
  }
  bool tone(uint16_t frequencyHz, uint16_t durationMs) override {
    if (!ready_ || durationMs == 0 || frequencyHz == 0) return false;
    current_ = "tone";
    playing_ = true;
    duration_ = durationMs;
    frequency_ = frequencyHz;
    return true;
  }
  void stop() override { playing_ = false; current_ = nullptr; }
  bool isReady() const override { return ready_; }
  bool isPlaying() const override { return playing_; }
  const char* currentSoundId() const override { return current_; }

  bool loop_ = false;
  uint16_t repeat_ = 0;
  uint16_t duration_ = 0;
  uint16_t frequency_ = 0;

private:
  bool ready_ = true;
  bool playing_ = false;
  const char* current_ = nullptr;
};

int main() {
  assert(!wledBuzzerServiceReady());
  assert(!wledBuzzerServicePlaying());
  assert(!wledBuzzerServicePlay("connect", false));
  assert(!wledBuzzerServicePlayRepeat("alarm", 3));
  assert(!wledBuzzerServiceBeep(120, 1000));
  assert(!wledBuzzerServiceTone(880, 200));
  assert(wledBuzzerServiceCurrentSoundId() == nullptr);
  wledBuzzerServiceStop();

  MockService service;
  service.publish();
  assert(wledBuzzerServiceReady());

  assert(wledBuzzerServicePlay("disconnect", true));
  assert(service.loop_);
  assert(wledBuzzerServicePlaying());
  assert(std::strcmp(wledBuzzerServiceCurrentSoundId(), "disconnect") == 0);

  assert(wledBuzzerServicePlayRepeat("alarm", 3));
  assert(service.repeat_ == 3);
  assert(std::strcmp(wledBuzzerServiceCurrentSoundId(), "alarm") == 0);

  assert(wledBuzzerServiceBeep(150, 1200));
  assert(service.duration_ == 150 && service.frequency_ == 1200);
  assert(std::strcmp(wledBuzzerServiceCurrentSoundId(), "beep") == 0);

  assert(wledBuzzerServiceTone(880, 220));
  assert(service.duration_ == 220 && service.frequency_ == 880);
  assert(std::strcmp(wledBuzzerServiceCurrentSoundId(), "tone") == 0);

  wledBuzzerServiceStop();
  assert(!wledBuzzerServicePlaying());
  service.unpublish();
  return 0;
}
