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
#elif defined(_WIN32)
#include <gl/GL.h>
#include <gl/GLU.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#include "../win/platform/wgl_patcher.h"
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

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

GLint fbo_zero;

typedef struct vao_st {
  GLuint vao;
  GLuint vbo_vertices;
  GLuint vbo_colors;
  GLuint vbo_uvs;
  GLuint vbo_normals;
  GLuint ebo_indices;
  int n_indices;
  int n_colors;
} vao_t;

ARR_DEF(vao_t);
vao_t_arr_t vaos;

GLuint shader = 0;

char* vertex_src = "#version 120\n"
"attribute vec3 a_position;\n"
"attribute vec4 a_color;\n"
"attribute vec2 a_uv;\n"
"attribute vec3 a_normal;\n"
"varying vec4 v_color;\n"
"varying vec2 v_uv;\n"
"varying vec3 v_normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform mat3 normal_matrix;\n"
"void main() {\n"
"  v_color = a_color;\n"
"  v_uv = a_uv;\n"
"  v_normal = normalize(normal_matrix * a_normal);\n"
"  gl_Position = projection*view*model*vec4(a_position, 1.0);\n"
"}\n";

char* fragment_src = "#version 120\n"
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

int compileShader(char* vertex_src, char* fragment_src){
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
  glLinkProgram(shaderProgram);
  checkCompileError(shaderProgram, "PROGRAM");
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return shaderProgram;
}

void g3d_impl_init(uint64_t ctx){
  #ifndef __APPLE__
  glewInit();
  #endif
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo_zero);
  ARR_INIT(vao_t,vaos);
  shader = compileShader(vertex_src,fragment_src);
  glEnable(GL_DEPTH_TEST);
}

void g3d_impl_flush(){

}

void g3d_impl_background(float r, float g, float b, float a){
  glClearColor(r,g,b,a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int g3d_impl__update_mesh(int vao, int flags,
  float* vertices, int n_vertices,
  int32_t* indices, int n_indices,
  float* colors, int n_colors,
  float* uvs, int n_uvs,
  float* normals, int n_normals
){
  vao_t mesh;
  if (vao == -1) {
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    glGenBuffers(1, &mesh.vbo_vertices);
    glGenBuffers(1, &mesh.vbo_colors);
    glGenBuffers(1, &mesh.vbo_uvs);
    glGenBuffers(1, &mesh.vbo_normals);
    glGenBuffers(1, &mesh.ebo_indices);
    vao = vaos.len;
    ARR_PUSH(vao_t,vaos,mesh);
  } else {
    mesh = vaos.data[vao];
    glBindVertexArray(mesh.vao);
  }
  if ((flags & DIRTY_VERTICES) && vertices && n_vertices > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_vertices * 3, vertices, GL_STATIC_DRAW);
  }
  if ((flags & DIRTY_INDICES) && indices && n_indices > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int32_t) * n_indices, indices, GL_STATIC_DRAW);
  }
  if ((flags & DIRTY_COLORS) && colors && n_colors > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_colors * 4, colors, GL_STATIC_DRAW);
  }
  if ((flags & DIRTY_UVS) && uvs && n_uvs > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_uvs);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_uvs * 2, uvs, GL_STATIC_DRAW);
  }
  if ((flags & DIRTY_NORMALS) && normals && n_normals > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_normals * 3, normals, GL_STATIC_DRAW);
  }
  glBindVertexArray(0);

  vaos.data[vao].n_indices = n_indices;
  vaos.data[vao].n_colors = n_colors;
  return vao;
}

void g3d_impl__perspective(float* out, float fov, float aspect, float nearZ, float farZ) {
  float tanHalfFov = tanf(fov / 2.0f);
  float zRange = nearZ - farZ;
  memset(out, 0, sizeof(float) * 16);
  out[0]  = 1.0f / (tanHalfFov * aspect);
  out[5]  = 1.0f / tanHalfFov;
  out[10] = - (farZ+nearZ) / (farZ-nearZ);
  out[11] = - 1.0f;
  out[14] = -(2.0f * farZ * nearZ) / (farZ-nearZ);
}

void g3d_impl__look_at(float* out, float* eye, float* center, float* up) {
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
  out[10] = 1;
  out[15] = 1;
  out[0*4+0] = s[0];
  out[1*4+0] = s[1];
  out[2*4+0] = s[2];
  out[0*4+1] = u[0];
  out[1*4+1] = u[1];
  out[2*4+1] = u[2];
  out[0*4+2] =-f[0];
  out[1*4+2] =-f[1];
  out[2*4+2] =-f[2];
  out[3*4+0] =-(s[0]*eye[0]+s[1]*eye[1]+s[2]*eye[2]);
  out[3*4+1] =-(u[0]*eye[0]+u[1]*eye[1]+u[2]*eye[2]);
  out[3*4+2] = (f[0]*eye[0]+f[1]*eye[1]+f[2]*eye[2]);
}

