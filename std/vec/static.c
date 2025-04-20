//
#include <math.h>
#include <stdlib.h>

void vec__mag(){
  int n = __peek_arg_size()/sizeof(float);
  float v[n];
  __pop_arg(v, n*sizeof(float));

  float s = 0;
  for (int i = 0; i < n; i++){
    float x = v[i];
    s += x*x;
  }
  s = sqrt(s);
  __put_ret(&s);
}

void vec__dir(){
  int n = __peek_arg_size()/sizeof(float);
  float v[n];
  __pop_arg(v, n*sizeof(float));
  float u[n];

  float s = 0;
  for (int i = 0; i < n; i++){
    float x = v[i];
    s += x*x;
  }
  if (s){
    s = 1.0/sqrt(s);
    for (int i = 0; i < n; i++){
      u[i] = v[i]*s;
    }
  }
  __put_ret(&u);
}