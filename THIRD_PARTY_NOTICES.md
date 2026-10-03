# Third-party notices

The optional ES8311 codec support in `audio/ES8311Codec.*` is adapted from the
Waveshare `ESP32-S3-RGB-Matrix` example sources supplied for the board. Those
sources are distributed by Waveshare under the Apache License 2.0. A copy of
that license is included as `audio/LICENSE-Waveshare-Apache-2.0.txt`.

Reference source package:
`ESP32-S3-RGB-Matrix-main`, example `arduino_v3.3.7/09_Music_Player`.

The rest of WLED Buzzer Usermod remains under the repository's EUPL-1.2
license unless a source file states otherwise.

The 22.05 kHz ES8311 clock coefficient used by the shared-I2S mode is derived
from Espressif's Apache-2.0 licensed ES8311 codec driver (ESP-ADF /
esp_codec_dev). It is used only to configure the ES8311 DAC to the same clock
family already produced by WLED AudioReactive on the Waveshare shared bus.


## Bundled audio samples

The embedded PCM samples in `audio/BuzzerAudioSamples.cpp` and
`audio/BuzzerAudioSamplesExtra.cpp` were generated from maintainer-supplied
reference audio supplied for the bundled sample set. This repository does not assert or grant
redistribution rights for the underlying audio content. Redistribution of the bundled PCM requires the maintainer to hold appropriate
rights for the underlying audio content. Any asset whose redistribution terms
are unclear should be replaced with an original or appropriately licensed
alternative before publishing the package.

The v0.2.0 audio additions were converted to 16 kHz mono / signed 16-bit PCM from
these maintainer-supplied source files:

- `victory.mp3` - SHA-256 `9a4b6c9d839b37fd40e5e3d586841c447cfb2c9bc714356d4cadeeadc910a5f8`
- `imperial_march.wav` - SHA-256 `8facfaa37aa24e0f604163ec824516b884b3fe11a0afddad3d325e626fc603a1`
- `trumpet.mp3` - SHA-256 `a9b4931031fcc72896168288450be242f96332add6a0558f839c92e1f1028fd9`
- `wakeup.mp3` - SHA-256 `8cdbc6fd3ec4478f2c470ace02a72ebe35e9a944117b791793cca55b87563e5b`

For v0.2.0, the converted `victory` PCM payload is additionally attenuated to 35% amplitude (about -9.1 dB) for level matching; this does not alter the source-file hash above.

The source media files themselves are not included in the release archive; only
the converted embedded PCM payload is present.
