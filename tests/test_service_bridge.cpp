#include "WLEDBuzzerService.h"

#include <cassert>
#include <cstring>

class MockService final : public WLEDBuzzerService {
public:
  void publish() { setInstance(this); }
  void unpublish() { setInstance(nullptr); }

  bool play(const char* soundId, bool loop = false) override {
    (void)loop;
    if (!ready_ || soundId == nullptr) return false;
    current_ = soundId;
    playing_ = true;
    return true;
  }
  bool playRepeat(const char*, uint16_t) override { return false; }
  bool beep(uint16_t, uint16_t) override { return false; }
  bool tone(uint16_t, uint16_t) override { return false; }
  void stop() override { playing_ = false; current_ = nullptr; }
  bool isReady() const override { return ready_; }
  bool isPlaying() const override { return playing_; }
  const char* currentSoundId() const override { return current_; }

private:
  bool ready_ = true;
  bool playing_ = false;
  const char* current_ = nullptr;
};

int main() {
  assert(!wledBuzzerServiceReady());
  assert(!wledBuzzerServicePlaying());
  assert(!wledBuzzerServicePlay("connect", false));
  assert(wledBuzzerServiceCurrentSoundId() == nullptr);
  wledBuzzerServiceStop();

  MockService service;
  service.publish();
  assert(wledBuzzerServiceReady());
  assert(wledBuzzerServicePlay("disconnect", false));
  assert(wledBuzzerServicePlaying());
  assert(std::strcmp(wledBuzzerServiceCurrentSoundId(), "disconnect") == 0);
  wledBuzzerServiceStop();
  assert(!wledBuzzerServicePlaying());
  service.unpublish();
  return 0;
}
