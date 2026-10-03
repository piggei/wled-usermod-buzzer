## v0.2.0

- Stable release promoted from v0.2.0-rc.1 after successful real-hardware qualification; no runtime or sound-definition changes from RC1.
- Qualified on Waveshare ESP32-S3 RGB Matrix with ES8311, including shared-I2S coexistence with AudioReactive, smooth speaker/video behavior, AudioReactive OFF -> ON -> OFF transitions, cold-boot volume persistence, backend switching, Web API control, and real iDotMatrix consumption.
- Qualified on ESP32-C3 for Active/Passive buzzer playback with the optional ES8311 backend and Audio-only catalog correctly absent.
- Includes sample-backed `victory` plus Audio-only `imperial_march`, `trumpet`, and `wakeup`; `victory` retains its validated note fallback on non-audio/no-samples builds.
- Retains the RC1 `victory` level adjustment (35% PCM amplitude, about -9.1 dB) and the qualified shared-mode 8x256 TX DMA configuration.
- Source and wiki documentation promoted from release-candidate wording to the stable v0.2.0 baseline.

## v0.2.0-rc.1

- Promoted the hardware-qualified v0.2.0 feature set to the first release candidate.
- Keeps the qualified Waveshare shared-I2S routing and 8x256 TX DMA buffering unchanged.
- Includes the sample-backed `victory` plus Audio-only `imperial_march`, `trumpet`, and `wakeup`; ESP32-C3 / Active/Passive-only builds do not expose the Audio-only IDs.
- Attenuated the `victory` PCM sample to 35% amplitude (about -9.1 dB) to match the perceived level of the other bundled audio cues more closely.
- Records successful hardware checks for smooth Audio + AudioReactive coexistence, AudioReactive OFF -> ON -> OFF transitions, cold-boot saved volume, backend switching, Web API control, real iDotMatrix consumption, Waveshare sample playback, and ESP32-C3 playback with the audio-only catalog absent.
- Removed pre-RC development-planning documentation and refreshed source/wiki documentation for RC1.

## v0.2.0-dev-b015

- Final pre-RC sound-catalog build; preserves the hardware-qualified dev-b014 shared-I2S routing, 8x256 DMA buffering, producer priority/affinity and AudioReactive coexistence runtime unchanged.
- Added maintainer-supplied embedded PCM for `victory`; Audio now uses the sample while Active/Passive and no-samples Audio builds retain the validated note melody.
- Added three true Audio-only sample IDs: `imperial_march`, `trumpet`, and `wakeup`. They have no buzzer-note fallback.
- Audio-only IDs are compiled only when bundled samples are enabled, are selectable only on the Audio backend, and are absent from Active/Passive-only and `WLED_BUZZER_DISABLE_AUDIO_SAMPLES` builds.
- Added the new PCM payload in `audio/BuzzerAudioSamplesExtra.cpp`; all new assets use 16 kHz mono / signed 16-bit PCM and the existing runtime resampler/proxy timing path.
- Extended host regressions to execute the sound registry in normal, Audio+samples, and Audio-with-samples-disabled configurations, and to validate all 15 embedded sample records.
- No changes to I2S clock routing, AudioReactive ownership, DMA geometry, task scheduling, GUI structure, Night Mode, public APIs, C bridge or the original 15 common note definitions.

## v0.2.0-dev-b014

