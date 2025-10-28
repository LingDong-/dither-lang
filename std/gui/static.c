//
#include <stdio.h>

#include "impl.c"

void gui__init(){
  gui_impl_init();
}
void gui__poll(){
  gui_impl_poll();
}
void gui__slider(){
  int t = __peek_arg_type();
  int n = __peek_arg_size();
  if (t == VART_F32){
    float __ARG(r);
    float __ARG(l);
    float __ARG(x);
    char* __ARG(name);
    gui_impl__slider1f(name,x,l,r);
  }
}

void gui__toggle(){
  int t = __peek_arg_type();
  int n = __peek_arg_size();

  int32_t __ARG(x);
  char* __ARG(name);
  gui_impl__toggle1i(name,x);
  
}

void gui__get(){
  char* __ARG(name);
  int typ = __peek_ret_type();
  if (typ == VART_F32){
    float f = gui_impl__get1f(name);
    __put_ret(&f);
  }else if (typ == VART_I32){
    int32_t f = gui_impl__get1i(name);
    __put_ret(&f);
  }
}

