//

#ifndef WIN_STATIC_C
#define WIN_STATIC_C

#include <stdio.h>
// #include <unistd.h>

#include "impl.c"

void win__init(){
  int32_t __ARG(h);
  int32_t __ARG(w);
  
  win_impl_init(w,h);
  
}

void win__poll(){
  __push_stack();
  void* o = __gc_alloc(VART_STT,28);
  __put_var(0,o);
  void* oo = o+4;

  ((void**)(oo))[0] = o;

  win_impl_poll(oo);

  // usleep(16667);

  __put_ret(&(o));

  __gc_run();
  __pop_stack();
}

void win__exit(){
  win_impl_exit();
}

#endif

