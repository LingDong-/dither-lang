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
#define COLOR_INVERT     7
#define ALPHA_COPY       0
#define ALPHA_DROP       16
#define ALPHA_PREMUL     32
#define ALPHA_STRAIGHTEN 48

#define THRESH_BINARY   256
#define THRESH_AUTO     512
#define THRESH_ADAPTIVE 768

#define MORPH_ERODE       16
#define MORPH_DILATE      32
#define MORPH_OPEN        48
#define MORPH_CLOSE       64
#define MORPH_SKELETONIZE 80

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
            out[(i*w+j)*oc+k] = inp[(i*w+j)*ic+k%icc]*div1/div0;\
          }\
        }\
      }\
    }else if ((flags & MASK_COLOR) == COLOR_INVERT){\
      int icc = (ic <= 2) ? 1 : 3;\
      int occ = (oc <= 2) ? 1 : 3;\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          for (int k = 0; k < occ; k++){\
            out[(i*w+j)*oc+k] = (div0-inp[(i*w+j)*ic+k%icc])*div1/div0;\
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
            out[(i*w+j)*oc+k] = inp[(i*w+j)*ic+icc-1-(k%icc)]*div1/div0;\
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
    if ((flags & MASK_ALPHA) != ALPHA_DROP && (oc == 2 || oc == 4) && (ic == 2 || ic == 4)){\
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
GENERATE_CONVERTER(uint8_t,255,float,1.0);
GENERATE_CONVERTER(float,1.0,uint8_t,255);
GENERATE_CONVERTER(float,1.0,float,1.0);


void img_impl_threshold(uint8_t* pix, int w, int h, int thresh, int flags){
  if ((flags & 0xff00) == THRESH_BINARY){
    for (int i = 0; i < h; i++){
      for (int j = 0; j < w; j++){
        int b = pix[i*w+j] > thresh;
        pix[i*w+j] = b ? 255 : 0;
      }
    }
  }else if ((flags & 0xff00) == THRESH_AUTO){
    int hsz = 256 * sizeof(int);
    if (tmp_buf_len < hsz){
      tmp_buf_len = hsz;
      tmp_buf = realloc(tmp_buf,hsz);
    }
    int* hist = (int*)tmp_buf;
    memset(hist,0,sizeof(int)*256);
    for (int i = 0; i < h; i++){
      for (int j = 0; j < w; j++){
        uint8_t v = pix[i*w+j];
        hist[v] ++;
      }
    }
    int64_t total = (int64_t)w*(int64_t)h;
    int64_t sum = 0;
    for (int i = 0; i < 256; i++) sum += i*(int64_t)hist[i];
    int64_t sumB = 0;
    int64_t wB = 0;
    double maxVar = 0;
    int threshold = 0;
    for (int t = 0; t < 256; t++){
      wB += hist[t];
      if (wB == 0) continue;
      int64_t wF = total - wB;
      if (wF == 0) break;
      sumB += t * hist[t];
      double mB = sumB / (double)wB;
      double mF = (sum - sumB) / (double)wF;
      double betweenVar = wB * wF * (mB-mF) * (mB-mF);
      if (betweenVar > maxVar){
        maxVar = betweenVar;
        threshold = t;
      }
    }
    thresh = threshold;

    for (int i = 0; i < h; i++){
      for (int j = 0; j < w; j++){
        int b = pix[i*w+j] > thresh;
        pix[i*w+j] = b ? 255 : 0;
      }
    }
  }else if ((flags & 0xff00) == THRESH_ADAPTIVE){
    int sz = w*h*sizeof(uint8_t)*2;
    if (tmp_buf_len < sz){
      tmp_buf_len = sz;
      tmp_buf = realloc(tmp_buf,tmp_buf_len);
    }
    uint8_t* blurx = (uint8_t*)(tmp_buf);
    uint8_t* blury = (uint8_t*)(tmp_buf + (w*h*sizeof(uint8_t)));
    int sig = flags & 0xff;
    int rad = sig*3;
    int ksz = rad*2+1;
    float kern[ksz];
    for (int i = 0; i < ksz; i++){
      kern[i] = exp(-(i-rad)*(i-rad)/(2.0*sig*sig));
    }
    for (int i = 0; i < h; i++){
      for (int j = 0; j < w; j++){
        float n = 0;
        float s = 0;
        for (int k = j-rad; k <= j+rad; k++){
          if (k < 0) continue;
          if (k >= w) continue;
          float ki = kern[k-j+rad];
          s += pix[i*w+k]*ki;
          n+=ki;
        }
        blurx[i*w+j] = s/n;
      }
    }
    for (int i = 0; i < h; i++){
      for (int j = 0; j < w; j++){
        float n = 0;
        float s = 0;       
        for (int k = i-rad; k <= i+rad; k++){
          if (k < 0) continue;
          if (k >= h) continue;
          float ki = kern[k-i+rad];
          s += blurx[k*w+j]*ki;
          n+=ki;
        }
        blury[i*w+j] = s/n;
      }
    }
    for (int i = 0; i < h; i++){
      for (int j = 0; j < w; j++){
        int b = pix[i*w+j] > blury[i*w+j]+thresh;
        pix[i*w+j] = b ? 255 : 0;
        // pix[i*w+j] = blurx[i*w+j];
      }
    }
  }
}

void erode_or_dilate(uint8_t* pix, int w, int h, uint8_t* kern, int rad, int flags, uint8_t* out){
  int v0 = ((flags & 0xf0) == MORPH_DILATE) ? 0 : 255;
  int ksz = rad*2+1;
  for (int i = 0; i < h; i++){
    for (int j = 0; j < w; j++){
      int v = v0;
      for (int k = i-rad; k <= i+rad; k++){
        if (k < 0) continue;
        if (k >= h) continue;
        for (int l = j-rad; l <= j+rad; l++){
          if (l < 0) continue;
          if (l >= w) continue;
          int e = kern[ (k-i+rad) * ksz + (l-j+rad) ];
          if (!e) continue;
          if (v0){
            v = MIN(v,pix[k*w+l]);
          }else{
            v = MAX(v,pix[k*w+l]);
          }
        }
      }
      out[i*w+j] = v;
    }
  }
}

int thinning_zs_iteration(uint8_t* im, int W, int H, int iter) {
  int diff = 0;
  for (int i = 1; i < H-1; i++){
    for (int j = 1; j < W-1; j++){
      int p2 = im[(i-1)*W+j]   & 1;
      int p3 = im[(i-1)*W+j+1] & 1;
      int p4 = im[(i)*W+j+1]   & 1;
      int p5 = im[(i+1)*W+j+1] & 1;
      int p6 = im[(i+1)*W+j]   & 1;
      int p7 = im[(i+1)*W+j-1] & 1;
      int p8 = im[(i)*W+j-1]   & 1;
      int p9 = im[(i-1)*W+j-1] & 1;
      int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
               (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
               (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
               (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
      int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
      int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
      int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);
      if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
        im[i*W+j] |= 2;
    }
  }
  for (int i = 0; i < H*W; i++){
    int marker = im[i]>>1;
    int old = im[i]&1;
    im[i] = old & (!marker);
    if ((!diff) && (im[i] != old)){
      diff = 1;
    }
  }
  return diff;
}

void thinning_zs(uint8_t* im, int W, int H){
  int diff = 1;
  do {
    diff &= thinning_zs_iteration(im,W,H,0);
    diff &= thinning_zs_iteration(im,W,H,1);
  }while (diff);
}

void img_impl_morphology(uint8_t* pix, int w, int h, int rad, int flags, uint8_t* out){
  if (((flags & 0xf0) == MORPH_SKELETONIZE)){
    int sz = w*h*sizeof(uint8_t);
    if (tmp_buf_len < sz){
      tmp_buf_len = sz;
      tmp_buf = realloc(tmp_buf,tmp_buf_len);
    }
    uint8_t* buf = (uint8_t*)tmp_buf;
    for (int i = 0; i < w*h; i++){
      buf[i] = pix[i] > 128 ? 1 : 0;
    }
    thinning_zs(buf,w,h);
    for (int i = 0; i < w*h; i++){
      out[i] = buf[i]?255:0;
    }
    return;
  }

  int ksz = rad*2+1;
  int sz = ksz*ksz*sizeof(uint8_t);
  uint8_t* proc = out;
  if (out == pix){
    sz += w*h*sizeof(uint8_t);
  }
  uint8_t* inter = NULL;
  int twostep = ((flags & 0xf0) == MORPH_OPEN) || ((flags & 0xf0) == MORPH_CLOSE);
  if (twostep){
    sz += w*h*sizeof(uint8_t);
  }
  if (tmp_buf_len < sz){
    tmp_buf_len = sz;
    tmp_buf = realloc(tmp_buf,tmp_buf_len);
  }
  sz = ksz*ksz*sizeof(uint8_t);
  if (out == pix){
    proc = (uint8_t*)(tmp_buf + sz);
    sz += w*h*sizeof(uint8_t);
  }
  if (twostep){
    inter = (uint8_t*)(tmp_buf + sz);
  }

  uint8_t* kern = (uint8_t*)tmp_buf;

  if ((flags & 0xf) == NORM_LINF){
    memset(kern, 1, ksz*ksz);
  }else if ((flags & 0xf) == NORM_L1){
    for (int i = 0; i < ksz; i++){
      for (int j = 0; j < ksz; j++){
        int d = abs(i-rad)+abs(j-rad);
        kern[i*ksz+j] = (d <= rad);
      }
    }
  }else if ((flags & 0xf) == NORM_L2){
    for (int i = 0; i < ksz; i++){
      for (int j = 0; j < ksz; j++){
        float d = hypot(i-rad,j-rad);
        kern[i*ksz+j] = (d <= rad);
      }
    }
  }
  if ((flags & 0xf0) == MORPH_ERODE || (flags & 0xf0) == MORPH_DILATE){
    erode_or_dilate(pix,w,h,kern,rad,flags,proc);
  }else if ((flags & 0xf0) == MORPH_OPEN){
    erode_or_dilate(pix,w,h,kern,rad,MORPH_ERODE,inter);
    erode_or_dilate(inter,w,h,kern,rad,MORPH_DILATE,proc);
  }else if ((flags & 0xf0) == MORPH_CLOSE){
    erode_or_dilate(pix,w,h,kern,rad,MORPH_DILATE,inter);
    erode_or_dilate(inter,w,h,kern,rad,MORPH_ERODE,proc);
  }

  if (out == proc) return;
  memcpy(out,proc,w*h*sizeof(uint8_t));
}