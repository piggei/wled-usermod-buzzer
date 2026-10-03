#define WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX
#include "../audio/BuzzerAudioConfig.h"

#include <cassert>
#include <cstring>

int main() {
#ifndef WLED_BUZZER_ENABLE_AUDIO
#error "Waveshare profile must enable the audio backend"
#endif
#ifndef WLED_BUZZER_AUDIO_SAMPLES
#error "Waveshare profile must enable bundled audio samples by default"
#endif
  assert(BuzzerAudioConfig::SAMPLE_RATE == 48000u);
  assert(BuzzerAudioConfig::BITS_PER_SAMPLE == 16u);
  assert(BuzzerAudioConfig::MCLK_HZ == 12288000u);
  assert(BuzzerAudioConfig::SHARED_SAMPLE_RATE == 22050u);
#ifndef I2S_USE_16BIT_SAMPLES
  assert(BuzzerAudioConfig::SHARED_BITS_PER_SAMPLE == 32u);
#endif
  assert(BuzzerAudioConfig::CODEC_I2C_SDA == 47);
  assert(BuzzerAudioConfig::CODEC_I2C_SCL == 48);
  assert(BuzzerAudioConfig::I2S_BCLK == 43);
  assert(BuzzerAudioConfig::I2S_LRCK == 38);
  assert(BuzzerAudioConfig::I2S_DOUT == 21);
  assert(BuzzerAudioConfig::I2S_DIN == 39);
  assert(BuzzerAudioConfig::I2S_MCLK == 12);
  assert(BuzzerAudioConfig::PA_ENABLE == 11);
  assert(BuzzerAudioConfig::I2S_PORT == 1);
  assert(std::strstr(BuzzerAudioConfig::PROFILE_NAME, "Waveshare") != nullptr);
  return 0;
}
