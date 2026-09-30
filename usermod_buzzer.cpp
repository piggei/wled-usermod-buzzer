#include "wled.h"
#include "BuzzerEngine.h"
#include "BuzzerSounds.h"
#include "BuzzerInput.h"
#include "WLEDBuzzerService.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <esp32-hal-ledc.h>
#include <esp_arduino_version.h>
#include <esp_timer.h>
#endif

namespace {
const char USERMOD_NAME[] PROGMEM = "Buzzer";
const char JSON_KEY[] PROGMEM = "buzzer";
const char CFG_ENABLED[] PROGMEM = "enabled";
const char CFG_PIN[] PROGMEM = "pin";
const char CFG_TYPE[] PROGMEM = "type";
const char CFG_TRIGGER[] PROGMEM = "trigger";
const char CFG_VOLUME[] PROGMEM = "volume";
const char CFG_SOUND[] PROGMEM = "sound";
// b001 configuration aliases, accepted on read for migration only.
const char CFG_OLD_TYPE[] PROGMEM = "buzzerType";
const char CFG_OLD_ACTIVE_HIGH[] PROGMEM = "activeHigh";
const char CFG_OLD_PASSIVE_TRIGGER[] PROGMEM = "passiveTrigger";
const char CFG_OLD_VOLUME[] PROGMEM = "passiveVolume";
const char CFG_OLD_SOUND[] PROGMEM = "testSound";

constexpr const char* BUZZER_VERSION = "0.1.0";
constexpr const char* BUZZER_BUILD = "rc.6";
constexpr uint16_t DEFAULT_TONE_HZ = 1000u;
constexpr uint16_t DEFAULT_BEEP_MS = 120u;
constexpr uint16_t MAX_TONE_HZ = 20000u;
constexpr uint16_t MAX_DURATION_MS = 60000u;
constexpr uint8_t LEDC_RESOLUTION_BITS = 10u;
constexpr uint32_t LEDC_MAX_DUTY = (1u << LEDC_RESOLUTION_BITS) - 1u;
constexpr uint64_t SERVICE_PERIOD_US = 2000u;

#if defined(ARDUINO_ARCH_ESP32)
constexpr uint8_t LEDC_UNASSIGNED = 255u;
#endif

enum : uint8_t {
  BUZZER_TYPE_ACTIVE = 0,
  BUZZER_TYPE_PASSIVE = 1,
};

enum : uint8_t {
  TRIGGER_HIGH = 0,
  TRIGGER_LOW = 1,
};

uint16_t clampFrequency(uint32_t value) {
  if (value < 20u) return 20u;
  if (value > MAX_TONE_HZ) return MAX_TONE_HZ;
  return static_cast<uint16_t>(value);
}

uint16_t clampDuration(uint32_t value) {
  if (value < 1u) return 1u;
  if (value > MAX_DURATION_MS) return MAX_DURATION_MS;
  return static_cast<uint16_t>(value);
}

uint8_t clampPercent(uint32_t value) {
  return value > 100u ? 100u : static_cast<uint8_t>(value);
}

enum class ParseResult : uint8_t {
  Missing = 0,
  Valid,
  Invalid,
};
}

class WLEDBuzzerUsermod final : public Usermod, public WLEDBuzzerService {
private:
  bool enabled_ = true;
  int8_t pin_ = -1;
  uint8_t buzzerType_ = BUZZER_TYPE_ACTIVE;
  uint8_t trigger_ = TRIGGER_HIGH;
  uint8_t volume_ = 100;
  String sound_ = "victory";

  bool setupComplete_ = false;
  bool pinAllocated_ = false;
  bool hardwareReady_ = false;
  bool pinUnavailable_ = false;
  int8_t hardwarePin_ = -1;
  uint8_t hardwareType_ = BUZZER_TYPE_ACTIVE;
  uint8_t hardwareTrigger_ = TRIGGER_HIGH;
  uint8_t hardwareVolume_ = 100;
  BuzzerEngine engine_;

#if defined(ARDUINO_ARCH_ESP32)
  uint8_t ledcChannel_ = LEDC_UNASSIGNED;
  esp_timer_handle_t serviceTimer_ = nullptr;
  bool serviceTimerRunning_ = false;
  bool engineThreadSafe_ = false;
#endif

  static void outputThunk(void* context, uint16_t frequencyHz, bool on) {
    static_cast<WLEDBuzzerUsermod*>(context)->writeOutput(frequencyHz, on);
  }

