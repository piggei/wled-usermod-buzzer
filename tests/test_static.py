#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
usermod = (root / "usermod_buzzer.cpp").read_text(encoding="utf-8")
readme = (root / "README.md").read_text(encoding="utf-8")
assert "Development status" not in readme
assert not (root / "docs" / "DEVELOPMENT_NEXT.md").exists()
library = (root / "library.json").read_text(encoding="utf-8")
sounds = (root / "BuzzerSounds.cpp").read_text(encoding="utf-8")

for token in [
    'BUZZER_VERSION = "0.2.0"',
    'BUZZER_BUILD = "release"',
    'PinManager::allocatePin(hardwarePin_, true, PinOwner::UM_Unspecified)',
    'PinManager::allocateLedc(1)',
    'PinManager::deallocateLedc(ledcChannel_, 1)',
    'esp_timer_start_periodic(serviceTimer_, SERVICE_PERIOD_US)',
    'server.on(F("/buzzer"), HTTP_GET',
    'playSoundRepeat(soundId.c_str(), static_cast<uint16_t>(repeat))',
    'Use either loop or repeat, not both.',
    'repeat must be an integer between 1 and 255.',
    'root.createNestedObject(FPSTR(JSON_KEY))',
    'WLEDBuzzerService::setInstance(this)',
    "addOption(dd,'Active',0)",
    "addOption(dd,'Passive',1)",
    "addOption(dd,'High',0)",
    "addOption(dd,'Low',1)",
    "p.textContent='Play'",
    "z.textContent='Stop'",
    'location.origin+\'/buzzer?play=\'',
    "d.createTextNode(' Show API commands')",
    "a.hidden=true",
    "c.onchange=()=>a.hidden=!c.checked",
]:
    assert token in usermod, token

for sound in [
    "beep", "double_beep", "triple_beep", "notification", "success", "victory", "fail",
    "yankee_doodle", "star_wars", "warning", "error", "connect", "disconnect", "attention", "alarm",
    "imperial_march", "trumpet", "wakeup",
]:
    assert f"`{sound}`" in readme, sound

assert '"version": "0.2.0"' in library
assert "iDotMatrix" not in usermod
assert "IDotMatrix" not in usermod
assert "delay(" not in usermod
assert "x.firstChild.textContent=''" not in usermod
assert "Output level for passive buzzer (0-100%)." not in usermod
assert "r.type='range'" in usermod
assert "r.min=0" in usermod
assert "r.max=100" in usermod
assert "r.step=1" in usermod
assert "Math.round(Number(r.value)||0)" in usermod
assert "n.textContent=v+'%'" in usermod
assert "V.firstChild.data='Volume: '" in usermod
assert "Active buzzers reproduce rhythm only. Passive buzzers reproduce note pitch." in usermod
assert "{2000, 90, 70}" in sounds
assert sounds.count("{2000, 90, 70}") == 2
assert "{2000, 90, 550}" in sounds
assert "{2000, 90, 0}" not in sounds
for token in ["{1047, 100, 50}", "{1319, 100, 50}", "{1568, 200, 100}", "{1568, 400, 100}"]:
    assert token in sounds, token
assert sounds.count("{1047, 100, 50}") == 2
assert sounds.count("{1319, 100, 50}") == 2
assert sounds.count("{1568, 200, 100}") == 1
assert sounds.count("{1568, 400, 100}") == 1
assert "SOUND_YANKEE_DOODLE" in sounds
assert sounds.count("// C5") >= 6
assert "hr{display:none}" in usermod and "Buzzer:enabled" in usermod
assert "{294, 150, 150}" in sounds
assert "{294, 190, 300}" in sounds
assert "{392, 140, 35}" not in sounds
assert "margin:16px 0 8px" in usermod

assert "{262, 300, 100}" in sounds

assert "{247, 300, 100}" in sounds

assert "{233, 300, 100}" in sounds

assert "{220, 300, 100}" in sounds

assert "{208, 600, 200}" in sounds

assert "{196, 800, 350}" in sounds

assert "{523, 560, 10}" in sounds

assert "{494, 560, 400}" in sounds

# Qualified sound regression checks.
assert "SOUND_STAR_WARS" in sounds
assert '{"star_wars", "Star Wars"' in sounds
assert "{1175, 150, 250}" in sounds
assert "{880, 80, 45}" not in sounds
assert "{880, 65, 25}" in sounds
assert "{1175, 130, 250}" in sounds
assert "{1175, 65, 25}" in sounds
assert "{880, 150, 250}" in sounds