- Performance/telemetry build for the remaining Waveshare Buzzer + AudioReactive stutter observed after the dev-b012 clock-routing fix.
- Preserves the qualified shared-clock topology: AudioReactive/I2S0 remains untouched; I2S1 shared TX remains slave and keeps the GPIO-matrix BCLK/LRCK taps.
- Increases the producer block from 128 to 256 stereo frames and the TX DMA descriptor count from 4 to 8. This raises buffered coverage to about 92.9 ms at 22.05 kHz shared mode and 42.7 ms at 48 kHz standalone mode.
- Keeps the audio producer at priority 1 and unpinned so the hardware comparison isolates buffering from scheduler-affinity changes.
- Adds `/json/info` telemetry for write count/errors/short writes, maximum I2S write duration, maximum producer gap, late-write count, core distribution and minimum free producer-task stack.
- Defines a late write as a producer gap larger than the currently configured total DMA coverage.
- Fixes the 16-to-32-bit sample expansion to use multiplication instead of left-shifting a negative signed value, avoiding C++ signed-shift undefined behavior.
- Corrects Audio diagnostics so the GPIO-matrix clock tap is reported only in shared mode; standalone mode reports local I2S1 clock-master operation.
- No UI, API, sound registry, sample payload, Night Mode, Active/Passive backend, shared-pin ownership or I2S0 RX changes.

## v0.2.0-dev-b013

- Cleanup/handoff build based on the hardware-qualified dev-b012 runtime; no playback, I2S, sample, API, UI, Night Mode or Active/Passive logic changes.
- Consolidated the Waveshare shared-I2S / AudioReactive design, failure history, qualification evidence and future guardrails in `docs/SHARED_I2S_AUDIOREACTIVE.md`.
- Updated README, TESTING and wiki language to distinguish the invalid early coexistence observations, the failed dev-b011 clock-tap bring-up, and the hardware-qualified dev-b012 solution.
- Removed ambiguous historical build-number comments from sound/sample source comments and fixed the duplicated/mislabeled dev-b012 changelog heading.
- Refreshed regression checks and package metadata for a clean development handoff.

## v0.2.0-dev-b012

- Fixed the first shared-I2S slave bring-up failure observed on real Waveshare hardware after b011.
- The Buzzer now explicitly enables the GPIO input buffers on shared BCLK/LRCK while preserving AudioReactive/I2S0 output routing; the GPIO matrix tap alone was insufficient for I2S1 slave clock reception on the tested ESP32-S3 setup.
- Shared mode now verifies that LRCK is physically toggling before installing/priming I2S1. PinManager ownership is treated only as ownership evidence, not proof that clocks are already running.
- Failed shared-clock detection returns without enabling the PA, configuring ES8311 or installing I2S1, preventing the 2-second retry loop from generating periodic disturbances in the microphone path.
- Shared-mode retry backoff is increased to 5 seconds and shared teardown no longer zeroes the DMA buffer before uninstall.
- No changes to the sample set, UI, Night Mode, public APIs, sound capabilities or Active/Passive backends.

## v0.2.0-dev-b011

- Fixed shared-I2S coexistence on Waveshare: I2S1 slave TX now taps AudioReactive BCLK/LRCK through the ESP32-S3 GPIO matrix instead of reconfiguring the physical clock pads.
- AudioReactive/I2S0 remains the sole owner/driver of GPIO43 (BCLK), GPIO38 (LRCK) and GPIO12 (MCLK).
- Added diagnostic text `clock tap GPIO-matrix` for shared-I2S mode.


- Added Waveshare shared-I2S coexistence mode for WLED AudioReactive.
- When AudioReactive owns GPIO 43/38/12 and microphone DIN GPIO39, Buzzer uses I2S1 as TX slave at 22.05 kHz and the AudioReactive word width instead of generating a second set of clocks.
- Shared mode never calls `i2s_set_clk()` for I2S1, never routes MCLK from Buzzer, and never touches/uninstalls the AudioReactive I2S0 RX driver.
- Shared mode no longer reinitializes the global board I2C bus; only ES8311 registers at address 0x18 are configured.
- Added automatic runtime switching between shared slave-TX and standalone 48 kHz master-TX when AudioReactive pin ownership changes.
- Added ES8311 22.05 kHz coefficients, bounded I2S writes, and diagnostics for mode/rate/bit width with explicit `RX I2S0 untouched` status.
- No changes to the b009 sound capability model, PCM sample set, UI ordering, Night Mode, APIs, or Active/Passive backends.

## v0.2.0-dev-b009

