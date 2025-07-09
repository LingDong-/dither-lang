#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <guiddef.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <wincodec.h>
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

typedef struct fbo_st {
  ID2D1RenderTarget* rt;
  IWICBitmap* bmp;
  ID2D1SolidColorBrush* brush;
  int w;
  int h;
} fbo_t;

ARR_DEF(fbo_t)
fbo_t_arr_t fbos;

D2D1_COLOR_F color_fill = {1.0f, 1.0f, 1.0f, 1.0f};
D2D1_COLOR_F color_stroke = {0.0f, 0.0f, 0.0f, 1.0f};

HWND hwnd;

ID2D1Factory* pFactory = NULL;
ID2D1HwndRenderTarget* pRenderTarget = NULL;
ID2D1SolidColorBrush* brush0 = NULL;
ID2D1SolidColorBrush* brush = NULL;

IDWriteFactory* writeFactory = NULL;
IDWriteTextFormat* textFormat = NULL;

ID2D1PathGeometry* geometry = NULL;
ID2D1GeometrySink* sink = NULL;

ID2D1RenderTarget* ctx0;
ID2D1RenderTarget* ctx;

IWICImagingFactory* wicFactory = NULL;

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
    { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
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

  CoCreateInstance(
    &CLSID_WICImagingFactory,
    NULL,
    CLSCTX_INPROC_SERVER,
    &IID_IWICImagingFactory,
    (LPVOID*)&wicFactory
  );

  ID2D1HwndRenderTarget_CreateSolidColorBrush(pRenderTarget, &color_stroke, NULL, &brush0);
  ID2D1HwndRenderTarget_BeginDraw(pRenderTarget);
  ctx = pRenderTarget;
  ctx0 = ctx;
  brush = brush0;
}

void gx_impl__flush(){
  ID2D1HwndRenderTarget_EndDraw(pRenderTarget,NULL,NULL);
  ID2D1HwndRenderTarget_BeginDraw(pRenderTarget);    
}

ID2D1RenderTarget* CreateWICRenderTarget(ID2D1Factory* d2dFactory, IWICImagingFactory* wicFactory, UINT w, UINT h, IWICBitmap** outBitmap) {
  IWICBitmap* wicBitmap = NULL;
  ID2D1RenderTarget* wicRenderTarget = NULL;
  IWICImagingFactory_CreateBitmap(
    wicFactory, w, h,
    &GUID_WICPixelFormat32bppPBGRA,
    WICBitmapCacheOnLoad,
    &wicBitmap
  );
  D2D1_RENDER_TARGET_PROPERTIES props = {
    .type = D2D1_RENDER_TARGET_TYPE_SOFTWARE,
    .pixelFormat = {
      .format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
    },
    .dpiX = 96.0f,
    .dpiY = 96.0f,
    .usage = D2D1_RENDER_TARGET_USAGE_NONE,
    .minLevel = D2D1_FEATURE_LEVEL_DEFAULT
  };
  ID2D1Factory_CreateWicBitmapRenderTarget(
    d2dFactory, wicBitmap, &props, &wicRenderTarget
  );

  *outBitmap = wicBitmap;
  return wicRenderTarget;
}


void gx_impl__init_graphics(void* data, int w, int h){
  fbo_t offscreen;
  offscreen.w = w;
  offscreen.h = h;
  offscreen.rt = CreateWICRenderTarget(pFactory, wicFactory, w, h, &(offscreen.bmp));
  ID2D1RenderTarget_CreateSolidColorBrush(offscreen.rt, &color_stroke, NULL, &(offscreen.brush));
  ARR_PUSH(fbo_t,fbos,offscreen);
  ((int32_t*)(data))[2] = fbos.len-1;
  ((int32_t*)(data))[3] = w;
  ((int32_t*)(data))[4] = h;
}

int cur_fbo = -1;
void gx_impl__begin_fbo(int fbo){
  cur_fbo = fbo;
  ID2D1RenderTarget* offscreen = fbos.data[fbo].rt;
  ctx = offscreen;
  brush = fbos.data[fbo].brush;
  ID2D1RenderTarget_BeginDraw(offscreen);
}

