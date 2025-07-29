//

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

EXPORTED void str_length(var_t* ret,  gstate_t* _g){
  stn_t* a = ARG_POP(_g,str);
  ret->u.i32 = a->n;
}

EXPORTED void str_chr(var_t* ret,  gstate_t* _g){
  uint32_t a = ARG_POP(_g,u32);

  stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+1+1);
  s->n = 1;
  s->w = 1;
  s->type = ret->type;
  s->data[0] = a;
  ret->u.str = s;
}

EXPORTED void str_decode(var_t* ret,  gstate_t* _g){
  stn_t* e = ARG_POP(_g,str);
  lst_t* a = ARG_POP(_g,lst);

  stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+a->n+1);
  s->n = a->n;
  s->w = 1;
  s->type = ret->type;
  memcpy(s->data, a->data, a->n);
  ret->u.str = s;
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "str." QUOTE(name), str_ ## name);

EXPORTED void lib_init_str(gstate_t* _g){
  QK_REG(length);
  QK_REG(chr);
  QK_REG(decode);
}


