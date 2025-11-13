//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL -framework CoreGraphics" || echo "-lGLEW -lGL")

#include <stdio.h>
// #include <unistd.h>

#include "impl.c"

void drw___size(){
  uint64_t __ARG(ctx);
  int32_t __ARG(h);
  int32_t __ARG(w);

  drw_impl__size(w,h,ctx);
}

void drw___init_graphics(){
  int32_t __ARG(h);
  int32_t __ARG(w);

  void* o;
  __pop_arg(&o, sizeof(o));

  drw_impl__init_graphics((char*)o+4,w,h);
}

void drw___begin_fbo(){
  int __ARG(fbo);

  drw_impl__begin_fbo(fbo);
}

void drw___end_fbo(){
  drw_impl__end_fbo();
}

void drw___read_pixels(){
  int __ARG(fbo);

  __arr_t* a = __gc_alloc(VART_ARR, sizeof(__arr_t)+12);
  a->ndim = 3;
  a->data = drw_impl__read_pixels(fbo,&(a->dims[1]),&(a->dims[0]));
  a->dims[2] = 4;
  a->n = a->dims[0]*a->dims[1]*a->dims[2];
  a->w = 1;
  a->t = VART_U08;

  __put_ret(&a);
}

void drw___write_pixels(){
  __arr_t* a;
  __pop_arg(&a, 8);

  int __ARG(fbo);

  drw_impl__write_pixels(fbo,a->data);
}

void drw___draw_texture(){
  float __ARG(h);
  float __ARG(w);
  float __ARG(y);
  float __ARG(x);

  int32_t __ARG(fbo);

  drw_impl__draw_texture(fbo,x,y,w,h);
}


void drw__push_matrix(){
  drw_impl_push_matrix();
}
void drw__pop_matrix(){
  drw_impl_pop_matrix();
}
void drw__rotate_deg(){
  float __ARG(ang);
  drw_impl_rotate_deg(ang);
}
void drw__translate(){
  float __ARG(y);
  float __ARG(x);

  drw_impl_translate(x,y);
}
void drw__scale(){
  float __ARG(y);
  float __ARG(x);

  drw_impl_scale(x,y);
}
void drw__apply_matrix(){
  float vec[9];
  __pop_arg(&vec, 9*sizeof(float));

  drw_impl_apply_matrix(vec);
}
void drw__reset_matrix(){
  drw_impl_reset_matrix();
}


void drw__background(){
  float __ARG(a);
  float __ARG(b);
  float __ARG(g);
  float __ARG(r);

  drw_impl_background(r,g,b,a);
}


void drw__fill(){
  float __ARG(a);
  float __ARG(b);
  float __ARG(g);
  float __ARG(r);

  drw_impl_fill(r,g,b,a);
}

void drw__stroke(){
  float __ARG(a);
  float __ARG(b);
  float __ARG(g);
  float __ARG(r);

  drw_impl_stroke(r,g,b,a);
}

void drw__no_fill(){
  drw_impl_no_fill();
}
void drw__no_stroke(){
  drw_impl_no_stroke();
}
void drw__stroke_weight(){
  float __ARG(x);

  drw_impl_stroke_weight(x);
}

void drw__begin_shape(){
  drw_impl_begin_shape();
}

void drw__vertex(){
  float __ARG(y);
  float __ARG(x);

  drw_impl_vertex(x,y);
}

void drw__next_contour(){
  int __ARG(bclose);

  drw_impl_next_contour(bclose);
}

void drw__end_shape(){
  int __ARG(bclose);

  drw_impl_end_shape(bclose);
}


void drw__line(){
  float __ARG(y1);
  float __ARG(x1);
  float __ARG(y0);
  float __ARG(x0);

  drw_impl_line(x0,y0,x1,y1);
}


void drw__ellipse(){
  float __ARG(h);
  float __ARG(w);
  float __ARG(y);
  float __ARG(x);
  drw_impl_ellipse(x,y,w,h);
}

void drw__rect(){
  float __ARG(h);
  float __ARG(w);
  float __ARG(y);
  float __ARG(x);
  drw_impl_rect(x,y,w,h);
}

void drw__point(){
  float __ARG(y);
  float __ARG(x);
  drw_impl_point(x,y);
}


void drw__text(){
  float __ARG(y);
  float __ARG(x);
  char* __ARG(s);

  drw_impl_text(s,x,y);
}

void drw___flush(){
  drw_impl__flush();
}