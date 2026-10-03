// SPDX-License-Identifier: Apache-2.0
#include "BuzzerAudioConfig.h"

#if defined(WLED_BUZZER_ENABLE_AUDIO)

#include "ES8311Codec.h"

namespace {
constexpr uint8_t ES8311_ADDRESS = 0x18;
constexpr BuzzerES8311Codec::Coeff COEFFS[] = {
  // 22.05 kHz coefficients used when I2S clocks are owned by WLED AudioReactive.
  {5644800,  22050, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {2822400,  22050, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {1411200,  22050, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {11289600, 44100, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {5644800,  44100, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {2822400,  44100, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {1411200,  44100, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {12288000, 48000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {18432000, 48000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {6144000,  48000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {3072000,  48000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
  {1536000,  48000, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
};
}

int BuzzerES8311Codec::findCoeff(uint32_t mclk, uint32_t rate) const {
  for (size_t i = 0; i < sizeof(COEFFS) / sizeof(COEFFS[0]); ++i) {
    if (COEFFS[i].mclk == mclk && COEFFS[i].rate == rate) return static_cast<int>(i);
  }
  return -1;
}

bool BuzzerES8311Codec::begin(int32_t sda, int32_t scl, uint32_t busFrequency, uint32_t sampleRate, uint8_t bitsPerSample, bool initializeBus) {
  if (wire_ == nullptr || sda < 0 || scl < 0) return false;
  // In shared-I2S mode the board I2C bus is already live (ES7210 / sensors).
  // Do not reinitialize it: only address the ES8311 at 0x18.
  if (initializeBus && !wire_->begin(sda, scl, busFrequency)) return false;

  wire_->beginTransmission(ES8311_ADDRESS);
  if (wire_->endTransmission() != 0) return false;

  bool ok = true;
  ok &= writeReg(0x00, 0x1F);
  delay(20);
  ok &= writeReg(0x00, 0x00);
  ok &= writeReg(0x00, 0x80);
  ok &= writeReg(0x01, 0x3F);

  uint8_t reg = readReg(0x06);
  reg &= ~(1U << 5);
  ok &= writeReg(0x06, reg);
  ok &= setSampleRate(sampleRate);
  ok &= setBitsPerSample(bitsPerSample);
  ok &= writeReg(0x0D, 0x01);
  ok &= writeReg(0x0E, 0x02);
  ok &= writeReg(0x12, 0x00);
  ok &= writeReg(0x13, 0x10);
  ok &= writeReg(0x1C, 0x6A);
  ok &= writeReg(0x37, 0x08);
  return ok;
}

bool BuzzerES8311Codec::setVolume(uint8_t volume) {
  if (volume > 100u) volume = 100u;
  const uint8_t value = volume == 0u ? 0u : static_cast<uint8_t>(((static_cast<uint16_t>(volume) * 256u) / 100u) - 1u);
  return writeReg(0x32, value);
}

bool BuzzerES8311Codec::setSampleRate(uint32_t sampleRate) {
  mclkHz_ = sampleRate * 256u;
  if (sampleRate > 64000u) mclkHz_ /= 2u;
  const int index = findCoeff(mclkHz_, sampleRate);
  if (index < 0) return false;
  const Coeff& c = COEFFS[index];

  bool ok = true;
  uint8_t reg = readReg(0x02);
  reg |= static_cast<uint8_t>((c.preDiv - 1u) << 5);
  reg |= static_cast<uint8_t>(c.preMulti << 3);
  ok &= writeReg(0x02, reg);
  ok &= writeReg(0x03, static_cast<uint8_t>((c.fsMode << 6) | c.adcOsr));
  ok &= writeReg(0x04, c.dacOsr);
  ok &= writeReg(0x05, static_cast<uint8_t>(((c.adcDiv - 1u) << 4) | (c.dacDiv - 1u)));

  reg = readReg(0x06);
  reg &= 0xE0;
  reg |= c.bclkDiv < 19u ? static_cast<uint8_t>(c.bclkDiv - 1u) : c.bclkDiv;
  ok &= writeReg(0x06, reg);

  reg = readReg(0x07);
  reg &= 0xC0;
  reg |= c.lrckH;
  ok &= writeReg(0x07, reg);
  ok &= writeReg(0x08, c.lrckL);
  return ok;
}

bool BuzzerES8311Codec::setBitsPerSample(uint8_t bits) {
  uint8_t code = 0;
  switch (bits) {
    case 16: code = 3; break;
    case 18: code = 2; break;
    case 20: code = 1; break;
    case 24: code = 0; break;
    case 32: code = 4; break;
    default: return false;
  }
  uint8_t reg09 = readReg(0x09);
  uint8_t reg0A = readReg(0x0A);
  reg09 = static_cast<uint8_t>((reg09 & ~(7u << 2)) | (code << 2));
  reg0A = static_cast<uint8_t>((reg0A & ~(7u << 2)) | (code << 2));
  return writeReg(0x09, reg09) && writeReg(0x0A, reg0A);
}

bool BuzzerES8311Codec::writeReg(uint8_t reg, uint8_t value) {
  wire_->beginTransmission(ES8311_ADDRESS);
  wire_->write(reg);
  wire_->write(value);
  return wire_->endTransmission() == 0;
}

uint8_t BuzzerES8311Codec::readReg(uint8_t reg) {
  wire_->beginTransmission(ES8311_ADDRESS);
  wire_->write(reg);
  if (wire_->endTransmission(false) != 0) return 0;
  if (wire_->requestFrom(static_cast<uint16_t>(ES8311_ADDRESS), static_cast<uint8_t>(1), true) != 1u) return 0;
  return wire_->available() ? wire_->read() : 0u;
}

#endif
