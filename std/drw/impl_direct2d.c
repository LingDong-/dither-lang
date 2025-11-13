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
#include "../../third_party/d2d1_1.h"
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include "../../third_party/dwrite.h"
#include <tmmintrin.h>
// unhack
#undef DECLARE_INTERFACE
#define DECLARE_INTERFACE(iface) typedef interface iface { struct iface ## Vtbl FAR* lpVtbl; } iface; typedef struct iface ## Vtbl iface ## Vtbl; struct iface ## Vtbl

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

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

ARR_DEF(D2D1_MATRIX_3X2_F)
D2D1_MATRIX_3X2_F_arr_t matrices;


int width;
int height;

int is_stroke=1;
int is_fill=1;
float line_width = 1;

typedef struct fbo_st {
  ID3D11Texture2D* gpuTexture;
  ID3D11Texture2D* stagingTexture;
  ID2D1Bitmap1* d2dTargetBitmap;
  int w;
  int h;
} fbo_t;

ARR_DEF(fbo_t)
fbo_t_arr_t fbos;

D2D1_COLOR_F color_fill = {1.0f, 1.0f, 1.0f, 1.0f};
D2D1_COLOR_F color_stroke = {0.0f, 0.0f, 0.0f, 1.0f};

HWND hwnd;
ID3D11Device*           d3dDevice = NULL;
ID3D11DeviceContext*    d3dContext = NULL;
IDXGISwapChain*         swapChain = NULL;
ID2D1Factory1*          d2dFactory = NULL;
ID2D1Device*            d2dDevice = NULL;
ID2D1DeviceContext*     d2dContext = NULL;
ID2D1Bitmap1*           d2dTargetBitmap = NULL;
ID2D1SolidColorBrush* brush = NULL;

IDWriteFactory* writeFactory = NULL;
IDWriteTextFormat* textFormat = NULL;

ID2D1PathGeometry* geometry = NULL;
ID2D1GeometrySink* sink = NULL;

ID2D1RenderTarget* ctx;

void drw_impl__size(int w, int h, uint64_t _hwnd){
  hwnd = (HWND)(void*)(uintptr_t)_hwnd;
  width = w;
  height = h;

  DXGI_SWAP_CHAIN_DESC sd = {
    .BufferCount = 2,
    .BufferDesc = {
      .Width = w,
      .Height = h,
      .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .RefreshRate = { 0, 1 }  // No fixed refresh
    },
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .OutputWindow = hwnd,
    .SampleDesc = { 1, 0 },
    .Windowed = TRUE,
    .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
    .Flags = 0
  };
  UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
  D3D_FEATURE_LEVEL featureLevel;
  D3D11CreateDeviceAndSwapChain(
    NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
    createDeviceFlags, NULL, 0,
    D3D11_SDK_VERSION, &sd,
    &swapChain, &d3dDevice, &featureLevel, &d3dContext
  );
  D2D1_FACTORY_OPTIONS opts = {0};
  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory1, &opts, (void**)&d2dFactory);
  IDXGIDevice* dxgiDevice = NULL;
  d3dDevice->lpVtbl->QueryInterface(d3dDevice, &IID_IDXGIDevice, (void**)&dxgiDevice);
  ID2D1Factory1_CreateDevice(d2dFactory, dxgiDevice, &d2dDevice);
  ID2D1Device_CreateDeviceContext(d2dDevice, D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContext);
  dxgiDevice->lpVtbl->Release(dxgiDevice);
  IDXGISurface* dxgiSurface = NULL;
  swapChain->lpVtbl->GetBuffer(swapChain, 0, &IID_IDXGISurface, (void**)&dxgiSurface);
  D2D1_BITMAP_PROPERTIES1 bp = {
    .pixelFormat = {
      .format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
    },
    .dpiX = 96.0f,
    .dpiY = 96.0f,
    .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW
  };
  d2dContext->lpVtbl->CreateBitmapFromDxgiSurface(d2dContext, dxgiSurface, &bp, &d2dTargetBitmap);
  dxgiSurface->lpVtbl->Release(dxgiSurface);

  d2dContext->lpVtbl->SetTarget(d2dContext, (ID2D1Image*)d2dTargetBitmap);
  ctx = (ID2D1RenderTarget*)d2dContext;

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

  ctx->lpVtbl->CreateSolidColorBrush(ctx,&color_stroke,NULL,&brush);
  ctx->lpVtbl->BeginDraw(ctx);
}

void drw_impl__flush(){
  ctx->lpVtbl->EndDraw(ctx,NULL,NULL);
  swapChain->lpVtbl->Present(swapChain, 0, 0); 
  ctx->lpVtbl->BeginDraw(ctx); 
}




