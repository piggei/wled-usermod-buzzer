#include "BuzzerAudioConfig.h"

#if defined(WLED_BUZZER_ENABLE_AUDIO)

#include "BuzzerAudioBackend.h"

#include <driver/i2s.h>
#include <driver/gpio.h>
#include <esp_idf_version.h>
#include <esp_timer.h>
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#include <esp_rom_gpio.h>
#include <soc/gpio_sig_map.h>
#include <soc/gpio_pins.h>
#endif

namespace {
constexpr TickType_t I2S_WRITE_TIMEOUT = pdMS_TO_TICKS(100);
constexpr int16_t SINE_LUT[64] = {
  0,3212,6393,9512,12539,15446,18204,20787,
  23170,25329,27245,28898,30273,31356,32137,32609,
  32767,32609,32137,31356,30273,28898,27245,25329,
  23170,20787,18204,15446,12539,9512,6393,3212,
  0,-3212,-6393,-9512,-12539,-15446,-18204,-20787,
  -23170,-25329,-27245,-28898,-30273,-31356,-32137,-32609,
  -32767,-32609,-32137,-31356,-30273,-28898,-27245,-25329,
  -23170,-20787,-18204,-15446,-12539,-9512,-6393,-3212,
};

constexpr i2s_port_t audioPort() {
  return BuzzerAudioConfig::I2S_PORT == 0 ? I2S_NUM_0 : I2S_NUM_1;
}

i2s_bits_per_sample_t bitsPerSample(uint8_t bits) {
  return bits == 32u ? I2S_BITS_PER_SAMPLE_32BIT : I2S_BITS_PER_SAMPLE_16BIT;
}

#if defined(CONFIG_IDF_TARGET_ESP32S3)
bool enableSharedClockInputs() {
  // GPIO43/GPIO38 are already driven by AudioReactive/I2S0. Enabling the
  // input buffer does not remove the existing output route, but it is required
  // before those pad levels can be consumed by a second peripheral through
  // the GPIO matrix.
  const esp_err_t bclk = gpio_input_enable(static_cast<gpio_num_t>(BuzzerAudioConfig::I2S_BCLK));
  const esp_err_t lrck = gpio_input_enable(static_cast<gpio_num_t>(BuzzerAudioConfig::I2S_LRCK));
  return bclk == ESP_OK && lrck == ESP_OK;
}

bool sharedLrckIsRunning(uint32_t timeoutUs = 5000u) {
  const gpio_num_t lrck = static_cast<gpio_num_t>(BuzzerAudioConfig::I2S_LRCK);
  const int initial = gpio_get_level(lrck);
  const int64_t deadline = esp_timer_get_time() + static_cast<int64_t>(timeoutUs);
  while (esp_timer_get_time() < deadline) {
    if (gpio_get_level(lrck) != initial) return true;
  }
  return false;
}
#endif
}