- Added per-sound backend capability masks for Active, Passive and Audio playback.
- The configuration Sound dropdown is now alphabetically sorted and filtered for the selected backend.
- Added the architecture required for future Audio-only sounds: they can be compiled only with the optional audio/sample feature and omitted from Active/Passive builds and menus.
- HTTP playback now returns `409 Sound not supported by current backend` for a known sound that is unavailable on the selected backend.
- No new samples and no changes to the qualified b008 audio/backend behavior.

## v0.2.0-dev-b008

- Added maintainer-supplied embedded PCM samples for `attention` and `error`; Audio (I2S) now has eleven sample-backed public sound IDs.
- Kept the cold-boot audio initialization fix from dev-b007 unchanged after successful hardware validation.
- Confirmed the compile-time boundary remains unchanged: standard Active/Passive builds do not expose the Audio type and do not include codec, I2S runtime, or PCM sample payloads.
- No changes to UI behavior, Night Mode, public APIs, C bridge, Active/Passive playback, or the existing sample set.

## v0.2.0-dev-b007

- Fixed the remaining cold-boot Audio (I2S) mute path observed on Waveshare hardware.
- Deferred the first ES8311/I2S initialization by 1.5 seconds so all WLED usermods finish their boot-time setup before the audio profile claims and configures its fixed hardware.
- Changed codec-volume application to the audio producer task: the saved volume is applied only after I2S DMA/clocks are confirmed active.
- Re-asserts the PA enable pin and re-applies the saved codec volume on the first playback transition, making a silent boot self-heal without touching the Volume control.
- Volume changes are now queued to the audio task instead of performing I2C writes from the settings/main-loop context.
- No changes to UI, sound definitions, embedded sample cuts, Active/Passive backends, Night Mode or public APIs.

## v0.2.0-dev-b006

- Added Audio (I2S) embedded PCM samples for `success` and `warning`; the bundled sample-backed set now contains nine public sound IDs.
- Hardened ES8311 cold-boot volume initialization: backend readiness now waits for the I2S producer to successfully write a DMA buffer before re-applying the saved codec volume.
- This specifically targets the observed cold-boot condition where GUI and iDotMatrix playback remained silent until the Volume control was changed.
- Active/Passive backends, UI layout, Night Mode, APIs, and existing sample cuts remain unchanged.

## v0.2.0-dev-b005

- Replaced the automatically derived `connect`, `disconnect`, and `star_wars` samples with maintainer-trimmed reference files.
- Added embedded PCM sample playback for `notification` and `yankee_doodle`; Audio (I2S) now has seven sample-backed public sound IDs.
- Hardened ES8311 cold-start initialization: PA is enabled during codec bring-up, volume is re-applied after a short silent I2S pre-roll, and failed Audio initialization is retried automatically every 2 seconds.
- This addresses the observed case where Play could be silent until a Volume/configuration change forced hardware reinitialization.
- Active/Passive backends, Night Mode, public APIs, sound IDs, and existing note definitions remain unchanged.

## v0.2.0-dev-b004

- Added optional embedded PCM sample playback to the qualified I2S/ES8311 backend.
- Added sample-backed Audio playback for `fail`, `star_wars`, `alarm`, `connect`, and `disconnect`; all other sounds keep tone synthesis.
- Converted bundled samples to 16 kHz mono / 16-bit PCM and resample them to the 48 kHz I2S output in the audio task.
- Trimmed `star_wars` to one supplied theme loop; engine repeat/loop provides subsequent executions.
- Split the supplied USB reference recording into separate `connect` and `disconnect` samples.
- Added Audio diagnostics reporting `source=sample` or `source=tone`.
- Added `WLED_BUZZER_DISABLE_AUDIO_SAMPLES` to allow the Audio backend without bundled sample data.
- Kept UI, Night Mode, public sound IDs, Active/Passive backends, HTTP/JSON API, C++ service, and weak C bridge unchanged.

## v0.2.0-dev-b003

- Reordered Buzzer Type before GPIO Pin.
- Hide GPIO Pin and Trigger level for Audio (I2S).
- Volume slider now uses integer 0-100 values.
- Audio volume now uses the ES8311 DAC volume register; PCM amplitude remains fixed so pitch is independent of volume.