fbo_t createBuffer(
  ID3D11Device* d3dDevice,
  ID2D1DeviceContext* d2dContext,
  UINT width,
  UINT height
) {
  fbo_t outBuffer;
  D3D11_TEXTURE2D_DESC texDesc = {
    .Width = width,
    .Height = height,
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
    .SampleDesc = {1, 0},
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = D3D11_RESOURCE_MISC_SHARED
  };
  d3dDevice->lpVtbl->CreateTexture2D(d3dDevice, &texDesc, NULL, &outBuffer.gpuTexture);
  texDesc.Usage = D3D11_USAGE_STAGING;
  texDesc.BindFlags = 0;
  texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  texDesc.MiscFlags = 0;
  d3dDevice->lpVtbl->CreateTexture2D(d3dDevice, &texDesc, NULL, &outBuffer.stagingTexture);
  IDXGISurface* dxgiSurface = NULL;
  outBuffer.gpuTexture->lpVtbl->QueryInterface(
    outBuffer.gpuTexture, &IID_IDXGISurface, (void**)&dxgiSurface
  );
  D2D1_BITMAP_PROPERTIES1 bp = {
    .pixelFormat = {
      .format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
    },
    .dpiX = 96.0f,
    .dpiY = 96.0f,
    .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET ,
    .colorContext = NULL
  };
  d2dContext->lpVtbl->CreateBitmapFromDxgiSurface(
    d2dContext, dxgiSurface, &bp, &outBuffer.d2dTargetBitmap
  );
  dxgiSurface->lpVtbl->Release(dxgiSurface);
  outBuffer.w = width;
  outBuffer.h = height;
  return outBuffer;
}

void drw_impl__init_graphics(void* data, int w, int h){
  fbo_t offscreen = createBuffer(d3dDevice,d2dContext,w,h);
  ARR_PUSH(fbo_t,fbos,offscreen);
  ((int32_t*)(data))[2] = fbos.len-1;
  ((int32_t*)(data))[3] = w;
  ((int32_t*)(data))[4] = h;
}

int cur_fbo = -1;
void drw_impl__begin_fbo(int fbo){
  cur_fbo = fbo;
  fbo_t offscreen = fbos.data[fbo];
  ctx->lpVtbl->EndDraw(ctx,NULL,NULL);
  d2dContext->lpVtbl->SetTarget(d2dContext, (ID2D1Image*)offscreen.d2dTargetBitmap);
  ctx->lpVtbl->BeginDraw(ctx);
}

void drw_impl__end_fbo(){
  ctx->lpVtbl->EndDraw(ctx,NULL,NULL);
  d2dContext->lpVtbl->SetTarget(d2dContext, (ID2D1Image*)d2dTargetBitmap);
  ctx->lpVtbl->BeginDraw(ctx);
  cur_fbo = -1;
}



uint8_t* readPixels(ID3D11DeviceContext* context, fbo_t* buffer) {
  D3D11_MAPPED_SUBRESOURCE mapped;
  uint8_t* pixels = (uint8_t*)malloc(buffer->w * buffer->h * 4);
  if (!pixels) return NULL;
  context->lpVtbl->CopyResource(context, (ID3D11Resource*)buffer->stagingTexture, (ID3D11Resource*)buffer->gpuTexture);
  context->lpVtbl->Map(
    context,
    (ID3D11Resource*)buffer->stagingTexture,
    0,
    D3D11_MAP_READ,
    0,
    &mapped
  );
  // for (UINT y = 0; y < buffer->h; ++y) {
  //   memcpy(
  //     pixels + y * buffer->w * 4,
  //     (uint8_t*)mapped.pData + y * mapped.RowPitch,
  //     buffer->w * 4
  //   );
  // }
  // for (int y = 0; y < buffer->h; ++y){
  //   for (int x = 0; x < buffer->w; ++x){
  //     int idx0 = (y*buffer->w+x)*4;
  //     int idx1 = y*mapped.RowPitch+x*4;
  //     pixels[idx0+0] = ((uint8_t*)mapped.pData)[idx1+2];
  //     pixels[idx0+1] = ((uint8_t*)mapped.pData)[idx1+1];
  //     pixels[idx0+2] = ((uint8_t*)mapped.pData)[idx1+0];
  //     pixels[idx0+3] = ((uint8_t*)mapped.pData)[idx1+3];
  //   }
  // }
  for (int y = 0; y < buffer->h; ++y) {
    uint8_t* dst = pixels + y * buffer->w * 4;
    uint8_t* src = (uint8_t*)mapped.pData + y * mapped.RowPitch;
    int x = 0;
    for (; x <= buffer->w - 4; x += 4) {
      __m128i bgra = _mm_loadu_si128((__m128i*)(src + x * 4));
      const __m128i mask = _mm_set_epi8(
        15, 12, 13, 14,
        11, 8, 9, 10,
        7, 4, 5, 6,
        3, 0, 1, 2
      );
      __m128i rgba = _mm_shuffle_epi8(bgra, mask);
      _mm_storeu_si128((__m128i*)(dst + x * 4), rgba);
    }
    for (; x < buffer->w; ++x) {
      int dst_idx = x * 4;
      int src_idx = x * 4;
      dst[dst_idx + 0] = src[src_idx + 2];
      dst[dst_idx + 1] = src[src_idx + 1];
      dst[dst_idx + 2] = src[src_idx + 0];
      dst[dst_idx + 3] = src[src_idx + 3];
    }
  }

  context->lpVtbl->Unmap(context, (ID3D11Resource*)buffer->stagingTexture, 0);
  return pixels;
}


