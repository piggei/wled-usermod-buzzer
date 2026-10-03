// SPDX-License-Identifier: Apache-2.0
#pragma once

#if defined(WLED_BUZZER_ENABLE_AUDIO)

#include <Arduino.h>
#include <Wire.h>

// Minimal ES8311 playback-side driver adapted from the Waveshare
// ESP32-S3-RGB-Matrix example package (Apache-2.0). See THIRD_PARTY_NOTICES.md.
class BuzzerES8311Codec {
public:
  explicit BuzzerES8311Codec(TwoWire* wire = &Wire) : wire_(wire) {}

  bool begin(int32_t sda, int32_t scl, uint32_t busFrequency, uint32_t sampleRate, uint8_t bitsPerSample, bool initializeBus = true);
  bool setVolume(uint8_t volume);
  bool setSampleRate(uint32_t sampleRate);
  bool setBitsPerSample(uint8_t bits);

public:
  struct Coeff {
    uint32_t mclk;
    uint32_t rate;
    uint8_t preDiv;
    uint8_t preMulti;
    uint8_t adcDiv;
    uint8_t dacDiv;
    uint8_t fsMode;
    uint8_t lrckH;
    uint8_t lrckL;
    uint8_t bclkDiv;
    uint8_t adcOsr;
    uint8_t dacOsr;
  };

private:
  int findCoeff(uint32_t mclk, uint32_t rate) const;
  bool writeReg(uint8_t reg, uint8_t value);
  uint8_t readReg(uint8_t reg);

  TwoWire* wire_ = nullptr;
  uint32_t mclkHz_ = 48000u * 256u;
};

#endif