bool BuzzerAudioBackend::begin(uint8_t volumePercent, bool sharedClockMode, bool initializeCodecBus) {
  if (ready_) return true;
  lastError_ = "initializing";

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
  (void)volumePercent;
  (void)sharedClockMode;
  (void)initializeCodecBus;
  lastError_ = "unsupported target";
  return false;
#else
  sharedClockMode_ = sharedClockMode;
  streamSampleRate_ = sharedClockMode_ ? BuzzerAudioConfig::SHARED_SAMPLE_RATE : BuzzerAudioConfig::SAMPLE_RATE;
  streamBitsPerSample_ = sharedClockMode_ ? BuzzerAudioConfig::SHARED_BITS_PER_SAMPLE : BuzzerAudioConfig::BITS_PER_SAMPLE;

  if (sharedClockMode_) {
    // PinManager ownership alone is not enough: AudioReactive can own the pins
    // before I2S0 has started producing clocks. Never install/prime I2S1 until
    // LRCK is physically toggling. This also prevents repeated failed bring-up
    // attempts from clicking the PA and perturbing the microphone path.
    if (!enableSharedClockInputs()) {
      lastError_ = "shared clock input enable failed";
      return false;
    }
    if (!sharedLrckIsRunning()) {
      lastError_ = "shared LRCK inactive";
      return false;
    }
  }

  pinMode(BuzzerAudioConfig::PA_ENABLE, OUTPUT);
  digitalWrite(BuzzerAudioConfig::PA_ENABLE, HIGH);

  i2s_config_t cfg{};
  cfg.mode = static_cast<i2s_mode_t>((sharedClockMode_ ? I2S_MODE_SLAVE : I2S_MODE_MASTER) | I2S_MODE_TX);
  cfg.sample_rate = streamSampleRate_;
  cfg.bits_per_sample = bitsPerSample(streamBitsPerSample_);
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
#if ESP_IDF_VERSION_MAJOR >= 5
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
#else
  cfg.communication_format = I2S_COMM_FORMAT_I2S;
#endif
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
#if ESP_IDF_VERSION_MAJOR >= 5
  cfg.dma_desc_num = DMA_BUFFER_COUNT;
  cfg.dma_frame_num = FRAMES_PER_BUFFER;
#else
  cfg.dma_buf_count = DMA_BUFFER_COUNT;
  cfg.dma_buf_len = FRAMES_PER_BUFFER;
#endif
  cfg.use_apll = false;
  cfg.tx_desc_auto_clear = true;
  cfg.fixed_mclk = sharedClockMode_ ? 0u : BuzzerAudioConfig::MCLK_HZ;

  if (i2s_driver_install(audioPort(), &cfg, 0, nullptr) != ESP_OK) {
    lastError_ = "I2S1 driver install failed";
    digitalWrite(BuzzerAudioConfig::PA_ENABLE, LOW);
    return false;
  }

  // Never let the slave-TX path reconfigure the physical BCLK/LRCK pads.
  // AudioReactive/I2S0 owns those GPIOs as master outputs. Calling
  // i2s_set_pin() with those pins for I2S1 slave mode changes pad direction
  // and can stop I2S0 clocks. Configure DOUT only, then *tap* the existing
  // clock levels into I2S1 through the GPIO matrix. A GPIO input can feed
  // multiple peripheral signals without disturbing the existing output route.
  i2s_pin_config_t pins{};
  pins.mck_io_num = sharedClockMode_ ? I2S_PIN_NO_CHANGE : BuzzerAudioConfig::I2S_MCLK;
  pins.bck_io_num = sharedClockMode_ ? I2S_PIN_NO_CHANGE : BuzzerAudioConfig::I2S_BCLK;
  pins.ws_io_num = sharedClockMode_ ? I2S_PIN_NO_CHANGE : BuzzerAudioConfig::I2S_LRCK;
  pins.data_out_num = BuzzerAudioConfig::I2S_DOUT;
  pins.data_in_num = I2S_PIN_NO_CHANGE;
  if (i2s_set_pin(audioPort(), &pins) != ESP_OK) {
    lastError_ = "I2S1 DOUT routing failed";
    i2s_driver_uninstall(audioPort());
    digitalWrite(BuzzerAudioConfig::PA_ENABLE, LOW);
    return false;
  }

  if (sharedClockMode_) {
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    esp_rom_gpio_connect_in_signal(
      static_cast<uint32_t>(BuzzerAudioConfig::I2S_BCLK),
      I2S1O_BCK_IN_IDX,
      false);
    esp_rom_gpio_connect_in_signal(
      static_cast<uint32_t>(BuzzerAudioConfig::I2S_LRCK),
      I2S1O_WS_IN_IDX,
      false);
#else
    lastError_ = "shared clock tap unsupported";
    i2s_driver_uninstall(audioPort());
    digitalWrite(BuzzerAudioConfig::PA_ENABLE, LOW);
    return false;
#endif
  }

  // Only the standalone path is allowed to program clock generation. In
  // shared-I2S mode AudioReactive/I2S0 is the sole clock owner.
  if (!sharedClockMode_ &&
      i2s_set_clk(audioPort(), streamSampleRate_, bitsPerSample(streamBitsPerSample_), I2S_CHANNEL_STEREO) != ESP_OK) {
    lastError_ = "I2S1 clock setup failed";
    i2s_driver_uninstall(audioPort());
    digitalWrite(BuzzerAudioConfig::PA_ENABLE, LOW);
    return false;
  }
  i2s_zero_dma_buffer(audioPort());

  if (!codec_.begin(
        BuzzerAudioConfig::CODEC_I2C_SDA,
        BuzzerAudioConfig::CODEC_I2C_SCL,
        400000u,
        streamSampleRate_,
        streamBitsPerSample_,
        initializeCodecBus)) {
    lastError_ = "ES8311 initialization failed";
    i2s_driver_uninstall(audioPort());
    digitalWrite(BuzzerAudioConfig::PA_ENABLE, LOW);
    return false;
  }
  if (!codec_.setVolume(100u)) {
    lastError_ = "ES8311 volume setup failed";
    i2s_driver_uninstall(audioPort());
    digitalWrite(BuzzerAudioConfig::PA_ENABLE, LOW);
    return false;
  }

  portENTER_CRITICAL(&lock_);
  volumePercent_ = volumePercent > 100u ? 100u : volumePercent;
  volumeRefreshPending_ = true;
  selectedSample_ = nullptr;
  samplePlaying_ = false;
  samplePositionQ32_ = 0;
  sampleGeneration_ = 0;
  toneOn_ = false;
  frequencyHz_ = 0;
  streamPrimed_ = false;
  writes_ = 0;
  writeErrors_ = 0;
  shortWrites_ = 0;
  maxWriteUs_ = 0;
  maxProducerGapUs_ = 0;
  lateWrites_ = 0;
  core0Runs_ = 0;
  core1Runs_ = 0;
  stackMinFree_ = 0;
  lastWriteStartUs_ = 0;
  running_ = true;
  portEXIT_CRITICAL(&lock_);

  if (xTaskCreate(taskThunk, "buzzer-audio", 5120, this, 1, &task_) != pdPASS) {
    lastError_ = "audio producer task creation failed";
    portENTER_CRITICAL(&lock_);
    running_ = false;
    portEXIT_CRITICAL(&lock_);
    i2s_driver_uninstall(audioPort());
    digitalWrite(BuzzerAudioConfig::PA_ENABLE, LOW);
    task_ = nullptr;
    return false;
  }

  // Shared mode only becomes ready if AudioReactive is really producing the
  // external clocks. This also prevents a false-ready slave TX after RX stops.
  for (uint16_t i = 0; i < 400u && (!streamPrimed_ || volumeRefreshPending_); ++i) delay(1);
  if (!streamPrimed_ || volumeRefreshPending_) {
    lastError_ = sharedClockMode_ ? "shared clock/DMA priming timeout" : "I2S/DMA priming timeout";
    end();
    return false;
  }

  ready_ = true;
  lastError_ = "none";
  return true;
#endif
}

