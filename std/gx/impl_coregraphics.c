//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework CoreGraphics" || echo "")

#include <math.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>

#include <CoreGraphics/CoreGraphics.h>


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

int is_stroke=1;
int is_fill=1;

void** ctx0;
void** ctx;

float line_width = 1;

ARR_DEF(CGContextRef)

CGContextRef_arr_t fbos;

void gx_impl__size(int w, int h, uint64_t _ctx){

  ctx0 = (void**)(uintptr_t)_ctx;
  ctx = ctx0;

  CGContextSetRGBFillColor(*ctx, 1,1,1,1);
  CGContextSetRGBStrokeColor(*ctx, 0,0,0,1);

  width = w;
  height = h;

  ARR_INIT(CGContextRef,fbos);
  
}

void gx_impl__flush(){
  
}

void gx_impl__init_graphics(void* data, int w, int h){
  size_t bytesPerRow = w * 4;
  CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  CGContextRef fbo = CGBitmapContextCreate(NULL, w, h, 8, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(colorSpace);

  ARR_PUSH(CGContextRef,fbos,fbo);

  ((int32_t*)(data))[2] = fbos.len-1;
  ((int32_t*)(data))[3] = w;
  ((int32_t*)(data))[4] = h;
}

void gx_impl__begin_fbo(int fbo){
  ctx = (void**)(&(fbos.data[fbo]));

}

void gx_impl__end_fbo(){
  ctx = ctx0;
  
}

void* gx_impl__read_pixels(int fbo, int* _w, int* _h){
  CGContextRef c = fbos.data[fbo];
  void *data = CGBitmapContextGetData(c);
  int w = CGBitmapContextGetWidth(c);
  int h = CGBitmapContextGetHeight(c);
  void* copy = malloc(w*h*4);

  size_t rowBytes = w*4;
  for (int y = 0; y < h; y++) {
    memcpy(copy + y * rowBytes,data + (h - 1 - y) * rowBytes,rowBytes);
  }
  *_w = w;
  *_h = h;
  return copy;
}


void gx_impl__write_pixels(int fbo, void* pixels){
  CGContextRef c = fbos.data[fbo];
  void *data = CGBitmapContextGetData(c);
  int w = CGBitmapContextGetWidth(c);
  int h = CGBitmapContextGetHeight(c);

  size_t rowBytes = w*4;
  for (int y = 0; y < h; y++) {
    memcpy(data + y * rowBytes,pixels + (h - 1 - y) * rowBytes,rowBytes);
  }
}

void gx_impl__draw_texture(int fbo, float x, float y, float w, float h){
  CGImageRef img = CGBitmapContextCreateImage(fbos.data[fbo]);
  CGContextDrawImage(*ctx, CGRectMake(x, y, w, h), img);
  CGImageRelease(img);
}


void gx_impl_push_matrix(){
  CGContextSaveGState(*ctx);
}
void gx_impl_pop_matrix(){
  CGContextRestoreGState(*ctx);
}
void gx_impl_rotate_deg(float ang){
  CGContextRotateCTM(*ctx,ang*M_PI/180.0);
}
void gx_impl_translate(float x, float y){
  CGContextTranslateCTM(*ctx,x,y);
}
void gx_impl_scale(float x, float y){
  CGContextScaleCTM(*ctx,x,y);
}
void gx_impl_apply_matrix(float* data){
  CGAffineTransform transform = CGAffineTransformMake(
    ((float*)data)[0],
    ((float*)data)[3],
    ((float*)data)[1],
    ((float*)data)[4],
    ((float*)data)[2],
    ((float*)data)[5] 
  );
  CGContextConcatCTM(*ctx, transform);
}
void gx_impl_reset_matrix(){
  CGContextConcatCTM(*ctx, CGAffineTransformInvert(CGContextGetCTM(*ctx)));

}


void gx_impl_background(float r, float g, float b, float a){
  CGContextSaveGState(*ctx);
  CGContextSetRGBFillColor(*ctx, r, g, b, a);
  CGContextFillRect(*ctx, CGRectMake(0, 0, width, height));
  CGContextRestoreGState(*ctx);
}


void gx_impl_fill(float r, float g, float b, float a){
  CGContextSetRGBFillColor(*ctx, r,g,b,a);
  is_fill = 1;
}

void gx_impl_stroke(float r, float g, float b, float a){
  CGContextSetRGBStrokeColor(*ctx, r,g,b,a);
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
  CGContextSetLineWidth(*ctx,x);
}
int is_first;

void gx_impl_begin_shape(){
  CGContextBeginPath(*ctx);
  is_first = 1;
}

void gx_impl_vertex(float x, float y){
  if (is_first){
    CGContextMoveToPoint(*ctx, x, y);
  }else{
    CGContextAddLineToPoint(*ctx, x, y);
  }
  is_first = 0;
}

void gx_impl_next_contour(int bclose){
  if (bclose){
    CGContextClosePath(*ctx);
  }
  is_first = 1;
}

void gx_impl_end_shape(int bclose){

  if (bclose){
    CGContextClosePath(*ctx);
  }
  if (is_fill && is_stroke){
    CGContextDrawPath(*ctx, kCGPathFillStroke);
  }else if (is_fill){
    CGContextFillPath(*ctx);
  }else if (is_stroke){
    CGContextStrokePath(*ctx);
  }
}

void gx_impl_line(float x0, float y0, float x1, float y1){

  if (is_stroke){
    CGContextBeginPath(*ctx);
    CGContextMoveToPoint(*ctx,x0,y0);
    CGContextAddLineToPoint(*ctx,x1,y1);
    CGContextStrokePath(*ctx);
  }
}


void gx_impl_ellipse(float x, float y, float w, float h){
  CGRect r = CGRectMake(x-w/2, y-h/2, w, h);
  if (is_fill){
    CGContextFillEllipseInRect(*ctx, r);
  }
  if (is_stroke){
    CGContextStrokeEllipseInRect(*ctx, r);
  }
}

void gx_impl_rect(float x, float y, float w, float h){
  CGRect r = CGRectMake(x, y, w, h);
  if (is_fill){
    CGContextFillRect(*ctx, r);
  }
  if (is_stroke){
    CGContextStrokeRect(*ctx, r);
  }
}

void gx_impl_point(float x, float y){
  if (is_stroke){
    CGContextBeginPath(*ctx);
    CGContextMoveToPoint(*ctx,x-line_width/2,y);
    CGContextAddLineToPoint(*ctx,x+line_width/2,y);
    CGContextStrokePath(*ctx);
  }
}

void gx_impl_text(char* str, float x, float y){
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  CGContextSaveGState(*ctx);
  CGContextTranslateCTM(*ctx, x, y-3);
  CGContextScaleCTM(*ctx, 1, -1.1);
  CGContextSelectFont(*ctx, "Courier", 14, kCGEncodingMacRoman);
  CGContextSetTextDrawingMode(*ctx, kCGTextFill);
  char* s = str;
  x = 0;
  while (*s){
    char q[] = {*s, 0};
    CGContextShowTextAtPoint(*ctx, x, 0, q, 1);
    x += 8;
    s++;
  }
  CGContextRestoreGState(*ctx);
  #pragma clang diagnostic pop
}
