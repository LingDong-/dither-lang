#include "../../src/interp.c"
#include "impl_cppstd.h"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

#define LEADING  1
#define TRAILING 2

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x


EXPORTED void regx_test(var_t* ret, gstate_t* _g){
  stn_t* re = ARG_POP(_g,str);
  stn_t* s = ARG_POP(_g,str);
  
  ret->u.i32 = regx_impl_test(s->data,re->data);
}

EXPORTED void regx_find(var_t* ret, gstate_t* _g){
  stn_t* re = ARG_POP(_g,str);
  stn_t* str = ARG_POP(_g,str);

  char*** groups;
  int* indices;
  int cnt = regx_impl_find(str->data,re->data,&groups,&indices);

  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  lst->n = cnt;
  lst->cap = lst->n;
  lst->w = 8;
  lst->type = ret->type;
  lst->data = malloc(lst->w*lst->cap);
  ret->u.lst = lst;


  for (int i = 0; i < cnt; i++){
    int n = 0;
    char** g = groups[i]+0;
    while (*(g++)){
      n++;
    }
    
    lst_t* l = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
    l->n = n;
    l->cap = n;
    l->w = sizeof(stn_t**);
    l->data = malloc(sizeof(stn_t**)*n);

    tup_t* tup = gc_alloc_(_g,sizeof(tup_t)+12);
    tup->type = (type_t*)(ret->type->u.elem.head->data);
    l->type = (type_t*)(tup->type->u.elem.tail->data);
    ((int32_t*)(tup->data))[0] = indices[i];
    ((lst_t**)(((char*)(tup->data))+4))[0] = l;
    ((tup_t**)(lst->data))[i] = tup;

    for (int j = 0; j < n; j++){
      int m = strlen(groups[i][j]);
      stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+m+1);
      strcpy(s->data, groups[i][j]);
      s->data[m] = 0;
      s->n = n;
      s->w = 1;
      s->type = (type_t*)(l->type->u.elem.head->data);
      ((stn_t**)(l->data))[j] = s;
      free(groups[i][j]);
    }
    free(groups[i]);
  }
  free(groups);
  free(indices);
}

EXPORTED void regx_replace(var_t* ret, gstate_t* _g){
  stn_t* rep = ARG_POP(_g,str);
  stn_t* re = ARG_POP(_g,str);
  stn_t* s = ARG_POP(_g,str);
  
  char* r = regx_impl_replace(s->data,re->data,rep->data);

  int n = strlen(r);
  stn_t* o = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+n+1);
  strcpy(o->data, r);

  o->data[n] = 0;
  o->n = n;
  o->w = 1;
  o->type = ret->type;

  free(r);
  ret->u.str = o;
}


#define QK_REG(name) register_cfunc(&(_g->cfuncs), "regx." QUOTE(name), regx_ ## name);

EXPORTED void lib_init_regx(gstate_t* _g){
  QK_REG(test);
  QK_REG(find);
  QK_REG(replace);
}

