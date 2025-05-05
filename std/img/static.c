//
#include <stdio.h>

#include "impl.c"

void img__decode(){
  __list_t* l = NULL;
  __pop_arg(&l, 8);


  __arr_t* a = __gc_alloc(VART_ARR, sizeof(__arr_t)+12);
  a->ndim = 3;

  a->data = (char*)img_impl_decode((uint8_t*)(l->data),l->n,&(a->dims[1]),&(a->dims[0]),&(a->dims[2]));
  a->n = a->dims[0]*a->dims[1]*a->dims[2];
  a->w = 1;
  a->t = VART_U08;

  __put_ret(&a);
}

void img__encode(){
  __arr_t* a = NULL;
  __pop_arg(&a, 8);

  char* __ARG(s);

  __list_t* l = __gc_alloc(VART_LST, sizeof(__list_t));
  l->w = 1;
  l->t = VART_U08;
  int n;
  l->data = (char*)img_impl_encode(s,(uint8_t*)(a->data),a->dims[1],a->dims[0],a->dims[2],&n);
  l->n = n;
  l->cap = n;

  __put_ret(&l);
}