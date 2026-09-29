#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
CXX="${CXX:-g++}"
CXXFLAGS=(-std=c++17 -Wall -Wextra -Werror -pedantic -I.)
mkdir -p build/host-tests
"$CXX" "${CXXFLAGS[@]}" BuzzerEngine.cpp BuzzerSounds.cpp tests/test_buzzer_engine.cpp -o build/host-tests/test_buzzer_engine
"$CXX" "${CXXFLAGS[@]}" BuzzerSounds.cpp tests/test_sound_registry.cpp -o build/host-tests/test_sound_registry
build/host-tests/test_buzzer_engine
build/host-tests/test_sound_registry
python3 tests/test_static.py
echo "All host tests passed."
