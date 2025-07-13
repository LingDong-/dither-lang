//
#include <math.h>
#include <stdlib.h>

void vec__mag(){
  int n = __peek_arg_size()/sizeof(float);
#ifdef _WIN32
  float* v = (float*)_alloca(n*sizeof(float));
#else
  float v[n];
#endif
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
#ifdef _WIN32
  float* v = (float*)_alloca(n*sizeof(float));
  float* u = (float*)_alloca(n*sizeof(float));
#else
  float v[n];
  float u[n];
#endif
  __pop_arg(v, n*sizeof(float));

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