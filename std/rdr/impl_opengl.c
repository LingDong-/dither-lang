#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#define glVertexAttribDivisor glVertexAttribDivisorARB
#define glDrawArraysInstanced glDrawArraysInstancedARB
#define glDrawElementsInstanced glDrawElementsInstancedARB
#define glBindVertexArray glBindVertexArrayAPPLE
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#elif defined(_WIN32)
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#pragma comment(lib, "opengl32.lib")
#include "../win/platform/wgl_patcher.h"
#else
#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glext.h>
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define MAT_TFRM(A,v) {((A)[0]*(v)[0]+(A)[1]*(v)[1]+(A)[2]*(v)[2]+(A)[3])/((A)[12]*(v)[0]+(A)[13]*(v)[1]+(A)[14]*(v)[2]+(A)[15]),((A)[4]*(v)[0]+(A)[5]*(v)[1]+(A)[6]*(v)[2]+(A)[7])/((A)[12]*(v)[0]+(A)[13]*(v)[1]+(A)[14]*(v)[2]+(A)[15]),((A)[8]*(v)[0]+(A)[9]*(v)[1]+(A)[10]*(v)[2]+(A)[11])/((A)[12]*(v)[0]+(A)[13]*(v)[1]+(A)[14]*(v)[2]+(A)[15])}
#define MAT_MULT(A,B) {(A)[0]*(B)[0]+(A)[1]*(B)[4]+(A)[2]*(B)[8]+(A)[3]*(B)[12],(A)[0]*(B)[1]+(A)[1]*(B)[5]+(A)[2]*(B)[9]+(A)[3]*(B)[13],(A)[0]*(B)[2]+(A)[1]*(B)[6]+(A)[2]*(B)[10]+(A)[3]*(B)[14],(A)[0]*(B)[3]+(A)[1]*(B)[7]+(A)[2]*(B)[11]+(A)[3]*(B)[15],(A)[4]*(B)[0]+(A)[5]*(B)[4]+(A)[6]*(B)[8]+(A)[7]*(B)[12],(A)[4]*(B)[1]+(A)[5]*(B)[5]+(A)[6]*(B)[9]+(A)[7]*(B)[13],(A)[4]*(B)[2]+(A)[5]*(B)[6]+(A)[6]*(B)[10]+(A)[7]*(B)[14],(A)[4]*(B)[3]+(A)[5]*(B)[7]+(A)[6]*(B)[11]+(A)[7]*(B)[15],(A)[8]*(B)[0]+(A)[9]*(B)[4]+(A)[10]*(B)[8]+(A)[11]*(B)[12],(A)[8]*(B)[1]+(A)[9]*(B)[5]+(A)[10]*(B)[9]+(A)[11]*(B)[13],(A)[8]*(B)[2]+(A)[9]*(B)[6]+(A)[10]*(B)[10]+(A)[11]*(B)[14],(A)[8]*(B)[3]+(A)[9]*(B)[7]+(A)[10]*(B)[11]+(A)[11]*(B)[15],(A)[12]*(B)[0]+(A)[13]*(B)[4]+(A)[14]*(B)[8]+(A)[15]*(B)[12],(A)[12]*(B)[1]+(A)[13]*(B)[5]+(A)[14]*(B)[9]+(A)[15]*(B)[13],(A)[12]*(B)[2]+(A)[13]*(B)[6]+(A)[14]*(B)[10]+(A)[15]*(B)[14],(A)[12]*(B)[3]+(A)[13]*(B)[7]+(A)[14]*(B)[11]+(A)[15]*(B)[15]}

#undef ARR_DEF
#define ARR_DEF(dtype) \
  typedef struct { int len; int cap; dtype* data; } dtype ## _arr_t;

#undef ARR_INIT
#define ARR_INIT(dtype,name) \
  name.len = 0;  \
  name.cap = 8; \
  name.data = (dtype*) malloc((name.cap)*sizeof(dtype));

#undef ARR_PUSH
#undef ARR_ITEM_FORCE_CAST
#ifdef _WIN32
#define ARR_ITEM_FORCE_CAST(dtype,item) item
#else
#define ARR_ITEM_FORCE_CAST(dtype,item) (dtype)item
#endif
#define ARR_PUSH(dtype,name,item) \
  if (name.cap < name.len+1){ \
    int hs = name.cap/2; \
    name.cap = name.len+MAX(1,hs); \
    name.data = (dtype*)realloc(name.data, (name.cap)*sizeof(dtype) ); \
  }\
  name.data[name.len] = ARR_ITEM_FORCE_CAST(dtype,item);\
  name.len += 1;

#undef ARR_POP
#define ARR_POP(dtype,name) (name.data[--name.len])

#undef ARR_CLEAR
#define ARR_CLEAR(dtype,name) {name.len = 0;}

