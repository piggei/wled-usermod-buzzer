#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
CXX="${CXX:-g++}"
CXXFLAGS=(-std=c++17 -Wall -Wextra -Werror -pedantic -I.)
mkdir -p build/host-tests
"$CXX" "${CXXFLAGS[@]}" BuzzerEngine.cpp BuzzerSounds.cpp tests/test_buzzer_engine.cpp -o build/host-tests/test_buzzer_engine
"$CXX" "${CXXFLAGS[@]}" BuzzerSounds.cpp tests/test_sound_registry.cpp -o build/host-tests/test_sound_registry
"$CXX" "${CXXFLAGS[@]}" tests/test_input_parsing.cpp -o build/host-tests/test_input_parsing
"$CXX" "${CXXFLAGS[@]}" tests/test_night_mode.cpp -o build/host-tests/test_night_mode
"$CXX" "${CXXFLAGS[@]}" WLEDBuzzerService.cpp tests/test_service_bridge.cpp -o build/host-tests/test_service_bridge
build/host-tests/test_buzzer_engine
build/host-tests/test_sound_registry
build/host-tests/test_input_parsing
build/host-tests/test_night_mode
build/host-tests/test_service_bridge
python3 tests/test_static.py
echo "All host tests passed."
