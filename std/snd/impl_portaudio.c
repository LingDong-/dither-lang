//CFLAGS+=$(echo "-lportaudio")

#include <math.h>
#include <stdio.h>

#include <portaudio.h>

int chan = 2;
int rate = 44100;

#define BUF_SIZE 2048
float buffer[BUF_SIZE];
int64_t curi = 0;
int64_t curo = 0;

PaStream* stream;

static int renderCallback(const void* input,
                          void* output,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData) {
  float* out = (float*)output;
  for (unsigned int i = 0; i < frameCount; ++i) {
    for (int j = 0; j < chan; j++){
      if (curo < curi){
        out[i*chan+j] = buffer[curo % BUF_SIZE];
        curo++;
      }else{
        out[i*chan+j] = 0.0;
      }
    }
  }
  return paContinue;
}


void snd_impl_init(int _rate, int _chan){
  rate = _rate;
  chan = _chan;
  Pa_Initialize();
  Pa_OpenDefaultStream(&stream,0,chan,paFloat32,rate,512,renderCallback,NULL);
  Pa_StartStream(stream);
}

void snd_impl_exit(){
  Pa_StopStream(stream);
  Pa_CloseStream(stream);
  Pa_Terminate();
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


