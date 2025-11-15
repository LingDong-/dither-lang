//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL -framework CoreGraphics" || echo "-lGLEW -lGL")

#include <stdio.h>
// #include <unistd.h>

#include "impl.c"

void frag__init(){
  uint64_t __ARG(ctx);
  frag_impl_init(ctx);
}

void frag___init_texture(){
  int32_t __ARG(flags);
  int32_t __ARG(h);
  int32_t __ARG(w);
  void* o;
  __pop_arg(&o, sizeof(o));
  frag_impl__init_texture((char*)o+4,w,h,flags);
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
    __vla(int32_t,x,(n/4));
    __pop_arg(x,n);
    char* __ARG(s);
    frag_impl_uniformi(s,x,n/4);
  }else if (t == VART_F32){
    __vla(float,x,(n/4));
    __pop_arg(x,n);
    char* __ARG(s);
    frag_impl_uniformf(s,x,n/4);
  }else if (t == VART_STT){
    void* o;
    __pop_arg(&o, sizeof(o));
    int fbo = ((int32_t*)((char*)o+4))[2];
    char* __ARG(s);
    frag_impl_uniform_sampler(s,fbo);
  }
}

void frag__render(){
  frag_impl_render();
}

void frag___sample(){
  float uv[2];
  __pop_arg((void*)uv,2*sizeof(float));
  int32_t __ARG(fbo);
}

void frag___write_pixels(){
  __arr_t* a;
  __pop_arg(&a, 8);

  int __ARG(fbo);

  frag_impl__write_pixels(fbo,a->data);
}

void frag___read_pixels(){
  int __ARG(fbo);

  __arr_t* a = __gc_alloc(VART_ARR, sizeof(__arr_t)+12);
  a->ndim = 3;
  a->data = frag_impl__read_pixels(fbo,&(a->dims[1]),&(a->dims[0]));
  a->dims[2] = 4;
  a->n = a->dims[0]*a->dims[1]*a->dims[2];
  a->w = 1;
  a->t = VART_U08;

  __put_ret(&a); 
}