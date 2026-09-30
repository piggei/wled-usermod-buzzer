#include "WLEDBuzzerService.h"

namespace {
WLEDBuzzerService* serviceInstance = nullptr;
}

WLEDBuzzerService* WLEDBuzzerService::instance() {
  return serviceInstance;
}

void WLEDBuzzerService::setInstance(WLEDBuzzerService* instance) {
  serviceInstance = instance;
}

extern "C" bool wledBuzzerServiceReady() {
  WLEDBuzzerService* service = WLEDBuzzerService::instance();
  return service != nullptr && service->isReady();
}

extern "C" bool wledBuzzerServicePlaying() {
  WLEDBuzzerService* service = WLEDBuzzerService::instance();
  return service != nullptr && service->isPlaying();
}

extern "C" bool wledBuzzerServicePlay(const char* soundId, bool loop) {
  WLEDBuzzerService* service = WLEDBuzzerService::instance();
  return service != nullptr && service->isReady() && service->play(soundId, loop);
}

extern "C" bool wledBuzzerServicePlayRepeat(const char* soundId, uint16_t repeatCount) {
  WLEDBuzzerService* service = WLEDBuzzerService::instance();
  return service != nullptr && service->isReady() && service->playRepeat(soundId, repeatCount);
}

extern "C" bool wledBuzzerServiceBeep(uint16_t durationMs, uint16_t frequencyHz) {
  WLEDBuzzerService* service = WLEDBuzzerService::instance();
  return service != nullptr && service->isReady() && service->beep(durationMs, frequencyHz);
}

extern "C" bool wledBuzzerServiceTone(uint16_t frequencyHz, uint16_t durationMs) {
  WLEDBuzzerService* service = WLEDBuzzerService::instance();
  return service != nullptr && service->isReady() && service->tone(frequencyHz, durationMs);
}

extern "C" void wledBuzzerServiceStop() {
  WLEDBuzzerService* service = WLEDBuzzerService::instance();
  if (service != nullptr) service->stop();
}

extern "C" const char* wledBuzzerServiceCurrentSoundId() {
  WLEDBuzzerService* service = WLEDBuzzerService::instance();
  return service != nullptr ? service->currentSoundId() : nullptr;
}
