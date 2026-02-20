#include <stdlib.h>

#define MASK_NORM 3
#define NORM_L1   1
#define NORM_L2   0
#define NORM_LINF 2

#define MASK_COLOR     7
#define MASK_ALPHA     (7<<4)
#define MASK_SCALE     (7<<8)
#define COLOR_COPY       0
#define COLOR_RGB_GRAY   1
#define COLOR_RGB_BGR    2
#define COLOR_RGB_HSV    3
#define COLOR_HSV_RGB    4
#define COLOR_LIN_SRGB   5
#define COLOR_SRGB_LIN   6
#define ALPHA_COPY       0
#define ALPHA_DROP       16
#define ALPHA_PREMUL     32
#define ALPHA_STRAIGHTEN 48

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

char* tmp_buf = NULL;
int tmp_buf_len = 0;

int EDT_f(int x, int i, int g_i) {
  return (x - i) * (x - i) + g_i * g_i;
}
int EDT_Sep(int i, int u, int g_i, int g_u) {
  return (u * u - i * i + g_u * g_u - g_i * g_i) / (2 * (u - i));
}
int MDT_f(int x, int i, int g_i) {	
  return abs(x-i) + g_i;	
}
int MDT_Sep(int i, int u, int g_i, int g_u) {
  if (g_u >= (g_i + u - i))
    return INT16_MAX;
  if (g_i > (g_u + u - i))
    return -INT16_MAX;
  return (g_u - g_i + u + i)/2;
}
int CDT_f(int x, int i, int g_i) {	
  return MAX(abs(x-i), g_i);
}
int CDT_Sep(int i, int u, int g_i, int g_u) {
  if (g_i <= g_u)
    return MAX(i+g_u, ((i+u)/2));
  else
    return MIN(u-g_i, ((i+u)/2));
}
void img_impl_dist_transform(uint8_t* b, int m, int n, int flags, float* dt){
  int (*f)(int,int,int);
  int (*Sep)(int,int,int,int);
  if ((flags&MASK_NORM) == NORM_L1){
    f = MDT_f;
    Sep = MDT_Sep;
  }else if ((flags&MASK_NORM) == NORM_L2){
    f = EDT_f;
    Sep = EDT_Sep;
  }else if ((flags&MASK_NORM) == NORM_LINF){
    f = CDT_f;
    Sep = CDT_Sep;
  }
  int do_voro = !!(flags&12);
  int tsz = m*n*2+m*2+m*2+m*n*1;
  if (tmp_buf_len < tsz){
    tmp_buf_len = tsz;
    tmp_buf = realloc(tmp_buf,tsz);
  }
  int16_t* g = (int16_t*)tmp_buf;
  int16_t* s = (int16_t*)(tmp_buf + (m*n*2));
  int16_t* t = (int16_t*)(tmp_buf + (m*n*2+m*2));
  uint8_t* v = (uint8_t*)(tmp_buf + (m*n*2+m*2+m*2));
  for (int x = 0; x < m; x++) {
    if (b[x + 0 * m]){
      g[x + 0 * m] = 0;
      v[x + 0 * m] = b[x];
    }else{
      g[x + 0 * m] = INT16_MAX;
      v[x + 0 * m] = 0;
    }
    for (int y = 1; y < n; y++) {
      if (b[x + y * m]){
        g[x + y * m] = 0;
        v[x + y * m] = b[x + y * m];
      }else{
        g[x + y * m] = 1 + g[x + (y - 1) * m];
        v[x + y * m] = v[x + (y - 1) * m];
      }
    }
    for (int y = n - 2; y >= 0; y--) {
      if (g[x + (y + 1) * m] < g[x + y * m]){
        g[x + y * m] = 1 + g[x + (y + 1) * m];
        v[x + y * m] = 1 + v[x + (y + 1) * m];
      }
    }
  }
  int q = 0;
  int w;
  for (int y = 0; y < n; y++) {
    q = 0;
    s[0] = 0;
    t[0] = 0;
    for (int u = 1; u < m; u++) {
      while (q >= 0 && f(t[q], s[q], g[s[q] + y * m]) > f(t[q], u, g[u + y * m]))
        q--;
      if (q < 0) {
        q = 0;
        s[0] = u;
      } else {
        w = 1 + Sep(s[q], u, g[s[q] + y * m], g[u + y * m]);
        if (w < m) {
          q++;
          s[q] = u;
          t[q] = w;
        }
      }
    }
    for (int u = m - 1; u >= 0; u--) {
      int d = f(u, s[q], g[s[q] + y * m]);
      if (f == EDT_f) d = sqrt(d);
      dt[u + y * m] = d;
      if (do_voro) b[u + y * m] = v[s[q] + y * m];
      if (u == t[q]) q--;
    }
  }
}

