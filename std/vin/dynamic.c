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

EXPORTED void vin_create(var_t* ret, gstate_t* _g){
  stn_t* pth = ARG_POP(_g,str);
  int flag = ARG_POP(_g,i32);
  
  int w,h;
  int id = vin_impl_create(flag,pth->data,&w,&h);

  obj_t* o = gc_alloc_(_g, sizeof(obj_t));
  o->type = ret->type;
  o->data = calloc(20,1);
  ((obj_t**)(o->data))[0] = o;

  ((int32_t*)(o->data))[2] = id;
  ((int32_t*)(o->data))[3] = w;
  ((int32_t*)(o->data))[4] = h;

  ret->u.obj = o;
}

EXPORTED void vin__read_pixels(var_t* ret, gstate_t* _g){
  int fbo = ARG_POP(_g,i32);

  arr_t* arr = (arr_t*)gc_alloc_(_g,sizeof(arr_t)+12);
  arr->data = (char*)vin_impl__read_pixels(fbo, &(arr->dims[1]), &(arr->dims[0]));
  arr->dims[2] = 4;
  arr->ndim = 3;
  arr->w = 1;
  arr->type = ret->type;
  arr->n = arr->dims[0]*arr->dims[1]*arr->dims[2];

  ret->u.arr = arr;
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "vin." QUOTE(name), vin_ ## name);


EXPORTED void lib_init_vin(gstate_t* _g){
  QK_REG(create)
  QK_REG(_read_pixels)
}


