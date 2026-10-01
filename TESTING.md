# WLED Buzzer Usermod - v0.1.1 test plan

This stable release starts from the qualified v0.1.0 runtime and adds only Night Mode plus completion of the optional C bridge. The existing sound registry, note timing, final repeat/loop gaps and active/passive backend must not regress.

## 1. Host regression tests

Run:

```bash
./run_host_tests.sh
```

Also verify the static test from another directory:

```bash
cd /tmp
python3 /path/to/wled-usermod-buzzer/tests/test_static.py
```

Expected:

```text
Static regression checks passed.
All host tests passed.
```

The host suite now also covers `HH:MM` parsing, same-day and overnight Night Mode intervals, equal-boundary full-day mute behavior, and all optional C bridge playback calls.

## 2. WLED build gate

Compile the intended WLED target and confirm the firmware contains `0.1.1` and `final`.

## 3. Configuration UI gate

The validated v0.1.0 UI must remain unchanged except for the new Night Mode controls. Confirm:

- no separator line below **Buzzer**;
- existing labels remain `Enabled:`, `GPIO Pin:`, `Buzzer Type:`, `Trigger level:`, `Volume:`, and `Sound:`;
- Active still hides only the passive Volume row and its explanatory text;
- `Night mode:` appears once;
- with Night Mode disabled, `From:` and `To:` are hidden;
- enabling Night Mode shows `From:` and `To:` as time controls;
- disabling it again immediately hides both fields;
- Play/Stop, save warning and Show API commands remain unchanged.

## 4. Night Mode behavior gate

Hardware checks already confirmed during pre-RC Night Mode testing:

- while inside the configured quiet interval, disabling Night Mode restores playback immediately and re-enabling it mutes playback again;
- Night Mode configuration persists across application/device reset.

Set WLED to a valid local time and test both interval forms:

- same day, for example `13:00 -> 14:00`;
- across midnight, for example `23:00 -> 07:00`.

Confirm:

- outside the interval, GUI/API/C++ playback works normally;
- inside the interval, `play`, `repeat`, `loop`, `beep` and `tone` are blocked;
- HTTP playback commands return `423 Buzzer muted by night mode.`;
- Stop always remains available;
- a sound/loop already playing when the quiet interval begins is stopped;
- playback does not automatically resume when the quiet interval ends;
- if WLED time is not yet valid, Night Mode does not mute playback;
- equal `From` and `To` values intentionally mean full-day mute while Night Mode is enabled.

## 5. Diagnostics gate

Check `/buzzer`, `/json/info`, and `/json/state`.

`/buzzer` and `/json/state` must expose Night Mode state and current `muted` status. `/json/info` may report:

- `muted by night mode`;
- Night Mode `disabled`;
- configured interval with `(active)` or `(muted)`;
- `(waiting for valid time)` if WLED has not acquired valid local time.

`ready` remains the hardware/service readiness state; a Night Mode mute does not make the service unavailable.

## 6. Complete optional C bridge gate

In addition to the v0.1.0 bridge calls, verify:

```cpp
wledBuzzerServicePlayRepeat("alarm", 3);
wledBuzzerServiceBeep(150, 1000);
wledBuzzerServiceTone(1200, 200);
```

The complete bridge surface is:

- `wledBuzzerServiceReady()`;
- `wledBuzzerServicePlaying()`;
- `wledBuzzerServicePlay()`;
- `wledBuzzerServicePlayRepeat()`;
- `wledBuzzerServiceBeep()`;
- `wledBuzzerServiceTone()`;
- `wledBuzzerServiceStop()`;
- `wledBuzzerServiceCurrentSoundId()`.

Repeat the optional-consumer test with iDotMatrix if convenient. Calls that request playback during Night Mode must return `false`.

## 7. Existing v0.1.0 regression

Perform a short smoke test of:

- passive playback;
- active playback;
- one-shot sound;
- `repeat=3`;
- `loop=1` plus Stop;
- one custom tone;
- one beep;
- iDotMatrix playback.

Do not retune the 15 qualified built-in sounds unless a genuine regression is found.

## 8. Package gate

Verify:

```bash
sha256sum -c MANIFEST.sha256
```

Archives must retain the fixed root directory:

```text
wled-usermod-buzzer/
```
