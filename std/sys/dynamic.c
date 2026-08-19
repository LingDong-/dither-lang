//

#include <stdio.h>
#include "../../src/interp.c"
#include "impl.h"


#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

EXPORTED void sys_gc_on(var_t* ret, gstate_t* _g){
  _g->flags &= ~GFLG_NOGC;
}

EXPORTED void sys_gc_off(var_t* ret, gstate_t* _g){
  _g->flags |= GFLG_NOGC;
}

EXPORTED void sys_gc(var_t* ret, gstate_t* _g){
  _g->flags |= GFLG_TRGC;
}

EXPORTED void sys_argv(var_t* ret, gstate_t* _g){
    
  int sn = _g->argc;
  stn_t** ss = malloc(sn*sizeof(stn_t*));

  for (int i = 0; i < sn; i++){
    int n = strlen(_g->argv[i]);
    stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+n+1);
    s->n = n;
    s->w = 1;
    s->type = (type_t*)(ret->type->u.elem.head->data);
    memcpy(s->data, _g->argv[i], n);
    ss[i] = s;
  }
  ret->u.lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  ret->u.lst->data = (char*)ss;
  ret->u.lst->n = sn;
  ret->u.lst->w = 8;
  ret->u.lst->type = ret->type;
  ret->u.lst->cap = sn; 

}

EXPORTED void sys_platform(var_t* ret, gstate_t* _g){
  char p[128];
  strcpy(p,"vm@");
  impl_platform(p+strlen(p));

  stn_t* s0 = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+strlen(p)+1);
  s0->n = strlen(p);
  s0->w = 1;
  s0->type = ret->type;
  strcpy(s0->data, p);

  ret->u.str = s0;
}

EXPORTED void sys_getenv(var_t* ret, gstate_t* _g){
  stn_t* s = ARG_POP(_g,str);

  char* p = getenv(s->data);
  if (p == NULL) p = "";

  stn_t* s0 = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+strlen(p)+1);
  s0->n = strlen(p);
  s0->w = 1;
  s0->type = ret->type;
  strcpy(s0->data, p);

  ret->u.str = s0;
}



EXPORTED void lib_init_sys(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "sys.gc_on",   sys_gc_on );
  register_cfunc(&(_g->cfuncs), "sys.gc_off",  sys_gc_off);
  register_cfunc(&(_g->cfuncs), "sys.gc",      sys_gc    );
  register_cfunc(&(_g->cfuncs), "sys.argv",    sys_argv  );
  register_cfunc(&(_g->cfuncs), "sys.platform",sys_platform  );
  register_cfunc(&(_g->cfuncs), "sys.getenv",  sys_getenv  );
}