#define DIRTY_VERTICES 1
#define DIRTY_INDICES  2
#define DIRTY_COLORS   4
#define DIRTY_UVS      8
#define DIRTY_NORMALS  16
#define DIRTY_BOUNDS   32

#define LOC_POSITION 0
#define LOC_COLOR    1
#define LOC_UV       2
#define LOC_NORMAL   3
#define LOC_MODEL    4
#define LOC_NM       8

#define CULL_FRUSTUM 16

GLint fbo_zero;

typedef struct mesh_st {
  GLuint VAO;
  GLuint vbo_vertices;
  GLuint vbo_colors;
  GLuint vbo_uvs;
  GLuint vbo_normals;
  GLuint ebo_indices;
  GLuint vbo_models;
  GLuint vbo_normal_matrices;
  int n_vertices;
  int n_colors;
  int n_uvs;
  int n_normals;
  int n_indices;
  float bsphere[4];
} mesh_t;


ARR_DEF(mesh_t);
mesh_t_arr_t meshes;

GLuint shader = 0;

const char* vertex_src = "#version 120\n"
"attribute vec3 a_position;\n"
"attribute vec4 a_color;\n"
"attribute vec2 a_uv;\n"
"attribute vec3 a_normal;\n"
"attribute mat4 a_model;\n"
"attribute mat3 a_normal_matrix;\n"
"varying vec4 v_color;\n"
"varying vec2 v_uv;\n"
"varying vec3 v_normal;\n"
"varying vec3 v_position;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main() {\n"
"  v_color = a_color;\n"
"  v_uv = a_uv;\n"
"  v_normal = normalize(a_normal_matrix * a_normal);\n"
"  vec4 world_pos = a_model * vec4(a_position, 1.0);\n"
"  vec4 view_pos = view * world_pos;\n"
"  v_position = world_pos.xyz/world_pos.w;\n"
"  gl_Position = projection * view_pos;\n"
"}\n";

const char* fragment_src = "#version 120\n"
"varying vec4 v_color;\n"
"varying vec2 v_uv;\n"
"varying vec3 v_normal;\n"
"void main() {\n"
"  gl_FragColor = v_color;\n"
"}\n";

void checkCompileError(GLuint shader, const char* type) {
  GLint success;
  char infoLog[512];
  if (strcmp(type, "PROGRAM") != 0) {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      printf("%s Shader compilation error:\n%s\n", type, infoLog);
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 512, NULL, infoLog);
      printf("Program linking error:\n%s\n", infoLog);
    }
  }
}

int compileShader(const char* vertex_src, const char* fragment_src){
  GLuint shaderProgram;
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertex_src, NULL);
  glCompileShader(vertexShader);
  checkCompileError(vertexShader, "VERTEX");
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragment_src, NULL);
  glCompileShader(fragmentShader);
  checkCompileError(fragmentShader, "FRAGMENT");
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  glBindAttribLocation(shaderProgram, LOC_POSITION,"a_position");
  glBindAttribLocation(shaderProgram, LOC_COLOR,   "a_color");
  glBindAttribLocation(shaderProgram, LOC_UV,      "a_uv");
  glBindAttribLocation(shaderProgram, LOC_NORMAL,  "a_normal");
  glBindAttribLocation(shaderProgram, LOC_MODEL,   "a_model");
  glBindAttribLocation(shaderProgram, LOC_NM,      "a_normal_matrix");

  glLinkProgram(shaderProgram);
  checkCompileError(shaderProgram, "PROGRAM");
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return shaderProgram;
}

void rdr_impl_init(uint64_t ctx){
  #ifndef __APPLE__
  glewInit();
  #endif
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo_zero);
  ARR_INIT(mesh_t,meshes);
  shader = compileShader(vertex_src,fragment_src);
  glEnable(GL_DEPTH_TEST);
  glEnable( GL_BLEND );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void rdr_impl_flush(){

  
}

