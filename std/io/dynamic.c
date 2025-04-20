//

#include <stdio.h>
#include "../../src/interp.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

EXPORTED void io_println(var_t* ret, gstate_t* _g){
  stn_t* s = ARG_POP(_g,str);

  printf("%s\n",s->data);
  
}

EXPORTED void io_print(var_t* ret, gstate_t* _g){
  stn_t* s = ARG_POP(_g,str);

  printf("%s",s->data);

}


EXPORTED void io_write_file(var_t* ret, gstate_t* _g){
  lst_t* l = ARG_POP(_g,lst);
  stn_t* s = ARG_POP(_g,str);

  FILE* fd = fopen(s->data, "w");
  fwrite(l->data,1,l->n,fd);
  fclose(fd);

}


EXPORTED void lib_init_io(gstate_t* _g){

  register_cfunc(&(_g->cfuncs), "io.println", io_println);
  register_cfunc(&(_g->cfuncs), "io.print", io_print);
  register_cfunc(&(_g->cfuncs), "io.write_file", io_write_file);
  

}


