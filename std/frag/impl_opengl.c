//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL" || echo "-lGLEW -lGL")

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif defined(_WIN32)
#include <windows.h>
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
#define ARR_PUSH(dtype,name,item) \
  if (name.cap < name.len+1){ \
    int hs = name.cap/2; \
    name.cap = name.len+MAX(1,hs); \
    name.data = (dtype*)realloc(name.data, (name.cap)*sizeof(dtype) ); \
  }\
  name.data[name.len] = (dtype)item;\
  name.len += 1;

#undef ARR_POP
#define ARR_POP(dtype,name) (name.data[--name.len])

#undef ARR_CLEAR
#define ARR_CLEAR(dtype,name) {name.len = 0;}

GLint fbo_zero;

int tex_cnt = 0;

const char* vertexShaderSource = "#version 120\n"
"attribute vec3 a_position;\n"
"attribute vec4 a_color;\n"
"attribute vec2 a_uv;\n"
"attribute vec3 a_normal;\n"
"varying vec4 v_color;\n"
"varying vec2 v_uv;\n"
"varying vec3 v_normal;\n"
"varying vec3 v_position;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform mat3 normal_matrix;\n"
"void main() {\n"
"  v_color = a_color;\n"
"  v_uv = a_uv;\n"
"  v_normal = normalize(normal_matrix * a_normal);\n"
"  vec4 world_pos = model * vec4(a_position, 1.0);\n"
"  vec4 view_pos = view * world_pos;\n"
"  v_position = world_pos.xyz/world_pos.w;\n"
"  gl_Position = projection * view_pos;\n"
"}\n";

GLfloat vertices[12] = {
  -1.0f, -1.0f, 0.0,
   1.0f, -1.0f, 0.0,
  -1.0f,  1.0f, 0.0,
   1.0f,  1.0f, 0.0
};

int width,height;

void frag_impl_init(uint64_t ctx){
  #ifndef __APPLE__
  glewInit();
  #endif
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo_zero);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  width = viewport[2];
  height = viewport[3];
  
}

void frag_impl__init_texture(void* data, int w, int h){
  glEnable(GL_TEXTURE_2D);

  GLuint fbo, fboTexture;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glGenTextures(1, &fboTexture);
  glBindTexture(GL_TEXTURE_2D, fboTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  GLuint depthBuffer;
  glGenRenderbuffers(1, &depthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_zero);
  
  ((int32_t*)(data))[2] = fbo;
}

void checkCompileErrors(GLuint shader, const char* type) {
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

int frag_impl_program(const char* src){
  GLuint shaderProgram;

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo_zero);

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  checkCompileErrors(vertexShader, "VERTEX");
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &src, NULL);
  glCompileShader(fragmentShader);
  checkCompileErrors(fragmentShader, "FRAGMENT");
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  checkCompileErrors(shaderProgram, "PROGRAM");

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

int oldw = 0;
int oldh = 0;

void frag_impl__begin(int prgm, int fbo){
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  oldw = viewport[2];
  oldh = viewport[3];

  if (fbo == 0) fbo = fbo_zero;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  int w,h;
  if (fbo == 0){
    w = width;
    h = height;
  }else{
    int tex;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
  }
  glViewport(0,0,w,h);

  glUseProgram(prgm);
  tex_cnt = 0;
}

void frag_impl_render(){
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  GLint shader = 0;
  GLuint vbo, vbo_uvs;
  glGetIntegerv(GL_CURRENT_PROGRAM, &shader);
  
  float iden[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, iden);
  glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, iden);
  glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, iden);
  float nm[9] = {1,0,0, 0,1,0, 0,0,1};
  glUniformMatrix4fv(glGetUniformLocation(shader, "normal_matrix"), 1, GL_FALSE, nm);


  float uvs[8];
  for (int i = 0; i < 4; i++){
    uvs[i*2] = (vertices[i*3]+1.0)*0.5;
    uvs[i*2+1] = (vertices[i*3+1]+1.0)*0.5;
  }
  glGenBuffers(1, &vbo_uvs);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
  glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

  GLint loc_uv = glGetAttribLocation(shader, "a_uv");
  glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
  glEnableVertexAttribArray(loc_uv);
  glVertexAttribPointer(loc_uv, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  GLint loc_color = glGetAttribLocation(shader, "a_color");
  glDisableVertexAttribArray(loc_color);
  glVertexAttrib4f(loc_color, 1.0f, 1.0f, 1.0f, 1.0f);

  GLint loc_normal = glGetAttribLocation(shader, "a_normal");
  glDisableVertexAttribArray(loc_normal);
  glVertexAttrib3f(loc_normal, 0.0f, 0.0f, 1.0f);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  GLuint posAttrib = glGetAttribLocation(shader, "a_position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(posAttrib);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &vbo_uvs);
}

void frag_impl_end(){

  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_zero);
  
  if (oldw) glViewport(0,0,oldw,oldh);
}

void frag_impl_uniformf(const char* s, float* x, int n){
  GLint currentProgram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  GLint uniform = glGetUniformLocation(currentProgram, s);
  if (n == 1){
    glUniform1f(uniform, x[0]);
  }else if (n == 2){
    glUniform2f(uniform, x[0], x[1]);
  }else if (n == 3){
    glUniform3f(uniform, x[0], x[1], x[2]);
  }else if (n == 4){
    glUniform4f(uniform, x[0], x[1], x[2], x[3]);
  }else if (n == 9){
    glUniformMatrix3fv(uniform, 1, GL_TRUE, x);
  }else if (n == 16){
    glUniformMatrix4fv(uniform, 1, GL_TRUE, x);
  }
}

void frag_impl_uniformi(const char* s, int* x, int n){
  GLint currentProgram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  GLint uniform = glGetUniformLocation(currentProgram, s);
  if (n == 1){
    glUniform1i(uniform, x[0]);
  }else if (n == 2){
    glUniform2i(uniform, x[0], x[1]);
  }else if (n == 3){
    glUniform3i(uniform, x[0], x[1], x[2]);
  }else if (n == 4){
    glUniform4i(uniform, x[0], x[1], x[2], x[3]);
  }
}

void frag_impl_uniform_sampler(const char* name, int fbo){

  GLint currentProgram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

  GLint currentFbo;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  GLint tex = 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &tex);
  glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);

  int idx = tex_cnt;
  glActiveTexture(GL_TEXTURE0+idx);
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint texLoc = glGetUniformLocation(currentProgram, name);
  glUniform1i(texLoc, idx);

  tex_cnt++;
}

void frag_impl__write_pixels(int fbo, void* pixels){
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  GLint tex = 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &tex);

  int w,h;
  glBindTexture(GL_TEXTURE_2D, tex);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

  void* row = malloc(w*4);
  for (int y = 0; y < h / 2; y++) {
    void* top = (char*)pixels + y * w*4;
    void* bot = (char*)pixels + (h - 1 - y) * w*4;
    memcpy(row, top, w*4);
    memcpy(top, bot, w*4);
    memcpy(bot, row, w*4);
  }
  
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_zero);

  for (int y = 0; y < h / 2; y++) {
    void* top = (char*)pixels + y * w*4;
    void* bot = (char*)pixels + (h - 1 - y) * w*4;
    memcpy(row, top, w*4);
    memcpy(top, bot, w*4);
    memcpy(bot, row, w*4);
  }

  free(row);
}