# b018 Star Wars reference-MP3 regression checks.
assert "{1568, 400, 100}" in sounds
assert sounds.count("{262, 100, 100}") == 3
for token in [
    "{349,1200,   0}", "{523,1200, 400}",
    "{466, 400,   0}", "{440, 400,   0}", "{392, 400,   0}",
    "{698,1200,   0}", "{523,1200, 400}",
]:
    assert token in sounds, token

# Release hardening and UI regression checks.
assert 'BUZZER_BUILD = "release"' in usermod
assert '#include "BuzzerInput.h"' in usermod
assert 'BuzzerInput::parseUnsignedDecimal' in usermod
assert 'BuzzerInput::parseBooleanText' in usermod
assert 'Use one buzzer command per request.' in usermod
assert 'tone must be an integer between 20 and 20000 Hz.' in usermod
assert 'duration must be an integer between 1 and 60000 ms.' in usermod
assert 'repeat must be an integer between 1 and 255.' in usermod
assert 'xSemaphoreTake(mutex_, portMAX_DELAY)' in (root / "BuzzerEngine.cpp").read_text(encoding="utf-8")
assert 'if (!engineThreadSafe_) return false;' in usermod
assert 'BuzzerEngine::Snapshot snapshot = engine_.snapshot();' in usermod
assert 'Snapshot snapshot() const;' in (root / "BuzzerEngine.h").read_text(encoding="utf-8")
assert "addInfo('Buzzer:enabled',1,'<style>" in usermod and "hr{display:none}" in usermod
assert "addInfo('Buzzer:pin',1,'');" in usermod
assert "addInfo('Buzzer:sound',1,'');" in usermod
assert "Enabled Enabled" not in usermod
assert "Sound Sound" not in usermod
assert 'curl "http://<WLED-IP>/buzzer?play=alarm&repeat=3"' in readme
assert 'curl "http://<WLED-IP>/buzzer?play=alarm&loop=1"' in readme
assert '20-20000 Hz' in readme
assert '1-60000 ms' in readme
assert 'idle' in readme
assert ' / ready / ' not in readme
assert "low-pitch" not in (root / "TESTING.md").read_text(encoding="utf-8")

# RC5 UI contract: do not re-wrap the non-volume rows.
for token in [
    "L(e,'Enabled:')", "L(p,'GPIO Pin:')", "L(t,'Buzzer Type:')",
    "L(g,'Trigger level:')", "L(s,'Sound:')", "V=w(r)",
    "V.firstChild.data='Volume: '",
]:
    assert token in usermod, token
for forbidden in ["E=w(e)", "T=w(t)", "S=w(s)"]:
    assert forbidden not in usermod, forbidden
assert ".sec:has([name=\\\"Buzzer:enabled\\\"])>hr{display:none}" in usermod
assert "P=w(p)" in usermod and "G=w(g)" in usermod
assert "if(P)P.hidden=a" in usermod and "if(G)G.hidden=a" in usermod
assert "if(V)V.hidden=!v" in usermod
assert "addInfo('Buzzer:enabled',1,'<style>" in usermod
assert "addInfo('Buzzer:enabled',1,'<style>.sec:has([name=\"Buzzer:enabled\"])>hr{display:none}</style>','Enabled:')" not in usermod
assert "addInfo('Buzzer:pin',1,'','GPIO:')" not in usermod
assert "addInfo('Buzzer:sound',1,'','Sound:')" not in usermod
assert "docs/wled-buzzer-usermod-gui.png" in readme
assert (root / "docs" / "wled-buzzer-usermod-gui.png").is_file()

# RC6 optional-consumer bridge contract.
service_h = (root / "WLEDBuzzerService.h").read_text(encoding="utf-8")
service_cpp = (root / "WLEDBuzzerService.cpp").read_text(encoding="utf-8")
for token in [
    "wledBuzzerServiceReady", "wledBuzzerServicePlaying",
    "wledBuzzerServicePlay", "wledBuzzerServicePlayRepeat",
    "wledBuzzerServiceBeep", "wledBuzzerServiceTone",
    "wledBuzzerServiceStop", "wledBuzzerServiceCurrentSoundId",
]:
    assert token in service_h, token
    assert token in service_cpp, token
