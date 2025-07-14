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
#include <gl/GL.h>
#include <gl/GLU.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_TEXTURE0 0x84C0
typedef ptrdiff_t GLsizeiptr;
#ifndef GLchar
typedef char GLchar;
#endif
typedef void (APIENTRYP PFNGLGENBUFFERSPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC)(GLenum, GLuint);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC)(GLenum, GLsizeiptr, const void*, GLenum);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC)(GLsizei, const GLuint*);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC)(GLenum);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar**, const GLint*);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC)(GLuint, GLenum, GLint*);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC)(GLuint, GLuint);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC)(GLuint);
typedef void (APIENTRYP PFNGLDELETESHADERPROC)(GLuint);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFERPROC)(GLenum, GLuint);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum);
typedef GLint (APIENTRYP PFNGLGETATTRIBLOCATIONPROC)(GLuint, const GLchar*);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
typedef void (APIENTRYP PFNGLUNIFORM1FPROC)(GLint, GLfloat);
typedef void (APIENTRYP PFNGLUNIFORM2FPROC)(GLint, GLfloat, GLfloat);
typedef void (APIENTRYP PFNGLUNIFORM3FPROC)(GLint, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRYP PFNGLUNIFORM4FPROC)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRYP PFNGLUNIFORM1IPROC)(GLint, GLint);
typedef void (APIENTRYP PFNGLUNIFORM2IPROC)(GLint, GLint, GLint);
typedef void (APIENTRYP PFNGLUNIFORM3IPROC)(GLint, GLint, GLint, GLint);
typedef void (APIENTRYP PFNGLUNIFORM4IPROC)(GLint, GLint, GLint, GLint, GLint);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC)(GLenum);
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)(GLenum, GLenum, GLenum, GLint*);
static PFNGLGENBUFFERSPROC glGenBuffers = NULL;
static PFNGLBINDBUFFERPROC glBindBuffer = NULL;
static PFNGLBUFFERDATAPROC glBufferData = NULL;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
static PFNGLCREATESHADERPROC glCreateShader = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
static PFNGLATTACHSHADERPROC glAttachShader = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
static PFNGLDELETESHADERPROC glDeleteShader = NULL;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
static PFNGLUNIFORM1FPROC glUniform1f = NULL;
static PFNGLUNIFORM2FPROC glUniform2f = NULL;
static PFNGLUNIFORM3FPROC glUniform3f = NULL;
static PFNGLUNIFORM4FPROC glUniform4f = NULL;
static PFNGLUNIFORM1IPROC glUniform1i = NULL;
static PFNGLUNIFORM2IPROC glUniform2i = NULL;
static PFNGLUNIFORM3IPROC glUniform3i = NULL;
static PFNGLUNIFORM4IPROC glUniform4i = NULL;
static PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv = NULL;
static void glewInit(void) {
  glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
  glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
  glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
  glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
  glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
  glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
  glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
  glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
  glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
  glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
  glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
  glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
  glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
  glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
  glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
  glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
  glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
  glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
  glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
  glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
  glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
  glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
  glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
  glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
  glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
  glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
  glUniform2i = (PFNGLUNIFORM2IPROC)wglGetProcAddress("glUniform2i");
  glUniform3i = (PFNGLUNIFORM3IPROC)wglGetProcAddress("glUniform3i");
  glUniform4i = (PFNGLUNIFORM4IPROC)wglGetProcAddress("glUniform4i");
  glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
}
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
"attribute vec2 a_position;\n"
"varying vec2 v_uv;\n"
"void main() {\n"
"  v_uv = (a_position + 1.0) * 0.5;\n"
"  gl_Position = vec4(a_position, 0.0, 1.0);\n"
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
  #ifndef __APPLE__
  glewInit();
  #endif
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
  if (fbo == 0) fbo = fbo_zero;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(prgm);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  posAttrib = glGetAttribLocation(prgm, "a_position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
  tex_cnt = 0;
}

void frag_impl_end(){

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(posAttrib);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);

  glUseProgram(0);
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