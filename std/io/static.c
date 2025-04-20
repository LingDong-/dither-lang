//
#include <stdio.h>

void io__println(){
  char* s;
  __pop_arg(&s, 8);
  printf("%s\n",s);
}
void io__print(){
  char* s;
  __pop_arg(&s, 8);
  printf("%s",s);
}

