//
#include <stdio.h>

#include "impl.c"

void io__println(){
  char* s;
  __pop_arg(&s, 8);
  io_impl_print(s);
  io_impl_print("\n");
}
void io__print(){
  char* s;
  __pop_arg(&s, 8);
  io_impl_print(s);
}

void io__write_file(){
  __list_t* a = NULL;
  __pop_arg(&a, 8);

  char* s;
  __pop_arg(&s, 8);

  io_impl_write_file(s,a->data,a->n);
}

void io__read_file(){
  char* __ARG(s);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 1;
  a->t = VART_U08;
  a->data = io_impl_read_file(s,&(a->n));
  a->cap = a->n;

  __put_ret(&a);
}