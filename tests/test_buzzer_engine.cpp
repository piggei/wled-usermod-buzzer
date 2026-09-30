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

  const BuzzerSound* connect = BuzzerSounds::find("connect");
  assert(connect != nullptr);
  assert(engine.play(*connect, 1000000u));
  assert(engine.isPlaying());
  assert(sink.on);
  assert(sink.frequency == 880);
  assert(sink.transitions == 1);

  engine.service(1064999u);
  assert(sink.on);
  engine.service(1065000u);
  assert(!sink.on);
  assert(sink.transitions == 2);

  engine.service(1089999u);
  assert(!sink.on);
  engine.service(1090000u);
  assert(sink.on);
  assert(sink.frequency == 1175);
  assert(sink.transitions == 3);

  engine.service(1220000u);
  assert(!engine.isPlaying());
  assert(!sink.on);
  assert(sink.transitions == 4);

  // Five milliseconds of service jitter on the first edge must not shift the
  // nominal start of the following note from 1,090,000 us.
  Sink jitterSink;
  BuzzerEngine jitter;
  jitter.attach(&output, &jitterSink);
  assert(jitter.play(*connect, 1000000u));
  jitter.service(1070000u);
  assert(jitter.lastLatenessUs() == 5000u);
  jitter.service(1089999u);
  assert(!jitterSink.on);
  jitter.service(1090000u);
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
  // Infinite loop must still be active after more than three complete plays.
  looping.service(2125000u);
  assert(!loopSink.on);
  looping.service(2150000u);
  assert(loopSink.on);
  looping.service(2200000u);
  assert(!loopSink.on);
  looping.service(2225000u);
  assert(loopSink.on);
  assert(looping.isLooping());

  // Finite repeat count means total executions, not extra repetitions.
  Sink repeatSink;
  BuzzerEngine repeated;
  repeated.attach(&output, &repeatSink);
  assert(repeated.playRepeat(loopSound, 4000000u, 3));
  assert(repeated.repeatRemaining() == 3u);
  repeated.service(4050000u);
  assert(!repeatSink.on);
  repeated.service(4075000u);
  assert(repeatSink.on);
  assert(repeated.repeatRemaining() == 2u);
  repeated.service(4125000u);
  assert(!repeatSink.on);
  repeated.service(4150000u);
  assert(repeatSink.on);
  assert(repeated.repeatRemaining() == 1u);
  repeated.service(4200000u);
  assert(!repeated.isPlaying());
  assert(!repeatSink.on);
  assert(repeated.repeatRemaining() == 0u);
  assert(repeatSink.transitions == 6u);
  assert(!repeated.playRepeat(loopSound, 5000000u, 0));

  // Trailing-gap contract: a one-shot stops immediately at the end of the
  // third pulse, while loop playback consumes the 550 ms trailing gap before
  // restarting the first pulse.
  const BuzzerSound* triple = BuzzerSounds::find("triple_beep");
  assert(triple != nullptr);
  Sink tripleOneShotSink;
  BuzzerEngine tripleOneShot;
  tripleOneShot.attach(&output, &tripleOneShotSink);
  assert(tripleOneShot.play(*triple, 6000000u));
  tripleOneShot.service(6090000u);  // first pulse ends
  tripleOneShot.service(6160000u);  // second pulse starts
  tripleOneShot.service(6250000u);  // second pulse ends
  tripleOneShot.service(6320000u);  // third pulse starts
  tripleOneShot.service(6410000u);  // third pulse ends; no following execution
  assert(!tripleOneShot.isPlaying());
  assert(!tripleOneShotSink.on);

  Sink tripleLoopSink;
  BuzzerEngine tripleLoop;
  tripleLoop.attach(&output, &tripleLoopSink);
  assert(tripleLoop.play(*triple, 7000000u, true));
  tripleLoop.service(7090000u);
  tripleLoop.service(7160000u);
  tripleLoop.service(7250000u);
  tripleLoop.service(7320000u);
  tripleLoop.service(7410000u);  // third pulse ends; enter 550 ms trailing gap
  assert(tripleLoop.isPlaying());
  assert(!tripleLoopSink.on);
  tripleLoop.service(7959999u);
  assert(!tripleLoopSink.on);
  tripleLoop.service(7960000u);  // first pulse of next execution
  assert(tripleLoopSink.on);
  assert(tripleLoopSink.frequency == 2000u);

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
