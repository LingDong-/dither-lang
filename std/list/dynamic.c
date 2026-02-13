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

EXPORTED void list_slice(var_t* ret,  gstate_t* _g){
  int j = ARG_POP(_g,i32);
  int i = ARG_POP(_g,i32);
  lst_t* lst = ARG_POP(_g,lst);

  if (j<0) j+=lst->n;
  if (i<0) i+=lst->n;

  int n = j-i;
  ret->u.lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  ret->u.lst->data = malloc(n*lst->w);
  ret->u.lst->n = n;
  ret->u.lst->w = lst->w;
  ret->u.lst->type = lst->type;
  ret->u.lst->cap = n;

  type_t* t = (type_t*)(lst->type->u.elem.head->data);
  if (t->vart == VART_VEC){
    for (int k = 0; k < n; k++){
      ((vec_t**)(ret->u.lst->data))[k] = vec_copy_(_g, ((vec_t**)(lst->data))[i+k]);
    }
  }else if (t->vart == VART_TUP){
    for (int k = 0; k < n; k++){
      ((tup_t**)(ret->u.lst->data))[k] = tup_copy_(_g, ((tup_t**)(lst->data))[i+k]);
    }
  }else{
    memcpy(ret->u.lst->data, lst->data + (i*lst->w), n*lst->w);
  }
}


EXPORTED void list_make(var_t* ret,  gstate_t* _g){
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
    }else if (ta->vart == VART_TUP){
      ptr = tup_copy_(_g,u->u.tup);
    }else{
      ptr = u->u.obj;
    }
    memcpy(ret->u.lst->data + (i*arr->w), &ptr, arr->w);
  }

  free(u);
}


EXPORTED void list_insert(var_t* ret,  gstate_t* _g){

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


EXPORTED void list_erase(var_t* ret,  gstate_t* _g){
  int j = ARG_POP(_g,i32);
  int i = ARG_POP(_g,i32);
  lst_t* a = ARG_POP(_g,lst);

  memmove(a->data + ((i)*a->w), a->data + ((j)*a->w), (a->n-j)*a->w);

  a->n -= (j-i);


  ret->u.i32 = a->n;
}


EXPORTED void list_length(var_t* ret,  gstate_t* _g){

  lst_t* a = ARG_POP(_g,lst);

  ret->u.i32 = a->n;
}

int _list_cmp(const void *a, const void *b){
  float c = (*(float *)a - *(float *)b);
  if (c < 0) return -1;
  if (c > 0) return 1;
  return 0;
}

EXPORTED void list__sort(var_t* ret,  gstate_t* _g){
  lst_t* b = ARG_POP(_g,lst);
  lst_t* a = ARG_POP(_g,lst);
  char* tmp = malloc(a->n*(a->w+4));
  for (int i = 0; i < a->n; i++){
    ((float*)(tmp + (i*(a->w+4)))) [0] = ((float*)b->data)[i];
    memcpy(tmp + (i*(a->w+4)+4), a->data + (i*a->w), a->w);
  }
  qsort(tmp, a->n, a->w+4, _list_cmp);
  for (int i = 0; i < a->n; i++){
    memcpy(a->data + (i*a->w), tmp + (i*(a->w+4)+4), a->w);
  }
  free(tmp);
}


EXPORTED void lib_init_list(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "list.slice", list_slice);
  register_cfunc(&(_g->cfuncs), "list.insert", list_insert);
  register_cfunc(&(_g->cfuncs), "list.erase", list_erase);
  register_cfunc(&(_g->cfuncs), "list.length", list_length);
  register_cfunc(&(_g->cfuncs), "list.make", list_make);
  register_cfunc(&(_g->cfuncs), "list._sort", list__sort);

  
}