void BuzzerAudioBackend::end() {
  if (!ready_ && task_ == nullptr) return;
  portENTER_CRITICAL(&lock_);
  toneOn_ = false;
  frequencyHz_ = 0;
  selectedSample_ = nullptr;
  samplePlaying_ = false;
  samplePositionQ32_ = 0;
  ++sampleGeneration_;
  running_ = false;
  portEXIT_CRITICAL(&lock_);

  for (uint8_t i = 0; task_ != nullptr && i < 60u; ++i) delay(2);
  digitalWrite(BuzzerAudioConfig::PA_ENABLE, LOW);
  // In slave mode the DMA clock belongs to AudioReactive. Do not perform a
  // zero-buffer transaction while tearing down a shared stream; just detach
  // our TX driver. Standalone mode keeps the historical zero-before-uninstall.
  if (!sharedClockMode_) i2s_zero_dma_buffer(audioPort());
  i2s_driver_uninstall(audioPort());
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  if (sharedClockMode_) {
    // Remove only our internal clock taps. This does not touch the GPIO pad
    // configuration or AudioReactive/I2S0 output routing.
    esp_rom_gpio_connect_in_signal(GPIO_MATRIX_CONST_ZERO_INPUT, I2S1O_BCK_IN_IDX, false);
    esp_rom_gpio_connect_in_signal(GPIO_MATRIX_CONST_ZERO_INPUT, I2S1O_WS_IN_IDX, false);
  }
#endif
  ready_ = false;
  streamPrimed_ = false;
  sharedClockMode_ = false;
  streamSampleRate_ = BuzzerAudioConfig::SAMPLE_RATE;
  streamBitsPerSample_ = BuzzerAudioConfig::BITS_PER_SAMPLE;
  phase_ = 0;
}

