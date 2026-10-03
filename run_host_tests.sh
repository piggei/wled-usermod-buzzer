#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
CXX="${CXX:-g++}"
CXXFLAGS=(-std=c++17 -Wall -Wextra -Werror -pedantic -I.)
mkdir -p build/host-tests
"$CXX" "${CXXFLAGS[@]}" BuzzerEngine.cpp BuzzerSounds.cpp tests/test_buzzer_engine.cpp -o build/host-tests/test_buzzer_engine
"$CXX" "${CXXFLAGS[@]}" BuzzerSounds.cpp tests/test_sound_registry.cpp -o build/host-tests/test_sound_registry
"$CXX" "${CXXFLAGS[@]}" -DWLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX BuzzerSounds.cpp tests/test_sound_registry.cpp -o build/host-tests/test_sound_registry_audio
"$CXX" "${CXXFLAGS[@]}" -DWLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX -DWLED_BUZZER_DISABLE_AUDIO_SAMPLES BuzzerSounds.cpp tests/test_sound_registry.cpp -o build/host-tests/test_sound_registry_audio_no_samples
"$CXX" "${CXXFLAGS[@]}" tests/test_input_parsing.cpp -o build/host-tests/test_input_parsing
"$CXX" "${CXXFLAGS[@]}" tests/test_night_mode.cpp -o build/host-tests/test_night_mode
"$CXX" "${CXXFLAGS[@]}" tests/test_audio_config.cpp -o build/host-tests/test_audio_config
"$CXX" "${CXXFLAGS[@]}" tests/test_audio_config_no_samples.cpp -o build/host-tests/test_audio_config_no_samples
"$CXX" "${CXXFLAGS[@]}" -DWLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX audio/BuzzerAudioSamples.cpp audio/BuzzerAudioSamplesExtra.cpp tests/test_audio_samples.cpp -o build/host-tests/test_audio_samples
"$CXX" "${CXXFLAGS[@]}" -c audio/BuzzerAudioSamples.cpp -o build/host-tests/audio-samples-disabled.o
"$CXX" "${CXXFLAGS[@]}" -c audio/BuzzerAudioSamplesExtra.cpp -o build/host-tests/audio-samples-extra-disabled.o
"$CXX" "${CXXFLAGS[@]}" -c audio/ES8311Codec.cpp -o build/host-tests/es8311-disabled.o
"$CXX" "${CXXFLAGS[@]}" -c audio/BuzzerAudioBackend.cpp -o build/host-tests/audio-backend-disabled.o
"$CXX" "${CXXFLAGS[@]}" WLEDBuzzerService.cpp tests/test_service_bridge.cpp -o build/host-tests/test_service_bridge
build/host-tests/test_buzzer_engine
build/host-tests/test_sound_registry
build/host-tests/test_sound_registry_audio
build/host-tests/test_sound_registry_audio_no_samples
build/host-tests/test_input_parsing
build/host-tests/test_night_mode
build/host-tests/test_audio_config
build/host-tests/test_audio_config_no_samples
build/host-tests/test_audio_samples
build/host-tests/test_service_bridge
python3 tests/test_static.py
echo "All host tests passed."
