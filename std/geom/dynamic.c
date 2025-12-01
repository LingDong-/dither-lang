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


#define QK_REG(name) register_cfunc(&(_g->cfuncs), "geom." QUOTE(name), geom_ ## name);

EXPORTED void lib_init_geom(gstate_t* _g){


}
