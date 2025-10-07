#include <stdio.h>

#include "../../src/interp.c"

#include "impl.c"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x

EXPORTED void font__lookup(var_t* ret,  gstate_t* _g){
  uint32_t flag = ARG_POP(_g,u32);
  var_t* u = ARG_POP_VAR_NO_FREE(_g);
  int id = ARG_POP(_g,i32);
  int gid;
  if (u->type->vart == VART_I32){
    gid = font_impl__lookup(id,u->u.i32,flag);
  }else if (u->type->vart == VART_LST){
    int n = u->u.lst->n;
    gid = font_impl__ligature(id,&n,(int32_t*)(u->u.lst->data),flag);
    memmove(u->u.lst->data, ((int32_t*)(u->u.lst->data)) + n, (u->u.lst->n - n)*sizeof(int32_t) );
    u->u.lst->n -= n;
  }
  free(u);
  ret->u.i32 = gid;
}

EXPORTED void font__advance(var_t* ret,  gstate_t* _g){
  uint32_t flag = ARG_POP(_g,u32);
  int hidx = ARG_POP(_g,i32);
  int gidx = ARG_POP(_g,i32);
  int id = ARG_POP(_g,i32);
  ret->u.f32 = font_impl__advance(id,gidx,hidx,flag);
}

EXPORTED void font__glyph(var_t* ret,  gstate_t* _g){
  int reso = ARG_POP(_g,i32);
  int gidx = ARG_POP(_g,i32);
  int id = ARG_POP(_g,i32);
  int n;
  int* m;
  float* polys = font_impl__glyph(id,gidx,reso,&n,&m);

  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  lst->type = ret->type;
  lst->n = n;
  lst->cap = n+1;
  lst->w = 8;
  lst->data = calloc(lst->w,lst->cap);

  int pidx = 0;
  for (int i = 0; i < n; i++){
    lst_t* l = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
    l->n = m[i];
    l->cap = m[i]+1;
    l->w = 8;
    l->data = calloc(l->w,l->cap);
    l->type = (type_t*)(ret->type->u.elem.head->data);
    for (int j = 0; j < m[i]; j++){
      vec_t* v = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(2*sizeof(float)));
      v->n = 2;
      v->type = (type_t*)(l->type->u.elem.head->data);
      v->w = sizeof(float);
      ((float*)v->data)[0] = polys[pidx++];
      ((float*)v->data)[1] = polys[pidx++];
      ((vec_t**)(l->data))[j] = v;
    }
    ((lst_t**)(lst->data))[i] = l;
  }

  ret->u.lst = lst;
}

EXPORTED void font_decode(var_t* ret,  gstate_t* _g){
  var_t* u = ARG_POP_VAR_NO_FREE(_g);

  int id;
  if (u->type->vart == VART_I32){
    int cset = u->u.i32;
    id = font_impl_hershey(cset);
  }else{
    lst_t* l = u->u.lst;
    id = font_impl_decode(l->n,(char*)(l->data));
  }
  free(u);

  obj_t* o = gc_alloc_(_g, sizeof(obj_t));
  o->type = ret->type;
  o->data = calloc(12,1);
  ((obj_t**)(o->data))[0] = o;
  ((int32_t*)(o->data))[2] = id;
  ret->u.obj = o;
}



#define QK_REG(name) register_cfunc(&(_g->cfuncs), "font." QUOTE(name), font_ ## name);

EXPORTED void lib_init_font(gstate_t* _g){
  QK_REG(_lookup);
  QK_REG(_advance);
  QK_REG(_glyph);
  QK_REG(decode);

}