void g3d_mat_impl_rotate(float* out, float* axis, float ang){
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
  out[0]  = t*x*x + c;
  out[1]  = t*x*y + s*z;
  out[2]  = t*x*z - s*y;
  out[3]  = 0.0f;
  out[4]  = t*x*y - s*z;
  out[5]  = t*y*y + c;
  out[6]  = t*y*z + s*x;
  out[7]  = 0.0f;
  out[8]  = t*x*z + s*y;
  out[9]  = t*y*z - s*x;
  out[10] = t*z*z + c;
  out[11] = 0.0f;
  out[12] = 0.0f;
  out[13] = 0.0f;
  out[14] = 0.0f;
  out[15] = 1.0f;
  // memset(out, 0, sizeof(float) * 16);
  // out[0] = out[5] = out[10] = out[15] = 1;
}

void compute_normal_mat(float* out, const float* modelMatrix) {
  float m[9] = {
    modelMatrix[0], modelMatrix[1], modelMatrix[2],
    modelMatrix[4], modelMatrix[5], modelMatrix[6],
    modelMatrix[8], modelMatrix[9], modelMatrix[10],
  };
  float inv[9];
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
GLint currentProgram = 0;
void g3d_impl__camera_begin(float* view, float* proj){
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  if (!currentProgram){
    glUseProgram(shader);
  }
  GLint program = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);
  GLint loc_view = glGetUniformLocation(program, "view");
  if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_FALSE, view);
  GLint loc_proj = glGetUniformLocation(program, "projection");
  if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_FALSE, proj);
}

void g3d_impl__camera_end(){
  if (!currentProgram){
    glUseProgram(0);
  }
  currentProgram = 0;
}

void g3d_impl__draw_mesh(int vao, int mode, float* model_matrix) {
  GLint program = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);

  vao_t mesh = vaos.data[vao];
  glBindVertexArray(mesh.vao);
  
  GLint loc_model = glGetUniformLocation(program, "model");
  if (loc_model >= 0) glUniformMatrix4fv(loc_model, 1, GL_FALSE, model_matrix);

  float nm[9] = {1,0,0, 0,1,0, 0,0,1};
  compute_normal_mat(nm,model_matrix);
  // printf("%f %f %f\n%f %f %f\n%f %f %f\n\n",nm[0],nm[1],nm[2],nm[3],nm[4],nm[5],nm[6],nm[7],nm[8]);
  GLint loc_nm = glGetUniformLocation(program, "normal_matrix");
  if (loc_nm >= 0) glUniformMatrix3fv(loc_nm, 1, GL_FALSE, nm);

  GLint loc_pos = glGetAttribLocation(program, "a_position");
  if (loc_pos >= 0) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_vertices);
    glEnableVertexAttribArray(loc_pos);
    glVertexAttribPointer(loc_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  }
  GLint loc_norm = glGetAttribLocation(program, "a_normal");
  if (loc_norm >= 0 && mesh.vbo_normals) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_normals);
    glEnableVertexAttribArray(loc_norm);
    glVertexAttribPointer(loc_norm, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  }
  GLint loc_uv = glGetAttribLocation(program, "a_uv");
  if (loc_uv >= 0 && mesh.vbo_uvs) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_uvs);
    glEnableVertexAttribArray(loc_uv);
    glVertexAttribPointer(loc_uv, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  }
  GLint loc_color = glGetAttribLocation(program, "a_color");
  if (loc_color >= 0 && mesh.vbo_colors) {
    if (mesh.n_colors){
      glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_colors);
      glEnableVertexAttribArray(loc_color);
      glVertexAttribPointer(loc_color, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }else{
      glDisableVertexAttribArray(loc_color);
      glVertexAttrib4f(loc_color, 1.0f, 1.0f, 1.0f, 1.0f);
    }
  }
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_indices);
  glDrawElements(mode, mesh.n_indices, GL_UNSIGNED_INT, 0);
  if (loc_pos >= 0)   glDisableVertexAttribArray(loc_pos);
  if (loc_norm >= 0)  glDisableVertexAttribArray(loc_norm);
  if (loc_uv >= 0)    glDisableVertexAttribArray(loc_uv);
  if (loc_color >= 0) glDisableVertexAttribArray(loc_color);
  glBindVertexArray(0);


}
