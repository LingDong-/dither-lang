//

#include <stdio.h>
#include "../../src/interp.c"

#include "impl.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

EXPORTED void io_println(var_t* ret, gstate_t* _g){
  stn_t* s = ARG_POP(_g,str);
  io_impl_print(s->data);
  io_impl_print("\n");
}

EXPORTED void io_print(var_t* ret, gstate_t* _g){
  stn_t* s = ARG_POP(_g,str);
  io_impl_print(s->data);
}

EXPORTED void io_write_file(var_t* ret, gstate_t* _g){
  lst_t* l = ARG_POP(_g,lst);
  stn_t* s = ARG_POP(_g,str);
  io_impl_write_file(s->data, l->data, l->n);
}

EXPORTED void lib_init_io(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "io.println", io_println);
  register_cfunc(&(_g->cfuncs), "io.print", io_print);
  register_cfunc(&(_g->cfuncs), "io.write_file", io_write_file);
}