void gx_impl__end_fbo(){
  ID2D1RenderTarget* offscreen = fbos.data[cur_fbo].rt;
  // ID2D1RenderTarget_Flush(offscreen,NULL,NULL);
  ID2D1RenderTarget_EndDraw(offscreen,NULL,NULL);
  cur_fbo = -1;
  ctx = ctx0;
  brush = brush0;
}


void* gx_impl__read_pixels(int fbo, int* _w, int* _h){
  uint8_t* dst = malloc(fbos.data[fbo].w*fbos.data[fbo].h*4);
  IWICBitmapLock* lock = NULL;
  int w = fbos.data[fbo].w;
  int h = fbos.data[fbo].h;
  WICRect rect = { 0, 0, w, h };
  IWICBitmap_Lock(fbos.data[fbo].bmp, &rect, WICBitmapLockRead, &lock);

  UINT stride = 0, size = 0;
  BYTE* pixels = NULL;
  IWICBitmapLock_GetStride(lock, &stride);
  IWICBitmapLock_GetDataPointer(lock, &size, &pixels);
  for (UINT y = 0; y < h; y++) {
    for (UINT x = 0; x < w; x++) {
      dst[(y * w + x) * 4 + 0] = pixels[y * stride + x * 4 + 2];
      dst[(y * w + x) * 4 + 1] = pixels[y * stride + x * 4 + 1];
      dst[(y * w + x) * 4 + 2] = pixels[y * stride + x * 4 + 0];
      dst[(y * w + x) * 4 + 3] = pixels[y * stride + x * 4 + 3];
    }
  }
  IWICBitmapLock_Release(lock);
  *_w = w;
  *_h = h;
  return dst;
}


void gx_impl__write_pixels(int fbo, void* pixels){
  ID2D1RenderTarget* offscreen = fbos.data[fbo].rt;
  int w = fbos.data[fbo].w;
  int h = fbos.data[fbo].h;
  WICRect rect = { 0, 0, w, h };
  IWICBitmapLock* lock = NULL;
  IWICBitmap_Lock(fbos.data[fbo].bmp, &rect, WICBitmapLockWrite, &lock);
  UINT stride, size;
  BYTE* dst = NULL;
  IWICBitmapLock_GetStride(lock, &stride);
  IWICBitmapLock_GetDataPointer(lock, &size, &dst);
  for (UINT y = 0; y < h; y++) {
    for (UINT x = 0; x < w; x++) {
      dst[y * stride + x * 4 + 2] = ((uint8_t*)pixels)[(y * w + x) * 4 + 0];
      dst[y * stride + x * 4 + 1] = ((uint8_t*)pixels)[(y * w + x) * 4 + 1];
      dst[y * stride + x * 4 + 0] = ((uint8_t*)pixels)[(y * w + x) * 4 + 2];
      dst[y * stride + x * 4 + 3] = ((uint8_t*)pixels)[(y * w + x) * 4 + 3];
    }
  }
  IWICBitmapLock_Release(lock);
}

void gx_impl__draw_texture(int fbo, float x, float y, float w, float h){
  ID2D1RenderTarget* offscreen = fbos.data[fbo].rt;
  ID2D1Bitmap* bitmap = NULL;
  ID2D1RenderTarget_CreateBitmapFromWicBitmap(
    ctx,
    fbos.data[fbo].bmp,
    NULL,
    &bitmap
  );
  D2D1_RECT_F rect = {x,y,x+w,y+h};
  ctx->lpVtbl->DrawBitmap(ctx, bitmap, &rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);
  ID2D1Bitmap_Release(bitmap);
}


