//
#include <stdio.h>

void guts__gc_on(){
  __gc_off = 0;
}

void guts__gc_off(){
  __gc_off = 1;
}

void guts__gc(){
  __gc_run();
}
