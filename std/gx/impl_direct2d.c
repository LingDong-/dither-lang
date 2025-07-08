#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <guiddef.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#define __CRT_UUID_DECL(type,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)
// #define __CRT_UUID_DECL(type,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const GUID IID_ ## type = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}};
#define WINBOOL long
#define _D2D1_H_
#include "../../third_party/d2d1.h"
#include "../../third_party/dwrite.h"

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

ARR_DEF(D2D1_MATRIX_3X2_F)
D2D1_MATRIX_3X2_F_arr_t matrices;


int width;
int height;

int is_stroke=1;
int is_fill=1;
float line_width = 1;

typedef ID2D1BitmapRenderTarget* ID2D1BitmapRenderTargetPtr;

ARR_DEF(ID2D1BitmapRenderTargetPtr)
ID2D1BitmapRenderTargetPtr_arr_t fbos;

D2D1_COLOR_F color_fill = {1.0f, 1.0f, 1.0f, 1.0f};
D2D1_COLOR_F color_stroke = {0.0f, 0.0f, 0.0f, 1.0f};

HWND hwnd;

ID2D1Factory* pFactory = NULL;
ID2D1HwndRenderTarget* pRenderTarget = NULL;
ID2D1SolidColorBrush* pBrush = NULL;

IDWriteFactory* writeFactory = NULL;
IDWriteTextFormat* textFormat = NULL;

ID2D1PathGeometry* geometry = NULL;
ID2D1GeometrySink* sink = NULL;

void gx_impl__size(int w, int h, uint64_t _hwnd){

  hwnd = (HWND)(void*)(uintptr_t)_hwnd;
  width = w;
  height = h;

  // printf("%p\n",hwnd);
  D2D1CreateFactory(
    D2D1_FACTORY_TYPE_SINGLE_THREADED,
    &IID_ID2D1Factory,
    NULL,
    (void**)&pFactory
  );

  RECT rc;
  GetClientRect(hwnd, &rc);

  D2D1_RENDER_TARGET_PROPERTIES props = {
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE },
      0.0f,
      0.0f,
      D2D1_RENDER_TARGET_USAGE_NONE,
      D2D1_FEATURE_LEVEL_DEFAULT
  };

  D2D1_SIZE_U size = { (UINT)(rc.right - rc.left), (UINT)(rc.bottom - rc.top) };

  D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = {
    hwnd,
    size,
    D2D1_PRESENT_OPTIONS_NONE
  };

  ID2D1Factory_CreateHwndRenderTarget(
    pFactory,
    &props,
    &hwndProps,
    &pRenderTarget
  );

  DWriteCreateFactory(
    DWRITE_FACTORY_TYPE_SHARED,
    &IID_IDWriteFactory,
    (IUnknown**)&writeFactory
  );
  
  IDWriteFactory_CreateTextFormat(
    writeFactory,
    L"Courier New",
    NULL,
    DWRITE_FONT_WEIGHT_NORMAL,
    DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL,
    15.0f,
    L"en-us",
    &textFormat
  );

  ID2D1HwndRenderTarget_CreateSolidColorBrush(pRenderTarget, &color_stroke, NULL, &pBrush);
  ID2D1HwndRenderTarget_BeginDraw(pRenderTarget);
}

void gx_impl__flush(){
  ID2D1HwndRenderTarget_EndDraw(pRenderTarget,NULL,NULL);
  ID2D1HwndRenderTarget_BeginDraw(pRenderTarget);    
}

void gx_impl__init_graphics(void* data, int w, int h){
  ID2D1BitmapRenderTargetPtr offscreen = NULL;
  D2D1_SIZE_U pixelSize = {w,h};
  ID2D1HwndRenderTarget_CreateCompatibleRenderTarget(
    pRenderTarget, NULL, &pixelSize, NULL, D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE, &offscreen
  );
  ARR_PUSH(ID2D1BitmapRenderTargetPtr,fbos,offscreen);
  ((int32_t*)(data))[2] = fbos.len-1;
  ((int32_t*)(data))[3] = w;
  ((int32_t*)(data))[4] = h;
}