BuzzerAudioBackend::Telemetry BuzzerAudioBackend::telemetry() {
  Telemetry snapshot;
  portENTER_CRITICAL(&lock_);
  snapshot.writes = writes_;
  snapshot.writeErrors = writeErrors_;
  snapshot.shortWrites = shortWrites_;
  snapshot.maxWriteUs = maxWriteUs_;
  snapshot.maxProducerGapUs = maxProducerGapUs_;
  snapshot.lateWrites = lateWrites_;
  snapshot.core0Runs = core0Runs_;
  snapshot.core1Runs = core1Runs_;
  snapshot.stackMinFree = stackMinFree_;
  snapshot.dmaCoverageUs = streamSampleRate_ > 0u
    ? static_cast<uint32_t>((static_cast<uint64_t>(FRAMES_PER_BUFFER) * DMA_BUFFER_COUNT * 1000000ull) / streamSampleRate_)
    : 0u;
  portEXIT_CRITICAL(&lock_);
  return snapshot;
}

void BuzzerAudioBackend::selectSample(const BuzzerAudioSample* sample) {
  portENTER_CRITICAL(&lock_);
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
  selectedSample_ = sample;
#else
  (void)sample;
  selectedSample_ = nullptr;
#endif
  samplePlaying_ = false;
  samplePositionQ32_ = 0;
  ++sampleGeneration_;
  portEXIT_CRITICAL(&lock_);
}

void BuzzerAudioBackend::setTone(uint16_t frequencyHz, bool on) {
  if (on) {
    pinMode(BuzzerAudioConfig::PA_ENABLE, OUTPUT);
    digitalWrite(BuzzerAudioConfig::PA_ENABLE, HIGH);
  }
  portENTER_CRITICAL(&lock_);
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
  if (on && selectedSample_ != nullptr) {
    toneOn_ = false;
    frequencyHz_ = 0;
    samplePlaying_ = true;
    samplePositionQ32_ = 0;
    volumeRefreshPending_ = true;
    ++sampleGeneration_;
    portEXIT_CRITICAL(&lock_);
    return;
  }
#endif
  samplePlaying_ = false;
  samplePositionQ32_ = 0;
  ++sampleGeneration_;
  frequencyHz_ = frequencyHz;
  toneOn_ = on && frequencyHz > 0u;
  if (toneOn_) volumeRefreshPending_ = true;
  portEXIT_CRITICAL(&lock_);
}

void BuzzerAudioBackend::setVolume(uint8_t volumePercent) {
  if (volumePercent > 100u) volumePercent = 100u;
  pinMode(BuzzerAudioConfig::PA_ENABLE, OUTPUT);
  digitalWrite(BuzzerAudioConfig::PA_ENABLE, HIGH);
  portENTER_CRITICAL(&lock_);
  volumePercent_ = volumePercent;
  volumeRefreshPending_ = true;
  portEXIT_CRITICAL(&lock_);
}

void BuzzerAudioBackend::taskThunk(void* context) {
  static_cast<BuzzerAudioBackend*>(context)->taskLoop();
}