## v0.2.0-dev-b002

- Fixed Arduino variant macro collisions by renaming the Waveshare codec I2C pin constants to `CODEC_I2C_SDA` / `CODEC_I2C_SCL`.
- Updated the legacy I2S configuration to use `dma_desc_num` / `dma_frame_num` on ESP-IDF 5+, avoiding deprecated alias warnings while keeping ESP-IDF 4 compatibility.
- Fixed `/json/info` audio status construction so Arduino flash strings and C strings are not mixed in a conditional expression.
- No changes to sound definitions, audio pin mapping, public APIs, or the active/passive baseline.

## v0.2.0-dev-b001

- Started the v0.2.0 audio-backend development line from the stable v0.1.1 baseline.
- Added an optional compile-time I2S audio backend; standard active/passive builds remain audio-free.
- Added the initial Waveshare ESP32-S3 RGB Matrix / ES8311 hardware profile using the manufacturer-documented pins.
- Added 48 kHz / 16-bit sine-wave rendering of the existing built-in note registry through I2S.
- Added `Audio (I2S)` as a third Buzzer Type only when the board audio profile is compiled.
- Reused the existing Volume, Night Mode, Web/JSON API and C/C++ service paths without changing the public sound IDs.
- Added explicit backend diagnostics and host regression coverage for the Waveshare audio profile.
- Added third-party attribution for the ES8311 codec initialization code derived from the Waveshare Apache-2.0 example package.
- No PCM/WAV samples are included in b001; sample playback remains a later v0.2.0 milestone after real-hardware qualification.

## v0.1.1

- Stable release of Night Mode and the complete optional weak-link C bridge.
- Promoted the validated v0.1.1-rc.2 runtime without functional changes.
- Includes configurable daily quiet intervals, conditional From/To controls, overnight/full-day mute handling, and immediate stop when Night Mode becomes active.
- Includes the complete weak-link C bridge for play, repeat, beep, tone, stop, readiness, playback state, and current sound ID.
- Keeps the v0.1.0 sound registry, playback timing, active/passive backends, HTTP/JSON APIs, and validated UI behavior unchanged.
- Final UI refinement keeps compact 120 px Night Mode time fields.

## v0.1.1-rc.2

- UI-only refinement: reduced the Night Mode `From` and `To` time input widths to 120 px for a more compact layout.
- No changes to Night Mode logic, sound playback, APIs, bridge, or sound registry.

## v0.1.1-rc.1

- Rebased the Night Mode and complete optional C bridge work as the v0.1.1 maintenance release candidate.
- Includes configurable daily Night Mode with conditional `From:`/`To:` controls, same-day and overnight intervals, full-day mute when both boundaries are equal, and automatic stop when the quiet interval becomes active.
- Completes the optional weak-link C bridge with repeat, beep and tone operations in addition to play/status/stop.
- Preserves the qualified v0.1.0 sound registry, note timing, repeat/loop gaps, active/passive backend, APIs and existing UI behavior.
- Includes the cosmetic Night Mode end-time label fix (`To:`).
- Records successful hardware checks for Night Mode enable/disable behavior and configuration persistence across reset.



## v0.1.0

- First stable release.
- Promoted the fully qualified RC9 runtime without functional changes.
- Includes active/passive buzzer support, 15 built-in sounds, finite repeat and infinite loop playback, strict HTTP/JSON control, C++ service API, and the optional weak-link consumer bridge.
- Release qualification completed on real hardware for passive and active buzzer playback, all built-in sounds, repeat/loop behavior, and iDotMatrix consumer integration.
- Updated source and wiki documentation from release-candidate wording to the stable v0.1.0 baseline.

## v0.1.0-rc.9

