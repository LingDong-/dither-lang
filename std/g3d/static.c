//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL -framework CoreGraphics" || echo "-lGLEW -lGL")

#include <stdio.h>
// #include <unistd.h>

#include "impl.c"

void g3d__init(){
  uint64_t __ARG(ctx);
  g3d_impl_init(ctx);
}

void g3d___update_mesh(){
  __list_t* __ARG(normals);
  __list_t* __ARG(uvs);
  __list_t* __ARG(colors);
  __list_t* __ARG(indices);
  __list_t* __ARG(vertices);
  int32_t __ARG(flags);
  int32_t __ARG(vao);
  // printf("%p %d\n",vertices->data,vertices->w);
  vao = g3d_impl__update_mesh(
    vao,flags,
    (float*)(vertices->data), vertices->n,
    (int32_t*)(indices->data), indices->n,
    (float*)(colors->data), colors->n,
    (float*)(uvs->data), uvs->n,
    (float*)(normals->data),normals->n
  );
  // printf("----- %d\n",vao);
  __put_ret(&vao);
}

void g3d___draw_mesh(){
  float transform[16];
  __pop_arg(transform, 64);

  int32_t __ARG(mode);
  int32_t __ARG(vao);
  // printf("%d %d\n",vao,mode);
  g3d_impl__draw_mesh(vao,mode,transform);
}

void g3d__flush(){
  g3d_impl_flush();
}

void g3d__background(){
  float __ARG(a);
  float __ARG(b);
  float __ARG(g);
  float __ARG(r);
  g3d_impl_background(r,g,b,a);
}

void g3d___look_at(){
  float up[3];
  float targ[3];
  float eye[3];
  __pop_arg(up,12);
  __pop_arg(targ,12);
  __pop_arg(eye,12);
  float out[16];
  g3d_impl__look_at(out,eye,targ,up);
  __put_ret(out);
}

void g3d___perspective(){
  float __ARG(zfar);
  float __ARG(znear);
  float __ARG(aspect);
  float __ARG(fov);
  float out[16];
  g3d_impl__perspective(out,fov*M_PI/180.0,aspect,znear,zfar);
  __put_ret(out);
}

void g3d___ortho(){
  float __ARG(zfar);
  float __ARG(znear);
  float __ARG(top);
  float __ARG(bottom);
  float __ARG(right);
  float __ARG(left);
  float out[16];
  g3d_impl__ortho(out,left,right,bottom,top,znear,zfar);
  __put_ret(out);
}

void g3d___camera_begin(){
  float proj[16];
  float view[16];
  __pop_arg(proj,64);
  __pop_arg(view,64);
  // printf("%f %f %f %f\n",view[5],view[6],view[7],view[8]);
  // printf("%f %f %f %f\n",proj[5],proj[6],proj[7],proj[8]);
  // printf("__________\n");
  g3d_impl__camera_begin(view,proj);
  
}

void g3d___camera_end(){
  g3d_impl__camera_end();
}

void g3d__mat__rotate_deg(){
  float __ARG(ang);
  float axis[3];
  float out[16];
  __pop_arg(axis,12);
  g3d_mat_impl_rotate(out,axis,ang*M_PI/180.0);
  __put_ret(out);
}

void g3d__text(){
  float transform[16];
  __pop_arg(transform, 64);
  char* __ARG(s);
  g3d_impl_text(s,transform);
}