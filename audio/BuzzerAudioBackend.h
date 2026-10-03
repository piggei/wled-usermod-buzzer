#pragma once

#include "BuzzerAudioConfig.h"

#if defined(WLED_BUZZER_ENABLE_AUDIO)

#include <Arduino.h>
#include <cstdint>

#include "BuzzerAudioSamples.h"
#include "ES8311Codec.h"

class BuzzerAudioBackend {
public:
  struct Telemetry {
    uint32_t writes = 0;
    uint32_t writeErrors = 0;
    uint32_t shortWrites = 0;
    uint32_t maxWriteUs = 0;
    uint32_t maxProducerGapUs = 0;
    uint32_t lateWrites = 0;
    uint32_t core0Runs = 0;
    uint32_t core1Runs = 0;
    uint32_t stackMinFree = 0;
    uint32_t dmaCoverageUs = 0;
  };

  static constexpr uint16_t FRAMES_PER_BUFFER = 256u;
  static constexpr uint8_t DMA_BUFFER_COUNT = 8u;

  bool begin(uint8_t volumePercent, bool sharedClockMode = false, bool initializeCodecBus = true);
  void end();
  void setTone(uint16_t frequencyHz, bool on);
  void setVolume(uint8_t volumePercent);
  void selectSample(const BuzzerAudioSample* sample);
  bool isReady() const { return ready_; }
  bool isSharedClockMode() const { return sharedClockMode_; }
  uint32_t streamSampleRate() const { return streamSampleRate_; }
  uint8_t streamBitsPerSample() const { return streamBitsPerSample_; }
  const char* profileName() const { return BuzzerAudioConfig::PROFILE_NAME; }
  const char* modeName() const { return sharedClockMode_ ? "shared-I2S" : "standalone"; }
  const char* lastError() const { return lastError_; }
  Telemetry telemetry();

private:
  static void taskThunk(void* context);
  void taskLoop();

  BuzzerES8311Codec codec_;
  TaskHandle_t task_ = nullptr;
  portMUX_TYPE lock_ = portMUX_INITIALIZER_UNLOCKED;
  volatile bool running_ = false;
  volatile bool streamPrimed_ = false;
  volatile bool toneOn_ = false;
  volatile uint16_t frequencyHz_ = 0;
  volatile uint8_t volumePercent_ = 100;
  volatile bool volumeRefreshPending_ = false;
  const BuzzerAudioSample* selectedSample_ = nullptr;
  volatile bool samplePlaying_ = false;
  uint64_t samplePositionQ32_ = 0;
  uint32_t sampleGeneration_ = 0;
  bool ready_ = false;
  bool sharedClockMode_ = false;
  uint32_t streamSampleRate_ = BuzzerAudioConfig::SAMPLE_RATE;
  uint8_t streamBitsPerSample_ = BuzzerAudioConfig::BITS_PER_SAMPLE;
  uint32_t phase_ = 0;
  volatile uint32_t writes_ = 0;
  volatile uint32_t writeErrors_ = 0;
  volatile uint32_t shortWrites_ = 0;
  volatile uint32_t maxWriteUs_ = 0;
  volatile uint32_t maxProducerGapUs_ = 0;
  volatile uint32_t lateWrites_ = 0;
  volatile uint32_t core0Runs_ = 0;
  volatile uint32_t core1Runs_ = 0;
  volatile uint32_t stackMinFree_ = 0;
  uint32_t lastWriteStartUs_ = 0;
  const char* lastError_ = "not initialized";
};

#endif
