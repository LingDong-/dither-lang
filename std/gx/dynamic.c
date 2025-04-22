//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL" || echo "-lGLEW -lGL")

#include "../../src/interp.c"
#include "impl.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x

EXPORTED void gx_size(var_t* ret, gstate_t* _g){
  int h = ARG_POP(_g,i32);
  int w = ARG_POP(_g,i32);

  gx_impl_size(w,h);
}

EXPORTED void gx_poll(var_t* ret, gstate_t* _g){

  obj_t* o = gc_alloc_(_g, sizeof(obj_t));
  o->type = ret->type;
  o->data = calloc(24,1);
  ((obj_t**)(o->data))[0] = o;

  gx_impl_poll(o->data);

  ret->u.obj = o;
  _g->flags |= GFLG_TRGC;
}


EXPORTED void gx__init_graphics(var_t* ret, gstate_t* _g){
  int h = ARG_POP(_g,i32);
  int w = ARG_POP(_g,i32);
  obj_t* o = ARG_POP(_g,obj);

  gx_impl__init_graphics(o->data,w,h);
}

EXPORTED void gx__begin_fbo(var_t* ret, gstate_t* _g){
  int fbo = ARG_POP(_g,i32);

  gx_impl__begin_fbo(fbo);
}

EXPORTED void gx__end_fbo(var_t* ret, gstate_t* _g){
  gx_impl__end_fbo();
}

EXPORTED void gx__draw_texture(var_t* ret, gstate_t* _g){
  float h = ARG_POP(_g,f32);
  float w = ARG_POP(_g,f32);
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);

  int tex = ARG_POP(_g,i32);

  gx_impl__draw_texture(tex,x,y,w,h);
}


EXPORTED void gx_push_matrix(var_t* ret, gstate_t* _g){
  gx_impl_push_matrix();
}
EXPORTED void gx_pop_matrix(var_t* ret, gstate_t* _g){
  gx_impl_pop_matrix();
}
EXPORTED void gx_rotate_deg(var_t* ret, gstate_t* _g){
  float ang = ARG_POP(_g,f32);
  gx_impl_rotate_deg(ang);
}
EXPORTED void gx_translate(var_t* ret, gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);

  gx_impl_translate(x,y);
}
EXPORTED void gx_scale(var_t* ret, gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);

  gx_impl_scale(x,y);
}
EXPORTED void gx_apply_matrix(var_t* ret, gstate_t* _g){
  vec_t* vec = ARG_POP(_g,vec);

  gx_impl_apply_matrix((float*)(vec->data));
}
EXPORTED void gx_reset_matrix(var_t* ret, gstate_t* _g){
  gx_impl_reset_matrix();
}


EXPORTED void gx_background(var_t* ret, gstate_t* _g){
  float a = ARG_POP(_g,f32);
  float b = ARG_POP(_g,f32);
  float g = ARG_POP(_g,f32);
  float r = ARG_POP(_g,f32);

  gx_impl_background(r,g,b,a);
}


EXPORTED void gx_fill(var_t* ret, gstate_t* _g){
  float a = ARG_POP(_g,f32);
  float b = ARG_POP(_g,f32);
  float g = ARG_POP(_g,f32);
  float r = ARG_POP(_g,f32);

  gx_impl_fill(r,g,b,a);
}

EXPORTED void gx_stroke(var_t* ret, gstate_t* _g){
  float a = ARG_POP(_g,f32);
  float b = ARG_POP(_g,f32);
  float g = ARG_POP(_g,f32);
  float r = ARG_POP(_g,f32);

  gx_impl_stroke(r,g,b,a);
}

EXPORTED void gx_no_fill(var_t* ret, gstate_t* _g){
  gx_impl_no_fill();
}
EXPORTED void gx_no_stroke(var_t* ret, gstate_t* _g){
  gx_impl_no_stroke();
}
EXPORTED void gx_stroke_weight(var_t* ret, gstate_t* _g){
  float x = ARG_POP(_g,f32);

  gx_impl_stroke_weight(x);
}

EXPORTED void gx_begin_shape(var_t* ret, gstate_t* _g){
  gx_impl_begin_shape();
}

EXPORTED void gx_vertex(var_t* ret, gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);

  gx_impl_vertex(x,y);
}

EXPORTED void gx_next_contour(var_t* ret, gstate_t* _g){
  int bclose = ARG_POP(_g,i32);

  gx_impl_next_contour(bclose);
}

EXPORTED void gx_end_shape(var_t* ret, gstate_t* _g){
  int bclose = ARG_POP(_g,i32);

  gx_impl_end_shape(bclose);
}


EXPORTED void gx_line(var_t* ret, gstate_t* _g){
  float y1 = ARG_POP(_g,f32);
  float x1 = ARG_POP(_g,f32);
  float y0 = ARG_POP(_g,f32);
  float x0 = ARG_POP(_g,f32);

  gx_impl_line(x0,y0,x1,y1);
}


EXPORTED void gx_ellipse(var_t* ret, gstate_t* _g){
  float h = ARG_POP(_g,f32);
  float w = ARG_POP(_g,f32);
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  gx_impl_ellipse(x,y,w,h);
}

EXPORTED void gx_rect(var_t* ret, gstate_t* _g){
  float h = ARG_POP(_g,f32);
  float w = ARG_POP(_g,f32);
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  gx_impl_rect(x,y,w,h);
}

EXPORTED void gx_point(var_t* ret, gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  gx_impl_point(x,y);
}


EXPORTED void gx_text(var_t* ret, gstate_t* _g){
  float y = ARG_POP(_g,f32);
  float x = ARG_POP(_g,f32);
  stn_t* s = ARG_POP(_g,str);

  gx_impl_text(s->data,x,y);
}


EXPORTED void gx_load_font(var_t* ret, gstate_t* _g){
  stn_t* s = ARG_POP(_g,str);
  int fb = gx_impl_load_font(s->data);
  ret->u.i32 = fb;
}

EXPORTED void gx_text_font(var_t* ret, gstate_t* _g){
  int x = ARG_POP(_g,i32);
  gx_impl_text_font(x);
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "gx." QUOTE(name), gx_ ## name);

EXPORTED void lib_init_gx(gstate_t* _g){
  QK_REG(size)
  QK_REG(poll)
  QK_REG(background)
  QK_REG(fill)
  QK_REG(stroke)
  QK_REG(stroke_weight)
  QK_REG(no_fill)
  QK_REG(no_stroke)
  QK_REG(begin_shape)
  QK_REG(vertex)
  QK_REG(next_contour)
  QK_REG(end_shape)
  QK_REG(line)
  QK_REG(ellipse)
  QK_REG(rect)
  QK_REG(point)
  QK_REG(text)
  QK_REG(text_font)
  QK_REG(load_font)

  QK_REG(_init_graphics)

  QK_REG(_begin_fbo)
  QK_REG(_end_fbo)

  QK_REG(_draw_texture)

  QK_REG(rotate_deg)
  QK_REG(translate)
  QK_REG(scale)
  QK_REG(push_matrix)
  QK_REG(pop_matrix)
  QK_REG(apply_matrix)
  QK_REG(reset_matrix)
}



