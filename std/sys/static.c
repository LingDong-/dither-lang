//
#include <stdio.h>
#include "impl.h"

void sys__gc_on(){
  __gc_off = 0;
}

void sys__gc_off(){
  __gc_off = 1;
}

void sys__gc(){
  __gc_run();
}


void sys__argv(){
  int sn = __g_argc;
  char** ss = malloc(sizeof(char*)*sn);

  for (int i = 0; i < sn; i++){
    int n = strlen(__g_argv[i]);
    char* s = __gc_alloc(VART_STR,n+1);
    memcpy(s, __g_argv[i], n);
    ss[i] = s;
  }

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->w = sizeof(char*);
  lst->t = VART_STR;
  lst->data = (char*)ss;
  lst->cap = sn;
  lst->n = sn;
  __put_ret(&lst);
}

void sys__platform(){
  char p[128];
  strcpy(p,"c@");
  impl_platform(p+strlen(p));

  char* s0 = __gc_alloc(VART_STR, strlen(p)+1); 
  strcpy(s0, p);

  __put_ret(&s0);
}

void sys__getenv(){
  char* __ARG(s);
  char* p = getenv(s);
  
  char* s0 = __gc_alloc(VART_STR, strlen(p)+1); 
  strcpy(s0, p);

  __put_ret(&s0);
}