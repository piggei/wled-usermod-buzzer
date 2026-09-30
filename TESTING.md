# WLED Buzzer Usermod - v0.1.0-rc.8 test plan

This is the final verification checklist before promotion to v0.1.0. RC8 keeps the validated UI, melodies and one-shot timing stable while ensuring every built-in sound has a deliberate inter-execution pause for repeat/loop playback.

## 1. Host regression tests

Run from the repository root:

```bash
./run_host_tests.sh
```

Also verify that the static test is independent of the current working directory:

```bash
cd /tmp
python3 /path/to/wled-usermod-buzzer/tests/test_static.py
```

Expected result:

```text
Static regression checks passed.
All host tests passed.
```

The host suite covers engine sequencing, scheduler jitter, finite repeat, infinite loop, custom tones, stop behavior, strict input parsing, and exact membership of the 15-sound registry.

## 2. WLED build gate

Compile the intended WLED release target with this usermod enabled. Record:

- WLED version/commit;
- PlatformIO environment;
- MCU;
- Arduino-ESP32 version;
- build result and any warning attributable to the usermod.

Confirm that the firmware contains `0.1.0` and `rc.8` and that **Config -> Usermods -> Buzzer** is available.

## 3. Configuration UI gate

The real WLED page must match the validated RC5 screenshot. Confirm:

- no separator line directly below the **Buzzer** title;
- `Enabled:` appears once;
- `GPIO Pin:` appears once;
- `Buzzer Type:` appears once;
- `Trigger level:` appears once;
- `Volume:` appears once and is visible only for Passive;
- the active/passive explanatory text is displayed correctly;
- `Sound:` appears once, followed by Play and Stop;
- the save-before-test warning remains below the test controls;
- **Show API commands** hides/shows the API examples;
- selecting **Active** hides only the passive-only Volume row and its explanatory text.

## 4. Built-in sound regression

The following 15 sound IDs remain the qualified registry:

- `beep`
- `double_beep`
- `triple_beep`
- `notification`
- `success`
- `victory`
- `yankee_doodle`
- `fail`
- `star_wars`
- `warning`
- `error`
- `connect`
- `disconnect`
- `attention`
- `alarm`

Perform a short passive-buzzer regression and a short active-buzzer regression. For repeat/loop separation, verify at least `beep`, `triple_beep`, one melody (for example `victory`), and `alarm`; also verify `triple_beep` specifically:

- one-shot playback still sounds as the original three 90 ms pulses with 70 ms internal gaps and no audible tail delay;
- repeat/loop playback inserts approximately 550 ms of silence between complete three-pulse groups.

Do not retune any other sound unless a genuine regression is found.

## 5. HTTP API positive tests

Quote URLs containing `&` in Bash:

```bash
curl "http://<WLED-IP>/buzzer?play=victory"
curl "http://<WLED-IP>/buzzer?play=alarm&repeat=3"
curl "http://<WLED-IP>/buzzer?play=alarm&loop=1"
curl "http://<WLED-IP>/buzzer?stop=1"
curl "http://<WLED-IP>/buzzer?tone=1000&duration=200"
curl "http://<WLED-IP>/buzzer?beep=150"
curl "http://<WLED-IP>/buzzer?beep=150&frequency=2000"
curl "http://<WLED-IP>/buzzer"
```

Expected:

- `repeat=3` produces exactly three total executions;
- `loop=1` continues until Stop;
- custom tone accepts 20-20000 Hz and 1-60000 ms;
- beep accepts 1-60000 ms and an optional 20-20000 Hz frequency;
- status is internally coherent.

## 6. HTTP API negative tests

These must return HTTP 400 and must not start an unintended sound:

```bash
curl -i "http://<WLED-IP>/buzzer?tone=-1&duration=200"
curl -i "http://<WLED-IP>/buzzer?tone=1000&duration=-1"
curl -i "http://<WLED-IP>/buzzer?beep=abc"
curl -i "http://<WLED-IP>/buzzer?play=alarm&repeat=0"
curl -i "http://<WLED-IP>/buzzer?play=alarm&repeat=256"
curl -i "http://<WLED-IP>/buzzer?play=alarm&repeat=3abc"
curl -i "http://<WLED-IP>/buzzer?play=alarm&loop=1&repeat=3"
curl -i "http://<WLED-IP>/buzzer?play=alarm&tone=1000"
```

Boolean text is case-insensitive for the supported forms (`0/1`, `false/true`, `off/on`).

## 7. JSON API gate

Verify one-shot, finite repeat, loop, stop, tone and beep through `/json/state`. Also verify that malformed types, out-of-range values, `repeat:0`, `repeat:256`, `loop + repeat`, and multiple command categories do not start ambiguous playback.

## 8. Reconfiguration/resource gate

Verify:

- enable/disable;
- GPIO change;
- Active/Passive change;
- High/Low trigger;
- passive volume;
- occupied GPIO failure;
- teardown followed by successful reinitialization.

## 9. Diagnostics gate

Check `/json/info`, `/json/state`, and `/buzzer` while idle and while playing. Textual info states may include:

- `disabled`;
- `GPIO not configured`;
- `synchronization unavailable`;
- `GPIO unavailable`;
- `not ready`;
- `idle`;
- `playing <sound>`.

The preferred timing source is `esp_timer 2ms`; a mutex-protected WLED-loop fallback is permitted if the timer cannot run.

## 10. C++ consumer integration gate

Use a real second usermod to exercise `WLEDBuzzerService`:

- `instance()`;
- `isReady()`;
- `play()`;
- `playRepeat()`;
- `beep()`;
- `tone()`;
- `stop()`;
- `isPlaying()`;
- `currentSoundId()`;
- behavior when the service is absent or not ready.

The planned iDotMatrix migration is the intended real-world consumer test.

## 11. Package gate

Verify:

```bash
sha256sum -c MANIFEST.sha256
```

Release archives must contain the fixed root directory:

```text
wled-usermod-buzzer/
```

## Promotion rule

Promote RC8 to **v0.1.0** only after the intended WLED target build, minimal active/passive hardware regression, API regression, resource/reconfiguration checks, and real C++ consumer integration all pass without requiring functional changes.