void rdr_impl_background(float r, float g, float b, float a){
  glClearColor(r,g,b,a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void compute_bounding_sphere(float* out, float* vertices, int n_vertices){
  float mx = INFINITY;
  float my = INFINITY;
  float mz = INFINITY;
  float Mx =-INFINITY;
  float My =-INFINITY;
  float Mz =-INFINITY;
  for (int i = 0; i < n_vertices; i++){
    float x = vertices[i*3+0];
    float y = vertices[i*3+1];
    float z = vertices[i*3+2];
    mx = fmin(x,mx);
    my = fmin(y,my);
    mz = fmin(z,mz);
    Mx = fmax(x,Mx);
    My = fmax(y,My);
    Mz = fmax(z,Mz);
  }
  out[0] = (mx+Mx)*0.5;
  out[1] = (my+My)*0.5;
  out[2] = (mz+Mz)*0.5;
  float dx = Mx-out[0];
  float dy = My-out[1];
  float dz = Mz-out[2];
  out[3] = sqrt(dx*dx+dy*dy+dz*dz);
}

int rdr_impl__update_mesh(int mesh_id, int flags, int mode,
  float* vertices, int n_vertices,
  int32_t* indices, int n_indices,
  float* colors, int n_colors,
  float* uvs, int n_uvs,
  float* normals, int n_normals
){
  mesh_t mesh;
  int need_push = 0;
  if (mesh_id == -1) {
    glGenBuffers(1, &mesh.vbo_vertices);
    glGenBuffers(1, &mesh.vbo_colors);
    glGenBuffers(1, &mesh.vbo_uvs);
    glGenBuffers(1, &mesh.vbo_normals);
    glGenBuffers(1, &mesh.ebo_indices);

    glGenBuffers(1, &mesh.vbo_models);
    glGenBuffers(1, &mesh.vbo_normal_matrices);

    glGenVertexArrays(1, &mesh.VAO);  
    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_models);
    for (int i = 0; i < 4; i++) {
      glEnableVertexAttribArray(LOC_MODEL + i);
      glVertexAttribPointer(
        LOC_MODEL + i, 
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(float)*16,
        (void*)(i * sizeof(float)*4)
      );
      glVertexAttribDivisor(LOC_MODEL + i, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_normal_matrices);
    for (int i = 0; i < 3; i++) {
      glEnableVertexAttribArray(LOC_NM + i);
      glVertexAttribPointer(
        LOC_NM + i, 
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(float)*9,
        (void*)(i * sizeof(float)*3)
      );
      glVertexAttribDivisor(LOC_NM + i, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_vertices);
    glEnableVertexAttribArray(LOC_POSITION);
    glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);


    need_push = 1;
  } else {
    mesh = meshes.data[mesh_id];
    glBindVertexArray(mesh.VAO);
  }


  if ((flags & DIRTY_VERTICES) && vertices && n_vertices > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_vertices * 3, vertices, GL_STATIC_DRAW);
  }
  if (((flags & DIRTY_BOUNDS) || (flags & DIRTY_VERTICES)) && vertices && n_vertices > 0){
    compute_bounding_sphere(mesh.bsphere, vertices, n_vertices);
  }
  
  if ((flags & DIRTY_INDICES) && indices && n_indices > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int32_t) * n_indices, indices, GL_STATIC_DRAW);
  }

  if (flags & DIRTY_COLORS){
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_colors);
    if (colors && n_colors > 0) {
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_colors * 4, colors, GL_STATIC_DRAW);
      glEnableVertexAttribArray(LOC_COLOR);
      glVertexAttribPointer(LOC_COLOR, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }else{
      glDisableVertexAttribArray(LOC_COLOR);
      glVertexAttrib4f(LOC_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
    }
  }
  if (flags & DIRTY_UVS){
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_uvs);
    if (uvs && n_uvs > 0) {
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_uvs * 2, uvs, GL_STATIC_DRAW);
      glEnableVertexAttribArray(LOC_UV);
      glVertexAttribPointer(LOC_UV, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }else{
      glDisableVertexAttribArray(LOC_UV);
      glVertexAttrib2f(LOC_UV, 0.0f, 0.0f);
    }
  }
  if (flags & DIRTY_NORMALS){
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_normals);
    if (normals && n_normals > 0) {
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_normals * 3, normals, GL_STATIC_DRAW);
      glEnableVertexAttribArray(LOC_NORMAL);
      glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }else{
      glDisableVertexAttribArray(LOC_NORMAL);
      glVertexAttrib3f(LOC_NORMAL, 0.0f, 0.0f, 1.0f);
    }
  }

  if (need_push){
    mesh_id = meshes.len;
    ARR_PUSH(mesh_t,meshes,mesh);
  }
  meshes.data[mesh_id].n_vertices = n_vertices;
  meshes.data[mesh_id].n_indices = n_indices;
  meshes.data[mesh_id].n_normals = n_normals;
  meshes.data[mesh_id].n_uvs = n_uvs;
  meshes.data[mesh_id].n_colors = n_colors;

  glBindVertexArray(0);
  return mesh_id;
}

void rdr_impl__ortho(float* out, float left, float right, float bottom, float top, float nearZ, float farZ) {
  memset(out, 0, sizeof(float) * 16);
  float rl = 1.0f / (right - left);
  float tb = 1.0f / (top - bottom);
  float fn = 1.0f / (farZ - nearZ);
  out[0]  = 2.0f * rl;
  out[5]  = 2.0f * tb;
  out[10] = -2.0f * fn;
  out[3] = -(right + left) * rl;
  out[7] = -(top + bottom) * tb;
  out[11] = -(farZ + nearZ) * fn;
  out[15] = 1.0f;
}

void rdr_impl__perspective(float* out, float fov, float aspect, float nearZ, float farZ) {
  float tanHalfFov = tanf(fov / 2.0f);
  memset(out, 0, sizeof(float) * 16);
  out[0]  = 1.0f / (tanHalfFov * aspect);
  out[5]  = 1.0f / tanHalfFov;
  out[10] = - (farZ+nearZ) / (farZ-nearZ);
  out[14] = - 1.0f;
  out[11] = -(2.0f * farZ * nearZ) / (farZ-nearZ);
}

