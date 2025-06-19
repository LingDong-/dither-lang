//

#include <stdint.h>

#include "impl.c"

void rand__random(){
  float x = rand_impl_random();
  __put_ret(&x);
}

void rand__seed(){
  uint32_t __ARG(x);
  rand_impl_seed(x);
}

void rand__noise_detail(){
  float __ARG(falloff);
  int32_t __ARG(lod);
  rand_impl_noise_detail(lod,falloff);
}

void rand__noise_reseed(){
  rand_impl_noise_reseed();
}

void rand__noise(){
  float __ARG(z);
  float __ARG(y);
  float __ARG(x);
  float r = rand_impl_noise(x,y,z);
  __put_ret(&r);
}