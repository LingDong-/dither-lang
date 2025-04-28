//

// #define DBG 1
#include "../../src/interp.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

EXPORTED void arr_reshape(var_t* ret,  gstate_t* _g){

  vec_t* vec = ARG_POP(_g,vec);
  arr_t* a = ARG_POP(_g,arr);
  memcpy(a->dims, vec->data, a->ndim*4);

  int n = 1;
  for (int i = 0; i < vec->n; i++){
    n *= ((int*)(vec->data))[i];
  }
  if (a->n != n){
    a->n = n;
    a->data = realloc(a->data,a->w*a->n);
  }
}


EXPORTED void arr_make(var_t* ret,  gstate_t* _g){
  var_t* u = ARG_POP_VAR_NO_FREE(_g);

  vec_t* vec = ARG_POP(_g,vec);

  type_t* ta = u->type;

  arr_t* arr = (arr_t*)gc_alloc_(_g,sizeof(arr_t)+vec->n*4);

  arr->ndim = vec->n;
  arr->w = type_size(ta);
  arr->type = ret->type;
  arr->n = 1;
  memcpy(arr->dims, vec->data, 4*arr->ndim);
  for (int i = 0; i < vec->n; i++){
    arr->n *= ((int*)(vec->data))[i];
  }
  arr->data = calloc(arr->w,arr->n);
  ret->u.arr = arr;

  for (int i = 0; i < arr->n; i++){
    void* ptr;
    if (ta->vart == VART_VEC){
      ptr = vec_copy_(_g,u->u.vec);
    }else if (ta->vart == VART_STR){
      ptr = stn_copy_(_g,u->u.str);
    }else{
      ptr = u->u.obj;
    }
    memcpy(ret->u.arr->data + (i*arr->w), &ptr, arr->w);
  }

  free(u);
}

EXPORTED void arr_shape(var_t* ret,  gstate_t* _g){

  arr_t* a = ARG_POP(_g,arr);

  vec_t* vec = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+a->ndim*4);
  vec->type = ret->type;
  vec->n = a->ndim;
  vec->w = 4;
  memcpy(vec->data, a->dims, a->ndim*4);

  ret->u.vec = vec;
}


EXPORTED void lib_init_arr(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "arr.reshape", arr_reshape);
  register_cfunc(&(_g->cfuncs), "arr.shape", arr_shape);
  register_cfunc(&(_g->cfuncs), "arr.make", arr_make);
}