void rdr_impl__look_at(float* out, float* eye, float* center, float* up) {
  float f[3] = { center[0]-eye[0], center[1]-eye[1], center[2]-eye[2] };
  float fn = sqrtf(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
  for (int i = 0; i < 3; ++i) f[i] /= fn;
  float s[3] = {
    f[1]*up[2] - f[2]*up[1],
    f[2]*up[0] - f[0]*up[2],
    f[0]*up[1] - f[1]*up[0]
  };
  float sn = sqrtf(s[0]*s[0] + s[1]*s[1] + s[2]*s[2]);
  for (int i = 0; i < 3; ++i) s[i] /= sn;
  float u[3] = {
    s[1]*f[2] - s[2]*f[1],
    s[2]*f[0] - s[0]*f[2],
    s[0]*f[1] - s[1]*f[0]
  };
  memset(out, 0, sizeof(float) * 16);
  out[0] = out[5] = out[10] = out[15] = 1;

  out[0 ] = s[0];
  out[1 ] = s[1];
  out[2 ] = s[2];
  out[4 ] = u[0];
  out[5 ] = u[1];
  out[6 ] = u[2];
  out[8 ] =-f[0];
  out[9 ] =-f[1];
  out[10] =-f[2];
  out[3 ] =-(s[0]*eye[0]+s[1]*eye[1]+s[2]*eye[2]);
  out[7 ] =-(u[0]*eye[0]+u[1]*eye[1]+u[2]*eye[2]);
  out[11] = (f[0]*eye[0]+f[1]*eye[1]+f[2]*eye[2]);

}

void rdr_mat_impl_rotate(float* out, float* axis, float ang){
  float x = axis[0], y = axis[1], z = axis[2];
  float len = sqrtf(x * x + y * y + z * z);
  if (len == 0.0f) {
    for (int i = 0; i < 16; ++i)
      out[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    return;
  }
  x /= len;
  y /= len;
  z /= len;
  float c = cosf(ang);
  float s = sinf(ang);
  float t = 1.0f - c;
  out[0 ] = t*x*x + c;
  out[4 ] = t*x*y + s*z;
  out[8 ] = t*x*z - s*y;
  out[12] = 0.0f;
  out[1 ] = t*x*y - s*z;
  out[5 ] = t*y*y + c;
  out[9 ] = t*y*z + s*x;
  out[13]  = 0.0f;
  out[2 ] = t*x*z + s*y;
  out[6 ] = t*y*z - s*x;
  out[10] = t*z*z + c;
  out[14] = 0.0f;
  out[3 ] = 0.0f;
  out[7 ] = 0.0f;
  out[11] = 0.0f;
  out[15] = 1.0f;
}

void compute_normal_mat(float* out, const float* modelMatrix) {

  // float m[9] = {
  //   modelMatrix[0], modelMatrix[4], modelMatrix[8],
  //   modelMatrix[1], modelMatrix[5], modelMatrix[9],
  //   modelMatrix[2], modelMatrix[6], modelMatrix[10],
  // };
  float m[9] = {
    modelMatrix[0], modelMatrix[1], modelMatrix[2],
    modelMatrix[4], modelMatrix[5], modelMatrix[6],
    modelMatrix[8], modelMatrix[9], modelMatrix[10],
  };
  float a00 = m[0], a01 = m[3], a02 = m[6];
  float a10 = m[1], a11 = m[4], a12 = m[7];
  float a20 = m[2], a21 = m[5], a22 = m[8];
  float b01 = a22 * a11 - a12 * a21;
  float b11 = -a22 * a10 + a12 * a20;
  float b21 = a21 * a10 - a11 * a20;
  float det = a00 * b01 + a01 * b11 + a02 * b21;
  if (fabsf(det) < FLT_EPSILON){
    memcpy(out,m,sizeof(m));
    return;
  }
  float invDet = 1.0f / det;
  out[0] = b01 * invDet;
  out[1] = (-a22 * a01 + a02 * a21) * invDet;
  out[2] = (a12 * a01 - a02 * a11) * invDet;
  out[3] = b11 * invDet;
  out[4] = (a22 * a00 - a02 * a20) * invDet;
  out[5] = (-a12 * a00 + a02 * a10) * invDet;
  out[6] = b21 * invDet;
  out[7] = (-a21 * a00 + a01 * a20) * invDet;
  out[8] = (a11 * a00 - a01 * a10) * invDet;
}

void extract_frustum_planes(float* out, float* m){

  float p[6][4] = {
    { m[3*4+0]+m[0*4+0], m[3*4+1]+m[0*4+1], m[3*4+2]+m[0*4+2], m[3*4+3]+m[0*4+3] }, // left
    { m[3*4+0]-m[0*4+0], m[3*4+1]-m[0*4+1], m[3*4+2]-m[0*4+2], m[3*4+3]-m[0*4+3] }, // right
    { m[3*4+0]+m[1*4+0], m[3*4+1]+m[1*4+1], m[3*4+2]+m[1*4+2], m[3*4+3]+m[1*4+3] }, // bottom
    { m[3*4+0]-m[1*4+0], m[3*4+1]-m[1*4+1], m[3*4+2]-m[1*4+2], m[3*4+3]-m[1*4+3] }, // top
    { m[3*4+0]+m[2*4+0], m[3*4+1]+m[2*4+1], m[3*4+2]+m[2*4+2], m[3*4+3]+m[2*4+3] }, // near
    { m[3*4+0]-m[2*4+0], m[3*4+1]-m[2*4+1], m[3*4+2]-m[2*4+2], m[3*4+3]-m[2*4+3] }, // far
  };
  for (int i = 0; i < 6; i++) {
    float a = p[i][0], b = p[i][1], c = p[i][2];
    float inv = 1.0f / sqrtf(a*a + b*b + c*c);
    out[i*4+0] = a * inv;
    out[i*4+1] = b * inv;
    out[i*4+2] = c * inv;
    out[i*4+3] = p[i][3] * inv;
  }
}

GLint currentProgram = 0;
float cached_view[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float cached_proj[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float cached_frust_planes[24];
void rdr_impl__camera_begin(float* view, float* proj){

  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  if (!currentProgram){
    glUseProgram(shader);
  }
  GLint program = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);
  GLint loc_view = glGetUniformLocation(program, "view");
  if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_TRUE, view);
  GLint loc_proj = glGetUniformLocation(program, "projection");
  if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_TRUE, proj);
  memcpy(cached_view,view,sizeof(cached_view));
  memcpy(cached_proj,proj,sizeof(cached_proj));
  float vp[16] = MAT_MULT(proj,view);
  extract_frustum_planes(cached_frust_planes,vp);
}

void rdr_impl__camera_end(){
  if (!currentProgram){
    glUseProgram(0);
  }
  currentProgram = 0;
}


int sphere_test(float* bsphere, float* m, float* planes){
  float center[3] = MAT_TFRM(m, bsphere);
  float sx = m[0]*m[0]+m[1]*m[1]+m[2]*m[2];
  float sy = m[4]*m[4]+m[5]*m[5]+m[6]*m[6];
  float sz = m[8]*m[8]+m[9]*m[9]+m[10]*m[10];
  float radius = sqrt(fmax(sx,fmax(sy,sz))) * bsphere[3];
  for (int i = 0; i < 6; i++) {
    float dist =  planes[i*4+0]*center[0] +
                  planes[i*4+1]*center[1] +
                  planes[i*4+2]*center[2] +
                  planes[i*4+3];
    if (dist < - radius) return 0;
  }
  
  return 1;
}


void mat_copy_transpose(float* dst, float* src){
  dst[0]=src[0]; dst[1]=src[4], dst[2]=src[8] ; dst[3]=src[12];
  dst[4]=src[1]; dst[5]=src[5], dst[6]=src[9] ; dst[7]=src[13];
  dst[8]=src[2]; dst[9]=src[6], dst[10]=src[10]; dst[11]=src[14];
  dst[12]=src[3]; dst[13]=src[7], dst[14]=src[11]; dst[15]=src[15];
}

float* buf_nm = NULL;
float * buf_model = NULL;
int n_buf_mat = 0;

void rdr_impl__draw_instances(int mesh_id, int mode, int n_models, float* model_matrices) {
  
  mesh_t mesh = meshes.data[mesh_id];
  glBindVertexArray(mesh.VAO);
  GLint program = 0;

  glGetIntegerv(GL_CURRENT_PROGRAM, &program);

  if (n_models > n_buf_mat){
    buf_nm = realloc(buf_nm, 9*sizeof(float)*n_models);
    buf_model = realloc(buf_model, 16*sizeof(float)*n_models);
    n_buf_mat = n_models;
  }

  int n_draw = n_models;
  if (mode & CULL_FRUSTUM){
    n_draw = 0;
    for (int i = 0; i < n_models; i++){
      int vis = sphere_test(mesh.bsphere, model_matrices+(i*16), cached_frust_planes);
      if (vis){
        mat_copy_transpose(buf_model+(n_draw*16), model_matrices+(i*16));
        n_draw++;
      }
    }
  }else{
    for (int i = 0; i < n_models; i++){
      mat_copy_transpose(buf_model+(i*16), model_matrices+(i*16));
    }
  }

  for (int i = 0; i < n_draw; i++){
    compute_normal_mat(buf_nm+i*9, buf_model+i*16);
  }

  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_models);
  glBufferData(GL_ARRAY_BUFFER, n_draw*16*sizeof(float), buf_model, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_normal_matrices);
  glBufferData(GL_ARRAY_BUFFER, n_draw*9*sizeof(float), buf_nm, GL_STATIC_DRAW);

  if (mesh.n_indices){
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_indices);
    glDrawElementsInstanced(mode&0xf,mesh.n_indices,GL_UNSIGNED_INT,(void*)0,n_draw);
  }else{
    glDrawArraysInstanced(mode&0xf, 0, mesh.n_vertices,n_draw);
  }
  glBindVertexArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void rdr_impl__draw_mesh(int mesh_id, int mode, float* model_matrix) {

  rdr_impl__draw_instances(mesh_id,mode,1,model_matrix);
}

