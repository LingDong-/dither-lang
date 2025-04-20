//CFLAGS+=$(echo "-lportaudio")

#include "../../src/interp.c"
#include <math.h>
#include <stdio.h>


#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "snd." QUOTE(name), snd_ ## name);

#include "impl_portaudio.c"

EXPORTED void snd_init(var_t* ret,  gstate_t* _g){
  int chan = ARG_POP(_g,i32);
  int rate = ARG_POP(_g,i32);
  snd_impl_init(rate,chan);
}

EXPORTED void snd_exit(var_t* ret,  gstate_t* _g){
  snd_impl_exit();
}

EXPORTED void snd_buffer_full(var_t* ret,  gstate_t* _g){

  ret->u.i32 = snd_impl_buffer_full();
}

EXPORTED void snd_put_sample(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);

  snd_impl_put_sample(x);
}

EXPORTED void lib_init_snd(gstate_t* _g){
  QK_REG(init)
  QK_REG(exit)
  QK_REG(put_sample)
  QK_REG(buffer_full)
}

