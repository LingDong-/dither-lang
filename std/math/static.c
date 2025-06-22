//
#include <math.h>
#include <stdlib.h>

void math__sin(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y = sinf(x);
  __put_ret(&y);
}

void math__cos(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y = cosf(x);
  __put_ret(&y);
}


void math__random(){
  float y = (float)rand()/(float)RAND_MAX;
  __put_ret(&y);
}

void math__min(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y;
  __pop_arg(&y, sizeof(y));
  float z = fminf(x,y);

  __put_ret(&z);
}

void math__max(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y;
  __pop_arg(&y, sizeof(y));
  float z = fmaxf(x,y);

  __put_ret(&z);
}


void math__atan2(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y;
  __pop_arg(&y, sizeof(y));
  float z = atan2(y,x);

  __put_ret(&z);
}


void math__round(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y = roundf(x);
  __put_ret(&y);
}

void math__floor(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y = floorf(x);
  __put_ret(&y);
}

void math__ceil(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y = ceilf(x);
  __put_ret(&y);
}

void math__abs(){
  float x;
  __pop_arg(&x, sizeof(x));
  float y = fabsf(x);
  __put_ret(&y);
}