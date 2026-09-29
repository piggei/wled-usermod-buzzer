#include "BuzzerEngine.h"

#include <climits>

bool BuzzerEngine::beginThreadSafe() {
#if defined(ARDUINO_ARCH_ESP32)
  if (mutex_ != nullptr) return true;
  mutex_ = xSemaphoreCreateMutex();
  return mutex_ != nullptr;
#else
  return true;
#endif
}

bool BuzzerEngine::lock(bool wait) const {
#if defined(ARDUINO_ARCH_ESP32)
  if (mutex_ == nullptr) return true;
  const TickType_t timeout = wait ? pdMS_TO_TICKS(COMMAND_LOCK_TIMEOUT_MS) : 0;
  return xSemaphoreTake(mutex_, timeout) == pdTRUE;
#else
  (void)wait;
  return true;
#endif
}

void BuzzerEngine::unlock() const {
#if defined(ARDUINO_ARCH_ESP32)
  if (mutex_ != nullptr) xSemaphoreGive(mutex_);
#endif
}

bool BuzzerEngine::play(const BuzzerSound& sound, uint64_t nowUs, bool loop) {
  if (!lock(true)) return false;
  const bool started = startLocked(sound.notes, sound.noteCount, sound.id, nowUs, loop);
  unlock();
  return started;
}

bool BuzzerEngine::playTone(uint16_t frequencyHz, uint16_t durationMs, uint64_t nowUs, const char* id) {
  if (frequencyHz == 0 || durationMs == 0) return false;
  if (!lock(true)) return false;
  customNote_.frequencyHz = frequencyHz;
  customNote_.durationMs = durationMs;
  customNote_.gapMs = 0;
  const bool started = startLocked(&customNote_, 1, id, nowUs, false);
  unlock();
  return started;
}

bool BuzzerEngine::startLocked(
  const BuzzerNote* notes,
  size_t noteCount,
  const char* id,
  uint64_t nowUs,
  bool loop
) {
  if (notes == nullptr || noteCount == 0 || id == nullptr || *id == '\0') return false;

  stopLocked();
  notes_ = notes;
  noteCount_ = noteCount;
  currentSoundId_ = id;
  noteIndex_ = 0;
  loop_ = loop;
  lastLatenessUs_ = 0;
  maxLatenessUs_ = 0;
  startCurrentNoteLocked(nowUs, false);
  return true;
}

bool BuzzerEngine::stop() {
  if (!lock(true)) return false;
  stopLocked();
  unlock();
  return true;
}

void BuzzerEngine::stopLocked() {
  setOutputLocked(0, false);
  notes_ = nullptr;
  noteCount_ = 0;
  currentSoundId_ = nullptr;
  noteIndex_ = 0;
  loop_ = false;
  phase_ = Phase::Idle;
  nextChangeAtUs_ = 0;
}

void BuzzerEngine::setOutputLocked(uint16_t frequencyHz, bool on) {
  if (!on) {
    if (!outputOn_) {
      outputFrequencyHz_ = 0;
      return;
    }
    outputOn_ = false;
    outputFrequencyHz_ = 0;
    if (outputCallback_ != nullptr) outputCallback_(outputContext_, 0, false);
    return;
  }

  if (outputOn_ && outputFrequencyHz_ == frequencyHz) return;
  outputOn_ = true;
  outputFrequencyHz_ = frequencyHz;
  if (outputCallback_ != nullptr) outputCallback_(outputContext_, frequencyHz, true);
}

void BuzzerEngine::startCurrentNoteLocked(uint64_t nowUs, bool preservePhase) {
  if (notes_ == nullptr || noteIndex_ >= noteCount_) {
    stopLocked();
    return;
  }

  const BuzzerNote& note = notes_[noteIndex_];
  const uint64_t durationUs = static_cast<uint64_t>(note.durationMs ? note.durationMs : 1u) * 1000u;
  phase_ = Phase::Tone;
  setOutputLocked(note.frequencyHz, note.frequencyHz != 0);
  if (preservePhase) scheduleNextLocked(nowUs, durationUs);
  else nextChangeAtUs_ = nowUs + durationUs;
}

bool BuzzerEngine::hasFollowingNoteLocked() const {
  if (noteCount_ == 0) return false;
  return (noteIndex_ + 1u < noteCount_) || loop_;
}

void BuzzerEngine::advanceNoteLocked(uint64_t nowUs) {
  if (noteCount_ == 0) {
    stopLocked();
    return;
  }

  ++noteIndex_;
  if (noteIndex_ >= noteCount_) {
    if (!loop_) {
      stopLocked();
      return;
    }
    noteIndex_ = 0;
  }
  startCurrentNoteLocked(nowUs, true);
}

void BuzzerEngine::scheduleNextLocked(uint64_t nowUs, uint64_t intervalUs) {
  const uint64_t phaseNext = nextChangeAtUs_ + intervalUs;
  if (nowUs >= phaseNext) nextChangeAtUs_ = nowUs + intervalUs;
  else nextChangeAtUs_ = phaseNext;
}

void BuzzerEngine::service(uint64_t nowUs) {
  if (!lock(false)) return;
  if (phase_ == Phase::Idle || nowUs < nextChangeAtUs_) {
    unlock();
    return;
  }

  const uint64_t lateness = nowUs - nextChangeAtUs_;
  lastLatenessUs_ = lateness > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(lateness);
  if (lastLatenessUs_ > maxLatenessUs_) maxLatenessUs_ = lastLatenessUs_;

  if (phase_ == Phase::Tone) {
    if (!hasFollowingNoteLocked()) {
      stopLocked();
      unlock();
      return;
    }

    const uint16_t gapMs = notes_[noteIndex_].gapMs;
    setOutputLocked(0, false);
    if (gapMs > 0) {
      phase_ = Phase::Gap;
      scheduleNextLocked(nowUs, static_cast<uint64_t>(gapMs) * 1000u);
    } else {
      advanceNoteLocked(nowUs);
    }
    unlock();
    return;
  }

  advanceNoteLocked(nowUs);
  unlock();
}

bool BuzzerEngine::isPlaying() const {
  if (!lock(true)) return false;
  const bool value = phase_ != Phase::Idle;
  unlock();
  return value;
}

bool BuzzerEngine::isLooping() const {
  if (!lock(true)) return false;
  const bool value = loop_ && phase_ != Phase::Idle;
  unlock();
  return value;
}

bool BuzzerEngine::outputOn() const {
  if (!lock(true)) return false;
  const bool value = outputOn_;
  unlock();
  return value;
}

const char* BuzzerEngine::currentSoundId() const {
  if (!lock(true)) return "";
  const char* value = currentSoundId_ != nullptr ? currentSoundId_ : "";
  unlock();
  return value;
}

size_t BuzzerEngine::currentNoteIndex() const {
  if (!lock(true)) return 0;
  const size_t value = noteIndex_;
  unlock();
  return value;
}

size_t BuzzerEngine::currentNoteCount() const {
  if (!lock(true)) return 0;
  const size_t value = noteCount_;
  unlock();
  return value;
}

uint16_t BuzzerEngine::currentFrequencyHz() const {
  if (!lock(true)) return 0;
  const uint16_t value = outputOn_ ? outputFrequencyHz_ : 0;
  unlock();
  return value;
}

uint32_t BuzzerEngine::lastLatenessUs() const {
  if (!lock(true)) return 0;
  const uint32_t value = lastLatenessUs_;
  unlock();
  return value;
}

uint32_t BuzzerEngine::maxLatenessUs() const {
  if (!lock(true)) return 0;
  const uint32_t value = maxLatenessUs_;
  unlock();
  return value;
}