void gx_impl_push_matrix(){
  D2D1_MATRIX_3X2_F m;
  ctx->lpVtbl->GetTransform(ctx,&m);
  ARR_PUSH(D2D1_MATRIX_3X2_F,matrices,m);
}
void gx_impl_pop_matrix(){
  D2D1_MATRIX_3X2_F m = ARR_POP(D2D1_MATRIX_3X2_F,matrices);
  ctx->lpVtbl->SetTransform(ctx,&m);
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
  ctx->lpVtbl->GetTransform(ctx,&m0);
  D2D1_POINT_2F center = {0,0};
  D2D1MakeRotateMatrix(ang,center,&m1);
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ctx->lpVtbl->SetTransform(ctx,&m);
}
void gx_impl_translate(float x, float y){
  D2D1_MATRIX_3X2_F m0;
  ctx->lpVtbl->GetTransform(ctx,&m0);
  D2D1_MATRIX_3X2_F m1 = {1,0,0,1,x,y};
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ctx->lpVtbl->SetTransform(ctx,&m);
}
void gx_impl_scale(float x, float y){
  D2D1_MATRIX_3X2_F m0;
  ctx->lpVtbl->GetTransform(ctx,&m0);
  D2D1_MATRIX_3X2_F m1 = {x,0,0,y,0,0};
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ctx->lpVtbl->SetTransform(ctx,&m);
}
void gx_impl_apply_matrix(float* data){
  D2D1_MATRIX_3X2_F m0;
  ctx->lpVtbl->GetTransform(ctx,&m0);
  D2D1_MATRIX_3X2_F m1 = {
    data[0],data[3],data[6],data[1],data[4],data[7]
  };
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ctx->lpVtbl->SetTransform(ctx,&m);
}
void gx_impl_reset_matrix(){
  D2D1_MATRIX_3X2_F m = {1,0,0,1,0,0};
  ctx->lpVtbl->SetTransform(ctx,&m);
}


void gx_impl_background(float r, float g, float b, float a){
  D2D1_COLOR_F c = {r,g,b,a};
  ID2D1SolidColorBrush_SetColor(brush,&c);
  D2D1_RECT_F rect = {0,0,width,height};
  ctx->lpVtbl->FillRectangle(ctx,&rect,brush);
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
  if (bclose){
    ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_CLOSED);
  }else{
    ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_OPEN);
  }
  is_first = 1;
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
    ID2D1SolidColorBrush_SetColor(brush,&color_fill);
    ctx->lpVtbl->FillGeometry(ctx,geometry,brush,NULL);
  }
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(brush,&color_stroke);
    ctx->lpVtbl->DrawGeometry(ctx,geometry,brush,line_width,NULL);
  }
  ID2D1PathGeometry_Release(geometry);
}

void gx_impl_line(float x0, float y0, float x1, float y1){
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(brush,&color_stroke);
    D2D1_POINT_2F start = {x0,y0};
    D2D1_POINT_2F end = {x1,y1};
    ctx->lpVtbl->DrawLine(ctx,start,end,brush,line_width,NULL);
  }
}



void gx_impl_ellipse(float x, float y, float w, float h){
  D2D1_ELLIPSE ellipse = {{x,y},w*0.5,h*0.5};
  if (is_fill){
    ID2D1SolidColorBrush_SetColor(brush,&color_fill);
    ctx->lpVtbl->FillEllipse(ctx,&ellipse,brush);
  }
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(brush,&color_stroke);
    ctx->lpVtbl->DrawEllipse(ctx,&ellipse,brush,line_width,NULL);
  }
}

void gx_impl_rect(float x, float y, float w, float h){
  D2D1_RECT_F rect = {x,y,x+w,y+h};
  if (is_fill){
    ID2D1SolidColorBrush_SetColor(brush,&color_fill);
    ctx->lpVtbl->FillRectangle(ctx,&rect,brush);
  }
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(brush,&color_stroke);
    ctx->lpVtbl->DrawRectangle(ctx,&rect,brush,line_width,NULL);
  }
}

void gx_impl_point(float x, float y){
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(brush,&color_stroke);
    D2D1_POINT_2F start = {x-line_width/2,y};
    D2D1_POINT_2F end = {x+line_width/2,y};
    ctx->lpVtbl->DrawLine(ctx,start,end,brush,line_width,NULL);
  }
}

void gx_impl_text(char* str, float x, float y){
  int n = strlen(str);
  ID2D1SolidColorBrush_SetColor(brush,&color_fill);
  for (int i = 0; i < n; i++){
    wchar_t wtext[2] = {str[i],0};
    D2D1_RECT_F layoutRect = {x+8*i,y-16,x+8*i+16,y+16};
    ctx->lpVtbl->DrawText(ctx,
      wtext,1,
      textFormat,
      &layoutRect,
      brush,
      D2D1_DRAW_TEXT_OPTIONS_NONE,
      DWRITE_MEASURING_MODE_NATURAL
    );
  }
}
