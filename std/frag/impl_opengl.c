//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL" || echo "-lGLEW -lGL")

#include <math.h>
#include <string.h>
#include <stdio.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
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

int width;
int height;
GLint fbo_zero;

int tex_cnt = 0;

const char* vertexShaderSource = "#version 120\n"
"attribute vec2 aPosition;\n"
"varying vec2 vUv;\n"
"void main() {\n"
"  vUv = (aPosition + 1.0) * 0.5;\n"
"  gl_Position = vec4(aPosition, 0.0, 1.0);\n"
"}\n";

GLfloat vertices[] = {
  -1.0f, -1.0f,
   1.0f, -1.0f,
  -1.0f,  1.0f,
   1.0f,  1.0f
};

GLint posAttrib;
GLuint vbo;


void frag_impl__size(int w, int h, uint64_t ctx){
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo_zero);
  width = w;
  height = h;
}

void frag_impl__init_texture(void* data){
  glEnable(GL_TEXTURE_2D);

  GLuint fbo, fboTexture;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glGenTextures(1, &fboTexture);
  glBindTexture(GL_TEXTURE_2D, fboTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
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

void frag_impl__begin(int prgm, int fbo){

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(prgm);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  posAttrib = glGetAttribLocation(prgm, "aPosition");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
  tex_cnt = 0;
}

void frag_impl_end(){
  GLint currentFbo;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(posAttrib);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_zero);
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