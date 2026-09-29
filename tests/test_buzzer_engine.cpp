#include "../BuzzerEngine.h"
#include "../BuzzerSounds.h"

#include <cassert>
#include <cstdint>
#include <cstring>

struct Sink {
  bool on = false;
  uint16_t frequency = 0;
  uint32_t transitions = 0;
};

static void output(void* context, uint16_t frequencyHz, bool on) {
  Sink* sink = static_cast<Sink*>(context);
  sink->on = on;
  sink->frequency = on ? frequencyHz : 0;
  ++sink->transitions;
}

int main() {
  Sink sink;
  BuzzerEngine engine;
  engine.attach(&output, &sink);

  const BuzzerSound* notification = BuzzerSounds::find("notification");
  assert(notification != nullptr);
  assert(engine.play(*notification, 1000000u));
  assert(engine.isPlaying());
  assert(sink.on);
  assert(sink.frequency == 880);
  assert(sink.transitions == 1);

  engine.service(1079999u);
  assert(sink.on);
  engine.service(1080000u);
  assert(!sink.on);
  assert(sink.transitions == 2);

  engine.service(1124999u);
  assert(!sink.on);
  engine.service(1125000u);
  assert(sink.on);
  assert(sink.frequency == 1175);
  assert(sink.transitions == 3);

  engine.service(1275000u);
  assert(!engine.isPlaying());
  assert(!sink.on);
  assert(sink.transitions == 4);

  // Five milliseconds of service jitter on the first edge must not shift the
  // nominal start of the following note from 1,125,000 us.
  Sink jitterSink;
  BuzzerEngine jitter;
  jitter.attach(&output, &jitterSink);
  assert(jitter.play(*notification, 1000000u));
  jitter.service(1085000u);
  assert(jitter.lastLatenessUs() == 5000u);
  jitter.service(1124999u);
  assert(!jitterSink.on);
  jitter.service(1125000u);
  assert(jitterSink.on);
  assert(jitterSink.frequency == 1175);

  // A looping one-note sound must insert its declared trailing gap before the
  // next repetition.
  const BuzzerNote loopNote[] = {{1000, 50, 25}};
  const BuzzerSound loopSound = {"loop_test", "Loop Test", loopNote, 1};
  Sink loopSink;
  BuzzerEngine looping;
  looping.attach(&output, &loopSink);
  assert(looping.play(loopSound, 2000000u, true));
  looping.service(2050000u);
  assert(!loopSink.on);
  looping.service(2075000u);
  assert(loopSink.on);
  assert(looping.isLooping());

  // Custom tones are one-shot and expose a stable synthetic sound ID.
  Sink toneSink;
  BuzzerEngine tone;
  tone.attach(&output, &toneSink);
  assert(tone.playTone(1500, 100, 3000000u, "tone"));
  assert(std::strcmp(tone.currentSoundId(), "tone") == 0);
  assert(toneSink.frequency == 1500);
  tone.service(3100000u);
  assert(!tone.isPlaying());
  assert(!toneSink.on);

  // stop() is idempotent and must not emit redundant output transitions.
  const uint32_t stoppedTransitions = toneSink.transitions;
  assert(tone.stop());
  assert(tone.stop());
  assert(toneSink.transitions == stoppedTransitions);

  return 0;
}