- Final verification candidate before v0.1.0.
- Kept runtime behavior, UI, sound registry, note timing, final repeat/loop gaps, APIs, hardware backend, and optional consumer bridge unchanged from RC8.
- Recorded successful real-world integration with iDotMatrix as an external buzzer-service consumer.
- Recorded completed active/passive hardware, built-in sound, repeat/loop, and API qualification from the RC cycle.
- Updated release documentation and final verification checklist.
- Removed redundant H1 page headings from the distributed GitHub Wiki sources because GitHub renders page titles automatically.

## v0.1.0-rc.8

- Generalized the RC7 trailing-gap policy to all 15 built-in sounds.
- Every built-in sound now has a non-zero final `gapMs` used only between complete executions during `repeat` or `loop` playback.
- One-shot playback is audibly unchanged because `BuzzerEngine` skips the final gap when no following execution exists.
- Existing validated final gaps are preserved (`triple_beep` 550 ms, `victory` 100 ms, `alarm` 180 ms); zero-gap sounds receive a sound-appropriate inter-execution pause.
- Added registry regression coverage requiring a non-zero final gap for every built-in sound.
- No UI, note frequency, note duration, internal-note gap, API, bridge, engine, or hardware-backend changes.

## v0.1.0-rc.7

- Added a 550 ms trailing gap to `triple_beep` for finite-repeat and infinite-loop playback.
- One-shot `triple_beep` remains audibly unchanged because the engine skips the final gap when no following execution exists.
- This gives repeating consumers such as iDotMatrix Alarm a clean pause between three-pulse groups without adding timing logic to the consumer.
- Kept the UI, playback engine, hardware backend, bridge API, sound IDs, and all other built-in sound definitions unchanged from RC6.

## v0.1.0-rc.6

- Added a stable `extern "C"` bridge for optional consumer usermods.
- Consumers can now weak-link the buzzer service without including the C++ service header, so PlatformIO does not auto-discover/compile the Buzzer repository when it is absent from `custom_usermods`.
- Kept the UI, playback engine, hardware backend and all 15 built-in sounds unchanged from RC5.

## v0.1.0-rc.5

- Final verification candidate for v0.1.0.
- Kept runtime code, UI behavior, and all 15 qualified sound definitions unchanged from RC4.
- Added the validated RC4/RC5 configuration screenshot to README and wiki.
- Reworked release testing/documentation into final-style user documentation and removed stale RC3/RC4 release-validation wording.
- Removed audit-history pages and audit navigation from the distributed wiki.

## v0.1.0-rc.4

- Reverted the RC3 configuration DOM rewrite that regressed the previously validated Usermod Settings layout.
- Restored the proven RC2/b009 UI structure; only the passive `Volume` row is wrapped for conditional visibility.
- Applied label changes without moving the Enabled/GPIO/Type/Trigger/Sound DOM rows.
- Defined the UI labels as `Enabled:`, `GPIO Pin:`, `Buzzer Type:`, `Trigger level:`, `Volume:`, and `Sound:`.
- Restored the title-separator suppression and the active-buzzer behavior where only Volume plus its explanatory text are hidden.
- Retained all RC3 API/input/concurrency/diagnostic hardening and all 15 qualified sound definitions unchanged.
- Removed the stale UI screenshot from the release package until RC4 rendering is confirmed on real WLED hardware.

## v0.1.0-rc.3

- Hardened HTTP numeric parsing: signed, malformed, partially numeric, empty and out-of-range values are rejected instead of being converted/clamped unexpectedly.
- Added strict, case-insensitive boolean parsing for HTTP flags and reject ambiguous multi-command requests.
- Aligned JSON tone/beep/repeat validation with the strict HTTP contract and reject ambiguous multi-command objects.
- Made stop/teardown deterministic: stop waits for the engine mutex, `esp_timer_stop()` is checked, and teardown aborts rather than releasing hardware after a failed stop.
- Removed the unsafe no-mutex fallback: if the engine mutex cannot be created, playback is disabled; WLED-loop timing fallback is retained only with mutex protection.
- Added atomic engine snapshots for `/buzzer`, `/json/info` and `/json/state` diagnostics.
- Fixed duplicated configuration labels (`Enabled Enabled`, `Pin GPIO`, `Sound Sound`) while preserving the established UI layout.
- Added host-testable strict input parsing, exact 15-sound registry assertions and cwd-independent static tests.
- Corrected README/wiki diagnostics terminology, HTTP ranges, version examples and RC3 documentation.
- Kept all 15 hardware-qualified sound definitions unchanged.