void rgb2hsv(float r, float g, float b, float* h, float* s, float* v){
  float rgbMin, rgbMax;
  rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
  rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
  *v = rgbMax;
  if (v == 0){
    *h = 0;
    *s = 0;
    return;
  }
  *s = (rgbMax - rgbMin) / rgbMax;
  if (*s == 0){
    *h = 0;
    return;
  }
  if (rgbMax == r)
    *h = 1.0/6.0 * (g - b) / (rgbMax - rgbMin);
  else if (rgbMax == g)
    *h = 2.0/6.0 + 1.0/6.0 * (b - r) / (rgbMax - rgbMin);
  else
    *h = 4.0/6.0 + 1.0/6.0 * (r - g) / (rgbMax - rgbMin);
}
void hsv2rgb(float h, float s, float v, float* r, float* g, float* b){
  if (s == 0){
    *r = v;
    *g = v;
    *b = v;
    return;
  }
  float hh = h+1.0;
  hh -= (int)hh;
  hh *= 6.0;
  int i = hh;
  float ff = hh-i;
  float p = v * (1.0 - s);
  float q = v * (1.0 - (s*ff));
  float t = v * (1.0 - (s*(1.0-ff)));
  if (i==0){
    *r = v; *g = t; *b = p;
  }else if (i==1){
    *r = q; *g = v; *b = p;
  }else if (i==2){
    *r = p; *g = v; *b = t;
  }else if (i==3){
    *r = p; *g = q; *b = v;
  }else if (i==4){
    *r = t; *g = p; *b = v;
  }else{
    *r = v; *g = p; *b = q;
  }
}

