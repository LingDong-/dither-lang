//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL -framework CoreGraphics" || echo "-lGLEW -lGL")

#include <stdio.h>
// #include <unistd.h>

#include "impl.c"

void gx___size(){
  uint64_t __ARG(ctx);
  int32_t __ARG(h);
  int32_t __ARG(w);

  gx_impl__size(w,h,ctx);
}

void gx___init_graphics(){
  int32_t __ARG(h);
  int32_t __ARG(w);

  void* o;
  __pop_arg(&o, sizeof(o));

  gx_impl__init_graphics(o+4,w,h);
}

void gx___begin_fbo(){
  int __ARG(fbo);

  gx_impl__begin_fbo(fbo);
}

void gx___end_fbo(){
  gx_impl__end_fbo();
}

void gx___read_pixels(){
  int __ARG(fbo);

  __arr_t* a = __gc_alloc(VART_ARR, sizeof(__arr_t)+12);
  a->ndim = 3;
  a->data = gx_impl__read_pixels(fbo,&(a->dims[1]),&(a->dims[0]));
  a->dims[2] = 4;
  a->n = a->dims[0]*a->dims[1]*a->dims[2];
  a->w = 1;
  a->t = VART_U08;

  __put_ret(&a);
}

void gx___write_pixels(){
  __arr_t* a;
  __pop_arg(&a, 8);

  int __ARG(fbo);

  gx_impl__write_pixels(fbo,a->data);
}

void gx___draw_texture(){
  float __ARG(h);
  float __ARG(w);
  float __ARG(y);
  float __ARG(x);

  int32_t __ARG(fbo);

  gx_impl__draw_texture(fbo,x,y,w,h);
}


void gx__push_matrix(){
  gx_impl_push_matrix();
}
void gx__pop_matrix(){
  gx_impl_pop_matrix();
}
void gx__rotate_deg(){
  float __ARG(ang);
  gx_impl_rotate_deg(ang);
}
void gx__translate(){
  float __ARG(y);
  float __ARG(x);

  gx_impl_translate(x,y);
}
void gx__scale(){
  float __ARG(y);
  float __ARG(x);

  gx_impl_scale(x,y);
}
void gx__apply_matrix(){
  float vec[9];
  __pop_arg(&vec, 9*sizeof(float));

  gx_impl_apply_matrix(vec);
}
void gx__reset_matrix(){
  gx_impl_reset_matrix();
}


void gx__background(){
  float __ARG(a);
  float __ARG(b);
  float __ARG(g);
  float __ARG(r);

  gx_impl_background(r,g,b,a);
}


void gx__fill(){
  float __ARG(a);
  float __ARG(b);
  float __ARG(g);
  float __ARG(r);

  gx_impl_fill(r,g,b,a);
}

void gx__stroke(){
  float __ARG(a);
  float __ARG(b);
  float __ARG(g);
  float __ARG(r);

  gx_impl_stroke(r,g,b,a);
}

void gx__no_fill(){
  gx_impl_no_fill();
}
void gx__no_stroke(){
  gx_impl_no_stroke();
}
void gx__stroke_weight(){
  float __ARG(x);

  gx_impl_stroke_weight(x);
}

void gx__begin_shape(){
  gx_impl_begin_shape();
}

void gx__vertex(){
  float __ARG(y);
  float __ARG(x);

  gx_impl_vertex(x,y);
}

void gx__next_contour(){
  int __ARG(bclose);

  gx_impl_next_contour(bclose);
}

void gx__end_shape(){
  int __ARG(bclose);

  gx_impl_end_shape(bclose);
}


void gx__line(){
  float __ARG(y1);
  float __ARG(x1);
  float __ARG(y0);
  float __ARG(x0);

  gx_impl_line(x0,y0,x1,y1);
}


void gx__ellipse(){
  float __ARG(h);
  float __ARG(w);
  float __ARG(y);
  float __ARG(x);
  gx_impl_ellipse(x,y,w,h);
}

void gx__rect(){
  float __ARG(h);
  float __ARG(w);
  float __ARG(y);
  float __ARG(x);
  gx_impl_rect(x,y,w,h);
}

void gx__point(){
  float __ARG(y);
  float __ARG(x);
  gx_impl_point(x,y);
}


void gx__text(){
  float __ARG(y);
  float __ARG(x);
  char* __ARG(s);

  gx_impl_text(s,x,y);
}

