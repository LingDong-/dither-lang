#include <stdio.h>

#include "../../src/interp.c"

#include "impl.c"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x

EXPORTED void rand_noise(var_t* ret,  gstate_t* _g){
  float z = ARG_POP(_g,f32);
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  ret->u.f32 = rand_impl_noise(x,y,z);
}
EXPORTED void rand_noise_detail(var_t* ret,  gstate_t* _g){
  float falloff = ARG_POP(_g,f32);
  int lod = ARG_POP(_g,i32);
  rand_impl_noise_detail(lod,falloff);
}
EXPORTED void rand_noise_reseed(var_t* ret,  gstate_t* _g){
  rand_impl_noise_reseed();
}
EXPORTED void rand_seed(var_t* ret,  gstate_t* _g){
  uint32_t x = ARG_POP(_g,u32);
  rand_impl_seed(x);
}
EXPORTED void rand_random(var_t* ret,  gstate_t* _g){
  ret->u.f32 = rand_impl_random();
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "rand." QUOTE(name), rand_ ## name);

EXPORTED void lib_init_rand(gstate_t* _g){
  QK_REG(random);
  QK_REG(seed);
  QK_REG(noise);
  QK_REG(noise_detail);
  QK_REG(noise_reseed);
}
