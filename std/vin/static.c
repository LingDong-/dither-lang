//
#include <stdio.h>

#include "impl.c"

void vin__create(){
  __push_stack();
  char* __ARG(pth);
  int32_t __ARG(flag);

  int w,h;
  int id = vin_impl_create(flag,pth,&w,&h);
  void* o = __gc_alloc(VART_STT,24);
  __put_var(0,o);
  void* oo = (char*)o+4;
  ((void**)(oo))[0] = o;
  ((int32_t*)(oo))[2] = id;
  ((int32_t*)(oo))[3] = w;
  ((int32_t*)(oo))[4] = h;
  __put_ret(&(o));
  __pop_stack();
}

void vin___read_pixels(){
  int __ARG(fbo);

  __arr_t* a = __gc_alloc(VART_ARR, sizeof(__arr_t)+12);
  a->ndim = 3;
  a->data = vin_impl__read_pixels(fbo,&(a->dims[1]),&(a->dims[0]));
  a->dims[2] = 4;
  a->n = a->dims[0]*a->dims[1]*a->dims[2];
  a->w = 1;
  a->t = VART_U08;

  __put_ret(&a);
}