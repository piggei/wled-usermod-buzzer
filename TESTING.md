# WLED Buzzer Usermod - v0.2.0 release qualification

## Qualified release baseline

On Waveshare with AudioReactive enabled, verify after a cold boot that `/json/info` reports a ready shared-I2S backend and that AudioReactive remains stable at the normal silence floor. The Buzzer must not enter a repeated `not ready / initializing` cycle. If AudioReactive clocks are not yet active, Buzzer must report `shared LRCK inactive` and retry without disturbing RX.

Hardware qualification on the tested Waveshare setup confirms the shared clock path, real-time speaker stream and AudioReactive coexistence. The 8x256 TX DMA configuration produces smooth speaker audio and AudioReactive bars; more than 20,000 observed writes completed with `err=0`, `short=0`, `late=0`, and `maxGap` about 13 ms against about 92.9 ms of shared-mode DMA coverage. Runtime AudioReactive OFF -> ON -> OFF transitions also passed.

Recommended regression sequence:

1. Cold boot with AudioReactive enabled and Buzzer type `Audio (I2S)`.
2. Confirm microphone peak remains normal in silence before any Buzzer playback.
3. Confirm Buzzer becomes ready without touching Volume or saving settings.
4. Play one sample and confirm AudioReactive reacts only acoustically while the speaker is sounding, then returns to its previous floor.
5. Disable/re-enable AudioReactive and verify clean standalone/shared transitions.

This stable release retains the qualified active/passive runtime and optional compile-time I2S/ES8311 backend. The sound registry, Night Mode, bridge, note timing, repeat/loop gaps and active/passive backends must not regress.

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

Compile the intended WLED target and confirm the firmware contains `0.2.0` and `release`.


## 2A. Shared-I2S real-time regression gate

With Waveshare Audio (I2S) and AudioReactive enabled, `/json/info` must still report `DMA 8x256`. At 22050 Hz the reported coverage should be about `92879us`; in standalone 48000 Hz mode it should be about `42666us`. During a long sample, verify `err=0`, `short=0`, `late=0`, speaker playback is smooth and the AudioReactive bars remain smooth. This is a release regression check: task priority, affinity, clock routing and DMA geometry must remain unchanged.

## 2B. Audio sample catalog gate

With bundled samples enabled on the Waveshare Audio profile:

Hardware release status: one-shot playback of the expanded sample catalog is **PASS** on Waveshare; Web API control is **PASS**; ESP32-C3 playback is **PASS** and the three Audio-only IDs are absent as designed.

1. `victory` must report `source:"sample"` and play the new reference audio.
2. `imperial_march`, `trumpet`, and `wakeup` must appear in the Sound dropdown only while `Audio (I2S)` is selected.
3. Play all three Audio-only sounds once from the GUI and once through the Web/JSON or service API.
4. Verify `repeat=3`, `loop=1` + Stop, and Night Mode on at least one Audio-only sound.
5. Switch to Active and Passive: the three Audio-only entries must be hidden/disabled and API requests for them must return the unsupported-backend result rather than synthesizing a substitute melody.
6. Build with `WLED_BUZZER_DISABLE_AUDIO_SAMPLES`: `imperial_march`, `trumpet`, and `wakeup` must be absent from the registry, while `victory` must remain available and fall back to its validated note melody.

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

Hardware checks already confirmed for Night Mode:

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

Do not retune the 15 qualified common buzzer sounds unless a genuine regression is found.

## 8. Package gate

Verify:

```bash
sha256sum -c MANIFEST.sha256
```

Archives must retain the fixed root directory:

```text
wled-usermod-buzzer/
```

## Optional audio backend regression

The audio backend is compile-time optional and must not change the normal active/passive build when `WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX` is absent.

For the Waveshare ESP32-S3 RGB Matrix profile, verify on real hardware:

1. Compile with `-D WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX`.
2. Confirm **Audio (I2S)** appears as the third Buzzer Type.
3. Select Audio, save, reboot if required by the surrounding WLED build, and confirm `/buzzer` reports `"backend":"audio"` and `"ready":true`.
4. Connect a suitable speaker to the board `SPK` connector and test `beep`, `notification`, `victory` and `alarm` from the GUI.
5. Verify the Volume slider changes I2S output level and that Stop silences the output immediately.
6. Verify `repeat=3`, `loop=1` + Stop, and Night Mode with the audio backend.
7. Verify switching back to Active/Passive releases the audio backend and the original GPIO/LEDC paths still operate normally.
8. Verify HUB75 refresh remains stable while I2S audio is playing.
9. Check for conflicts with any other component using I2S1 or the fixed profile pins 11/12/21/38/43/47/48.
10. Verify a build without the audio profile still offers only Active/Passive and passes all existing regressions.

Verify that the Sound dropdown is alphabetically sorted and filtered by backend capability. The original 15 sounds remain common to all backends. With bundled samples enabled, `imperial_march`, `trumpet`, and `wakeup` are additional Audio-only entries and must never become selectable on Active/Passive.

Re-run the sample set (`fail`, `star_wars`, `alarm`, `connect`, `disconnect`, `notification`, `yankee_doodle`, `success`, `warning`, `attention`, `error`, `victory`, `imperial_march`, `trumpet`, `wakeup`) to confirm there is no playback regression. `/json/state` must report `source:"sample"` for all of these on Audio. A build with `WLED_BUZZER_DISABLE_AUDIO_SAMPLES` must omit the three Audio-only IDs and fall back to tone synthesis for common sounds such as `victory`.

## Cold-boot audio-volume regression

Hardware status: **PASS** on the Waveshare baseline. Re-run only as a release regression if desired.

1. Save an Audio (I2S) volume other than 100%.
2. Reboot the board without touching Usermod settings.
3. Trigger playback from both the GUI and an external consumer such as iDotMatrix.
4. Audio must be audible immediately and at the saved level; moving the Volume slider must not be required.


## AudioReactive coexistence regression

Hardware status: **PASS**, including smooth playback/video and AudioReactive OFF -> ON -> OFF transitions. Re-run as a release smoke test if desired.

On Waveshare ESP32-S3 RGB Matrix with AudioReactive configured for the board microphone:

The earlier b010 observation made while Buzzer was not actually running the Audio backend is **not** qualification evidence. Confirm the selected Buzzer Type is `Audio (I2S)` before running this matrix.

1. Cold boot with Buzzer Audio (I2S) selected and AudioReactive enabled.
2. Before playing any buzzer sound, confirm AudioReactive peak/noise floor matches a build without Buzzer (no idle saturation).
3. `/json/info` must report `shared-I2S`, `TX I2S1 slave`, `rate 22050 Hz`, `clock tap GPIO-matrix`, `RX I2S0 untouched`, and `DMA 8x256`.
4. Play both a PCM-backed sound and a tone-backed sound; both must be audible.
5. AudioReactive may react acoustically to the speaker while a sound is playing, but must return to the normal noise floor after playback.
6. Disable AudioReactive or release its shared pins; Buzzer must reinitialize to standalone I2S1 master mode without reboot.
7. Re-enable AudioReactive; Buzzer must return to shared-I2S mode.
