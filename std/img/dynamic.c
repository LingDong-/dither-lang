//

#include <stdio.h>
#include "../../src/interp.c"

#include "impl.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x

EXPORTED void img_decode(var_t* ret, gstate_t* _g){
  lst_t* l = ARG_POP(_g,lst);

  int w,h,chan;
  uint8_t* data = img_impl_decode((uint8_t*)(l->data), l->n, &w, &h, &chan);

  arr_t* arr = (arr_t*)gc_alloc_(_g,sizeof(arr_t)+12);

  arr->ndim = 3;
  arr->w = 1;
  arr->type = ret->type;
  arr->n = w*h*chan;
  arr->dims[0] = h;
  arr->dims[1] = w;
  arr->dims[2] = chan;

  arr->data = (char*)data;
  ret->u.arr = arr;
}

EXPORTED void img_encode(var_t* ret, gstate_t* _g){
  arr_t* a = ARG_POP(_g,arr);
  stn_t* s = ARG_POP(_g,str);

  int n;
  void* data = img_impl_encode(s->data, (uint8_t*)(a->data), a->dims[1], a->dims[0], a->dims[2], &n);

  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));

  lst->n = n;
  lst->cap = n;
  lst->w = 1;
  lst->type = ret->type;
  lst->data = data;
  ret->u.lst = lst;
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "img." QUOTE(name), img_ ## name);


EXPORTED void lib_init_img(gstate_t* _g){
  QK_REG(decode)
  QK_REG(encode)
}


