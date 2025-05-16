//

#include <stdio.h>
#include "../../src/interp.c"


#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

EXPORTED void guts_gc_on(var_t* ret, gstate_t* _g){
  _g->flags &= ~GFLG_NOGC;
}

EXPORTED void guts_gc_off(var_t* ret, gstate_t* _g){
  _g->flags |= GFLG_NOGC;
}

EXPORTED void guts_gc(var_t* ret, gstate_t* _g){
  _g->flags |= GFLG_TRGC;
}


EXPORTED void lib_init_guts(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "guts.gc_on", guts_gc_on );
  register_cfunc(&(_g->cfuncs), "guts.gc_off",guts_gc_off);
  register_cfunc(&(_g->cfuncs), "guts.gc",    guts_gc    );
}