## v0.1.0-rc.2

- Fixed JSON playback semantics so an explicitly supplied `repeat: 0` is invalid instead of falling back to one-shot playback.
- Kept JSON finite repeat limited to 1-255 and mutually exclusive with infinite `loop`, matching the HTTP API.
- Added regression coverage for explicit JSON repeat-member detection and RC2 version metadata.
- Recorded hardware validation of `success`, `attention`, and active-buzzer playback.
- Cleaned the duplicated changelog heading inherited from development history.
- Updated README, TESTING, and release documentation; no UI or sound-library changes.

## v0.1.0-rc.1

- Promoted the b019 feature set to the first release candidate.
- Kept the frozen configuration UI and validated sound definitions unchanged.
- Documented Bash/curl URL quoting for requests containing `&`.
- Aligned JSON `repeat` validation with the HTTP API: 1-255 and mutually exclusive with `loop`.
- Reworked README and TESTING into release-candidate documentation and removed stale development-test contradictions.
- Removed generated host-test binaries from the release package.

## v0.1.0-dev-b019

- Added finite sound repetition with `repeat=N` (1-255 total executions).
- Kept `loop=1` as infinite playback until explicit stop.
- HTTP API rejects simultaneous `loop` and `repeat` parameters.
- Added JSON `repeat` support and public C++ `playRepeat()` service method.
- Added engine regressions for exact finite repeat counts and continued infinite looping.
- No UI or sound-library changes from b018.

## v0.1.0-dev-b018

- Rebuilt `star_wars` directly from the supplied MP3 reference instead of the earlier hand reconstruction.
- Correct sequence: three C4 staccato cuts, F4 long, C5 long, 400 ms rest, Bb4, A4, G4, F5 long, C5 long.
- Preserved all other sounds and the frozen UI unchanged.

## v0.1.0-dev-b017

- Corrected `star_wars` opening to exactly three short G4 cuts followed by the original long G4 note.
- The three cuts use the same pitch as the following long note.
- Extended only the penultimate `star_wars` G5 note from 400 ms to 600 ms.
- No UI changes and no changes to any other sound.

## v0.1.0-dev-b016

- Corrected `star_wars` to the exact requested three-part order: four G4 cuts, then the original first pattern, then the original second pattern.
- Removed the b015 timing reinterpretation: the two original melody patterns are restored unchanged, including the 400 ms G5 and final D5 notes.
- No UI changes and no changes to any other sound.

## v0.1.0-dev-b015

- Rebuilt `star_wars` from the new three-pattern BeepBox arrangement.
- Added the four short G4 cuts as a separate intro before the original first G4 note instead of replacing it.
- Preserved the complete two-block melody after the cut intro.
- Extended both final `star_wars` notes to 600 ms for the sustained ending requested during hardware tuning.
- No UI changes.

## v0.1.0-dev-b014

- Marked `victory` as hardware-validated and frozen.
- Reworked only the opening of `star_wars`: the first G4 is now rendered as four short cuts over the same 600 ms span.
- Extended the penultimate `star_wars` G5 note from 400 ms to 600 ms.
- No UI changes.

## v0.1.0-dev-b013

- Reconstructed `star_wars` from both BeepBox patterns shown in the supplied screenshots, preserving their relative pitches, durations, and rests.
- Extended only the final `victory` note from 200 ms to 400 ms; all other `victory` timing remains unchanged.
- Marked `notification`, `connect`, and `disconnect` as hardware-validated.
- No UI changes.

## v0.1.0-dev-b012

