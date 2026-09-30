#include "../BuzzerInput.h"

#include <cassert>
#include <cstdint>
#include <initializer_list>

int main() {
  uint32_t value = 0;
  assert(BuzzerInput::parseUnsignedDecimal("1", 1, 255, value) && value == 1);
  assert(BuzzerInput::parseUnsignedDecimal("255", 1, 255, value) && value == 255);
  assert(BuzzerInput::parseUnsignedDecimal("20000", 20, 20000, value) && value == 20000);
  assert(BuzzerInput::parseUnsignedDecimal("60000", 1, 60000, value) && value == 60000);

  for (const char* bad : {"", "-1", "+1", "0", "256", "3abc", "abc", " 3", "3 ", "999999999999"}) {
    assert(!BuzzerInput::parseUnsignedDecimal(bad, 1, 255, value));
  }
  assert(!BuzzerInput::parseUnsignedDecimal("19", 20, 20000, value));
  assert(!BuzzerInput::parseUnsignedDecimal("20001", 20, 20000, value));
  assert(!BuzzerInput::parseUnsignedDecimal("0", 1, 60000, value));
  assert(!BuzzerInput::parseUnsignedDecimal("60001", 1, 60000, value));

  bool flag = false;
  assert(BuzzerInput::parseBooleanText("1", flag) && flag);
  assert(BuzzerInput::parseBooleanText("TRUE", flag) && flag);
  assert(BuzzerInput::parseBooleanText("On", flag) && flag);
  assert(BuzzerInput::parseBooleanText("0", flag) && !flag);
  assert(BuzzerInput::parseBooleanText("False", flag) && !flag);
  assert(BuzzerInput::parseBooleanText("OFF", flag) && !flag);
  assert(!BuzzerInput::parseBooleanText("yes", flag));
  assert(!BuzzerInput::parseBooleanText("2", flag));
  assert(!BuzzerInput::parseBooleanText("", flag));
  return 0;
}
