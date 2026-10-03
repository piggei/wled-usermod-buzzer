# WLED Buzzer Usermod

Standalone active/passive buzzer service for WLED, with an optional compile-time I2S audio backend.

**Stable release: v0.2.0**

This is the stable v0.2.0 release. It preserves the hardware-qualified shared-I2S clock routing and 8x256 TX DMA buffering, and includes four additional PCM sounds: a sample-backed `victory` plus three true Audio-only entries (`imperial_march`, `trumpet`, `wakeup`). The `victory` PCM is attenuated to 35% amplitude (about -9.1 dB) for better level matching with the rest of the audio catalog.

On the Waveshare ESP32-S3 RGB Matrix, the **shared-I2S electrical/ownership and real-time coexistence paths are hardware-qualified**: Buzzer follows the AudioReactive clocks without reconfiguring or uninstalling I2S0 RX, speaker playback is smooth, AudioReactive bars remain smooth, and AudioReactive OFF -> ON -> OFF transitions have been verified without loss of Buzzer operation.

Previous stable release: **v0.1.1**

![WLED Buzzer Usermod configuration](docs/wled-buzzer-usermod-gui.png)

The screenshot shows the standard active/passive build; the optional `Audio (I2S)` type appears only in firmware compiled with an audio profile.

## Features

- Active buzzer support using static GPIO output.
- Passive buzzer support using ESP32 LEDC tones.
- Shared High/Low trigger level for active and passive buzzers.
- Passive-buzzer volume control with a 0-100 slider.
- Fully non-blocking sound engine.
- 2 ms `esp_timer` scheduler with mutex-protected WLED-loop fallback if the timer cannot run.
- GPIO and LEDC allocation through WLED `PinManager`.
- Built-in sound registry shared by playback and the configuration UI.
- Sound selector with **Play** and **Stop** controls in Usermod Settings.
- HTTP/Webhook API, WLED JSON API, and reusable C++ service API.
- One-shot, finite-repeat, and infinite-loop playback modes.
- Timing diagnostics through WLED JSON info.
- Configurable Night Mode with daily quiet interval and automatic mute at the configured boundary.
- Complete optional weak-link C bridge matching the C++ playback operations.
- Optional compile-time I2S audio backend.
- Initial Waveshare ESP32-S3 RGB Matrix / ES8311 hardware profile.
- Zero audio-backend firmware overhead when audio support is not compiled.


## Optional I2S audio backend

`v0.2.0` keeps the hardware-qualified Audio/AudioReactive runtime unchanged. Audio remains **compile-time optional**. Standard ESP32/C3 active/passive builds therefore keep the lightweight runtime and do not include the ES8311/I2S code, PCM samples, or Audio-only sound IDs.

The first supported profile is the **Waveshare ESP32-S3 RGB Matrix** from the manufacturer reference package. The profile uses:

- ES8311 codec at I2C address `0x18`
- I2C SDA `GPIO47`
- I2C SCL `GPIO48`
- I2S BCLK `GPIO43`
- I2S LRCK/WS `GPIO38`
- I2S DOUT `GPIO21`
- I2S MCLK `GPIO12`
- PA enable `GPIO11`
- I2S peripheral `I2S_NUM_1`
- standalone mode: 48 kHz, 16-bit stereo output
- shared AudioReactive mode: 22.05 kHz, AudioReactive word width, I2S1 TX slave

Enable this profile at compile time with:

```ini
build_flags =
  ${env.build_flags}
  -D WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX
```

The board profile automatically enables `WLED_BUZZER_ENABLE_AUDIO` and, by default, the bundled audio samples. Define `WLED_BUZZER_DISABLE_AUDIO_SAMPLES` as an additional build flag if you want the I2S/ES8311 backend but do not want the embedded samples. Standard builds without the audio profile include neither the audio backend nor the sample data.

When compiled, **Audio (I2S)** appears as a third Buzzer Type. The existing sound registry and public APIs do not change: `play("victory")`, `repeat`, `loop`, Night Mode, HTTP/JSON commands and the C/C++ service all use the selected backend transparently.

Audio files are converted offline to **16 kHz mono / 16-bit PCM** and stored as flash-resident sample data; the audio task resamples them to the active output clock: 48 kHz in standalone mode or 22.05 kHz in shared AudioReactive mode. The bundled sample-backed sound IDs are now `alarm`, `attention`, `connect`, `disconnect`, `error`, `fail`, `imperial_march`, `notification`, `star_wars`, `success`, `trumpet`, `victory`, `wakeup`, `warning`, and `yankee_doodle`. `victory` keeps its validated note melody as the Active/Passive and no-samples fallback. `imperial_march`, `trumpet`, and `wakeup` are true Audio-only entries and have no buzzer-note approximation. Active and Passive playback of the original 15 common sounds is unchanged.

