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
  }else if (x->type->vart == VART_I32){
    gui_impl__slider1i(s->data,x->u.i32,l->u.i32,r->u.i32);
  }

  free(r);
  free(l);
  free(x);
}

EXPORTED void gui_toggle(var_t* ret, gstate_t* _g){
  var_t* x = ARG_POP_VAR_NO_FREE(_g);
  stn_t* s = ARG_POP(_g,str);

  gui_impl__toggle1i(s->data,x->u.i32);
  
  free(x);
}

EXPORTED void gui_field(var_t* ret, gstate_t* _g){
  var_t* x = ARG_POP_VAR_NO_FREE(_g);
  stn_t* s = ARG_POP(_g,str);

  gui_impl__field1s(s->data,x->u.str->data);

  free(x);
}

EXPORTED void gui_get(var_t* ret, gstate_t* _g){
  stn_t* s = ARG_POP(_g,str);
  if (ret->type->vart == VART_F32){
    ret->u.f32 = gui_impl__get1f(s->data);
  }else if (ret->type->vart == VART_I32){
    ret->u.i32 = gui_impl__get1i(s->data);
  }else if (ret->type->vart == VART_STR){
    char* x = gui_impl__get1s(s->data);
    int l = strlen(x);
    stn_t* str = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+l+1);
    str->n = l;
    str->w = 1;
    str->type = ret->type;
    strcpy(str->data, x);
    ret->u.str = str;
  }
}

EXPORTED void gui_poll(var_t* ret, gstate_t* _g){
  gui_impl_poll();
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "gui." QUOTE(name), gui_ ## name);


EXPORTED void lib_init_gui(gstate_t* _g){
  QK_REG(init)
  QK_REG(slider)
  QK_REG(toggle)
  QK_REG(field)
  QK_REG(get)
  QK_REG(poll)
}