  uint64_t nowUs() const {
#if defined(ARDUINO_ARCH_ESP32)
    return static_cast<uint64_t>(esp_timer_get_time());
#else
    return static_cast<uint64_t>(millis()) * 1000u;
#endif
  }

  bool triggerLow() const {
    return hardwareTrigger_ == TRIGGER_LOW;
  }

  uint32_t passiveIdleDuty() const {
    return triggerLow() ? LEDC_MAX_DUTY : 0u;
  }

  uint32_t passiveToneDuty() const {
    const uint32_t halfScale = LEDC_MAX_DUTY / 2u;
    const uint32_t activeDuty = (halfScale * hardwareVolume_) / 100u;
    return triggerLow() ? (LEDC_MAX_DUTY - activeDuty) : activeDuty;
  }

  int passiveIdleLevel() const {
    return triggerLow() ? HIGH : LOW;
  }

#if defined(ARDUINO_ARCH_ESP32)
  static void serviceTimerThunk(void* context) {
    auto* self = static_cast<WLEDBuzzerUsermod*>(context);
    self->engine_.service(static_cast<uint64_t>(esp_timer_get_time()));
  }

  bool stopServiceTimer() {
    if (serviceTimer_ == nullptr || !serviceTimerRunning_) return true;
    const esp_err_t result = esp_timer_stop(serviceTimer_);
    if (result != ESP_OK) {
      DEBUG_PRINTF_P(PSTR("[Buzzer] esp_timer_stop failed: %d\n"), static_cast<int>(result));
      return false;
    }
    serviceTimerRunning_ = false;
    return true;
  }

  bool startServiceTimer() {
    if (!engineThreadSafe_) return false;
    if (serviceTimer_ == nullptr) {
      esp_timer_create_args_t args{};
      args.callback = &WLEDBuzzerUsermod::serviceTimerThunk;
      args.arg = this;
      args.dispatch_method = ESP_TIMER_TASK;
      args.name = "wled-buzzer";
      args.skip_unhandled_events = true;
      if (esp_timer_create(&args, &serviceTimer_) != ESP_OK) {
        serviceTimer_ = nullptr;
        return false;
      }
    }
    if (serviceTimerRunning_) return true;
    if (esp_timer_start_periodic(serviceTimer_, SERVICE_PERIOD_US) != ESP_OK) return false;
    serviceTimerRunning_ = true;
    return true;
  }
#endif

  bool attachPassiveBuzzer() {
#if !defined(ARDUINO_ARCH_ESP32)
    return false;
#else
    ledcChannel_ = PinManager::allocateLedc(1);
    if (ledcChannel_ == LEDC_UNASSIGNED) return false;

    pinMode(hardwarePin_, OUTPUT);
    digitalWrite(hardwarePin_, passiveIdleLevel());

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    if (!ledcAttachChannel(hardwarePin_, DEFAULT_TONE_HZ, LEDC_RESOLUTION_BITS, ledcChannel_)) {
      PinManager::deallocateLedc(ledcChannel_, 1);
      ledcChannel_ = LEDC_UNASSIGNED;
      return false;
    }
#else
    if (ledcSetup(ledcChannel_, DEFAULT_TONE_HZ, LEDC_RESOLUTION_BITS) <= 0.0) {
      PinManager::deallocateLedc(ledcChannel_, 1);
      ledcChannel_ = LEDC_UNASSIGNED;
      return false;
    }
    ledcAttachPin(hardwarePin_, ledcChannel_);
#endif
    return true;
#endif
  }

  void detachPassiveBuzzer() {
#if defined(ARDUINO_ARCH_ESP32)
    if (ledcChannel_ == LEDC_UNASSIGNED) return;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcDetach(hardwarePin_);
#else
    ledcDetachPin(hardwarePin_);
#endif
    PinManager::deallocateLedc(ledcChannel_, 1);
    ledcChannel_ = LEDC_UNASSIGNED;
#endif
  }

  void writeOutput(uint16_t frequencyHz, bool on) {
    if (!hardwareReady_ || hardwarePin_ < 0) return;

    if (hardwareType_ == BUZZER_TYPE_PASSIVE) {
#if defined(ARDUINO_ARCH_ESP32)
      if (on && frequencyHz > 0) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcWriteTone(hardwarePin_, frequencyHz);
        ledcWrite(hardwarePin_, passiveToneDuty());
#else
        ledcWriteTone(ledcChannel_, frequencyHz);
        ledcWrite(ledcChannel_, passiveToneDuty());
#endif
      } else {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcWrite(hardwarePin_, passiveIdleDuty());
#else
        ledcWrite(ledcChannel_, passiveIdleDuty());
#endif
      }
#endif
      return;
    }

