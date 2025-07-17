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

EXPORTED void g3d_init(var_t* ret, gstate_t* _g){
  uint64_t ctx = ARG_POP(_g,u64);
  g3d_impl_init(ctx);
}

void* copy_list_vec_pack(lst_t* lst, int vsz){
  if (!lst->n) return NULL;
  void* data = malloc(lst->n*vsz);
  for (int i = 0; i < lst->n; i++){
    vec_t* v = ((vec_t**)(lst->data))[i];
    memcpy((char*)data + i*vsz, v->data, vsz);
  }
  return data;
}

EXPORTED void g3d__update_mesh(var_t* ret, gstate_t* _g){
  lst_t* normals = ARG_POP(_g,lst);
  lst_t* uvs     = ARG_POP(_g,lst);
  lst_t* colors  = ARG_POP(_g,lst);
  lst_t* indices = ARG_POP(_g,lst);
  lst_t* vertices= ARG_POP(_g,lst);
  int32_t flags = ARG_POP(_g,i32);
  int32_t vao = ARG_POP(_g,i32);

  void *p_vertices=NULL, *p_colors=NULL, *p_uvs=NULL, *p_normals=NULL;
  if (flags & DIRTY_VERTICES) p_vertices = copy_list_vec_pack(vertices,12);
  if (flags & DIRTY_COLORS) p_colors = copy_list_vec_pack(colors,16);
  if (flags & DIRTY_UVS) p_uvs = copy_list_vec_pack(uvs,8);
  if (flags & DIRTY_NORMALS) p_normals = copy_list_vec_pack(normals,12);
  
  vao = g3d_impl__update_mesh(
    vao,flags,
    p_vertices, vertices->n,
    indices->data, indices->n,
    p_colors, colors->n,
    p_uvs, uvs->n,
    p_normals, normals->n
  );
  free(p_vertices);
  free(p_colors);
  free(p_uvs);
  free(p_normals);
  
  ret->u.i32 = vao;
}

EXPORTED void g3d__draw_mesh(var_t* ret, gstate_t* _g){
  vec_t* transform= ARG_POP(_g,vec);
  int32_t mode= ARG_POP(_g,i32);
  int32_t vao = ARG_POP(_g,i32);
  g3d_impl__draw_mesh(vao,mode,transform->data);
}

EXPORTED void g3d_flush(var_t* ret, gstate_t* _g){
  g3d_impl_flush();
}

EXPORTED void g3d_background(var_t* ret, gstate_t* _g){
  float a = ARG_POP(_g,f32);
  float b = ARG_POP(_g,f32);
  float g = ARG_POP(_g,f32);
  float r = ARG_POP(_g,f32);
  g3d_impl_background(r,g,b,a);
}

EXPORTED void g3d__look_at(var_t* ret, gstate_t* _g){
  vec_t* up   = ARG_POP(_g,vec);
  vec_t* targ = ARG_POP(_g,vec);
  vec_t* eye  = ARG_POP(_g,vec);

  vec_t* out = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(16*sizeof(float)));
  out->n = 16;
  out->type = ret->type;
  out->w = sizeof(float);
  g3d_impl__look_at(out->data,eye->data,targ->data,up->data);
  ret->u.vec = out;
}


EXPORTED void g3d__perspective(var_t* ret, gstate_t* _g){
  float zfar = ARG_POP(_g,f32);
  float znear = ARG_POP(_g,f32);
  float aspect = ARG_POP(_g,f32);
  float fov = ARG_POP(_g,f32);
  vec_t* out = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(16*sizeof(float)));
  out->n = 16;
  out->type = ret->type;
  out->w = sizeof(float);
  g3d_impl__perspective(out->data,fov*M_PI/180.0,aspect,znear,zfar);
  ret->u.vec = out;
}

EXPORTED void g3d__camera_begin(var_t* ret, gstate_t* _g){
  vec_t* proj = ARG_POP(_g,vec);
  vec_t* view = ARG_POP(_g,vec);
  g3d_impl__camera_begin(view->data,proj->data);
}

EXPORTED void g3d__camera_end(var_t* ret, gstate_t* _g){
  g3d_impl__camera_end();
}

EXPORTED void g3d_mat_rotate_deg(var_t* ret, gstate_t* _g){
  float ang = ARG_POP(_g,f32);
  vec_t* axis = ARG_POP(_g,vec);
  vec_t* out = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(16*sizeof(float)));
  out->n = 16;
  out->type = ret->type;
  out->w = sizeof(float);
  g3d_mat_impl_rotate(out->data,axis->data,ang*M_PI/180.0);
  ret->u.vec = out;
}


#define QK_REG(name) register_cfunc(&(_g->cfuncs), "g3d." QUOTE(name), g3d_ ## name);
#define QK_REG_SUB(nmsp,name) register_cfunc(&(_g->cfuncs), "g3d." QUOTE(nmsp) "." QUOTE(name), g3d_ ## nmsp ## _ ## name);

EXPORTED void lib_init_g3d(gstate_t* _g){
  QK_REG(init);
  QK_REG(_draw_mesh);
  QK_REG(_update_mesh);
  QK_REG(flush);
  QK_REG(background);
  QK_REG(_look_at);
  QK_REG(_perspective);
  QK_REG(_camera_begin);
  QK_REG(_camera_end);
  QK_REG_SUB(mat,rotate_deg);
}



