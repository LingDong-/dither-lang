//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework CoreAudio -framework AudioToolbox -framework CoreServices -framework Foundation" || echo "-lportaudio")


#include <stdint.h>

#include "impl.c"

void snd__init(){
  int32_t __ARG(chan);
  int32_t __ARG(rate);

  snd_impl_init(rate,chan);
}

void snd__buffer_full(){
  int32_t x = snd_impl_buffer_full();
  __put_ret(&x);
}

void snd__put_sample(){
  float __ARG(x);
  snd_impl_put_sample(x);
}

void snd__exit(){
  snd_impl_exit();
}