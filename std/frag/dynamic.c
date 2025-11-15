//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL -framework CoreGraphics" || echo "-lGLEW -lGL")

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

EXPORTED void frag_init(var_t* ret, gstate_t* _g){
  
  uint64_t ctx = ARG_POP(_g,u64);

  frag_impl_init(ctx);
}

EXPORTED void frag__init_texture(var_t* ret, gstate_t* _g){
  int flags = ARG_POP(_g,i32);
  int h = ARG_POP(_g,i32);
  int w = ARG_POP(_g,i32);
  obj_t* o = ARG_POP(_g,obj);
  frag_impl__init_texture(o->data,w,h,flags);
}

EXPORTED void frag_program(var_t* ret, gstate_t* _g){
  stn_t* src = ARG_POP(_g,str);
  int prgm = frag_impl_program(src->data);
  ret->u.i32 = prgm;
}

EXPORTED void frag__begin(var_t* ret, gstate_t* _g){
  int fbo = ARG_POP(_g,i32);
  int prgm = ARG_POP(_g,i32);

  frag_impl__begin(prgm, fbo);
}

EXPORTED void frag_render(var_t* ret, gstate_t* _g){
  frag_impl_render();
}

EXPORTED void frag_end(var_t* ret, gstate_t* _g){
  frag_impl_end();
}

EXPORTED void frag_uniform(var_t* ret, gstate_t* _g){
  var_t* u = ARG_POP_VAR_NO_FREE(_g);
  stn_t* s = ARG_POP(_g,str);

  if (u->type->vart == VART_I32){
    frag_impl_uniformi(s->data,&(u->u.i32),1);
  }else if (u->type->vart == VART_F32){
    frag_impl_uniformf(s->data,&(u->u.f32),1);
  }else if (u->type->vart == VART_VEC){
    type_t* ta = (type_t*)(u->type->u.elem.head->data);
    if (ta->vart == VART_I32){
      frag_impl_uniformi(s->data, (int32_t*)(u->u.vec->data), u->u.vec->n);
    }else if (ta->vart == VART_F32){
      frag_impl_uniformf(s->data, (float*)(u->u.vec->data), u->u.vec->n);
    }
  }else if (u->type->vart == VART_STT){
    int fbo = ((int32_t*)(u->u.obj->data))[2];
    frag_impl_uniform_sampler(s->data,fbo);
  }

  free(u);
}

EXPORTED void frag__sample(var_t* ret, gstate_t* _g){
  vec_t* v = ARG_POP(_g,vec);
  int fbo = ARG_POP(_g,i32);
}

EXPORTED void frag__write_pixels(var_t* ret, gstate_t* _g){
  arr_t* arr = ARG_POP(_g,arr);
  int fbo = ARG_POP(_g,i32);

  frag_impl__write_pixels(fbo,arr->data);
}

EXPORTED void frag__read_pixels(var_t* ret, gstate_t* _g){
  int fbo = ARG_POP(_g,i32);

  arr_t* arr = (arr_t*)gc_alloc_(_g,sizeof(arr_t)+12);
  arr->data = (char*)frag_impl__read_pixels(fbo, &(arr->dims[1]), &(arr->dims[0]));
  arr->dims[2] = 4;
  arr->ndim = 3;
  arr->w = 1;
  arr->type = ret->type;
  arr->n = arr->dims[0]*arr->dims[1]*arr->dims[2];

  ret->u.arr = arr;
}


#define QK_REG(name) register_cfunc(&(_g->cfuncs), "frag." QUOTE(name), frag_ ## name);

EXPORTED void lib_init_frag(gstate_t* _g){
  QK_REG(init)
  QK_REG(_init_texture)
  QK_REG(program)
  QK_REG(_begin)
  QK_REG(end)
  QK_REG(uniform)
  QK_REG(_sample)
  QK_REG(_write_pixels)
  QK_REG(_read_pixels)
  QK_REG(render)
}



