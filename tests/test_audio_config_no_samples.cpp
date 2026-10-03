#define WLED_BUZZER_AUDIO_WAVESHARE_S3_MATRIX
#define WLED_BUZZER_DISABLE_AUDIO_SAMPLES
#include "../audio/BuzzerAudioConfig.h"

int main() {
#ifndef WLED_BUZZER_ENABLE_AUDIO
#error "Waveshare profile must still enable audio"
#endif
#ifdef WLED_BUZZER_AUDIO_SAMPLES
#error "Disable flag must exclude bundled samples"
#endif
  return 0;
}
