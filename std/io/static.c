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