int cur_fbo = -1;
void gx_impl__begin_fbo(int fbo){
  cur_fbo = fbo;
  ID2D1BitmapRenderTargetPtr offscreen = fbos.data[fbo];
  ID2D1BitmapRenderTarget_BeginDraw(offscreen);
}

void gx_impl__end_fbo(){
  ID2D1BitmapRenderTargetPtr offscreen = fbos.data[cur_fbo];
  ID2D1BitmapRenderTarget_EndDraw(offscreen,NULL,NULL);
  cur_fbo = -1;
}

void* gx_impl__read_pixels(int fbo, int* _w, int* _h){
  return NULL;
}


void gx_impl__write_pixels(int fbo, void* pixels){

}

void gx_impl__draw_texture(int fbo, float x, float y, float w, float h){
  ID2D1BitmapRenderTargetPtr offscreen = fbos.data[fbo];
  ID2D1Bitmap* bitmap = NULL;
  ID2D1BitmapRenderTarget_GetBitmap(offscreen, &bitmap);
  D2D1_RECT_F rect = {x,y,x+w,y+h};
  // pRenderTarget->lpVtbl->Base.DrawBitmap(pRenderTarget, bitmap, &rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);
}


void gx_impl_push_matrix(){
  D2D1_MATRIX_3X2_F m;
  ID2D1HwndRenderTarget_GetTransform(pRenderTarget,&m);
  ARR_PUSH(D2D1_MATRIX_3X2_F,matrices,m);
}
void gx_impl_pop_matrix(){
  D2D1_MATRIX_3X2_F m = ARR_POP(D2D1_MATRIX_3X2_F,matrices);
  ID2D1HwndRenderTarget_SetTransform(pRenderTarget,&m);
}

void matrixMultiply(
    D2D1_MATRIX_3X2_F *out,
    const D2D1_MATRIX_3X2_F *a,
    const D2D1_MATRIX_3X2_F *b
) {
    out->m11 = a->m11 * b->m11 + a->m12 * b->m21;
    out->m12 = a->m11 * b->m12 + a->m12 * b->m22;
    out->m21 = a->m21 * b->m11 + a->m22 * b->m21;
    out->m22 = a->m21 * b->m12 + a->m22 * b->m22;
    out->dx  = a->dx  * b->m11 + a->dy  * b->m21 + b->dx;
    out->dy  = a->dx  * b->m12 + a->dy  * b->m22 + b->dy;
}

void gx_impl_rotate_deg(float ang){
  D2D1_MATRIX_3X2_F m0,m1;
  ID2D1HwndRenderTarget_GetTransform(pRenderTarget,&m0);
  D2D1_POINT_2F center = {0,0};
  D2D1MakeRotateMatrix(ang,center,&m1);
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ID2D1HwndRenderTarget_SetTransform(pRenderTarget,&m);
}
void gx_impl_translate(float x, float y){
  D2D1_MATRIX_3X2_F m0;
  ID2D1HwndRenderTarget_GetTransform(pRenderTarget,&m0);
  D2D1_MATRIX_3X2_F m1 = {1,0,0,1,x,y};
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ID2D1HwndRenderTarget_SetTransform(pRenderTarget,&m);
}
void gx_impl_scale(float x, float y){
  D2D1_MATRIX_3X2_F m0;
  ID2D1HwndRenderTarget_GetTransform(pRenderTarget,&m0);
  D2D1_MATRIX_3X2_F m1 = {x,0,0,y,0,0};
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ID2D1HwndRenderTarget_SetTransform(pRenderTarget,&m);
}
void gx_impl_apply_matrix(float* data){
  D2D1_MATRIX_3X2_F m0;
  ID2D1HwndRenderTarget_GetTransform(pRenderTarget,&m0);
  D2D1_MATRIX_3X2_F m1 = {
    data[0],data[3],data[6],data[1],data[4],data[7]
  };
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ID2D1HwndRenderTarget_SetTransform(pRenderTarget,&m);
}
void gx_impl_reset_matrix(){
  D2D1_MATRIX_3X2_F m = {1,0,0,1,0,0};
  ID2D1HwndRenderTarget_SetTransform(pRenderTarget, &m);
}