assert 'extern "C"' in service_h



# v0.1.1 Night Mode contract.
schedule = (root / "BuzzerSchedule.h").read_text(encoding="utf-8")
for token in [
    'CFG_NIGHT_MODE[] PROGMEM = "nightMode"',
    'CFG_NIGHT_FROM[] PROGMEM = "nightFrom"',
    'CFG_NIGHT_TO[] PROGMEM = "nightTo"',
    'nightFrom_ = "23:00"', 'nightTo_ = "07:00"',
    'BuzzerSchedule::isMutedAtMinute', 'Buzzer muted by night mode.',
    'state["muted"] = isNightMutedNow();',
    'state.add(F("muted by night mode"))',
    "L(m,'Night mode:')", "N0.firstChild.data='From: '", "L(f1,'To:')",
    "if(f0){f0.type='time';f0.style.width='120px'}", "if(f1){f1.type='time';f1.style.width='120px'}",
    'if(N0)N0.hidden=!v', 'if(N1)N1.hidden=!v',
    'if (setupComplete_ && mutedAfterConfig && engine_.isPlaying()) stopPlayback();',
]:
    assert token in usermod, token
assert 'parseClockHHMM' in schedule
assert 'startMinute == endMinute' in schedule
assert (root / "tests" / "test_night_mode.cpp").exists()


# Handoff documentation contract.
assert (root / "docs" / "SHARED_I2S_AUDIOREACTIVE.md").is_file()
assert "hardware-qualified" in (root / "docs" / "SHARED_I2S_AUDIOREACTIVE.md").read_text(encoding="utf-8").lower()

print("Static regression checks passed.")

# v0.2.0 optional I2S audio backend contract.
audio_cfg = (root / "audio" / "BuzzerAudioConfig.h").read_text(encoding="utf-8")
audio_backend = (root / "audio" / "BuzzerAudioBackend.cpp").read_text(encoding="utf-8")
codec = (root / "audio" / "ES8311Codec.cpp").read_text(encoding="utf-8")

# v0.2.0 current cold-boot ordering regression.
assert "AUDIO_BOOT_DEFER_MS = 1500u" in usermod
assert "audioBootInitPending_" in usermod
assert "volumeRefreshPending_" in (root / "audio" / "BuzzerAudioBackend.h").read_text(encoding="utf-8")
assert "codec_.setVolume(100u)" in audio_backend
assert "if (streamPrimed_ && refreshVolume)" in audio_backend
assert "digitalWrite(BuzzerAudioConfig::PA_ENABLE, HIGH)" in audio_backend
for path in [
    root / "audio" / "BuzzerAudioConfig.h",
    root / "audio" / "BuzzerAudioBackend.h",
    root / "audio" / "BuzzerAudioBackend.cpp",
    root / "audio" / "ES8311Codec.h",
    root / "audio" / "ES8311Codec.cpp",
    root / "THIRD_PARTY_NOTICES.md",
    root / "tests" / "test_audio_config.cpp",
]:
    assert path.is_file(), path
for token in [
    "WLED_BUZZER_ENABLE_AUDIO", "WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX",
    "CODEC_I2C_SDA = 47", "CODEC_I2C_SCL = 48", "I2S_BCLK = 43", "I2S_LRCK = 38",
    "I2S_DOUT = 21", "I2S_MCLK = 12", "PA_ENABLE = 11", "I2S_PORT = 1",
]:
    assert token in audio_cfg, token
for token in [
    "i2s_driver_install", "i2s_set_pin", "i2s_set_clk", "i2s_write",
    "xTaskCreate", "SINE_LUT", "CONFIG_IDF_TARGET_ESP32S3",
]:
    assert token in audio_backend, token
assert "ES8311_ADDRESS = 0x18" in codec
assert "audio/BuzzerAudioConfig.h" in usermod
assert "audio/BuzzerAudioBackend.h" in usermod
assert "BUZZER_TYPE_AUDIO = 2" in usermod
assert "addOption(dd,'Audio (I2S)',2)" in usermod
assert "audioBackend_.begin(hardwareVolume_, sharedI2s, !sharedI2s)" in usermod
assert 'state["backend"]' in usermod
assert '+<audio/*.cpp>' in library
assert 'WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX' in readme

