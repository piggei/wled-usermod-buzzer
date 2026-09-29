#include "../BuzzerSounds.h"

#include <cassert>
#include <cstring>

int main() {
  assert(BuzzerSounds::count() >= 10);
  assert(BuzzerSounds::find("triple_beep") != nullptr);
  assert(BuzzerSounds::find("victory") != nullptr);
  assert(BuzzerSounds::find("fail") != nullptr);
  assert(BuzzerSounds::find("alarm") != nullptr);
  assert(BuzzerSounds::find("missing") == nullptr);

  for (size_t i = 0; i < BuzzerSounds::count(); ++i) {
    const BuzzerSound& a = BuzzerSounds::at(i);
    assert(a.id != nullptr && *a.id != '\0');
    assert(a.label != nullptr && *a.label != '\0');
    assert(a.notes != nullptr);
    assert(a.noteCount > 0);
    for (size_t n = 0; n < a.noteCount; ++n) {
      assert(a.notes[n].durationMs > 0);
      assert(a.notes[n].frequencyHz > 0);
    }
    for (size_t j = i + 1; j < BuzzerSounds::count(); ++j) {
      assert(std::strcmp(a.id, BuzzerSounds::at(j).id) != 0);
    }
  }
  return 0;
}
