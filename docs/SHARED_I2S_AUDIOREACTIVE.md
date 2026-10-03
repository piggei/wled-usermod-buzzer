# Waveshare shared-I2S / AudioReactive coexistence

This note documents the hardware problem, the failed approaches, the qualified solution, and the rules that future changes must preserve.

## Hardware topology

On the Waveshare ESP32-S3 RGB Matrix profile, microphone input and speaker output share the physical I2S clocks:

- MCLK: GPIO12
- BCLK: GPIO43
- LRCK/WS: GPIO38
- microphone data (ES7210 -> ESP32-S3): GPIO39
- speaker data (ESP32-S3 -> ES8311): GPIO21
- ES8311 I2C: GPIO47/GPIO48, address 0x18
- speaker PA enable: GPIO11

WLED AudioReactive uses I2S0 as the microphone RX/master side. The Buzzer backend uses I2S1 for speaker TX.

## Failure that triggered the redesign

The first Audio backend used an independent I2S1 master at 48 kHz. On this board that meant a second peripheral attempted to program clock routing on pins already used by AudioReactive. The visible symptom was an AudioReactive input that was saturated or otherwise invalid even while the room was quiet.

A first shared-slave attempt removed Buzzer clock generation but still let the legacy I2S helper configure BCLK/LRCK. That could disturb the physical I2S0 master routes. A later GPIO-matrix-only attempt avoided that pad reconfiguration, but real hardware showed that I2S1 could remain unclocked: the Buzzer stayed `not ready` and repeated retries could disturb the microphone path.

## Hardware-qualified clock-routing solution

The qualified implementation uses a Buzzer-side shared-I2S mode:

1. AudioReactive remains the only physical clock master and keeps ownership of I2S0 RX.
2. Buzzer installs I2S1 as TX slave.
3. In shared mode, `i2s_set_pin()` configures only speaker DOUT GPIO21; it does not configure MCLK/BCLK/LRCK.
4. GPIO43 and GPIO38 keep their existing I2S0 output routes, but their input buffers are also enabled.
5. The ESP32-S3 GPIO matrix taps those pad levels into the I2S1 TX-slave BCLK/WS inputs.
6. Buzzer verifies that LRCK is physically toggling before installing/priming the shared TX stream.
7. Buzzer does not call `i2s_set_clk()` in shared mode and never installs, resets, stops or uninstalls I2S0.
8. In shared mode, the global I2C bus is not reinitialized; only the ES8311 device at 0x18 is configured.
9. Shared teardown disconnects only the internal I2S1 clock taps and does not modify the AudioReactive GPIO routing.

The tested shared stream follows the AudioReactive configuration used by the target WLED build: 22.05 kHz, normally 32-bit slots (16-bit when `I2S_USE_16BIT_SAMPLES` is compiled).

## Mode selection

The Buzzer checks WLED PinManager ownership of the four shared AudioReactive pins. If all are owned by AudioReactive, it requests shared slave-TX mode. If only some are owned, the bus is considered to be transitioning and Buzzer stays stopped rather than becoming a competing master. If none are owned, Buzzer may use its standalone 48 kHz / 16-bit I2S1 master mode.

Pin ownership is not treated as proof that clocks are running. Shared initialization additionally checks physical LRCK activity.

## Qualification evidence

On the tested Waveshare ESP32-S3 RGB Matrix / WLED 17.0.0-devV5 setup, dev-b012 verified the clock-routing/ownership part of the design with Buzzer configured as `Audio (I2S)` and AudioReactive enabled:

- cold boot completed without the previous permanent saturation/not-ready condition;
- AudioReactive remained stable at silence;
- Buzzer playback was operational;
- the RX path was not intentionally reconfigured by the Buzzer backend.

dev-b014 then closed the remaining real-time quality issue by increasing the TX producer block to 256 frames and the DMA depth to 8 descriptors without changing clock routing, task priority or affinity. On real hardware, speaker playback and AudioReactive bars became smooth; a captured shared-mode run exceeded 20,000 writes with `err=0`, `short=0`, `late=0`, `maxWrite` about 12.9 ms and `maxGap` about 13.0 ms against about 92.9 ms of DMA coverage. AudioReactive OFF -> ON -> OFF runtime transitions also passed. This qualifies both the shared-clock topology and the current buffering/scheduling behavior as the v0.2.0 stable baseline.

## Diagnostics

When shared mode is ready, `/json/info` reports the profile plus a line containing:

```text
shared-I2S | TX I2S1 slave | rate 22050 Hz | ... | clock tap GPIO-matrix + input-enable | RX I2S0 untouched
```

`/json/state` exposes `audioMode`, `audioRate` and `audioBits`.

The qualified runtime telemetry reports DMA geometry and producer timing in `/json/info`: write/error/short-write counters, maximum write duration, maximum producer gap, late-write count, observed core distribution and minimum free producer-task stack. A producer gap larger than the reported DMA coverage is treated as a late write. v0.2.0 keeps this runtime and telemetry unchanged.

Initialization errors are surfaced through the `Buzzer audio` entry, including `shared LRCK inactive` and shared DMA/clock priming failures.

## Guardrails for future work

Do not change the shared Waveshare path in ways that:

- call I2S control/uninstall APIs on I2S0;
- route Buzzer MCLK/BCLK/LRCK in shared mode;
- call `i2s_set_clk()` for shared I2S1;
- reinitialize the global I2C bus while AudioReactive is active;
- assume PinManager ownership means clocks are already present;
- fall back to standalone master while AudioReactive owns only part of the shared bus during a transition.

If the AudioReactive sample rate/slot format changes in a future WLED build, update or negotiate the shared TX format before declaring the backend ready. Do not treat successful clock sharing alone as proof of acceptable streaming quality; the TX starvation telemetry and perceptual coexistence test must also pass.
