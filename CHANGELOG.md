# Changelog

## 0.1.0 - b006

- Corrected the generated settings labels to `Buzzer type:` and `Volume:` without duplicated words.
- Moved the active/passive pitch explanation directly below the passive Volume control; it remains hidden together with Volume for active buzzers.
- Kept the save-before-test warning below the test controls and added more visual spacing before the collapsible API section.
- Reworked `error` as two low 294 Hz tones with a 150 ms gap, deliberately more separated than `double_beep`.

## 0.1.0 - b005

- Replaced the passive Volume numeric field/help text with a 0-100 slider and live percentage readout; the whole control remains hidden for active buzzers.
- Restored `triple_beep` to the original iDotMatrix passive trill: 2 kHz, three 90 ms pulses with 70 ms gaps.
- Retuned `victory` from the supplied reference audio using its measured F-major onset contour and timing rather than the previous octave-expanded approximation.
- Kept the collapsible Web API section and the remaining b004 UI/API behavior unchanged.

## 0.1.0 - b004

- Restored the complete passive `Volume (%)` row text while retaining conditional hiding for active buzzers.
- Updated the passive volume help text to `Output level for passive buzzer (0-100%).`.
- Reworked `triple_beep` to the classic iDotMatrix alarm cadence: three 90 ms pulses separated by 70 ms gaps, rendered at a lower 800 Hz pitch on passive buzzers.
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
