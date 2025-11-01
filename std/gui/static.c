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
    if (n == sizeof(float)){
      float __ARG(r);
      float __ARG(l);
      float __ARG(x);
      char* __ARG(name);
      gui_impl__slider1f(name,x,l,r);
    }else{
      int l = n/sizeof(float);
      __vla(float,r,l);
      __vla(float,m,l);
      __vla(float,x,l);
      __pop_arg(r, n);
      __pop_arg(m, n);
      __pop_arg(x, n);
      char* __ARG(s);
      __vla(char,name,strlen(s)+8);
      for (int i = 0; i < l; i++){
        sprintf(name,"%s[%d]",s,i);
        gui_impl__slider1f(name,x[i],m[i],r[i]);
      }
    }
  }else if (t == VART_I32){
    int32_t __ARG(r);
    int32_t __ARG(l);
    int32_t __ARG(x);
    char* __ARG(name);
    gui_impl__slider1i(name,x,l,r);
  }
}

void gui__toggle(){
  int32_t __ARG(x);
  char* __ARG(name);
  gui_impl__toggle1i(name,x);
  
}

void gui__field(){
  char* __ARG(x);
  char* __ARG(name);
  gui_impl__field1s(name,x);
  
}

void gui__get(){
  char* __ARG(name);
  int typ = __peek_ret_type();
  int siz = __peek_ret_size();
  if (typ == VART_F32){
    if (siz == sizeof(float)){
      float f = gui_impl__get1f(name);
      __put_ret(&f);
    }else{
      int l = siz/sizeof(float);
      __vla(char,s,strlen(name)+8);
      __vla(float,v,l);
      for (int i = 0; i < l; i++){
        sprintf(s,"%s[%d]",name,i);
        float f = gui_impl__get1f(s);
        v[i] = f;
      }
      __put_ret(v);
    }
  }else if (typ == VART_I32){
    int32_t f = gui_impl__get1i(name);
    __put_ret(&f);
  }else if (typ == VART_STR){
    char* f = gui_impl__get1s(name);
    int n = strlen(f);
    char* o = __gc_alloc(VART_STR,n+1);
    strcpy(o,f);
    __put_ret(&o);
  }
}