Cold-boot audio initialization now waits until the I2S producer has successfully primed the DMA stream before the saved ES8311 volume is re-applied. This avoids the observed case where playback remained silent after reboot until the Volume slider was moved.

`star_wars`, `connect`, `disconnect`, `notification`, and `yankee_doodle` use the maintainer-trimmed files exactly as supplied. Repeat/loop behavior is provided by the existing playback engine rather than duplicated inside the samples.

The sound registry carries a backend capability mask. The original 15 sounds remain common to Active, Passive and Audio. v0.2.0 includes three **Audio-only** sample effects: `imperial_march`, `trumpet`, and `wakeup`. They are compiled only under the optional sample feature, appear only for `Audio (I2S)`, and are absent from C3/Active/Passive-only firmware and menus.

For `Audio (I2S)` in the current build:

- **Buzzer Type** appears before GPIO Pin;
- **GPIO Pin** and **Trigger level** are hidden for `Audio (I2S)` because the profile uses fixed pins;
- **Volume** is an integer 0-100 slider and controls ES8311 DAC output level, without changing note frequency;
- the fixed profile pins above are used instead;
- Night Mode works at the common service layer and therefore mutes audio too.

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for the ES8311 driver attribution. For the shared bus design, failure history and guardrails, see [docs/SHARED_I2S_AUDIOREACTIVE.md](docs/SHARED_I2S_AUDIOREACTIVE.md).

## Built-in sounds

- `alarm`
- `attention`
- `beep`
- `connect`
- `disconnect`
- `double_beep`
- `error`
- `fail`
- `notification`
- `star_wars`
- `success`
- `triple_beep`
- `victory`
- `warning`
- `yankee_doodle`

Additional Audio-only sounds when bundled samples are enabled:

- `imperial_march`
- `trumpet`
- `wakeup`

Passive buzzers reproduce note pitch and timing. Active buzzers ignore pitch and reproduce the timing/rhythm of the same sound definitions.

All 15 common buzzer-note sounds have been explicitly validated on real passive-buzzer hardware; basic active-buzzer playback has also been validated on real hardware.

The Sound dropdown is alphabetically sorted and filtered by backend capability. The original 15 public sounds support all backends. With bundled samples enabled, the Waveshare Audio profile adds three Audio-only entries; they are hidden for Active/Passive and are not present at all in Active/Passive-only or no-samples firmware.

All built-in sounds use the same repeat/loop separation policy: Each sound has a non-zero final gap that is consumed only when another complete execution follows. One-shot playback therefore ends immediately after the final audible note, while `repeat` and `loop` playback get a deliberate pause between executions.

## Configuration

Open **Config -> Usermods -> Buzzer** and configure:

- **Enabled**
- **GPIO Pin:** GPIO used by Active/Passive backends; hidden for the fixed-profile Audio backend
- **Buzzer Type:** Active / Passive / optional Audio (I2S)
- **Trigger level:** High / Low for Active/Passive; hidden for Audio
- **Volume:** 0-100 slider for Passive and Audio; hidden for Active
- **Night mode:** enable/disable the daily quiet interval
- **From / To:** quiet-period boundaries; these fields are shown only when Night Mode is enabled
- **Sound:** built-in sound used by the Play test control

Save the configuration before testing after changing hardware settings.

Night Mode uses WLED local time. If WLED does not yet have valid time, Night Mode does not mute playback. Intervals that cross midnight are supported (for example `23:00` -> `07:00`). If `From` and `To` are equal while Night Mode is enabled, the buzzer is muted for the full day.

If playback is already active when the quiet interval begins, it is stopped and does not resume automatically when the interval ends. The Night Mode interface and configuration persistence have been validated on real hardware.

## HTTP API

The endpoint is:

```text
GET /buzzer
```

Play once:

```text
GET /buzzer?play=victory
```

Repeat a sound a finite number of times. `repeat` is the **total number of executions** and accepts 1-255:

```text
GET /buzzer?play=alarm&repeat=3
```

Loop indefinitely until explicitly stopped:

```text
GET /buzzer?play=alarm&loop=1
```