void BuzzerAudioBackend::taskLoop() {
  union AudioFrames {
    int16_t s16[FRAMES_PER_BUFFER * 2u];
    int32_t s32[FRAMES_PER_BUFFER * 2u];
  } frames{};

  while (true) {
    uint16_t frequency = 0;
    bool toneOn = false;
    bool running = false;
    const BuzzerAudioSample* sample = nullptr;
    bool samplePlaying = false;
    uint64_t samplePosition = 0;
    uint32_t sampleGeneration = 0;
    bool refreshVolume = false;
    uint8_t requestedVolume = 100u;

    portENTER_CRITICAL(&lock_);
    frequency = frequencyHz_;
    toneOn = toneOn_;
    running = running_;
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
    sample = selectedSample_;
    samplePlaying = samplePlaying_;
    samplePosition = samplePositionQ32_;
    sampleGeneration = sampleGeneration_;
#endif
    refreshVolume = volumeRefreshPending_;
    requestedVolume = volumePercent_;
    portEXIT_CRITICAL(&lock_);
    if (!running) break;

    if (streamPrimed_ && refreshVolume) {
      pinMode(BuzzerAudioConfig::PA_ENABLE, OUTPUT);
      digitalWrite(BuzzerAudioConfig::PA_ENABLE, HIGH);
      if (codec_.setVolume(requestedVolume)) {
        portENTER_CRITICAL(&lock_);
        if (volumePercent_ == requestedVolume) volumeRefreshPending_ = false;
        portEXIT_CRITICAL(&lock_);
      }
    }

    bool sampleEnded = false;
    const uint64_t sampleStep = samplePlaying && sample != nullptr && sample->sampleRate > 0u
      ? (static_cast<uint64_t>(sample->sampleRate) << 32) / streamSampleRate_
      : 0u;
    const uint32_t phaseStep = toneOn && frequency > 0u
      ? static_cast<uint32_t>((static_cast<uint64_t>(frequency) << 32) / streamSampleRate_)
      : 0u;
    constexpr int32_t amplitude = 16000;

    auto nextValue = [&]() -> int16_t {
#if defined(WLED_BUZZER_AUDIO_SAMPLES)
      if (samplePlaying && sample != nullptr && sample->pcm != nullptr && sample->frameCount > 0u && sampleStep > 0u) {
        const uint32_t index = static_cast<uint32_t>(samplePosition >> 32);
        if (index < sample->frameCount) {
          const int16_t value = sample->pcm[index];
          samplePosition += sampleStep;
          return value;
        }
        sampleEnded = true;
        return 0;
      }
#endif
      if (toneOn && phaseStep != 0u) {
        const uint8_t index = static_cast<uint8_t>(phase_ >> 26);
        const int16_t value = static_cast<int16_t>((static_cast<int32_t>(SINE_LUT[index]) * amplitude) / 32767);
        phase_ += phaseStep;
        return value;
      }
      return 0;
    };

    size_t bytesToWrite = 0;
    if (streamBitsPerSample_ == 32u) {
      for (size_t i = 0; i < FRAMES_PER_BUFFER; ++i) {
        const int32_t value = static_cast<int32_t>(nextValue()) * 65536;
        frames.s32[i * 2u] = value;
        frames.s32[i * 2u + 1u] = value;
      }
      bytesToWrite = sizeof(frames.s32);
    } else {
      for (size_t i = 0; i < FRAMES_PER_BUFFER; ++i) {
        const int16_t value = nextValue();
        frames.s16[i * 2u] = value;
        frames.s16[i * 2u + 1u] = value;
      }
      bytesToWrite = sizeof(frames.s16);
    }

    const uint32_t writeStartUs = static_cast<uint32_t>(esp_timer_get_time());
    const uint32_t producerGapUs = lastWriteStartUs_ == 0u ? 0u : writeStartUs - lastWriteStartUs_;
    lastWriteStartUs_ = writeStartUs;

    size_t written = 0;
    const esp_err_t writeResult = i2s_write(audioPort(), &frames, bytesToWrite, &written, I2S_WRITE_TIMEOUT);
    const uint32_t writeDurationUs = static_cast<uint32_t>(esp_timer_get_time()) - writeStartUs;
    if (writeResult == ESP_OK && written > 0u) streamPrimed_ = true;

    const uint32_t dmaCoverageUs = streamSampleRate_ > 0u
      ? static_cast<uint32_t>((static_cast<uint64_t>(FRAMES_PER_BUFFER) * DMA_BUFFER_COUNT * 1000000ull) / streamSampleRate_)
      : 0u;
    const BaseType_t core = xPortGetCoreID();
    const bool sampleStack = ((writes_ + 1u) & 0x3Fu) == 0u;
    const uint32_t stackFree = sampleStack ? static_cast<uint32_t>(uxTaskGetStackHighWaterMark(nullptr)) : 0u;

    portENTER_CRITICAL(&lock_);
    ++writes_;
    if (writeResult != ESP_OK) ++writeErrors_;
    if (writeResult == ESP_OK && written != bytesToWrite) ++shortWrites_;
    if (writeDurationUs > maxWriteUs_) maxWriteUs_ = writeDurationUs;
    if (producerGapUs > maxProducerGapUs_) maxProducerGapUs_ = producerGapUs;
    if (producerGapUs > dmaCoverageUs && dmaCoverageUs > 0u) ++lateWrites_;
    if (core == 0) ++core0Runs_;
    else if (core == 1) ++core1Runs_;
    if (sampleStack && (stackMinFree_ == 0u || stackFree < stackMinFree_)) stackMinFree_ = stackFree;
    portEXIT_CRITICAL(&lock_);

#if defined(WLED_BUZZER_AUDIO_SAMPLES)
    if (samplePlaying && sample != nullptr) {
      portENTER_CRITICAL(&lock_);
      if (sampleGeneration_ == sampleGeneration && selectedSample_ == sample) {
        samplePositionQ32_ = samplePosition;
        if (sampleEnded || static_cast<uint32_t>(samplePosition >> 32) >= sample->frameCount) samplePlaying_ = false;
      }
      portEXIT_CRITICAL(&lock_);
    }
#endif
  }

  task_ = nullptr;
  vTaskDelete(nullptr);
}

#endif