void writePixels(ID3D11DeviceContext* context, fbo_t* buffer, const uint8_t* pixels) {
  D3D11_MAPPED_SUBRESOURCE mapped;
  context->lpVtbl->Map(
    context,
    (ID3D11Resource*)buffer->stagingTexture,
    0,
    D3D11_MAP_WRITE,
    0,
    &mapped
  );
  // for (UINT y = 0; y < buffer->h; ++y) {
  //   memcpy(
  //     (uint8_t*)mapped.pData + y * mapped.RowPitch,
  //     pixels + y * buffer->w * 4,
  //     buffer->w * 4
  //   );
  // }
  // for (int y = 0; y < buffer->h; ++y){
  //   for (int x = 0; x < buffer->w; ++x){
  //     int idx0 = (y*buffer->w+x)*4;
  //     int idx1 = y*mapped.RowPitch+x*4;
  //     ((uint8_t*)mapped.pData)[idx1+2] = pixels[idx0+0];
  //     ((uint8_t*)mapped.pData)[idx1+1] = pixels[idx0+1];
  //     ((uint8_t*)mapped.pData)[idx1+0] = pixels[idx0+2];
  //     ((uint8_t*)mapped.pData)[idx1+3] = pixels[idx0+3];
  //   }
  // }
  for (int y = 0; y < buffer->h; ++y) {
    const uint8_t* src = pixels + y * buffer->w * 4;
    uint8_t* dst = (uint8_t*)mapped.pData + y * mapped.RowPitch;
    int x = 0;
    for (; x <= buffer->w - 4; x += 4) {
      __m128i bgra = _mm_loadu_si128((__m128i*)(src + x * 4));
      const __m128i mask = _mm_set_epi8(
        15, 12, 13, 14,
        11, 8, 9, 10,
        7, 4, 5, 6,
        3, 0, 1, 2
      );
      __m128i rgba = _mm_shuffle_epi8(bgra, mask);
      _mm_storeu_si128((__m128i*)(dst + x * 4), rgba);
    }
    for (; x < buffer->w; ++x) {
      int dst_idx = x * 4;
      int src_idx = x * 4;
      dst[dst_idx + 0] = src[src_idx + 2];
      dst[dst_idx + 1] = src[src_idx + 1];
      dst[dst_idx + 2] = src[src_idx + 0];
      dst[dst_idx + 3] = src[src_idx + 3];
    }
  }
  context->lpVtbl->Unmap(context, (ID3D11Resource*)buffer->stagingTexture, 0);
  context->lpVtbl->CopyResource(context, (ID3D11Resource*)buffer->gpuTexture, (ID3D11Resource*)buffer->stagingTexture);
}



void* drw_impl__read_pixels(int fbo, int* _w, int* _h){
  uint8_t* pixels = readPixels(d3dContext, &(fbos.data[fbo]));
  int w = fbos.data[fbo].w;
  int h = fbos.data[fbo].h;
  *_w = w;
  *_h = h;
  return pixels;
}


void drw_impl__write_pixels(int fbo, void* pixels){
  writePixels(d3dContext,&(fbos.data[fbo]),pixels);
}

void drw_impl__draw_texture(int fbo, float x, float y, float w, float h){
  D2D1_RECT_F rect = {x,y,x+w,y+h};
  ctx->lpVtbl->DrawBitmap(
    ctx,
    (ID2D1Bitmap*)fbos.data[fbo].d2dTargetBitmap,
    &rect,
    1.0f,
    D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
    NULL
  );
}


