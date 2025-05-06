#define MINIAUDIO_IMPLEMENTATION
#include "../../third_party/miniaudio.h"

int chan = 2;
int rate = 44100;

#define BUF_SIZE 2048
float buffer[BUF_SIZE];
int64_t curi = 0;
int64_t curo = 0;

ma_device device;

static void renderCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
  float* out = (float*)output;
  for (ma_uint32 i = 0; i < frameCount; ++i) {
    for (int j = 0; j < chan; j++){
      if (curo < curi){
        out[i*chan+j] = buffer[curo % BUF_SIZE];
        curo++;
      }else{
        out[i*chan+j] = 0.0;
      }
    }
  }
  (void)input;
}


void snd_impl_init(int _rate, int _chan){
  rate = _rate;
  chan = _chan;

  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.format   = ma_format_f32;
  config.playback.channels = chan;
  config.sampleRate        = rate;
  config.dataCallback      = renderCallback;
  ma_device_init(NULL, &config, &device);
  ma_device_start(&device);
}

void snd_impl_exit(){
  ma_device_uninit(&device);
}

int snd_impl_buffer_full(){
  int64_t n = (int64_t)BUF_SIZE - (curi-curo);
  return (n <= 0);
}

void snd_impl_put_sample(float x){
  int64_t n = (int64_t)BUF_SIZE - (curi-curo);
  if (n > 0){
    buffer[curi%BUF_SIZE] = x;
    curi++;
  }
}