Stop playback:

```text
GET /buzzer?stop=1
```

Play a custom tone. Frequency must be **20-20000 Hz** and duration must be **1-60000 ms**:

```text
GET /buzzer?tone=1000&duration=200
```

Play a beep. The `beep` value is the duration in milliseconds (**1-60000 ms**); optional `frequency` must be **20-20000 Hz**:

```text
GET /buzzer?beep=150
GET /buzzer?beep=150&frequency=2000
```

Get the current buzzer status:

```text
GET /buzzer
```

`loop` and `repeat` are mutually exclusive. The HTTP endpoint accepts one command category per request (`stop`, `play`, `tone`, or `beep`). Numeric values are parsed strictly; malformed, signed, or out-of-range values return HTTP 400.

### curl / Bash examples

When an URL contains `&`, quote the whole URL so the shell does not interpret `&` as a background operator:

```bash
curl "http://<WLED-IP>/buzzer?play=alarm&repeat=3"
curl "http://<WLED-IP>/buzzer?play=alarm&loop=1"
curl "http://<WLED-IP>/buzzer?stop=1"
```

Replace `<WLED-IP>` with the IP address or hostname of the WLED device.

## WLED JSON API

Commands can also be sent through the normal WLED `/json/state` endpoint.

Play once:

```json
{"buzzer":{"play":"victory"}}
```

Repeat exactly three times:

```json
{"buzzer":{"play":"alarm","repeat":3}}
```

Loop indefinitely:

```json
{"buzzer":{"play":"alarm","loop":true}}
```

Stop:

```json
{"buzzer":{"stop":true}}
```

Tone:

```json
{"buzzer":{"tone":1000,"duration":200}}
```

Beep:

```json
{"buzzer":{"beep":150,"frequency":2000}}
```

For JSON playback, `repeat` accepts 1-255 and must not be combined with `loop`. An explicitly supplied `repeat: 0`, a value above 255, or a `loop` + `repeat` combination is ignored and does not start playback.

The current state is exposed under `buzzer` in `/json/state`, including `ready`, `muted`, Night Mode settings, `playing`, current sound, loop state, remaining repetitions, current note, note count, and current frequency.

## Internal C++ API

Other usermods can include `WLEDBuzzerService.h` and access the singleton service:

```cpp
#include "WLEDBuzzerService.h"

if (WLEDBuzzerService* buzzer = WLEDBuzzerService::instance()) {
  if (buzzer->isReady()) {
    buzzer->play("notification");
  }
}
```

Available playback calls include:

```cpp
buzzer->play("victory");
buzzer->play("alarm", true);       // infinite loop
buzzer->playRepeat("alarm", 3);   // exactly three executions
buzzer->beep(150);
buzzer->tone(1200, 200);
buzzer->stop();
```

Consumers can also query:

```cpp
buzzer->isReady();
buzzer->isPlaying();
buzzer->currentSoundId();
```

The service is out-of-tree and does not require a patch to WLED `const.h` or a reserved core usermod ID.

### Optional consumer bridge

The small `extern "C"` optional-consumer bridge introduced during the RC cycle is retained for usermods that must compile and link even when the Buzzer Usermod is not part of the firmware. A consumer can weak-link these symbols and detect their presence at runtime without including `WLEDBuzzerService.h`; this avoids PlatformIO LDF pulling the Buzzer repository into builds where it was not selected in `custom_usermods`.

Exported bridge symbols are:

```cpp
wledBuzzerServiceReady();
wledBuzzerServicePlaying();
wledBuzzerServicePlay(...);
wledBuzzerServicePlayRepeat(...);
wledBuzzerServiceBeep(...);
wledBuzzerServiceTone(...);
wledBuzzerServiceStop();
wledBuzzerServiceCurrentSoundId();
```

The bridge is now symmetric with the C++ service playback operations while retaining optional weak-link usage.

The optional bridge has been validated in a real consumer integration: iDotMatrix uses the standalone Buzzer Usermod for its sound playback while remaining buildable when the service is absent.

## WLED build integration

Use the repository as an out-of-tree WLED usermod through `custom_usermods`, for example:

```ini
custom_usermods =
  ${env:esp32dev.custom_usermods}
  symlink:///absolute/path/to/wled-usermod-buzzer
```

The repository must be available at the path referenced by the PlatformIO environment.

## Diagnostics

`/json/info` reports:

