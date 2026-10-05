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

void sys__command(){
  int32_t __ARG(flags);
  char* __ARG(cmd);

  int st;
  int len;
  char* p = sys_impl_command(cmd,flags,&len,&st);
  
  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->w = 1;
  lst->t = VART_U08;
  lst->data = p;
  lst->cap = len;
  lst->n = len;

  char* tup = __gc_alloc(VART_TUP,27);
  ((char*)tup)[0]  = VART_I32;
  ((char*)tup)[5]  = VART_LST;
  ((char*)tup)[10] = 0;

  *(int32_t*)(tup+1)  = 15;
  *(int32_t*)(tup+6)  = 19;
  *(int32_t*)(tup+11) = 27;

  *(int32_t*)(tup+15) = st;
  *(__list_t**)(tup+19) = lst;

  __put_ret(&tup);
}
