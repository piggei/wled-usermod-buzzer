#include "../BuzzerSounds.h"

#include <cassert>
#include <cstring>

int main() {
  static const char* EXPECTED_IDS[] = {
    "beep", "double_beep", "triple_beep", "notification", "success",
    "victory", "yankee_doodle", "fail", "star_wars", "warning",
    "error", "connect", "disconnect", "attention", "alarm",
  };
  assert(BuzzerSounds::count() == (sizeof(EXPECTED_IDS) / sizeof(EXPECTED_IDS[0])));
  for (const char* id : EXPECTED_IDS) assert(BuzzerSounds::find(id) != nullptr);
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
