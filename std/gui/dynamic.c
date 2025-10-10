//

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

EXPORTED void gui_init(var_t* ret, gstate_t* _g){
  gui_impl_init();
}

EXPORTED void gui_slider(var_t* ret, gstate_t* _g){
  var_t* r = ARG_POP_VAR_NO_FREE(_g);
  var_t* l = ARG_POP_VAR_NO_FREE(_g);
  var_t* x = ARG_POP_VAR_NO_FREE(_g);
  stn_t* s = ARG_POP(_g,str);

  if (x->type->vart == VART_F32){
    gui_impl__slider1f(s->data,x->u.f32,l->u.f32,r->u.f32);
  }
}

EXPORTED void gui_get(var_t* ret, gstate_t* _g){
  stn_t* s = ARG_POP(_g,str);
  if (ret->type->vart == VART_F32){
    ret->u.f32 = gui_impl__get1f(s->data);
  }
}

EXPORTED void gui_poll(var_t* ret, gstate_t* _g){
  gui_impl_poll();
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "gui." QUOTE(name), gui_ ## name);


EXPORTED void lib_init_gui(gstate_t* _g){
  QK_REG(init)
  QK_REG(slider)
  QK_REG(get)
  QK_REG(poll)
}


