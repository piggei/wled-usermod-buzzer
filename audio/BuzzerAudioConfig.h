#pragma once

// Optional audio backend. Defining the board profile automatically enables
// audio support; default builds remain active/passive-only.
#if defined(WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX) && !defined(WLED_BUZZER_ENABLE_AUDIO)
#define WLED_BUZZER_ENABLE_AUDIO
#endif

#if defined(WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX) && !defined(WLED_BUZZER_DISABLE_AUDIO_SAMPLES) && !defined(WLED_BUZZER_AUDIO_SAMPLES)
#define WLED_BUZZER_AUDIO_SAMPLES
#endif

#if defined(WLED_BUZZER_ENABLE_AUDIO)

#include <cstdint>

namespace BuzzerAudioConfig {

constexpr uint32_t SAMPLE_RATE = 48000u;
constexpr uint8_t BITS_PER_SAMPLE = 16u;
constexpr uint32_t MCLK_HZ = SAMPLE_RATE * 256u;

// WLED AudioReactive uses I2S0 as the clock master at 22.05 kHz by default.
// When the Waveshare microphone owns the shared BCLK/LRCK/MCLK pins, the
// Buzzer backend switches I2S1 to TX slave mode and follows those clocks.
constexpr uint32_t SHARED_SAMPLE_RATE = 22050u;
#ifdef I2S_USE_16BIT_SAMPLES
constexpr uint8_t SHARED_BITS_PER_SAMPLE = 16u;
#else
constexpr uint8_t SHARED_BITS_PER_SAMPLE = 32u;
#endif

#if defined(WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX)
constexpr const char* PROFILE_NAME = "Waveshare ESP32-S3 RGB Matrix / ES8311";
constexpr int CODEC_I2C_SDA = 47;
constexpr int CODEC_I2C_SCL = 48;
constexpr int I2S_BCLK = 43;
constexpr int I2S_LRCK = 38;
constexpr int I2S_DOUT = 21;
constexpr int I2S_DIN = 39;
constexpr int I2S_MCLK = 12;
constexpr int PA_ENABLE = 11;
constexpr int I2S_PORT = 1;
#else
#error "WLED_BUZZER_ENABLE_AUDIO currently requires WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX"
#endif

} // namespace BuzzerAudioConfig

#endif // WLED_BUZZER_ENABLE_AUDIO