    digitalWrite(hardwarePin_, on ? (triggerLow() ? LOW : HIGH) : (triggerLow() ? HIGH : LOW));
  }

  bool teardownHardware() {
#if defined(ARDUINO_ARCH_ESP32)
    if (!stopServiceTimer()) return false;
#endif
    if (!engine_.stop()) {
      DEBUG_PRINTLN(F("[Buzzer] unable to stop engine during teardown"));
      return false;
    }

    if (hardwareReady_ && hardwarePin_ >= 0) {
      writeOutput(0, false);
      if (hardwareType_ == BUZZER_TYPE_PASSIVE) detachPassiveBuzzer();
      pinMode(hardwarePin_, INPUT);
    }

    if (pinAllocated_ && hardwarePin_ >= 0) {
      PinManager::deallocatePin(hardwarePin_, PinOwner::UM_Unspecified);
    }

    pinAllocated_ = false;
    hardwareReady_ = false;
    pinUnavailable_ = false;
    hardwarePin_ = -1;
    return true;
  }

  void setupHardware() {
    if (!teardownHardware()) return;
    if (!enabled_ || pin_ < 0) return;
#if defined(ARDUINO_ARCH_ESP32)
    if (!engineThreadSafe_) {
      DEBUG_PRINTLN(F("[Buzzer] synchronization unavailable; hardware playback disabled"));
      return;
    }
#endif

    hardwarePin_ = pin_;
    hardwareType_ = buzzerType_;
    hardwareTrigger_ = trigger_;
    hardwareVolume_ = volume_;

    if (!PinManager::isPinOk(hardwarePin_, true) ||
        !PinManager::allocatePin(hardwarePin_, true, PinOwner::UM_Unspecified)) {
      pinUnavailable_ = true;
      DEBUG_PRINTF_P(PSTR("[Buzzer] GPIO %d unavailable\n"), hardwarePin_);
      return;
    }
    pinAllocated_ = true;

    if (hardwareType_ == BUZZER_TYPE_PASSIVE) {
      if (!attachPassiveBuzzer()) {
        DEBUG_PRINTF_P(PSTR("[Buzzer] LEDC setup failed on GPIO %d\n"), hardwarePin_);
        PinManager::deallocatePin(hardwarePin_, PinOwner::UM_Unspecified);
        pinAllocated_ = false;
        pinMode(hardwarePin_, INPUT);
        return;
      }
      hardwareReady_ = true;
      writeOutput(0, false);
    } else {
      pinMode(hardwarePin_, OUTPUT);
      hardwareReady_ = true;
      writeOutput(0, false);
    }

#if defined(ARDUINO_ARCH_ESP32)
    if (!startServiceTimer()) {
      DEBUG_PRINTLN(F("[Buzzer] esp_timer unavailable; using WLED loop timing"));
    }
#endif

    DEBUG_PRINTF_P(
      PSTR("[Buzzer] ready: GPIO=%d type=%s\n"),
      hardwarePin_,
      hardwareType_ == BUZZER_TYPE_PASSIVE ? "passive" : "active"
    );
  }

  bool canPlay() const {
#if defined(ARDUINO_ARCH_ESP32)
    if (!engineThreadSafe_) return false;
#endif
    return enabled_ && hardwareReady_ && !pinUnavailable_ && hardwarePin_ >= 0;
  }

  bool playSound(const char* soundId, bool loop) {
    if (!canPlay()) return false;
    const BuzzerSound* sound = BuzzerSounds::find(soundId);
    if (sound == nullptr) return false;
    return engine_.play(*sound, nowUs(), loop);
  }

  bool playSoundRepeat(const char* soundId, uint16_t repeatCount) {
    if (!canPlay() || repeatCount == 0) return false;
    const BuzzerSound* sound = BuzzerSounds::find(soundId);
    if (sound == nullptr) return false;
    return engine_.playRepeat(*sound, nowUs(), repeatCount);
  }

  bool hasQueryParam(AsyncWebServerRequest* request, const char* name) const {
    return request->hasParam(name) || request->hasParam(name, true);
  }

  String queryParam(AsyncWebServerRequest* request, const char* name) const {
    if (request->hasParam(name)) return request->getParam(name)->value();
    if (request->hasParam(name, true)) return request->getParam(name, true)->value();
    return String();
  }

  ParseResult parseUnsignedParam(
    AsyncWebServerRequest* request,
    const char* name,
    uint32_t minimum,
    uint32_t maximum,
    uint32_t& output
  ) const {
    if (!hasQueryParam(request, name)) return ParseResult::Missing;
    const String value = queryParam(request, name);
    if (value.isEmpty()) return ParseResult::Invalid;

    return BuzzerInput::parseUnsignedDecimal(value.c_str(), minimum, maximum, output)
      ? ParseResult::Valid
      : ParseResult::Invalid;
  }

  ParseResult parseBoolParam(AsyncWebServerRequest* request, const char* name, bool& output) const {
    if (!hasQueryParam(request, name)) return ParseResult::Missing;
    const String value = queryParam(request, name);
    return BuzzerInput::parseBooleanText(value.c_str(), output)
      ? ParseResult::Valid
      : ParseResult::Invalid;
  }

  bool stopPlayback() {
    return engine_.stop();
  }

  void sendStatus(AsyncWebServerRequest* request) const {
    const BuzzerEngine::Snapshot snapshot = engine_.snapshot();
    String body;
    body.reserve(240);
    body += F("{\"version\":\"");
    body += BUZZER_VERSION;
    body += F("\",\"build\":\"");
    body += BUZZER_BUILD;
    body += F("\",\"enabled\":");
    body += enabled_ ? F("true") : F("false");
    body += F(",\"ready\":");
    body += canPlay() ? F("true") : F("false");
    body += F(",\"playing\":");
    body += snapshot.playing ? F("true") : F("false");
    body += F(",\"sound\":\"");
    body += snapshot.soundId;
    body += F("\",\"loop\":");
    body += snapshot.looping ? F("true") : F("false");
    body += F(",\"repeatRemaining\":");
    body += snapshot.repeatRemaining;
    body += F("}");
    request->send(200, "application/json", body);
  }

  void registerApiEndpoint() {
    server.on(F("/buzzer"), HTTP_GET, [this](AsyncWebServerRequest* request) {
      const bool hasStop = hasQueryParam(request, "stop");
      const bool hasPlay = hasQueryParam(request, "play");
      const bool hasTone = hasQueryParam(request, "tone");
      const bool hasBeep = hasQueryParam(request, "beep");
      const uint8_t commandCount = static_cast<uint8_t>(hasStop) + static_cast<uint8_t>(hasPlay) +
        static_cast<uint8_t>(hasTone) + static_cast<uint8_t>(hasBeep);

      if (commandCount > 1u) {
        request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("Use one buzzer command per request."));
        return;
      }

      if (hasStop) {
        bool stopRequested = false;
        if (parseBoolParam(request, "stop", stopRequested) != ParseResult::Valid) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("stop must be 0/1, false/true or off/on."));
          return;
        }
        if (!stopRequested) {
          sendStatus(request);
          return;
        }
        if (!stopPlayback()) {
          request->send(503, FPSTR(CONTENT_TYPE_PLAIN), F("Unable to stop buzzer sound."));
          return;
        }
        request->send(200, FPSTR(CONTENT_TYPE_PLAIN), F("ok"));
        return;
      }

      if (hasPlay) {
        const String soundId = queryParam(request, "play");
        if (soundId.isEmpty()) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("play requires a sound id."));
          return;
        }
        if (!canPlay()) {
          request->send(409, FPSTR(CONTENT_TYPE_PLAIN), F("Buzzer hardware is not ready."));
          return;
        }
        if (BuzzerSounds::find(soundId.c_str()) == nullptr) {
          request->send(404, FPSTR(CONTENT_TYPE_PLAIN), F("Unknown buzzer sound."));
          return;
        }

        bool loop = false;
        const ParseResult loopResult = parseBoolParam(request, "loop", loop);
        if (loopResult == ParseResult::Invalid) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("loop must be 0/1, false/true or off/on."));
          return;
        }

        uint32_t repeat = 0;
        const ParseResult repeatResult = parseUnsignedParam(request, "repeat", 1u, 255u, repeat);
        if (repeatResult == ParseResult::Invalid) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("repeat must be an integer between 1 and 255."));
          return;
        }
        if (loop && repeatResult == ParseResult::Valid) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("Use either loop or repeat, not both."));
          return;
        }

        const bool started = repeatResult == ParseResult::Valid
          ? playSoundRepeat(soundId.c_str(), static_cast<uint16_t>(repeat))
          : playSound(soundId.c_str(), loop);
        if (!started) {
          request->send(503, FPSTR(CONTENT_TYPE_PLAIN), F("Unable to start buzzer sound."));
          return;
        }
        request->send(200, FPSTR(CONTENT_TYPE_PLAIN), F("ok"));
        return;
      }

      if (hasTone) {
        uint32_t frequency = 0;
        if (parseUnsignedParam(request, "tone", 20u, MAX_TONE_HZ, frequency) != ParseResult::Valid) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("tone must be an integer between 20 and 20000 Hz."));
          return;
        }
        uint32_t duration = DEFAULT_BEEP_MS;
        const ParseResult durationResult = parseUnsignedParam(request, "duration", 1u, MAX_DURATION_MS, duration);
        if (durationResult == ParseResult::Invalid) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("duration must be an integer between 1 and 60000 ms."));
          return;
        }
        if (!tone(static_cast<uint16_t>(frequency), static_cast<uint16_t>(duration))) {
          request->send(409, FPSTR(CONTENT_TYPE_PLAIN), F("Buzzer hardware is not ready."));
          return;
        }
        request->send(200, FPSTR(CONTENT_TYPE_PLAIN), F("ok"));
        return;
      }

      if (hasBeep) {
        uint32_t duration = 0;
        if (parseUnsignedParam(request, "beep", 1u, MAX_DURATION_MS, duration) != ParseResult::Valid) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("beep must be an integer between 1 and 60000 ms."));
          return;
        }
        uint32_t frequency = DEFAULT_TONE_HZ;
        const ParseResult frequencyResult = parseUnsignedParam(request, "frequency", 20u, MAX_TONE_HZ, frequency);
        if (frequencyResult == ParseResult::Invalid) {
          request->send(400, FPSTR(CONTENT_TYPE_PLAIN), F("frequency must be an integer between 20 and 20000 Hz."));
          return;
        }
        if (!beep(static_cast<uint16_t>(duration), static_cast<uint16_t>(frequency))) {
          request->send(409, FPSTR(CONTENT_TYPE_PLAIN), F("Buzzer hardware is not ready."));
          return;
        }
        request->send(200, FPSTR(CONTENT_TYPE_PLAIN), F("ok"));
        return;
      }

      sendStatus(request);
    });
  }

