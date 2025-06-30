//

// #define DBG 1
#include "../../src/interp.c"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

EXPORTED void c_slice(var_t* ret,  gstate_t* _g){
  int j = ARG_POP(_g,i32);
  int i = ARG_POP(_g,i32);
  lst_t* lst = ARG_POP(_g,lst);

  int n = j-i;
  ret->u.lst->data = malloc(n*lst->w);
  ret->u.lst->n = n;
  ret->u.lst->w = lst->w;
  ret->u.lst->type = lst->type;
  ret->u.lst->cap = n;

  memcpy(ret->u.lst->data, lst->data + (i*lst->w), n*lst->w);

}


EXPORTED void c_make(var_t* ret,  gstate_t* _g){
  var_t* u = ARG_POP_VAR_NO_FREE(_g);

  int n = ARG_POP(_g,i32);
  if (n < 0) n = 0;

  type_t* ta = u->type;

  lst_t* arr = (lst_t*)gc_alloc_(_g,sizeof(lst_t));

  arr->n = n;
  arr->cap = (n+1);
  arr->w = type_size(ta);
  arr->type = ret->type;
  arr->data = calloc(arr->w,arr->cap);
  ret->u.lst = arr;

  for (int i = 0; i < n; i++){
    void* ptr;
    if (ta->vart == VART_VEC){
      ptr = vec_copy_(_g,u->u.vec);
    }else if (ta->vart == VART_STR){
      ptr = stn_copy_(_g,u->u.str);
    }else{
      ptr = u->u.obj;
    }
    memcpy(ret->u.lst->data + (i*arr->w), &ptr, arr->w);
  }

  free(u);
}


EXPORTED void c_insert(var_t* ret,  gstate_t* _g){

  var_t* u = ARG_POP_VAR_NO_FREE(_g);

  int i = ARG_POP(_g,i32);
  lst_t* a = ARG_POP(_g,lst);

  if (a->n + 1 > a->cap){
    a->cap = (a->cap+1)*2;
    a->data = realloc(a->data,(a->cap)*(a->w));
  }
  memmove(a->data + ((i+1)*a->w), a->data + (i*a->w), (a->n-i)*a->w);

  char* x = (char*)(void*)(&(u->u));
  for (int j = 0; j < a->w; j++){
    ((char*)(a->data))[i*a->w+j] = x[j];
  }
  a->n ++;

  ret->u.i32 = a->n;
  free(u);
}


EXPORTED void c_erase(var_t* ret,  gstate_t* _g){
  int j = ARG_POP(_g,i32);
  int i = ARG_POP(_g,i32);
  lst_t* a = ARG_POP(_g,lst);

  memmove(a->data + ((i)*a->w), a->data + ((j)*a->w), (a->n-j)*a->w);

  a->n -= (j-i);


  ret->u.i32 = a->n;
}


EXPORTED void c_length(var_t* ret,  gstate_t* _g){

  lst_t* a = ARG_POP(_g,lst);

  ret->u.i32 = a->n;

}


EXPORTED void lib_init_list(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "list.slice", c_slice);
  register_cfunc(&(_g->cfuncs), "list.insert", c_insert);
  register_cfunc(&(_g->cfuncs), "list.erase", c_erase);
  register_cfunc(&(_g->cfuncs), "list.length", c_length);
  register_cfunc(&(_g->cfuncs), "list.make", c_make);

  
}


