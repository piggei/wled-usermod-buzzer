# WLED Buzzer Usermod

Standalone buzzer service for WLED.

Version: **0.1.0**  
Development build: **b005**

## Features in b005

- ESP32 active buzzer support using static GPIO output.
- ESP32 passive buzzer support using LEDC tones.
- Shared High/Low trigger level for both active and passive buzzers.
- Configurable passive volume with a 0-100 slider.
- Non-blocking sound engine serviced by a 2 ms `esp_timer`, with WLED-loop fallback.
- WLED GPIO allocation through `PinManager`.
- WLED LEDC channel allocation through `PinManager` for passive buzzers.
- Built-in sound registry shared by the playback engine and the configuration UI.
- Configuration-page sound selector with **Play** and **Stop** buttons.
- Collapsible **Show API commands** section in the configuration UI.
- Simple HTTP webhook API.
- WLED `/json/state` command API.
- Public C++ service interface for other usermods.
- Timing diagnostics in WLED JSON info.

## Built-in sounds

- `beep`
- `double_beep`
- `triple_beep`
- `notification`
- `success`
- `victory`
- `fail`
- `warning`
- `error`
- `connect`
- `disconnect`
- `attention`
- `alarm`

Passive buzzers reproduce the note frequencies. Active buzzers ignore pitch and reproduce the timing/rhythm of the same sound definitions.
The `error` sound uses two low 294 Hz tones with a wider gap than `double_beep`.

## Configuration

Open **Config -> Usermods -> Buzzer** and configure:

- Enabled
- GPIO
- Buzzer type: Active / Passive
- Trigger level: High / Low (for both active and passive buzzers)
- Volume: 0-100 slider (passive only)
- Sound selector with Play / Stop

Save the configuration before using the test controls after a hardware setting change.

## HTTP API

Play a built-in sound:

```text
GET /buzzer?play=victory
```

Loop a built-in sound until stopped:

```text
GET /buzzer?play=alarm&loop=1
```

Stop playback:

```text
GET /buzzer?stop=1
```

Play a custom tone:

```text
GET /buzzer?tone=1000&duration=200
```

Play a simple beep. The value is the duration in milliseconds:

```text
GET /buzzer?beep=150
GET /buzzer?beep=150&frequency=2000
```

Get current status:

```text
GET /buzzer
```

## WLED JSON API

The usermod accepts commands through the normal WLED `/json/state` endpoint.

Play:

```json
{"buzzer":{"play":"victory"}}
```

Loop:

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

The current state is also added to `/json/state` under `buzzer`.

## Internal usermod API

Other usermods can include `WLEDBuzzerService.h` and use the singleton service:

```cpp
#include "WLEDBuzzerService.h"

if (WLEDBuzzerService* buzzer = WLEDBuzzerService::instance()) {
  if (buzzer->isReady()) buzzer->play("notification");
}
```

Available calls:

```cpp
buzzer->play("victory");
buzzer->play("alarm", true);
buzzer->beep(150);
buzzer->tone(1200, 200);
buzzer->stop();
```

This first build deliberately keeps the internal integration compile-time and out-of-tree. It does not require a patch to WLED `const.h` or a reserved core usermod ID.

## Build integration

Use the repository as an out-of-tree WLED usermod through `custom_usermods`, for example:

```ini
custom_usermods =
  ${env:esp32dev.custom_usermods}
  symlink:///absolute/path/to/wled-usermod-buzzer
```

## Diagnostics

WLED `/json/info` reports:

- usermod version/build
- ready / idle / playing state
- current sound
- timing source (`esp_timer 2ms` or WLED loop fallback)
- last and maximum scheduler lateness

## Current scope / limitations

- b005 targets ESP32-family WLED builds.
- Passive output requires LEDC.
- Active buzzers cannot reproduce pitch; only the timing of a melody is retained.
- The HTTP endpoint is intended for the same trusted network model as the normal WLED HTTP API.
- Built-in sounds are intentionally compact; `victory` and `fail` use recognizable fanfare / cartoon-loss style motifs adapted for a monophonic buzzer.
- The registry can be extended without changing API semantics.