assert "codec_.setVolume(100u)" in audio_backend
assert "constexpr int32_t amplitude = 16000" in audio_backend
assert "16000 * static_cast<int32_t>(volume)" not in audio_backend
assert "config[FPSTR(CFG_TYPE)] = buzzerType_;\n    config[FPSTR(CFG_PIN)] = pin_;" in usermod

# v0.2.0 embedded sample backend contract.
audio_samples_h = (root / "audio" / "BuzzerAudioSamples.h").read_text(encoding="utf-8")
audio_samples_cpp = (root / "audio" / "BuzzerAudioSamples.cpp").read_text(encoding="utf-8")
audio_samples_extra_h = (root / "audio" / "BuzzerAudioSamplesExtra.h").read_text(encoding="utf-8")
audio_samples_extra_cpp = (root / "audio" / "BuzzerAudioSamplesExtra.cpp").read_text(encoding="utf-8")
for path in [root / "audio" / "BuzzerAudioSamples.h", root / "audio" / "BuzzerAudioSamples.cpp", root / "audio" / "BuzzerAudioSamplesExtra.h", root / "audio" / "BuzzerAudioSamplesExtra.cpp", root / "tests" / "test_audio_samples.cpp", root / "tests" / "test_audio_config_no_samples.cpp"]:
    assert path.is_file(), path
for token in ["WLED_BUZZER_AUDIO_SAMPLES", "WLED_BUZZER_DISABLE_AUDIO_SAMPLES", "BuzzerAudioSamples::find", "BuzzerAudioSamples::count"]:
    assert token in (audio_cfg + audio_samples_h + audio_samples_cpp), token
for token in ["fail", "star_wars", "alarm", "connect", "disconnect", "notification", "yankee_doodle", "success", "warning", "attention", "error"]:
    assert f'\"{token}\"' in audio_samples_cpp, token
for token in ["victory", "imperial_march", "trumpet", "wakeup"]:
    assert f'\"{token}\"' in audio_samples_extra_cpp, token
assert "BuzzerAudioSamplesExtra::find(soundId)" in audio_samples_cpp
assert "BuzzerAudioSamplesExtra::count()" in audio_samples_cpp
assert "sample->proxySound" in usermod
assert "audioBackend_.selectSample(sample)" in usermod
assert 'state["source"]' in usermod
assert 'return BuzzerAudioSamples::find(soundId) != nullptr ? "sample" : "tone";' in usermod
assert "sample->pcm[index]" in audio_backend
assert "samplePositionQ32_" in audio_backend
assert "sample->sampleRate" in audio_backend
assert "streamPrimed_" in audio_backend
assert "producer has written a DMA" in audio_backend or "streamPrimed_ && refreshVolume" in audio_backend
assert "AUDIO_INIT_RETRY_MS = 2000u" in usermod
assert "AUDIO_SHARED_INIT_RETRY_MS = 5000u" in usermod
assert "lastAudioInitAttemptMs_" in usermod
assert "setupHardware();" in usermod
assert "digitalWrite(BuzzerAudioConfig::PA_ENABLE, HIGH);" in audio_backend
assert "codec_.setVolume(requestedVolume)" in audio_backend

# v0.2.0 per-backend sound capability and alphabetical UI contract.
sounds_h = (root / "BuzzerSounds.h").read_text(encoding="utf-8")
for token in ["BUZZER_BACKEND_ACTIVE", "BUZZER_BACKEND_PASSIVE", "BUZZER_BACKEND_AUDIO", "BUZZER_BACKEND_ALL", "backendMask", "supportsBackend"]:
    assert token in sounds_h or token in sounds, token
for token in ["dd.lastChild._b=", "o.hidden=h", "o.disabled=h", "Sound not supported by current backend.", "soundSupportedByCurrentBackend"]:
    assert token in usermod, token
for token in [
    '{"imperial_march", "Imperial March", nullptr, 0u, BUZZER_BACKEND_AUDIO}',
    '{"trumpet", "Trumpet", nullptr, 0u, BUZZER_BACKEND_AUDIO}',
    '{"wakeup", "Wake Up", nullptr, 0u, BUZZER_BACKEND_AUDIO}',
]:
    assert token in sounds, token
assert "#if defined(WLED_BUZZER_AUDIO_SAMPLES)" in sounds