void gx_impl_background(float r, float g, float b, float a){
  D2D1_COLOR_F c = {r,g,b,a};
  ID2D1SolidColorBrush_SetColor(pBrush,&c);
  D2D1_RECT_F rect = {0,0,width,height};
  ID2D1HwndRenderTarget_FillRectangle(pRenderTarget, &rect, pBrush);
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
  line_width = x;
}

int is_first = 0;

void gx_impl_begin_shape(){
  ID2D1Factory_CreatePathGeometry(pFactory, &geometry);
  ID2D1PathGeometry_Open(geometry, &sink);
  is_first = 1;
}

void gx_impl_vertex(float x, float y){
  D2D1_POINT_2F p = {x,y};
  if (is_first){
    ID2D1GeometrySink_BeginFigure(sink,p,D2D1_FIGURE_BEGIN_FILLED);
  }else{
    ID2D1GeometrySink_AddLine(sink,p);
  }
  is_first = 0;
}

void gx_impl_next_contour(int bclose){

}

void gx_impl_end_shape(int bclose){
  if (bclose){
    ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_CLOSED);
  }else{
    ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_OPEN);
  }
  ID2D1GeometrySink_Close(sink);
  ID2D1GeometrySink_Release(sink);

  if (is_fill){
    ID2D1SolidColorBrush_SetColor(pBrush,&color_fill);
    ID2D1HwndRenderTarget_FillGeometry(pRenderTarget, geometry, pBrush, NULL);
  }
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(pBrush,&color_stroke);
    ID2D1HwndRenderTarget_DrawGeometry(pRenderTarget, geometry, pBrush, line_width, NULL);
  }
  ID2D1PathGeometry_Release(geometry);
}

void gx_impl_line(float x0, float y0, float x1, float y1){
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(pBrush,&color_stroke);
    D2D1_POINT_2F start = {x0,y0};
    D2D1_POINT_2F end = {x1,y1};
    ID2D1HwndRenderTarget_DrawLine(pRenderTarget,start,end,pBrush,line_width,NULL);
  }
}



void gx_impl_ellipse(float x, float y, float w, float h){
  D2D1_ELLIPSE ellipse = {{x,y},w*0.5,h*0.5};
  if (is_fill){
    ID2D1SolidColorBrush_SetColor(pBrush,&color_fill);
    ID2D1HwndRenderTarget_FillEllipse(pRenderTarget, &ellipse, pBrush);
  }
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(pBrush,&color_stroke);
    ID2D1HwndRenderTarget_DrawEllipse(pRenderTarget, &ellipse, pBrush,line_width,NULL);
  }
}

void gx_impl_rect(float x, float y, float w, float h){
  D2D1_RECT_F rect = {x,y,x+w,y+h};
  if (is_fill){
    ID2D1SolidColorBrush_SetColor(pBrush,&color_fill);
    ID2D1HwndRenderTarget_FillRectangle(pRenderTarget, &rect, pBrush);
  }
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(pBrush,&color_stroke);
    ID2D1HwndRenderTarget_DrawRectangle(pRenderTarget, &rect, pBrush,line_width,NULL);
  }
}

void gx_impl_point(float x, float y){
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(pBrush,&color_stroke);
    D2D1_POINT_2F start = {x-line_width/2,y};
    D2D1_POINT_2F end = {x+line_width/2,y};
    ID2D1HwndRenderTarget_DrawLine(pRenderTarget,start,end,pBrush,line_width,NULL);
  }
}

void gx_impl_text(char* str, float x, float y){
  int n = strlen(str);
  ID2D1SolidColorBrush_SetColor(pBrush,&color_fill);
  for (int i = 0; i < n; i++){
    wchar_t wtext[2] = {str[i],0};
    D2D1_RECT_F layoutRect = {x+8*i,y,x+8*i+16,y+32};
    ID2D1HwndRenderTarget_DrawText(
      pRenderTarget,
      wtext,1,
      textFormat,
      &layoutRect,
      pBrush,
      D2D1_DRAW_TEXT_OPTIONS_NONE,
      DWRITE_MEASURING_MODE_NATURAL
    );
  }
}