#define FONT_W 8
#define FONT_H 16
#define FONT_COLS 16
#define FONT_ROWS 8
#define FONT_N 128
#define FONT_TEX_W (FONT_COLS * FONT_W)
#define FONT_TEX_H (FONT_ROWS * FONT_H)
GLuint font_texture = -1;
GLuint text_vbo = 0;
GLuint text_uv_vbo = 0;
GLuint text_shader = 0;

uint8_t font_bitmap[FONT_N*FONT_H] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x08,0x08,0x00,0x00,
  0x00,0x00,0x22,0x22,0x22,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x12,0x12,0x12,0x7E,0x24,0x24,0x7E,0x48,0x48,0x48,0x00,0x00,
  0x00,0x00,0x00,0x00,0x08,0x3E,0x49,0x48,0x38,0x0E,0x09,0x49,0x3E,0x08,0x00,0x00,
  0x00,0x00,0x00,0x00,0x31,0x4A,0x4A,0x34,0x08,0x08,0x16,0x29,0x29,0x46,0x00,0x00,
  0x00,0x00,0x00,0x00,0x1C,0x22,0x22,0x14,0x18,0x29,0x45,0x42,0x46,0x39,0x00,0x00,
  0x00,0x00,0x08,0x08,0x08,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x04,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x08,0x08,0x04,0x00,
  0x00,0x00,0x00,0x20,0x10,0x10,0x08,0x08,0x08,0x08,0x08,0x08,0x10,0x10,0x20,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x49,0x2A,0x1C,0x2A,0x49,0x08,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x7F,0x08,0x08,0x08,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x08,0x08,0x10,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,
  0x00,0x00,0x00,0x00,0x02,0x02,0x04,0x08,0x08,0x10,0x10,0x20,0x40,0x40,0x00,0x00,
  0x00,0x00,0x00,0x00,0x18,0x24,0x42,0x46,0x4A,0x52,0x62,0x42,0x24,0x18,0x00,0x00,
  0x00,0x00,0x00,0x00,0x08,0x18,0x28,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x02,0x0C,0x10,0x20,0x40,0x40,0x7E,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x02,0x1C,0x02,0x02,0x42,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x04,0x0C,0x14,0x24,0x44,0x44,0x7E,0x04,0x04,0x04,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x7C,0x02,0x02,0x02,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x1C,0x20,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7E,0x02,0x02,0x04,0x04,0x04,0x08,0x08,0x08,0x08,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x3E,0x02,0x02,0x02,0x04,0x38,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x08,0x08,0x10,0x00,
  0x00,0x00,0x00,0x00,0x00,0x02,0x04,0x08,0x10,0x20,0x10,0x08,0x04,0x02,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x40,0x20,0x10,0x08,0x04,0x08,0x10,0x20,0x40,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x02,0x04,0x08,0x08,0x00,0x08,0x08,0x00,0x00,
  0x00,0x00,0x00,0x00,0x1C,0x22,0x4A,0x56,0x52,0x52,0x52,0x4E,0x20,0x1E,0x00,0x00,
  0x00,0x00,0x00,0x00,0x18,0x24,0x24,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x7C,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x40,0x40,0x40,0x40,0x42,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x78,0x44,0x42,0x42,0x42,0x42,0x42,0x42,0x44,0x78,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x7C,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x7C,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x40,0x40,0x4E,0x42,0x42,0x46,0x3A,0x00,0x00,
  0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3E,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
  0x00,0x00,0x00,0x00,0x1F,0x04,0x04,0x04,0x04,0x04,0x04,0x44,0x44,0x38,0x00,0x00,
  0x00,0x00,0x00,0x00,0x42,0x44,0x48,0x50,0x60,0x60,0x50,0x48,0x44,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,
  0x00,0x00,0x00,0x00,0x42,0x42,0x66,0x66,0x5A,0x5A,0x42,0x42,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x42,0x62,0x62,0x52,0x52,0x4A,0x4A,0x46,0x46,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x7C,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x5A,0x66,0x3C,0x03,0x00,
  0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x7C,0x48,0x44,0x44,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x40,0x30,0x0C,0x02,0x42,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7F,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x00,
  0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x41,0x41,0x41,0x22,0x22,0x22,0x14,0x14,0x08,0x08,0x00,0x00,
  0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x5A,0x5A,0x66,0x66,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x42,0x42,0x24,0x24,0x18,0x18,0x24,0x24,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x41,0x41,0x22,0x22,0x14,0x08,0x08,0x08,0x08,0x08,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7E,0x02,0x02,0x04,0x08,0x10,0x20,0x40,0x40,0x7E,0x00,0x00,
  0x00,0x00,0x00,0x0E,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x0E,0x00,
  0x00,0x00,0x00,0x00,0x40,0x40,0x20,0x10,0x10,0x08,0x08,0x04,0x02,0x02,0x00,0x00,
  0x00,0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x70,0x00,
  0x00,0x00,0x18,0x24,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,
  0x00,0x20,0x10,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x02,0x3E,0x42,0x42,0x46,0x3A,0x00,0x00,
  0x00,0x00,0x00,0x40,0x40,0x40,0x5C,0x62,0x42,0x42,0x42,0x42,0x62,0x5C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x40,0x40,0x40,0x40,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x02,0x02,0x02,0x3A,0x46,0x42,0x42,0x42,0x42,0x46,0x3A,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x0C,0x10,0x10,0x10,0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x02,0x3A,0x44,0x44,0x44,0x38,0x20,0x3C,0x42,0x42,0x3C,
  0x00,0x00,0x00,0x40,0x40,0x40,0x5C,0x62,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x08,0x08,0x00,0x18,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
  0x00,0x00,0x00,0x04,0x04,0x00,0x0C,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x48,0x30,
  0x00,0x00,0x00,0x40,0x40,0x40,0x44,0x48,0x50,0x60,0x50,0x48,0x44,0x42,0x00,0x00,
  0x00,0x00,0x00,0x18,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x76,0x49,0x49,0x49,0x49,0x49,0x49,0x49,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x5C,0x62,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x5C,0x62,0x42,0x42,0x42,0x42,0x62,0x5C,0x40,0x40,
  0x00,0x00,0x00,0x00,0x00,0x00,0x3A,0x46,0x42,0x42,0x42,0x42,0x46,0x3A,0x02,0x02,
  0x00,0x00,0x00,0x00,0x00,0x00,0x5C,0x62,0x42,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x40,0x30,0x0C,0x02,0x42,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x7C,0x10,0x10,0x10,0x10,0x10,0x0C,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x46,0x3A,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x24,0x24,0x24,0x18,0x18,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x49,0x49,0x49,0x49,0x49,0x49,0x36,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x24,0x18,0x18,0x24,0x42,0x42,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x26,0x1A,0x02,0x02,0x3C,
  0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x02,0x04,0x08,0x10,0x20,0x40,0x7E,0x00,0x00,
  0x00,0x00,0x00,0x0C,0x10,0x10,0x08,0x08,0x10,0x20,0x10,0x08,0x08,0x10,0x10,0x0C,
  0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
  0x00,0x00,0x00,0x30,0x08,0x08,0x10,0x10,0x08,0x04,0x08,0x10,0x10,0x08,0x08,0x30,
  0x00,0x00,0x00,0x31,0x49,0x46,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

