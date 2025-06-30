//

// #define DBG 1
#include "../../src/interp.c"

#ifdef _WIN32
#include "impl_winapi.c"
#else
#include "impl_posix.c"
#endif

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

EXPORTED void time_fps(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);
  ret->u.f32 = time_impl_fps(x);
}


EXPORTED void time_millis(var_t* ret,  gstate_t* _g){
  ret->u.f64 = time_impl_millis();
}

EXPORTED void time_delay(var_t* ret,  gstate_t* _g){
  float x = ARG_POP(_g,f32);
  time_impl_delay(x);
}

EXPORTED void time_stamp(var_t* ret,  gstate_t* _g){
  ret->u.f64 = time_impl_stamp();
}

EXPORTED void time_local(var_t* ret,  gstate_t* _g){
  double x = ARG_POP(_g,f64);
  tup_t* y = gc_alloc_(_g,sizeof(tup_t)+24);
  y->type = ret->type;
  time_impl_local(x,
    ((int32_t*)y->data)+0,
    ((int32_t*)y->data)+1,
    ((int32_t*)y->data)+2,
    ((int32_t*)y->data)+3,
    ((int32_t*)y->data)+4,
    ((int32_t*)y->data)+5
  );
  ret->u.tup = y;
}

EXPORTED void lib_init_time(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "time.fps", time_fps);
  register_cfunc(&(_g->cfuncs), "time.millis", time_millis);
  register_cfunc(&(_g->cfuncs), "time.delay", time_delay);
  register_cfunc(&(_g->cfuncs), "time.stamp", time_stamp);
  register_cfunc(&(_g->cfuncs), "time.local", time_local);
}


