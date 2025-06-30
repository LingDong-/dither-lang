//

#include <math.h>
#include "../../src/interp.c"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x


EXPORTED void c_v_mag(var_t* ret,  gstate_t* _g){
  vec_t* vec = ARG_POP(_g,vec);

  float s = 0;
  for (int i = 0; i < vec->n; i++){
    float x = ((float*)(vec->data))[i];
    s += x*x;
  }
  s = sqrt(s);

  ret->u.f32 = s;
}

EXPORTED void c_v_dir(var_t* ret,  gstate_t* _g){
  vec_t* vec = ARG_POP(_g,vec);
  vec_t* uec = vec_copy_(_g,vec);

  float s = 0;
  for (int i = 0; i < vec->n; i++){
    float x = ((float*)(vec->data))[i];
    s += x*x;
  }
  if (s){
    s = 1.0/sqrt(s);
    for (int i = 0; i < vec->n; i++){
      ((float*)(uec->data))[i] *= s;
    }
  }
  ret->u.vec = uec;
}


#define QK_REG(name) register_cfunc(&(_g->cfuncs), "vec." QUOTE(name), c_v_ ## name);


EXPORTED void lib_init_vec(gstate_t* _g){
  QK_REG(mag)
  QK_REG(dir)
}