#define GENERATE_CONVERTER(dtype0,div0,dtype1,div1)\
  void img_impl_convert_ ## dtype0 ## _ ## dtype1 (\
    void* void_inp, int w, int h, int ic, int flags,\
    void* void_out, int oc\
  ){\
    dtype0* inp = (dtype0*) void_inp;\
    dtype1* out = (dtype1*) void_out;\
    if ((flags & MASK_ALPHA) == ALPHA_STRAIGHTEN && (ic == 2 || ic == 4)){\
      int n = w*h*ic*sizeof(dtype0);\
      if (n > tmp_buf_len){\
        tmp_buf_len = n;\
        tmp_buf = realloc(tmp_buf, tmp_buf_len);\
      }\
      dtype0* tmp = (dtype0*)tmp_buf;\
      int icc = (ic <= 2) ? 1 : 3;\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          tmp[(i*w+j)*ic+icc] = inp[(i*w+j)*ic+icc];\
          for (int k = 0; k < ic; k++){\
            if (inp[(i*w+j)*ic+icc] == 0){\
              tmp[(i*w+j)*ic+k] = 0;\
            }else{\
              tmp[(i*w+j)*ic+k] = (inp[(i*w+j)*ic+k]*div0/inp[(i*w+j)*ic+icc]);\
            }\
          }\
        }\
      }\
      inp = tmp;\
    }\
    if ((flags & MASK_COLOR) == COLOR_COPY || \
        ((flags & MASK_COLOR) == COLOR_RGB_GRAY) && ic <= 2){\
      int icc = (ic <= 2) ? 1 : 3;\
      int occ = (oc <= 2) ? 1 : 3;\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          for (int k = 0; k < occ; k++){\
            out[(i*w+j)*oc+k] = inp[(i*w+j)*ic+k%icc];\
          }\
        }\
      }\
    }else if ((flags & MASK_COLOR) == COLOR_RGB_GRAY){\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          float ir = inp[(i*w+j)*ic+0];\
          float ig = inp[(i*w+j)*ic+1];\
          float ib = inp[(i*w+j)*ic+2];\
          out[(i*w+j)*oc] = (ir*0.2126 + ig*0.7152 + ib*0.0722)/div0*div1;\
        }\
      }\
    }else if ((flags & MASK_COLOR) == COLOR_RGB_BGR){\
      int icc = (ic <= 2) ? 1 : 3;\
      int occ = (oc <= 2) ? 1 : 3;\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          for (int k = 0; k < occ; k++){\
            out[(i*w+j)*oc+k] = inp[(i*w+j)*ic+icc-1-(k%icc)];\
          }\
        }\
      }\
    }else if ((flags & MASK_COLOR) == COLOR_RGB_HSV || (flags & MASK_COLOR) == COLOR_HSV_RGB){\
      void (*F)(float,float,float,float*,float*,float*);\
      if ((flags & MASK_COLOR) == COLOR_RGB_HSV) F = rgb2hsv;\
      if ((flags & MASK_COLOR) == COLOR_HSV_RGB) F = hsv2rgb;\
      int icc = (ic <= 2) ? 1 : 3;\
      int occ = (oc <= 2) ? 1 : 3;\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          float ir = (float)(inp[(i*w+j)*ic+0%icc])/div0;\
          float ig = (float)(inp[(i*w+j)*ic+1%icc])/div0;\
          float ib = (float)(inp[(i*w+j)*ic+2%icc])/div0;\
          float oh,os,ov;\
          F(ir,ig,ib,&oh,&os,&ov);\
          out[(i*w+j)*oc+0%occ] = oh*div1;\
          out[(i*w+j)*oc+1%occ] = os*div1;\
          out[(i*w+j)*oc+2%occ] = ov*div1;\
        }\
      }\
    }else if ((flags & MASK_COLOR) == COLOR_SRGB_LIN || (flags & MASK_COLOR) == COLOR_LIN_SRGB){\
      float gamma = ((flags & MASK_COLOR) == COLOR_SRGB_LIN) ? 2.2 : 0.45;\
      int icc = (ic <= 2) ? 1 : 3;\
      int occ = (oc <= 2) ? 1 : 3;\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          for (int k = 0; k < occ; k++){\
            out[(i*w+j)*oc+k] = powf((float)(inp[(i*w+j)*ic+k%icc])/div0,gamma)*div1;\
          }\
        }\
      }\
    }\
    if ((flags & MASK_ALPHA) != ALPHA_DROP && (oc == 2 || oc == 4)){\
      int icc = (ic <= 2) ? 1 : 3;\
      int occ = (oc <= 2) ? 1 : 3;\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          out[(i*w+j)*oc+occ] = inp[(i*w+j)*ic+icc];\
        }\
      }\
    }\
    if ((flags & MASK_ALPHA) == ALPHA_PREMUL && (oc == 2 || oc == 4)){\
      int icc = (ic <= 2) ? 1 : 3;\
      int occ = (oc <= 2) ? 1 : 3;\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          for (int k = 0; k < oc; k++){\
            out[(i*w+j)*oc+k] = (out[(i*w+j)*ic+k]*inp[(i*w+j)*ic+icc])*div1/div0/div0;\
          }\
        }\
      }\
    }\
  }

GENERATE_CONVERTER(uint8_t,255,uint8_t,255);
GENERATE_CONVERTER(uint8_t,255,float,1);
GENERATE_CONVERTER(float,1,uint8_t,255);
GENERATE_CONVERTER(float,1,float,1);