public:
  void setup() override {
    engine_.attach(&WLEDBuzzerUsermod::outputThunk, this);
#if defined(ARDUINO_ARCH_ESP32)
    engineThreadSafe_ = engine_.beginThreadSafe();
    if (!engineThreadSafe_) {
      DEBUG_PRINTLN(F("[Buzzer] mutex allocation failed; playback disabled"));
    }
#endif
    WLEDBuzzerService::setInstance(this);
    registerApiEndpoint();
    setupHardware();
    setupComplete_ = true;
  }

  void loop() override {
    if (!enabled_) return;
#if defined(ARDUINO_ARCH_ESP32)
    if (engineThreadSafe_ && !serviceTimerRunning_) engine_.service(nowUs());
#else
    engine_.service(nowUs());
#endif
  }

  bool play(const char* soundId, bool loop = false) override {
    return playSound(soundId, loop);
  }

  bool playRepeat(const char* soundId, uint16_t repeatCount) override {
    return playSoundRepeat(soundId, repeatCount);
  }

  bool beep(uint16_t durationMs = DEFAULT_BEEP_MS, uint16_t frequencyHz = DEFAULT_TONE_HZ) override {
    if (!canPlay()) return false;
    return engine_.playTone(clampFrequency(frequencyHz), clampDuration(durationMs), nowUs(), "beep");
  }

  bool tone(uint16_t frequencyHz, uint16_t durationMs) override {
    if (!canPlay()) return false;
    return engine_.playTone(clampFrequency(frequencyHz), clampDuration(durationMs), nowUs(), "tone");
  }

  void stop() override {
    if (!engine_.stop()) DEBUG_PRINTLN(F("[Buzzer] stop failed"));
  }

  bool isReady() const override {
    return canPlay();
  }

  bool isPlaying() const override {
    return engine_.isPlaying();
  }

  const char* currentSoundId() const override {
    return engine_.currentSoundId();
  }

  void addToJsonInfo(JsonObject& root) override {
    const BuzzerEngine::Snapshot snapshot = engine_.snapshot();
    JsonObject user = root["u"];
    if (user.isNull()) user = root.createNestedObject("u");

    JsonArray version = user.createNestedArray(F("Buzzer"));
    version.add(String(BUZZER_VERSION) + " " + BUZZER_BUILD);

    JsonArray state = user.createNestedArray(F("Buzzer state"));
    if (!enabled_) state.add(F("disabled"));
    else if (pin_ < 0) state.add(F("GPIO not configured"));
#if defined(ARDUINO_ARCH_ESP32)
    else if (!engineThreadSafe_) state.add(F("synchronization unavailable"));
#endif
    else if (pinUnavailable_) state.add(F("GPIO unavailable"));
    else if (!hardwareReady_) state.add(F("not ready"));
    else if (snapshot.playing) state.add(String(F("playing ")) + snapshot.soundId);
    else state.add(F("idle"));

#if defined(ARDUINO_ARCH_ESP32)
    JsonArray timing = user.createNestedArray(F("Buzzer timing"));
    timing.add(String(serviceTimerRunning_ ? F("esp_timer 2ms") : F("WLED loop")) +
      F(", late=") + snapshot.lastLatenessUs + F("us, max=") + snapshot.maxLatenessUs + F("us"));
#endif
  }

  void addToJsonState(JsonObject& root) override {
    const BuzzerEngine::Snapshot snapshot = engine_.snapshot();
    JsonObject state = root.createNestedObject(FPSTR(JSON_KEY));
    state["ready"] = canPlay();
    state["playing"] = snapshot.playing;
    state["sound"] = snapshot.soundId;
    state["loop"] = snapshot.looping;
    state["repeatRemaining"] = snapshot.repeatRemaining;
    state["note"] = snapshot.playing ? static_cast<uint32_t>(snapshot.noteIndex + 1u) : 0u;
    state["notes"] = static_cast<uint32_t>(snapshot.noteCount);
    state["frequency"] = snapshot.frequencyHz;
  }

  void readFromJsonState(JsonObject& root) override {
    JsonObject command = root[FPSTR(JSON_KEY)];
    if (command.isNull()) return;

    const bool hasStop = !command["stop"].isNull();
    const bool hasPlay = !command["play"].isNull();
    const bool hasTone = !command["tone"].isNull();
    const bool hasBeep = !command["beep"].isNull();
    const uint8_t commandCount = static_cast<uint8_t>(hasStop) + static_cast<uint8_t>(hasPlay) +
      static_cast<uint8_t>(hasTone) + static_cast<uint8_t>(hasBeep);
    if (commandCount > 1u) return;

    if (hasStop) {
      if (!command["stop"].is<bool>()) return;
      if (command["stop"].as<bool>()) stop();
      return;
    }

    if (hasPlay) {
      if (!command["play"].is<const char*>()) return;
      const char* soundId = command["play"].as<const char*>();
      if (soundId == nullptr || *soundId == '\0') return;

      bool loop = false;
      if (!command["loop"].isNull()) {
        if (!command["loop"].is<bool>()) return;
        loop = command["loop"].as<bool>();
      }

      JsonVariant repeatVariant = command["repeat"];
      const bool hasRepeat = !repeatVariant.isNull();
      if (hasRepeat) {
        if (!(repeatVariant.is<long>() || repeatVariant.is<unsigned long>())) return;
        const long repeatValue = repeatVariant.as<long>();
        if (loop || repeatValue < 1 || repeatValue > 255) return;
        playRepeat(soundId, static_cast<uint16_t>(repeatValue));
      } else {
        play(soundId, loop);
      }
      return;
    }

    if (hasTone) {
      JsonVariant toneVariant = command["tone"];
      if (!(toneVariant.is<long>() || toneVariant.is<unsigned long>())) return;
      const long frequency = toneVariant.as<long>();
      if (frequency < 20 || frequency > static_cast<long>(MAX_TONE_HZ)) return;

      long duration = DEFAULT_BEEP_MS;
      if (!command["duration"].isNull()) {
        JsonVariant durationVariant = command["duration"];
        if (!(durationVariant.is<long>() || durationVariant.is<unsigned long>())) return;
        duration = durationVariant.as<long>();
        if (duration < 1 || duration > static_cast<long>(MAX_DURATION_MS)) return;
      }
      tone(static_cast<uint16_t>(frequency), static_cast<uint16_t>(duration));
      return;
    }

    if (hasBeep) {
      JsonVariant beepVariant = command["beep"];
      if (!(beepVariant.is<long>() || beepVariant.is<unsigned long>())) return;
      const long duration = beepVariant.as<long>();
      if (duration < 1 || duration > static_cast<long>(MAX_DURATION_MS)) return;

      long frequency = DEFAULT_TONE_HZ;
      if (!command["frequency"].isNull()) {
        JsonVariant frequencyVariant = command["frequency"];
        if (!(frequencyVariant.is<long>() || frequencyVariant.is<unsigned long>())) return;
        frequency = frequencyVariant.as<long>();
        if (frequency < 20 || frequency > static_cast<long>(MAX_TONE_HZ)) return;
      }
      beep(static_cast<uint16_t>(duration), static_cast<uint16_t>(frequency));
    }
  }

  void addToConfig(JsonObject& root) override {
    JsonObject config = root.createNestedObject(FPSTR(USERMOD_NAME));
    config[FPSTR(CFG_ENABLED)] = enabled_;
    config[FPSTR(CFG_PIN)] = pin_;
    config[FPSTR(CFG_TYPE)] = buzzerType_;
    config[FPSTR(CFG_TRIGGER)] = trigger_;
    config[FPSTR(CFG_VOLUME)] = volume_;
    config[FPSTR(CFG_SOUND)] = sound_;
  }

  bool readFromConfig(JsonObject& root) override {
    JsonObject config = root[FPSTR(USERMOD_NAME)];
    if (config.isNull()) return false;

    const bool previousEnabled = enabled_;
    const int8_t previousPin = pin_;
    const uint8_t previousType = buzzerType_;
    const uint8_t previousTrigger = trigger_;
    const uint8_t previousVolume = volume_;

    bool complete = true;
    complete &= getJsonValue(config[FPSTR(CFG_ENABLED)], enabled_, true);
    complete &= getJsonValue(config[FPSTR(CFG_PIN)], pin_, int8_t(-1));

    if (!config[FPSTR(CFG_TYPE)].isNull()) getJsonValue(config[FPSTR(CFG_TYPE)], buzzerType_, uint8_t(BUZZER_TYPE_ACTIVE));
    else complete &= getJsonValue(config[FPSTR(CFG_OLD_TYPE)], buzzerType_, uint8_t(BUZZER_TYPE_ACTIVE));

    if (!config[FPSTR(CFG_TRIGGER)].isNull()) {
      getJsonValue(config[FPSTR(CFG_TRIGGER)], trigger_, uint8_t(TRIGGER_HIGH));
    } else if (buzzerType_ == BUZZER_TYPE_ACTIVE && !config[FPSTR(CFG_OLD_ACTIVE_HIGH)].isNull()) {
      bool activeHigh = true;
      getJsonValue(config[FPSTR(CFG_OLD_ACTIVE_HIGH)], activeHigh, true);
      trigger_ = activeHigh ? TRIGGER_HIGH : TRIGGER_LOW;
    } else {
      complete &= getJsonValue(config[FPSTR(CFG_OLD_PASSIVE_TRIGGER)], trigger_, uint8_t(TRIGGER_HIGH));
    }

    if (!config[FPSTR(CFG_VOLUME)].isNull()) getJsonValue(config[FPSTR(CFG_VOLUME)], volume_, uint8_t(100));
    else complete &= getJsonValue(config[FPSTR(CFG_OLD_VOLUME)], volume_, uint8_t(100));

    if (!config[FPSTR(CFG_SOUND)].isNull()) getJsonValue(config[FPSTR(CFG_SOUND)], sound_, String("victory"));
    else complete &= getJsonValue(config[FPSTR(CFG_OLD_SOUND)], sound_, String("victory"));

    if (buzzerType_ > BUZZER_TYPE_PASSIVE) buzzerType_ = BUZZER_TYPE_ACTIVE;
    if (trigger_ > TRIGGER_LOW) trigger_ = TRIGGER_HIGH;
    volume_ = clampPercent(volume_);
    if (BuzzerSounds::find(sound_.c_str()) == nullptr) sound_ = "victory";

    if (setupComplete_ &&
        (previousEnabled != enabled_ ||
         previousPin != pin_ ||
         previousType != buzzerType_ ||
         previousTrigger != trigger_ ||
         previousVolume != volume_)) {
      setupHardware();
    }

    return complete;
  }

  void appendConfigData() override {
    oappend(F("dd=addDropdown('Buzzer','type');addOption(dd,'Active',0);addOption(dd,'Passive',1);"));
    oappend(F("dd=addDropdown('Buzzer','trigger');addOption(dd,'High',0);addOption(dd,'Low',1);"));
    oappend(F("dd=addDropdown('Buzzer','sound');"));
    for (size_t i = 0; i < BuzzerSounds::count(); ++i) {
      const BuzzerSound& sound = BuzzerSounds::at(i);
      oappend(F("addOption(dd,'"));
      oappend(sound.label);
      oappend(F("','"));
      oappend(sound.id);
      oappend(F("');"));
    }

    // Preserve the validated DOM structure. Do not wrap or move the other rows.
    oappend(F("addInfo('Buzzer:enabled',1,'<style>.sec:has([name=\"Buzzer:enabled\"])>hr{display:none}</style>');"));
    oappend(F("addInfo('Buzzer:pin',1,'');"));
    oappend(F("addInfo('Buzzer:type',1,'');"));
    oappend(F("addInfo('Buzzer:trigger',1,'');"));
    oappend(F("addInfo('Buzzer:volume',1,'<br><i style=\"color:#fa0\">Active buzzers reproduce rhythm only. Passive buzzers reproduce note pitch.</i>');"));
    oappend(F("addInfo('Buzzer:sound',1,'');"));

    oappend(F("setTimeout(()=>{d.querySelectorAll('hr').forEach(e=>{let b=e.getBoundingClientRect();if(b.top<220&&b.width>innerWidth*.7)e.style.display='none'});let q=k=>{let a=d.getElementsByName('Buzzer:'+k);return a[a.length-1]},L=(e,x)=>{if(!e)return;let n=e.previousSibling;while(n&&n.nodeName!='BR'){if(n.nodeType==3&&n.data.trim()){n.data=x+' ';return}n=n.previousSibling}},w=e=>{if(!e)return;let b=e;while(b.previousSibling&&b.previousSibling.nodeName!='BR')b=b.previousSibling;let x=d.createElement('span');b.before(x);while(x.nextSibling){let n=x.nextSibling;x.append(n);if(n.nodeName=='BR')break}return x},e=q('enabled'),p=q('pin'),t=q('type'),g=q('trigger'),r=q('volume'),s=q('sound'),V=w(r);L(e,'Enabled:');L(p,'GPIO Pin:');L(t,'Buzzer Type:');L(g,'Trigger level:');L(s,'Sound:');if(V&&V.firstChild)V.firstChild.data='Volume: ';if(r){r.type='range';r.min=0;r.max=100;r.style.width='170px';let n=d.createElement('span'),u=()=>n.textContent=r.value+'%';r.after(n);r.oninput=u;u()}if(s){let f=u=>fetch(u).then(async x=>{if(!x.ok)alert(await x.text())}),p=d.createElement('button'),z=d.createElement('button');p.type=z.type='button';p.textContent='Play';z.textContent='Stop';p.onclick=()=>f('/buzzer?play='+encodeURIComponent(s.value));z.onclick=()=>f('/buzzer?stop=1');s.after(p);p.after(z);let n=d.createElement('div');n.style='margin:8px 0;color:#fa0';n.innerHTML='<i>Save the configuration before testing hardware changes.</i>';z.after(n);let h=d.createElement('label'),c=d.createElement('input');h.style='display:block;margin:16px 0 8px';c.type='checkbox';h.append(c,d.createTextNode(' Show API commands'));n.after(h);let a=d.createElement('div');a.hidden=true;a.innerHTML='<b>Web API</b><br>Play selected sound:<br><code id=\"bzurl\"></code><br>Loop a sound:<br><code id=\"bzloop\"></code><br>Stop:<br><code>'+location.origin+'/buzzer?stop=1</code><br>Tone:<br><code>'+location.origin+'/buzzer?tone=1000&duration=200</code><br>Beep:<br><code>'+location.origin+'/buzzer?beep=150</code>';h.after(a);c.onchange=()=>a.hidden=!c.checked;let U=()=>{let u=location.origin+'/buzzer?play='+s.value;d.getElementById('bzurl').textContent=u;d.getElementById('bzloop').textContent=u+'&loop=1'};s.onchange=U;U()}let H=()=>{if(V)V.hidden=!(t&&t.value==1)};if(t){t.onchange=H;H()}},0);"));
  }
};

static WLEDBuzzerUsermod buzzerUsermod;
REGISTER_USERMOD(buzzerUsermod);