- usermod version/build;
- `disabled` / `GPIO not configured` / `synchronization unavailable` / `GPIO unavailable` / `not ready` / `muted by night mode` / `idle` / `playing <sound>` state;
- Night Mode interval and whether it is currently muted, active, disabled, or waiting for valid WLED time;
- current sound while playing;
- timing source (`esp_timer 2ms` or mutex-protected WLED loop fallback);
- last and maximum scheduler lateness;
- for Audio (I2S), stream mode/rate/word width plus DMA geometry and producer telemetry: writes, write errors, short writes, maximum `i2s_write()` duration, maximum producer gap, late-write count, observed core distribution and minimum free task stack.

`/json/state` reports the live playback state under the `buzzer` object. With the Audio backend it also reports `source` as `sample` or `tone`; sample-backed playback reports frequency `0` because pitch is encoded in the PCM sample rather than generated by the note synthesizer.

## Scope and limitations

- v0.2.0 keeps the qualified ESP32 active/passive behavior and the hardware-qualified ESP32-S3 shared-I2S runtime.
- If the engine mutex cannot be created, playback is disabled rather than exposing an unsafe asynchronous fallback.
- Passive playback requires LEDC resources.
- Active buzzers reproduce rhythm only; they cannot reproduce melody pitch.
- The HTTP endpoint assumes the same trusted-network model as the WLED HTTP interface.
- Common buzzer-note sounds are intentionally compact and monophonic; optional Audio-only PCM effects may be longer.
- The sound registry can be extended without changing the public playback API.

## Source package layout

Release archives use this fixed top-level directory:

```text
wled-usermod-buzzer/
```

This allows update/build scripts to consume subsequent archives without version-specific directory names.

## Release qualification

v0.2.0 is based on the hardware-qualified v0.2.0 runtime. On the tested Waveshare ESP32-S3 RGB Matrix / WLED 17.0.0-devV5 setup, 8x256 TX buffering removed the earlier speaker/AudioReactive stutter; speaker audio and AudioReactive video effects are smooth, and a captured shared-mode run exceeded 20,000 writes with `err=0`, `short=0`, `late=0`, and `maxGap` about 13 ms against about 92.9 ms of DMA coverage. Runtime AudioReactive OFF -> ON -> OFF transitions also passed.

Cold-boot saved volume, Audio <-> Active/Passive switching, Web API control, real iDotMatrix consumption, the expanded audio sample catalog, and ESP32-C3 Active/Passive playback were verified on hardware. The C3 build does not contain or expose the Audio-only sound IDs.

### AudioReactive coexistence (Waveshare)

When AudioReactive owns the shared Waveshare audio bus (`BCLK=43`, `LRCK=38`, `MCLK=12`, `DIN=39`), Buzzer uses:

- I2S0: untouched, owned by AudioReactive RX
- I2S1: TX slave only
- DOUT: GPIO21 to ES8311
- shared sample rate: 22050 Hz (WLED AudioReactive default)
- shared word width: 32 bit by default, or 16 bit when WLED is built with `I2S_USE_16BIT_SAMPLES`
- BCLK/LRCK are **tapped through the ESP32-S3 GPIO input matrix**, not reconfigured as I2S1 pads

The first shared-I2S experiments demonstrated two separate failure modes: configuring the shared clock pads through the legacy I2S pin helper could disturb the AudioReactive master routes, while a GPIO-matrix tap alone was insufficient on the tested ESP32-S3 until the GPIO input buffers were explicitly enabled. The qualified implementation configures only DOUT through the I2S pin helper, enables BCLK/LRCK input sensing without removing the I2S0 output routes, taps those clocks internally into I2S1, and verifies physical LRCK activity before bringing up the slave TX path.

If AudioReactive does not own the shared clock pins, Buzzer keeps the standalone 48 kHz I2S1 master mode. `/json/info` reports the GPIO-matrix clock tap only in shared mode and reports `local I2S1 clock master` in standalone mode.

The qualified runtime uses 8 DMA descriptors of 256 stereo frames. At 22.05 kHz shared rate this is about 92.9 ms of TX DMA coverage; at 48 kHz standalone rate it is about 42.7 ms. The producer task remains priority 1 and unpinned.

### Shared-bus transition safety

On the Waveshare profile, the Buzzer backend also treats partial PinManager ownership of the shared AudioReactive pins as a transition state. During that window it keeps the audio backend stopped instead of becoming an independent clock master. This prevents a start/stop race while AudioReactive acquires or releases BCLK/LRCK/MCLK/DIN one pin at a time.
