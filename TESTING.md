# Testing - v0.1.0 b005

## Host tests

Run:

```bash
./run_host_tests.sh
```

The host tests verify the sound registry and the non-blocking sequencing engine without WLED hardware.

## Hardware smoke test

1. Build WLED with this usermod enabled.
2. Open Config -> Usermods -> Buzzer.
3. Configure a free GPIO and save.
4. Select every built-in sound and use Play / Stop.
5. Confirm `/buzzer?play=victory` works from a browser.
6. Confirm `/buzzer?play=alarm&loop=1` repeats until `/buzzer?stop=1`.
7. For a passive buzzer, verify pitch changes between notes.
8. For an active buzzer, verify the same entries reproduce distinct timing patterns.
9. Verify `triple_beep` produces three 90 ms low-pitch beeps with about 70 ms silence between pulses, matching the classic alarm-style cadence.
10. Compare `victory` and `fail` on passive hardware for the intended fanfare / cartoon-loss character.
11. Switch Type to Active and confirm both the Volume control and its passive-only description disappear.
12. Confirm API commands remain hidden until **Show API commands** is checked.
13. Test Trigger level = High and Low with both active and passive hardware as appropriate.
14. Check `/json/info` for timing and lateness diagnostics.
15. Check `/json/state` for current buzzer state.
16. Send JSON playback and stop commands through `/json/state`.

## Regression points inherited by design

- Playback must never use `delay()`.
- The 2 ms timer callback must never wait for the engine mutex.
- Small timer lateness must not accumulate from note to note.
- Stopping an already idle engine must not emit extra hardware transitions.
- Hardware reconfiguration must release the old GPIO and LEDC allocation before claiming the new configuration.

## b005 UI/audio checks

- With Type = Passive, verify Volume is a 0-100 slider with a live percentage readout.
- With Type = Active, verify the complete Volume slider row is hidden.
- Verify Triple Beep is the original high 2 kHz three-pulse trill.
- Compare Victory against the supplied reference; confirm the F-major contour is recognizable.
