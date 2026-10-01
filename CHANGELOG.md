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

## v0.2.0-dev-b002

- UI-only follow-up to dev-b001.
- Fixed the Night Mode end-time label so it is rendered as `To:` instead of `NightTo`.
- No changes to Night Mode behavior, C bridge, sound library, playback engine, APIs, or existing validated UI controls.

## v0.2.0-dev-b001

- Added configurable Night Mode with `From`/`To` daily quiet times; the time fields are shown only while Night Mode is enabled.
- Night Mode uses WLED local time, supports intervals crossing midnight, and leaves playback enabled when local time is not yet valid.
- Playback already running when the quiet interval begins is stopped and does not automatically resume.
- Added Night Mode diagnostics to `/buzzer`, `/json/info`, and `/json/state`.
- Completed the optional weak-link C bridge with `playRepeat`, `beep`, and `tone` functions in addition to the existing play/status/stop functions.
- Added host tests for time parsing, same-day/overnight/full-day quiet intervals, and the complete C bridge.
- Kept the v0.1.0 sound registry, note timing, repeat/loop gaps, and existing active/passive UI behavior unchanged.

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

## v0.1.0-dev-b011

- Transposed `fail` up by one octave (+12 semitones) while preserving all note durations and gaps.
- Extended the penultimate `Yankee Doodle` note to the same 560 ms duration as the final note.
- Marked `alarm` as hardware-validated; all previously validated sounds remain unchanged.

## v0.1.0-dev-b010

- Replaced `fail` with the exact monophonic pattern decoded from the supplied BeepBox v9 URL.
- Corrected the `Yankee Doodle` ending: restored the B4 note, made it the held penultimate/final retained note, and removed only the trailing G4.
- Doubled all `alarm` note durations while preserving the existing gaps and pitches.
- Marked `warning` as hardware-validated; previously validated sounds remain unchanged.

## v0.1.0-dev-b009

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
