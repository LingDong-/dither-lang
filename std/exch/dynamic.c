//

// #define DBG 1
#include "../../src/interp.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

EXPORTED void exch__encode_json(var_t* ret,  gstate_t* _g){

}


EXPORTED void exch__decode_json(var_t* ret,  gstate_t* _g){

}


EXPORTED void lib_init_exch(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "exch._encode_json", exch__encode_json);
  register_cfunc(&(_g->cfuncs), "exch._decode_json", exch__decode_json);
}