const char* text_vertex_src = "#version 120\n"
"attribute vec3 a_position;\n"
"attribute vec4 a_color;\n"
"attribute vec2 a_uv;\n"
"attribute vec3 a_normal;\n"
"attribute mat4 a_model;\n"
"attribute mat3 a_normal_matrix;\n"
"varying vec2 v_uv;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main() {\n"
"  v_uv = a_uv;\n"
"  vec4 p = projection * view * a_model * vec4(a_position, 1.0);\n"
"  gl_Position = p;\n"
"}\n";

const char* text_fragment_src = "#version 120\n"
"varying vec2 v_uv;\n"
"uniform sampler2D font_atlas;\n"
"void main() {\n"
"  gl_FragColor = texture2D(font_atlas,v_uv);\n"
"}\n";


GLuint vao_text = 0;

void build_font_texture() {
  unsigned char tex_data[FONT_TEX_H][FONT_TEX_W];
  memset(tex_data, 0, FONT_TEX_W*FONT_TEX_H);
  for (int ch = 0; ch < FONT_N; ++ch) {
    int gx = (ch % FONT_COLS) * FONT_W;
    int gy = (ch / FONT_COLS) * FONT_H;
    for (int row = 0; row < FONT_H; ++row) {
      uint8_t bits = font_bitmap[ch*FONT_H+row];
      for (int col = 0; col < FONT_W; ++col) {
        if (bits & (1 << (7 - col))) {
          tex_data[gy + row][gx + col] = 255;
        }
      }
    }
  }
  glGenTextures(1, &font_texture);
  glBindTexture(GL_TEXTURE_2D, font_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, FONT_TEX_W, FONT_TEX_H, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tex_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenVertexArrays(1, &vao_text);
  glBindVertexArray(vao_text);

  glGenBuffers(1, &text_vbo);
  glGenBuffers(1, &text_uv_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, text_uv_vbo);
  glEnableVertexAttribArray(LOC_UV);
  glVertexAttribPointer(LOC_UV, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
  glEnableVertexAttribArray(LOC_POSITION);
  glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glDisableVertexAttribArray(LOC_COLOR);
  glVertexAttrib4f(LOC_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);

  glDisableVertexAttribArray(LOC_NORMAL);
  glVertexAttrib3f(LOC_NORMAL, 0.0f, 0.0f, 1.0f);

  glBindVertexArray(0);

  text_shader = compileShader(text_vertex_src,text_fragment_src);
}

void rdr_impl_text(char* str, float* model_matrix){
  if (font_texture == -1){
    build_font_texture();
  }
  GLint prev_prog = 0;
  GLint program = 0;


  glGetIntegerv(GL_CURRENT_PROGRAM, &prev_prog);

  if (prev_prog == 0 || prev_prog == shader){
    
    glUseProgram(text_shader);
    program = text_shader;
    GLint loc_view = glGetUniformLocation(program, "view");
    if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_TRUE, cached_view);
    GLint loc_proj = glGetUniformLocation(program, "projection");
    if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_TRUE, cached_proj);
  }else{
    program = prev_prog;
  }

  glBindVertexArray(vao_text);

  int len = strlen(str);

  float* vertices = malloc(len*18*sizeof(float));
  float* uvs = malloc(len*12*sizeof(float));

  for (int i = 0; i < len; i++) {
    unsigned char ch = str[i]-' ';
    int cx = ch % FONT_COLS;
    int cy = ch / FONT_COLS;
    float u0 = cx * (FONT_W / (float)FONT_TEX_W);
    float v0 = cy * (FONT_H / (float)FONT_TEX_H);
    float u1 = u0 + (FONT_W / (float)FONT_TEX_W);
    float v1 = v0 + (FONT_H / (float)FONT_TEX_H);
    float x0 = i * FONT_W;
    float y0 = 0;
    float x1 = x0 + FONT_W;
    float y1 = y0 + FONT_H;
    vertices[i*18+0]=x0;vertices[i*18+1]=y0;vertices[i*18+2]=0;
    vertices[i*18+3]=x1;vertices[i*18+4]=y0;vertices[i*18+5]=0;
    vertices[i*18+6]=x1;vertices[i*18+7]=y1;vertices[i*18+8]=0;
    vertices[i*18+9]=x0;vertices[i*18+10]=y0;vertices[i*18+11]=0;
    vertices[i*18+12]=x1;vertices[i*18+13]=y1;vertices[i*18+14]=0;
    vertices[i*18+15]=x0;vertices[i*18+16]=y1;vertices[i*18+17]=0;
    uvs[i*12+0]=u0;uvs[i*12+1]=v0;
    uvs[i*12+2]=u1;uvs[i*12+3]=v0;
    uvs[i*12+4]=u1;uvs[i*12+5]=v1;
    uvs[i*12+6]=u0;uvs[i*12+7]=v0;
    uvs[i*12+8]=u1;uvs[i*12+9]=v1;
    uvs[i*12+10]=u0;uvs[i*12+11]=v1;
  }

  glDisableVertexAttribArray(LOC_MODEL + 0);
  glVertexAttrib4f(LOC_MODEL + 0, model_matrix[0],model_matrix[4],model_matrix[8],model_matrix[12]);
  glDisableVertexAttribArray(LOC_MODEL + 1);
  glVertexAttrib4f(LOC_MODEL + 1, model_matrix[1],model_matrix[5],model_matrix[9],model_matrix[13]);
  glDisableVertexAttribArray(LOC_MODEL + 2);
  glVertexAttrib4f(LOC_MODEL + 2, model_matrix[2],model_matrix[6],model_matrix[10],model_matrix[14]);
  glDisableVertexAttribArray(LOC_MODEL + 3);
  glVertexAttrib4f(LOC_MODEL + 3, model_matrix[3],model_matrix[7],model_matrix[11],model_matrix[15]);

  float nm[9] = {1,0,0, 0,1,0, 0,0,1};
  for (int i = 0; i < 3; i++) {
    glDisableVertexAttribArray(LOC_NM + i);
    glVertexAttrib3fv(LOC_NM + i, &nm[i*4]);
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
  glBufferData(GL_ARRAY_BUFFER, len*18*sizeof(float), vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, text_uv_vbo);
  glBufferData(GL_ARRAY_BUFFER, len*12*sizeof(float), uvs, GL_DYNAMIC_DRAW);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font_texture);
  GLint tex_loc = glGetUniformLocation(program, "font_atlas");
  glUniform1i(tex_loc, 0);
  glDrawArrays(GL_TRIANGLES, 0, len*6);


  free(vertices);
  free(uvs);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(prev_prog);
}
