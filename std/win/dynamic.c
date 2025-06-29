//

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

EXPORTED void win_init(var_t* ret, gstate_t* _g){
  int flags = ARG_POP(_g,i32);
  int h = ARG_POP(_g,i32);
  int w = ARG_POP(_g,i32);

  ret->u.u64 = win_impl_init(w,h,flags);
}

EXPORTED void win_poll(var_t* ret, gstate_t* _g){

  obj_t* o = gc_alloc_(_g, sizeof(obj_t));
  o->type = ret->type;
  o->data = calloc(24,1);
  ((obj_t**)(o->data))[0] = o;

  win_impl_poll(o->data);

  ret->u.obj = o;
  _g->flags |= GFLG_TRGC;
}

EXPORTED void win_exit(var_t* ret, gstate_t* _g){

  win_impl_exit();

}


#define QK_REG(name) register_cfunc(&(_g->cfuncs), "win." QUOTE(name), win_ ## name);

EXPORTED void lib_init_win(gstate_t* _g){
  QK_REG(init)
  QK_REG(poll)
  QK_REG(exit)
}




