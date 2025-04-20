//CFLAGS=$([ "$(uname)" == "Darwin" ] && echo "-framework OpenGL -I/opt/X11/include -L/opt/X11/lib -lX11 -lXext -lGl" || echo "-lGLEW -lGL -I/usr/include/X11 -L/usr/lib -lX11 -lXext")

#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glext.h>
#endif

#if !defined(_WIN32)
#ifndef CALLBACK
#define CALLBACK
#endif
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


#include <GL/glu.h>
#include <GL/glx.h>

int width;
int height;

Display *display;
Window root;
XVisualInfo *vi;
Colormap cmap;
XSetWindowAttributes swa;
Window win;
GLXContext glxContext;
XEvent event;
GLuint fontBase = -1;

typedef struct color_st{
  float r;
  float g;
  float b;
  float a;
}color_t;

color_t color_fill;
color_t color_stroke;

int is_stroke=1;
int is_fill=1;

void gx_impl_size(int w, int h){

  display = XOpenDisplay(NULL);
  root = DefaultRootWindow(display);
  static int visual_attribs[] = {
    GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_ALPHA_SIZE, 8,
    // GLX_SAMPLE_BUFFERS, 1,
    // GLX_SAMPLES, 4,
    None
  };
  vi = glXChooseVisual(display, 0, visual_attribs);
  // vi = glXChooseVisual(display, 0, (int[]) {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_ALPHA_SIZE, 8, None});
  cmap = XCreateColormap(display, root, vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;
  win = XCreateWindow(display, root, 0, 0, w, h, 0, vi->depth, InputOutput, vi->visual,
                      CWColormap | CWEventMask, &swa);

  glxContext = glXCreateContext(display, vi, NULL, GL_TRUE);
  glXMakeCurrent(display, win, glxContext);
  // glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // glEnable(GL_MULTISAMPLE);

  color_fill.r=1, color_fill.g=1, color_fill.b=1, color_fill.a=1;
  color_stroke.r=0, color_stroke.g=0, color_stroke.b=0, color_stroke.a=1;

  XMapWindow(display, win);
  width = w;
  height = h;

}

int get_key_code(){
  char keybuf[32];
  KeySym keysym;
  int len = XLookupString(&event.xkey, keybuf, sizeof(keybuf), &keysym, NULL);
  if (len == 1){
    return keybuf[0];
  }
  return (int)keysym;
}

int kig = 0; //defeat autorepeat

void gx_impl_poll(void* data){

  char keybuf[16];
  int n;
  if ((n = XPending(display)) > 0) {
    XNextEvent(display, &event);
    if (event.type == ConfigureNotify || event.type == MotionNotify){
      while (XEventsQueued(display, QueuedAfterReading)){
        XEvent nev;
        XPeekEvent(display, &nev);
        if (nev.type == event.type){
          XNextEvent(display, &event);
        }else{
          break;
        }
      }
    }
    if (event.type == ButtonPress) {
      ((int32_t*)(data))[2] = 1;
      ((int32_t*)(data))[3] = event.xbutton.button;
      ((float*  )(data))[4] = event.xbutton.x;
      ((float*  )(data))[5] = event.xbutton.y;
    }else if (event.type == ButtonRelease) {
      ((int32_t*)(data))[2] = 2;
      ((int32_t*)(data))[3] = event.xbutton.button;
      ((float*  )(data))[4] = event.xbutton.x;
      ((float*  )(data))[5] = event.xbutton.y;
    }else if (event.type == MotionNotify) {
      ((int32_t*)(data))[2] = 3;
      ((int32_t*)(data))[3] = 0;
      ((float*  )(data))[4] = event.xmotion.x;
      ((float*  )(data))[5] = event.xmotion.y;
    }else if (event.type == KeyPress) {
      if (kig){
        kig = 0;
      }else{
        ((int32_t*)(data))[2] = 4;
        ((int32_t*)(data))[3] = get_key_code();
        ((float*  )(data))[4] = event.xkey.x;
        ((float*  )(data))[5] = event.xkey.y;
      }
    }else if (event.type == KeyRelease) {
      if (XEventsQueued(display, QueuedAfterReading)){
        XEvent nev;
        XPeekEvent(display, &nev);
        if (nev.type == KeyPress && nev.xkey.time == event.xkey.time &&
            nev.xkey.keycode == event.xkey.keycode){
          kig = 1;
        }
      }
      if (!kig){
        ((int32_t*)(data))[2] = 5;
        ((int32_t*)(data))[3] = get_key_code();
        ((float*  )(data))[4] = event.xkey.x;
        ((float*  )(data))[5] = event.xkey.y;
      }
    }else if (event.type == ConfigureNotify){
      width = event.xconfigure.width;
      height = event.xconfigure.height;

      glViewport(0, 0, width, height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, width, height, 0, -1, 1);
      glMatrixMode(GL_MODELVIEW);
    }
  }

  glXSwapBuffers(display, win);

}


void gx_impl__init_graphics(void* data, int w, int h){

  glEnable(GL_TEXTURE_2D);

  GLuint fbo, fboTexture;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glGenTextures(1, &fboTexture);
  glBindTexture(GL_TEXTURE_2D, fboTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  ((int32_t*)(data))[2] = fbo;
  ((int32_t*)(data))[3] = fboTexture;
  ((int32_t*)(data))[4] = w;
  ((int32_t*)(data))[5] = h;

}

void gx_impl__begin_fbo(int fbo){

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  int w,h;
  GLint tex = 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
  glBindTexture(GL_TEXTURE_2D, 0);
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
}

void gx_impl__end_fbo(){
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, height, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
}

void gx_impl__draw_texture(int tex, float x, float y, float w, float h){
  // printf("%d %f %f %f %f\n", tex,x,y,w,h);
  // glClear(GL_COLOR_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_2D, tex);
  glEnable(GL_TEXTURE_2D);
  // glClearColor(1.0,1.0,1.0,1.0);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor4f(1,1,1,1);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 1.0f); glVertex2f(x,y);
  glTexCoord2f(1.0f, 1.0f); glVertex2f(x+w,y);
  glTexCoord2f(1.0f, 0.0f); glVertex2f(x+w,y+h);
  glTexCoord2f(0.0f, 0.0f); glVertex2f(x,y+h);
  glEnd();
  glDisable(GL_TEXTURE_2D);

}


void gx_impl_push_matrix(){
  glPushMatrix();
}
void gx_impl_pop_matrix(){
  glPopMatrix();
}
void gx_impl_rotate_deg(float ang){
  glRotatef(ang, 0.0f, 0.0f, 1.0f);
}
void gx_impl_translate(float x, float y){
  glTranslatef(x,y,0);
}
void gx_impl_scale(float x, float y){
  glScalef(x,y,1.0);
}
void gx_impl_apply_matrix(float* data){

  float m00 = ((float*)(data))[0];
  float m01 = ((float*)(data))[1];
  float m02 = ((float*)(data))[2];
  float m10 = ((float*)(data))[3];
  float m11 = ((float*)(data))[4];
  float m12 = ((float*)(data))[5];
  float m20 = ((float*)(data))[6];
  float m21 = ((float*)(data))[7];
  float m22 = ((float*)(data))[8];

  float mat[16] = {
    m00,m10,0,m20,
    m01,m11,0,m21,
    0,  0,  1,  0,
    m02,m12,0,m22,
  };
  glMultMatrixf(mat);
}
void gx_impl_reset_matrix(){
  glLoadIdentity();
}


void gx_impl_background(float r, float g, float b, float a){
  glClearColor(r,g,b,a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void gx_impl_fill(float r, float g, float b, float a){
  color_fill.r = r;
  color_fill.g = g;
  color_fill.b = b;
  color_fill.a = a;
  is_fill = 1;
}

void gx_impl_stroke(float r, float g, float b, float a){
  color_stroke.r = r;
  color_stroke.g = g;
  color_stroke.b = b;
  color_stroke.a = a;
  is_stroke = 1;
}

void gx_impl_no_fill(){
  is_fill = 0;
}
void gx_impl_no_stroke(){
  is_stroke = 0;
}
void gx_impl_stroke_weight(float x){
  glLineWidth(x);
  glPointSize(x);
}

typedef struct pt_st{
  double x;
  double y;
  double z;
  int64_t last;
} pt_t;

ARR_DEF(pt_t);
pt_t_arr_t poly = {0};


void gx_impl_begin_shape(){
  if (poly.cap){
    poly.len = 0;
  }else{
    ARR_INIT(pt_t,poly);
  }
}

void gx_impl_vertex(float x, float y){
  pt_t p;
  p.x = x;
  p.y = y;
  p.last = 0;
  ARR_PUSH(pt_t,poly,p);
}

void gx_impl_next_contour(int bclose){
  if (poly.len){
    poly.data[poly.len-1].last = (bclose<<1)|1;
  }
}

typedef struct db6_st{
  GLdouble data[6];
} db6_t;

ARR_DEF(db6_t)
db6_t_arr_t tcbv = {0};

ARR_DEF(GLfloat)
GLfloat_arr_t vertices = {0};

int tess_mode = 0;

void CALLBACK tessBeginCB(GLenum which){
  tess_mode = which;
  if (!vertices.cap){
    ARR_INIT(GLfloat,vertices)
  }else{
    vertices.len = 0;
  }
}
void CALLBACK tessEndCB(){
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, vertices.data);
  glDrawArrays(tess_mode, 0, vertices.len/2);
  glDisableClientState(GL_VERTEX_ARRAY);
}
void CALLBACK tessVertexCB(const GLvoid *data){
  const GLdouble *ptr = (const GLdouble*)data;
  GLfloat x = ptr[0];
  GLfloat y = ptr[1];
  ARR_PUSH(GLfloat,vertices,x);
  ARR_PUSH(GLfloat,vertices,y);
}
void CALLBACK tessVertexCB2(const GLvoid *data){
  const GLdouble *ptr = (const GLdouble*)data;
  GLfloat x = ptr[0];
  GLfloat y = ptr[1];
  ARR_PUSH(GLfloat,vertices,x);
  ARR_PUSH(GLfloat,vertices,y);
}
void CALLBACK tessCombineCB(const GLdouble newVtx[3], const GLdouble *nbrVtx[4], const GLfloat nbrWgh[4], GLdouble **outData){
  db6_t nv;
  int idx = tcbv.len;
  ARR_PUSH(db6_t, tcbv, nv);
  tcbv.data[idx].data[0] = newVtx[0];
  tcbv.data[idx].data[1] = newVtx[1];
  tcbv.data[idx].data[2] = newVtx[2];
  tcbv.data[idx].data[3] = nbrWgh[0] * nbrVtx[0][3] +  nbrWgh[1] * nbrVtx[1][3] + nbrWgh[2] * nbrVtx[2][3] + nbrWgh[3] * nbrVtx[3][3];
  tcbv.data[idx].data[4] = nbrWgh[0] * nbrVtx[0][4] +  nbrWgh[1] * nbrVtx[1][4] + nbrWgh[2] * nbrVtx[2][4] + nbrWgh[3] * nbrVtx[3][4];
  tcbv.data[idx].data[5] = nbrWgh[0] * nbrVtx[0][5] +  nbrWgh[1] * nbrVtx[1][5] + nbrWgh[2] * nbrVtx[2][5] + nbrWgh[3] * nbrVtx[3][5];
  *outData = tcbv.data[idx].data;
}
void CALLBACK tessErrorCB(GLenum errorCode){
  const GLubyte *errorStr;
  errorStr = gluErrorString(errorCode);
  printf("%s\n",errorStr);
}

void gx_impl_end_shape(int bclose){

  if (poly.len){
    poly.data[poly.len-1].last = (bclose<<1);
  }
  // glBegin(GL_TRIANGLES);
  // for (int i = 0; i < poly.len; i++){
  //   printf("%f %f\n",poly.data[i].x, poly.data[i].y);
  //   glVertex2f(poly.data[i].x, poly.data[i].y);
  // }
  // glEnd();
  // printf("-----\n");

  if (is_fill && poly.len > 2 && color_fill.a){
    glColor4f(color_fill.r, color_fill.g, color_fill.b, color_fill.a);

    if (poly.len > 3 || poly.data[0].last || poly.data[1].last){
      if (!tcbv.cap){
        ARR_INIT(db6_t,tcbv)
      }else{
        tcbv.len = 0;
      }
      GLUtesselator * tess = gluNewTess();
      gluTessCallback(tess, GLU_TESS_BEGIN,  (void (CALLBACK*)())tessBeginCB);
      gluTessCallback(tess, GLU_TESS_END,    (void (CALLBACK*)())tessEndCB);
      gluTessCallback(tess, GLU_TESS_ERROR,  (void (CALLBACK*)())tessErrorCB);
      gluTessCallback(tess, GLU_TESS_VERTEX, (void (CALLBACK*)())tessVertexCB);
      gluTessCallback(tess, GLU_TESS_COMBINE,(void (CALLBACK*)())tessCombineCB);

      gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

      gluTessBeginPolygon(tess, 0);
      gluTessBeginContour(tess);
      for (int i = 0; i < poly.len; i++){
        gluTessVertex(tess, (double*)(poly.data+i), (double*)(poly.data+i));
        if (poly.data[i].last & 1){
          gluTessEndContour(tess);
          gluTessBeginContour(tess);
        }
      }
      gluTessEndContour(tess);
      gluTessEndPolygon(tess);
      gluDeleteTess(tess);
    }else if (poly.len == 3){
      GLfloat vs[] = {
        poly.data[0].x,poly.data[0].y,
        poly.data[1].x,poly.data[1].y,
        poly.data[2].x,poly.data[2].y,
      };
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(2, GL_FLOAT, 0, vs);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDisableClientState(GL_VERTEX_ARRAY);
    }
  }

  if (is_stroke && color_stroke.a){
    glColor4f(color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_DOUBLE, sizeof(pt_t), poly.data);
    int start = 0;
    for (int i = 0; i < poly.len; i++){
      if (poly.data[i].last || i == poly.len-1){
        glDrawArrays((poly.data[i].last & 2 )?GL_LINE_LOOP:GL_LINE_STRIP, start, i+1-start);
        start = i+1;
      }
    }
    glDisableClientState(GL_VERTEX_ARRAY);

  }
}

#define ELLIPSE_DETAIL 32
float lvs[ELLIPSE_DETAIL][2];

void gx_impl_line(float x0, float y0, float x1, float y1){
  lvs[1][1] = y1;
  lvs[1][0] = x1;
  lvs[0][1] = y0;
  lvs[0][0] = x0;

  if (is_stroke && color_stroke.a){
    glColor4f(color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, lvs);
    glDrawArrays(GL_LINES,0,4);
    glDisableClientState(GL_VERTEX_ARRAY);
  }
}


float unit_circle[ELLIPSE_DETAIL][2];
int did_unit_circle = 0;

void gx_impl_ellipse(float x, float y, float w, float h){

  if (!did_unit_circle){
    did_unit_circle = 1;
    unit_circle[0][0] = 0;
    unit_circle[0][1] = 0;
    for (int i = 1; i < ELLIPSE_DETAIL; i++){
      float a = (float)(i-1)/(ELLIPSE_DETAIL-2) * M_PI*2;
      float x = cos(a);
      float y = sin(a);
      unit_circle[i][0] = x;
      unit_circle[i][1] = y;
    }
  }
  
  for (int i = 0; i < ELLIPSE_DETAIL; i++){
    lvs[i][0] = unit_circle[i][0]*w*0.5+x;
    lvs[i][1] = unit_circle[i][1]*h*0.5+y;
  }
  glEnableClientState(GL_VERTEX_ARRAY);
  if (is_fill && color_fill.a){
    glColor4f(color_fill.r, color_fill.g, color_fill.b, color_fill.a);
    glVertexPointer(2, GL_FLOAT, 0, lvs);
    glDrawArrays(GL_TRIANGLE_FAN,0,ELLIPSE_DETAIL);
  }
  if (is_stroke && color_stroke.a){
    glColor4f(color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a);
    glVertexPointer(2, GL_FLOAT, 0, lvs);
    glDrawArrays(GL_LINE_LOOP,1,ELLIPSE_DETAIL-2);
  }
  glDisableClientState(GL_VERTEX_ARRAY);
}

void gx_impl_rect(float x, float y, float w, float h){

  lvs[0][0] = x;
  lvs[0][1] = y;
  lvs[1][0] = x+w;
  lvs[1][1] = y;
  lvs[2][0] = x+w;
  lvs[2][1] = y+h;
  lvs[3][0] = x;
  lvs[3][1] = y+h;

  glEnableClientState(GL_VERTEX_ARRAY);
  if (is_fill && color_fill.a){
    glColor4f(color_fill.r, color_fill.g, color_fill.b, color_fill.a);
    glVertexPointer(2, GL_FLOAT, 0, lvs);
    glDrawArrays(GL_QUADS,0,4);
  }
  if (is_stroke && color_stroke.a){
    glColor4f(color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a);
    glVertexPointer(2, GL_FLOAT, 0, lvs);
    glDrawArrays(GL_LINE_LOOP,0,4);
  }
  glDisableClientState(GL_VERTEX_ARRAY);
}

void gx_impl_point(float x, float y){
  lvs[0][1] = y;
  lvs[0][0] = x;

  if (is_stroke && color_stroke.a){
    glEnableClientState(GL_VERTEX_ARRAY);
    glColor4f(color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a);
    glVertexPointer(2, GL_FLOAT, 0, lvs);
    glDrawArrays(GL_POINTS,0,1);
    glDisableClientState(GL_VERTEX_ARRAY);
  }
  
}


void gx_impl_text(char* s, float x, float y){

  if (is_fill && color_fill.a){
    if (fontBase == -1){
      fontBase = glGenLists(96);
      XFontStruct *font = XLoadQueryFont(display, "fixed");
      glXUseXFont(font->fid, 32, 96, fontBase);
      XFreeFont(display, font);
    }
    glColor4f(color_fill.r, color_fill.g, color_fill.b, color_fill.a);
    glRasterPos2f(x, y);
    glPushAttrib(GL_LIST_BIT);
    glListBase(fontBase - 32);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, s);
    glPopAttrib();
  }
  
}


int gx_impl_load_font(char* s){

  int fb = glGenLists(96);
  XFontStruct *font = XLoadQueryFont(display, s);
  glXUseXFont(font->fid, 32, 96, fb);
  XFreeFont(display, font);

  return fb;
}

void gx_impl_text_font(int x){
  fontBase = x;
}