# v0.2.0 Waveshare shared-I2S / AudioReactive coexistence contract.
assert "SHARED_SAMPLE_RATE = 22050u" in audio_cfg
assert "SHARED_BITS_PER_SAMPLE" in audio_cfg
assert "I2S_DIN = 39" in audio_cfg
assert "I2S_MODE_SLAVE" in audio_backend
assert "sharedClockMode_ ? I2S_PIN_NO_CHANGE : BuzzerAudioConfig::I2S_BCLK" in audio_backend
assert "sharedClockMode_ ? I2S_PIN_NO_CHANGE : BuzzerAudioConfig::I2S_LRCK" in audio_backend
assert "esp_rom_gpio_connect_in_signal" in audio_backend
assert "I2S1O_BCK_IN_IDX" in audio_backend
assert "I2S1O_WS_IN_IDX" in audio_backend
assert "GPIO_MATRIX_CONST_ZERO_INPUT" in audio_backend
assert "lastError()" in (root / "audio" / "BuzzerAudioBackend.h").read_text(encoding="utf-8")
assert "shared clock/DMA priming timeout" in audio_backend
assert "audioBackend_.lastError()" in usermod
assert "sharedClockMode_ ? I2S_PIN_NO_CHANGE : BuzzerAudioConfig::I2S_MCLK" in audio_backend
assert "if (!sharedClockMode_ &&" in audio_backend and "i2s_set_clk" in audio_backend
assert "initializeCodecBus" in audio_backend
assert "if (initializeBus && !wire_->begin" in codec
assert "{5644800,  22050" in codec
assert "audioReactiveOwnsSharedI2S" in usermod
assert "PinManager::getPinOwner(pin) == PinOwner::UM_Audioreactive" in usermod
for pin in ["I2S_BCLK", "I2S_LRCK", "I2S_MCLK", "I2S_DIN"]:
    assert f"audioReactiveOwnsPin(BuzzerAudioConfig::{pin})" in usermod
assert "audioBackend_.begin(hardwareVolume_, sharedI2s, !sharedI2s)" in usermod
assert 'F(" bit | clock tap GPIO-matrix + input-enable | RX I2S0 untouched")' in usermod
assert 'state["audioMode"]' in usermod
assert 'state["audioRate"]' in usermod
assert 'state["audioBits"]' in usermod
assert "AUDIO_MODE_RECHECK_MS = 2000u" in usermod
assert "audioReactiveOwnsAnySharedI2S" in usermod
assert "audioReactiveI2STransitioning" in usermod
assert "AudioReactive I2S ownership transition; audio init deferred" in usermod
assert "AudioReactive I2S ownership transition; suspending audio backend" in usermod
# The Buzzer backend must never call control APIs on AudioReactive's I2S0 RX.
for forbidden in [
    "i2s_driver_uninstall(I2S_NUM_0",
    "i2s_set_clk(I2S_NUM_0",
    "i2s_set_pin(I2S_NUM_0",
    "i2s_zero_dma_buffer(I2S_NUM_0",
]:
    assert forbidden not in audio_backend, forbidden

# Shared clock electrical-sense regression.
assert "#include <driver/gpio.h>" in audio_backend
assert "gpio_input_enable" in audio_backend
assert "sharedLrckIsRunning" in audio_backend
assert 'lastError_ = "shared LRCK inactive"' in audio_backend
assert "if (!sharedClockMode_) i2s_zero_dma_buffer(audioPort());" in audio_backend

# Qualified TX starvation instrumentation and buffering contract.
audio_backend_h = (root / "audio" / "BuzzerAudioBackend.h").read_text(encoding="utf-8")
assert "FRAMES_PER_BUFFER = 256u" in audio_backend_h
assert "DMA_BUFFER_COUNT = 8u" in audio_backend_h
assert "cfg.dma_desc_num = DMA_BUFFER_COUNT" in audio_backend
assert "cfg.dma_buf_count = DMA_BUFFER_COUNT" in audio_backend
assert "maxProducerGapUs" in audio_backend_h
assert "lateWrites" in audio_backend_h
assert "uxTaskGetStackHighWaterMark" in audio_backend
assert "xPortGetCoreID" in audio_backend
assert "producerGapUs > dmaCoverageUs" in audio_backend
assert "static_cast<int32_t>(nextValue()) * 65536" in audio_backend
assert "static_cast<int32_t>(nextValue()) << 16" not in audio_backend
assert "DMA " in usermod and "maxWrite=" in usermod and "maxGap=" in usermod
assert "local I2S1 clock master" in usermod
