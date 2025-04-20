//
#include <math.h>
#include "../../src/interp.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif


EXPORTED void c_sin(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = sinf(x);
  
  ret->u.f32 = y;
}


EXPORTED void c_cos(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = cosf(x);
  ret->u.f32 = y;
}


EXPORTED void c_random(var_t* ret,  gstate_t* _g){
  float y = (float)rand()/(float)RAND_MAX;
  
  ret->u.f32 = y;
}

EXPORTED void c_abs(var_t* ret, gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = fabs(x);
  

  ret->u.f32 = y;
  
}

EXPORTED void c_floor(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = floor(x);
  
  ret->u.f32 = y;
}

EXPORTED void c_min(var_t* ret,  gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  float z = fmin(x,y);
  ret->u.f32 = z;
}

EXPORTED void c_max(var_t* ret,  gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  float z = fmax(x,y);
  ret->u.f32 = z;
}

EXPORTED void c_atan2(var_t* ret,  gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  float z = atan2(x,y);
  ret->u.f32 = z;
}


EXPORTED void c_hypot(var_t* ret,  gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  float z = hypotf(x,y);
  ret->u.f32 = z;
}

EXPORTED void lib_init_math(gstate_t* _g){

  register_cfunc(&(_g->cfuncs), "math.sin", c_sin);
  register_cfunc(&(_g->cfuncs), "math.cos", c_cos);
  register_cfunc(&(_g->cfuncs), "math.abs", c_abs);
  register_cfunc(&(_g->cfuncs), "math.floor", c_floor);
  register_cfunc(&(_g->cfuncs), "math.random", c_random);
  register_cfunc(&(_g->cfuncs), "math.min", c_min);
  register_cfunc(&(_g->cfuncs), "math.max", c_max);
  register_cfunc(&(_g->cfuncs), "math.atan2", c_atan2);
  register_cfunc(&(_g->cfuncs), "math.hypot", c_hypot);

}
