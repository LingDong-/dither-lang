//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL -framework CoreGraphics" || echo "-lGLEW -lGL")

#include "../../src/interp.c"
#include "impl.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x

EXPORTED void frag__size(var_t* ret, gstate_t* _g){
  uint64_t ctx = ARG_POP(_g,u64);
  int h = ARG_POP(_g,i32);
  int w = ARG_POP(_g,i32);

  frag_impl__size(w,h,ctx);
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "frag." QUOTE(name), frag_ ## name);

EXPORTED void lib_init_gx(gstate_t* _g){
  QK_REG(_size)
}



