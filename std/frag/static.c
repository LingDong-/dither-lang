//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL -framework CoreGraphics" || echo "-lGLEW -lGL")

#include <stdio.h>
// #include <unistd.h>

#include "impl.c"

void frag___size(){
  uint64_t __ARG(ctx);
  int32_t __ARG(h);
  int32_t __ARG(w);

  frag_impl__size(w,h,ctx);
}

void frag___init_texture(){
  void* o;
  __pop_arg(&o, sizeof(o));
  frag_impl__init_texture(o+4);
}

void frag__program(){
  char* __ARG(src);
  int prgm = frag_impl_program(src);
  __put_ret(&prgm);
}

void frag___begin(){
  int32_t __ARG(fbo);
  int32_t __ARG(prgm);
  frag_impl__begin(prgm, fbo);
}

void frag__end(){
  frag_impl_end();
}

void frag__uniform(){
  int t = __peek_arg_type();
  int n = __peek_arg_size();
  if (t == VART_I32){
    int32_t x[n/4];
    __pop_arg(x,n);
    char* __ARG(s);
    frag_impl_uniformi(s,x,n/4);
  }else if (t == VART_F32){
    float x[n/4];
    __pop_arg(x,n);
    char* __ARG(s);
    frag_impl_uniformf(s,x,n/4);
  }else if (t == VART_STT){
    void* o;
    __pop_arg(&o, sizeof(o));
    int fbo = ((int32_t*)(o+4))[2];
    char* __ARG(s);
    frag_impl_uniform_sampler(s,fbo);
  }
}


void frag___sample(){
  float uv[2];
  __pop_arg((void*)uv,2*sizeof(float));
  int32_t __ARG(fbo);
}