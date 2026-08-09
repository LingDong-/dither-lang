//

#include "impl_cppstd.h"

void regx__test(){
  char* __ARG(re);
  char* __ARG(s);
  int b = regx_impl_test(s,re);
  __put_ret(&b);
}

void regx__find(){
  char* __ARG(re);
  char* __ARG(str);

  char*** groups;
  int* indices;
  int cnt = regx_impl_find(str,re,&groups,&indices);

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->w = sizeof(void*);
  lst->t = VART_TUP;
  lst->data = (char*)malloc(sizeof(void*)*cnt);
  lst->cap = cnt;
  lst->n = cnt;

  for (int i = 0; i < cnt; i++){
    int n = 0;
    char** g = groups[i]+0;
    while (*(g++)){
      n++;
    }
    
    __list_t* l = __gc_alloc(VART_LST, sizeof(__list_t));
    l->n = n;
    l->t = VART_STR;
    l->cap = n;
    l->w = sizeof(char**);
    l->data = malloc(sizeof(char**)*n);

    char* tup = __gc_alloc(VART_TUP,27);
    ((char*)tup)[0]  = VART_I32;
    ((char*)tup)[5]  = VART_LST;
    ((char*)tup)[10] = 0;
    *(int32_t*)(tup+1)  = 15;
    *(int32_t*)(tup+6)  = 19;
    *(int32_t*)(tup+11) = 27;

    *(int32_t*)(tup+15) = indices[i];
    *(__list_t**)(tup+19) = l;
    ((char**)(lst->data))[i] = tup;

    for (int j = 0; j < n; j++){
      int m = strlen(groups[i][j]);
      char* s = __gc_alloc(VART_STR,m+1);
      strcpy(s, groups[i][j]);
      s[m] = 0;
      ((char**)(l->data))[j] = s;
      free(groups[i][j]);
    }
    free(groups[i]);
  }
  free(groups);
  free(indices);
  __put_ret(&lst);
}

void regx__replace(){
  char* __ARG(rep);
  char* __ARG(re);
  char* __ARG(s);
  char* o = regx_impl_replace(s,re,rep);
  __put_ret(&o);
}