- Added `star_wars`, a compact monophonic opening cue.
- Simplified `notification` to its former second note only.
- Raised `connect` / `disconnect` to the former notification pitch pair (880/1175 Hz).
- Added a double-speed `victory` trial by halving note durations and rests.
- Marked `fail` and `yankee_doodle` as hardware-validated.
- No UI changes.

## v0.1.0-dev-b012

- Transposed `fail` up by one octave (+12 semitones) while preserving all note durations and gaps.
- Extended the penultimate `Yankee Doodle` note to the same 560 ms duration as the final note.
- Marked `alarm` as hardware-validated; all previously validated sounds remain unchanged.

## v0.1.0-dev-b012

- Replaced `fail` with the exact monophonic pattern decoded from the supplied BeepBox v9 URL.
- Corrected the `Yankee Doodle` ending: restored the B4 note, made it the held penultimate/final retained note, and removed only the trailing G4.
- Doubled all `alarm` note durations while preserving the existing gaps and pitches.
- Marked `warning` as hardware-validated; previously validated sounds remain unchanged.

## v0.1.0-dev-b012

- Shortened all `Victory` rests by 50% while preserving its notes and note durations.
- Added `Yankee Doodle` with the first two phrases (12 notes).
- Removed the horizontal separator directly below the Buzzer section title.

## 0.1.0 - b007

- Replaced `victory` with the exact monophonic pattern decoded from the supplied BeepBox v9 song URL.
- Preserved the BeepBox timing at 150 BPM and transposed the complete melody by +12 semitones (one octave).
- No other sound definitions were changed.
- Configuration UI otherwise remains frozen from b006.

## 0.1.0 - b006

- Corrected the generated settings labels to `Buzzer type:` and `Volume:` without duplicated words.
- Moved the active/passive pitch explanation directly below the passive Volume control; it remains hidden together with Volume for active buzzers.
- Kept the save-before-test warning below the test controls and added more visual spacing before the collapsible API section.
- Reworked `error` as two low 294 Hz tones with a 150 ms gap, deliberately more separated than `double_beep`.

## 0.1.0 - b005

- Replaced the passive Volume numeric field/help text with a 0-100 slider and live percentage readout; the whole control remains hidden for active buzzers.
- Restored `triple_beep` to the original high passive trill: 2 kHz, three 90 ms pulses with 70 ms gaps.
- Retuned `victory` from the supplied reference audio using its measured F-major onset contour and timing rather than the previous octave-expanded approximation.
- Kept the collapsible Web API section and the remaining b004 UI/API behavior unchanged.

## 0.1.0 - b004

- Restored the complete passive `Volume (%)` row text while retaining conditional hiding for active buzzers.
- Updated the passive volume help text to `Output level for passive buzzer (0-100%).`.
- Reworked `triple_beep` to the classic three-pulse alarm cadence: three 90 ms pulses separated by 70 ms gaps, rendered at a lower 800 Hz pitch on passive buzzers.
- Kept the collapsible Web API section and the remaining b003 UI/API behavior unchanged.

## 0.1.0 - b003

- Corrected the configuration label to `Trigger level`.
- Passive-only volume help is now hidden together with the Volume field when Type is Active.
- Added a `Show API commands` checkbox; Web API examples are hidden by default.
- Reworked `victory` into a short ascending fanfare inspired by the supplied success reference.
- Reworked `fail` into a chromatic cartoon-style descending motif inspired by the supplied failure reference.

## 0.1.0 - b002

- Unified High/Low trigger-level setting for active and passive buzzers.
- Simplified configuration field names and labels.
- Added `triple_beep` built-in sound.
- Moved the save-before-test warning directly below the Play/Stop controls.
- Moved and expanded Web API instructions below the test warning.
- Added b001 configuration-key migration on read.

## 0.1.0 - b001

Initial development build of the standalone WLED Buzzer Usermod.

- Active and passive buzzer backends.
- Non-blocking note sequencer.
- Built-in sound registry.
- HTTP and WLED JSON APIs.
- Configuration-page sound test controls.
- Public internal service interface.
- PinManager GPIO and LEDC allocation.
- Timing diagnostics and host regression tests.

