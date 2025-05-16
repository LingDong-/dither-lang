//
#include <math.h>
#include "../../src/interp.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif


EXPORTED void math_sin(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = sinf(x);
  
  ret->u.f32 = y;
}


EXPORTED void math_cos(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = cosf(x);
  ret->u.f32 = y;
}


EXPORTED void math_random(var_t* ret,  gstate_t* _g){
  float y = (float)rand()/(float)RAND_MAX;
  
  ret->u.f32 = y;
}

EXPORTED void math_abs(var_t* ret, gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = fabs(x);
  

  ret->u.f32 = y;
  
}

EXPORTED void math_floor(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = floor(x);
  
  ret->u.f32 = y;
}

EXPORTED void math_min(var_t* ret,  gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  float z = fmin(x,y);
  ret->u.f32 = z;
}

EXPORTED void math_max(var_t* ret,  gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  float z = fmax(x,y);
  ret->u.f32 = z;
}

EXPORTED void math_atan2(var_t* ret,  gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  float z = atan2(x,y);
  ret->u.f32 = z;
}


EXPORTED void math_hypot(var_t* ret,  gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  float z = hypotf(x,y);
  ret->u.f32 = z;
}

EXPORTED void math_round(var_t* ret, gstate_t* _g){
  float x = ARG_POP(_g,f32);
  float y = roundf(x);
  ret->u.f32 = y;
}

EXPORTED void lib_init_math(gstate_t* _g){

  register_cfunc(&(_g->cfuncs), "math.sin", math_sin);
  register_cfunc(&(_g->cfuncs), "math.cos", math_cos);
  register_cfunc(&(_g->cfuncs), "math.abs", math_abs);
  register_cfunc(&(_g->cfuncs), "math.floor", math_floor);
  register_cfunc(&(_g->cfuncs), "math.round", math_round);
  register_cfunc(&(_g->cfuncs), "math.random", math_random);
  register_cfunc(&(_g->cfuncs), "math.min", math_min);
  register_cfunc(&(_g->cfuncs), "math.max", math_max);
  register_cfunc(&(_g->cfuncs), "math.atan2", math_atan2);
  register_cfunc(&(_g->cfuncs), "math.hypot", math_hypot);

}