void drw_impl_push_matrix(){
  D2D1_MATRIX_3X2_F m;
  ctx->lpVtbl->GetTransform(ctx,&m);
  ARR_PUSH(D2D1_MATRIX_3X2_F,matrices,m);
}
void drw_impl_pop_matrix(){
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

void drw_impl_rotate_deg(float ang){
  D2D1_MATRIX_3X2_F m0,m1;
  ctx->lpVtbl->GetTransform(ctx,&m0);
  D2D1_POINT_2F center = {0,0};
  D2D1MakeRotateMatrix(ang,center,&m1);
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ctx->lpVtbl->SetTransform(ctx,&m);
}
void drw_impl_translate(float x, float y){
  D2D1_MATRIX_3X2_F m0;
  ctx->lpVtbl->GetTransform(ctx,&m0);
  D2D1_MATRIX_3X2_F m1 = {1,0,0,1,x,y};
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ctx->lpVtbl->SetTransform(ctx,&m);
}
void drw_impl_scale(float x, float y){
  D2D1_MATRIX_3X2_F m0;
  ctx->lpVtbl->GetTransform(ctx,&m0);
  D2D1_MATRIX_3X2_F m1 = {x,0,0,y,0,0};
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ctx->lpVtbl->SetTransform(ctx,&m);
}
void drw_impl_apply_matrix(float* data){
  D2D1_MATRIX_3X2_F m0;
  ctx->lpVtbl->GetTransform(ctx,&m0);
  D2D1_MATRIX_3X2_F m1 = {
    data[0],data[3],data[6],data[1],data[4],data[7]
  };
  D2D1_MATRIX_3X2_F m;
  matrixMultiply(&m,&m1,&m0);
  ctx->lpVtbl->SetTransform(ctx,&m);
}
void drw_impl_reset_matrix(){
  D2D1_MATRIX_3X2_F m = {1,0,0,1,0,0};
  ctx->lpVtbl->SetTransform(ctx,&m);
}


void drw_impl_background(float r, float g, float b, float a){
  D2D1_COLOR_F c = {r,g,b,a};
  ID2D1SolidColorBrush_SetColor(brush,&c);
  D2D1_RECT_F rect = {0,0,width,height};
  ctx->lpVtbl->FillRectangle(ctx,&rect,brush);
}


void drw_impl_fill(float r, float g, float b, float a){
  color_fill.r = r;
  color_fill.g = g;
  color_fill.b = b;
  color_fill.a = a;
  is_fill = 1;
}

void drw_impl_stroke(float r, float g, float b, float a){
  color_stroke.r = r;
  color_stroke.g = g;
  color_stroke.b = b;
  color_stroke.a = a;
  is_stroke = 1;
}

void drw_impl_no_fill(){
  is_fill = 0;
}
void drw_impl_no_stroke(){
  is_stroke = 0;
}
void drw_impl_stroke_weight(float x){
  line_width = x;
}

int is_first = 0;
int did_first = 0;

void drw_impl_begin_shape(){
  ID2D1Factory_CreatePathGeometry(d2dFactory, &geometry);
  ID2D1PathGeometry_Open(geometry, &sink);
  is_first = 1;
  did_first = 0;
}

void drw_impl_vertex(float x, float y){
  D2D1_POINT_2F p = {x,y};
  if (is_first){
    ID2D1GeometrySink_BeginFigure(sink,p,D2D1_FIGURE_BEGIN_FILLED);
  }else{
    ID2D1GeometrySink_AddLine(sink,p);
  }
  is_first = 0;
  did_first = 1;
}

void drw_impl_next_contour(int bclose){
  if (bclose){
    ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_CLOSED);
  }else{
    ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_OPEN);
  }
  is_first = 1;
  did_first = 0;
}

void drw_impl_end_shape(int bclose){
  if (did_first){
    if (bclose){
      ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_CLOSED);
    }else{
      ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_OPEN);
    }
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

void drw_impl_line(float x0, float y0, float x1, float y1){
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(brush,&color_stroke);
    D2D1_POINT_2F start = {x0,y0};
    D2D1_POINT_2F end = {x1,y1};
    ctx->lpVtbl->DrawLine(ctx,start,end,brush,line_width,NULL);
  }
}



void drw_impl_ellipse(float x, float y, float w, float h){
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

void drw_impl_rect(float x, float y, float w, float h){
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

void drw_impl_point(float x, float y){
  if (is_stroke){
    ID2D1SolidColorBrush_SetColor(brush,&color_stroke);
    D2D1_POINT_2F start = {x-line_width/2,y};
    D2D1_POINT_2F end = {x+line_width/2,y};
    ctx->lpVtbl->DrawLine(ctx,start,end,brush,line_width,NULL);
  }
}

void drw_impl_text(char* str, float x, float y){
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
