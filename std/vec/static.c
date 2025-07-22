//
#include <math.h>
#include <stdlib.h>

void vec__mag(){
  int n = __peek_arg_size()/sizeof(float);
  __vla(float,v,n);
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

  __vla(float,v,n);
  __vla(float,u,n);

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
  __put_ret(u);
}

void vec__dot(){
  int n = __peek_arg_size()/sizeof(float);
  __vla(float,v,n);
  __vla(float,u,n);
  __pop_arg(v, n*sizeof(float));
  __pop_arg(u, n*sizeof(float));

  float s = 0;
  for (int i = 0; i < n; i++){
    s += u[i]*v[i];
  }
  __put_ret(&s);
}