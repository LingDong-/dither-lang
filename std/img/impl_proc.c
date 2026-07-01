#ifdef __APPLE__
#include <Accelerate/Accelerate.h>
#define USE_ACCELERATE
#endif

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

#define BORDER_ZERO 0
#define BORDER_COPY 16

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#if _WIN32
#define VLA(dtype,name,n) dtype* name = (dtype*)_alloca((n)*sizeof(dtype));
#else
#define VLA(dtype,name,n) dtype name[n];
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
    return INT16_MIN;
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
int inttyp_pix_get(void* b, int idx, int dsize){
  int r = 0;
  memcpy(&r, b+(idx*dsize), dsize);
  return r;
}
void inttyp_pix_set(void* b, int idx, int dsize, int val){
  memcpy(b+(idx*dsize), &val, dsize);
}
void img_impl_dist_transform(void* b, int m, int n, int dsize, int flags, float* dt){
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
  int tsz = m*n*2+m*2+m*2+m*n*4;
  if (tmp_buf_len < tsz){
    tmp_buf_len = tsz;
    tmp_buf = realloc(tmp_buf,tsz);
  }
  int16_t* g = (int16_t*)tmp_buf;
  int16_t* s = (int16_t*)(tmp_buf + (m*n*2));
  int16_t* t = (int16_t*)(tmp_buf + (m*n*2+m*2));
  int* v = (int*)(tmp_buf + (m*n*2+m*2+m*2));

  for (int x = 0; x < m; x++) {
    if (inttyp_pix_get(b, x + 0 * m, dsize)){
      g[x + 0 * m] = 0;
      v[x + 0 * m] = inttyp_pix_get(b, x + 0 * m, dsize);
    }else{
      g[x + 0 * m] = m+n;
      v[x + 0 * m] = 0;
    }
    for (int y = 1; y < n; y++) {
      if (inttyp_pix_get(b, x + y * m, dsize)){
        g[x + y * m] = 0;
        v[x + y * m] = inttyp_pix_get(b, x + y * m, dsize);
      }else{
        g[x + y * m] = 1 + g[x + (y - 1) * m];
        v[x + y * m] = v[x + (y - 1) * m];
      }
    }
    for (int y = n - 2; y >= 0; y--) {
      if (g[x + (y + 1) * m] < g[x + y * m]){
        g[x + y * m] = 1 + g[x + (y + 1) * m];
        v[x + y * m] = v[x + (y + 1) * m];
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
      dt[u + y * m] = d;
      if (f == EDT_f) dt[u + y * m] = sqrt(dt[u + y * m]);
      if (do_voro) inttyp_pix_set(b, u + y * m, dsize, v[s[q] + y * m]);
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
    int icc = (ic <= 2) ? 1 : 3;\
    int occ = (oc <= 2) ? 1 : 3;\
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
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          for (int k = 0; k < occ; k++){\
            out[(i*w+j)*oc+k] = inp[(i*w+j)*ic+k%icc]*div1/div0;\
          }\
        }\
      }\
    }else if ((flags & MASK_COLOR) == COLOR_INVERT){\
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
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          for (int k = 0; k < occ; k++){\
            out[(i*w+j)*oc+k] = powf((float)(inp[(i*w+j)*ic+k%icc])/div0,gamma)*div1;\
          }\
        }\
      }\
    }\
    if ((flags & MASK_ALPHA) != ALPHA_DROP && (oc == 2 || oc == 4) && (ic == 2 || ic == 4)){\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          out[(i*w+j)*oc+occ] = inp[(i*w+j)*ic+icc];\
        }\
      }\
    }\
    if ((flags & MASK_ALPHA) == ALPHA_PREMUL && (oc == 2 || oc == 4)){\
      for (int i = 0; i < h; i++){\
        for (int j = 0; j < w; j++){\
          for (int k = 0; k < occ; k++){\
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
    VLA(float,kern,ksz);
    for (int i = 0; i < ksz; i++){
      kern[i] = exp(-(i-rad)*(i-rad)/(2.0*sig*sig));
    }
    #ifdef USE_ACCELERATE
      vImage_Buffer src = { pix, h, w, w * sizeof(uint8_t) };
      vImage_Buffer dst = { blury, h, w, w * sizeof(uint8_t) };
      float sum = 0;
      for (int i = 0; i < ksz; i++)sum += kern[i];
      for (int i = 0; i < ksz; i++)kern[i]/=sum;
      vImageSepConvolve_Planar8(&src,&dst,(void*)blurx,0,0,kern,ksz,kern,ksz,0,0,kvImageTruncateKernel);
    #else
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
    #endif
    for (int i = 0; i < h; i++){
      for (int j = 0; j < w; j++){
        int b = pix[i*w+j] > blury[i*w+j]+thresh;
        pix[i*w+j] = b ? 255 : 0;
        // pix[i*w+j] = blury[i*w+j];
      }
    }
  }
}

void erode_or_dilate(uint8_t* pix, int w, int h, uint8_t* kern, int rad, int flags, uint8_t* out){
  int ksz = rad*2+1;

  #ifdef USE_ACCELERATE
    vImage_Buffer src = { pix, h, w, w * sizeof(uint8_t) };
    vImage_Buffer dst = { out, h, w, w * sizeof(uint8_t) };
    if ((flags & 0xf0) == MORPH_DILATE){
      for (int i = 0; i < ksz*ksz; i++) kern[i] = kern[i]?0:255;
      vImageDilate_Planar8(&src,&dst,0,0,kern,ksz,ksz,kvImageNoFlags);
    }else{
      for (int i = 0; i < ksz*ksz; i++) kern[i] = kern[i]?255:0;
      vImageErode_Planar8(&src,&dst,0,0,kern,ksz,ksz,kvImageNoFlags);
    }
  #else
    int v0 = ((flags & 0xf0) == MORPH_DILATE) ? 0 : 255;
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
  #endif
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


#define CONVOLVE_SIMPLE \
  int kx = (kw-1)/2;\
  int ky = (kh-1)/2;\
  int border = (flags&0xf0)==BORDER_COPY;\
  for (int i = 0; i < h; i++){\
    for (int j = 0; j < w; j++){\
      float s = 0.0;\
      for (int k = 0; k < kh; k++){\
        for (int l = 0; l < kw; l++){\
          int ii = i+k-kx;\
          int jj = j+l-ky;\
          int iii = MIN(MAX(ii,0),h-1);\
          int jjj = MIN(MAX(jj,0),w-1);\
          if (border || (iii==ii&&jjj==jj)){\
            s += pix[iii*w+jjj]*kern[k*kw+l];\
          }\
        }\
      }\
      proc[i*w+j] = s;\
    }\
  }

void img_impl_convolve_u8(uint8_t* pix, int w, int h, float* kern, int kw, int kh, int flags, uint8_t* out){
  uint8_t* proc = out;
  if (out == pix){
    int sz = w*h*sizeof(uint8_t);
    if (tmp_buf_len < sz){
      tmp_buf_len = sz;
      tmp_buf = realloc(tmp_buf,tmp_buf_len);
    }
    proc = (uint8_t*)tmp_buf;
  }
  #ifdef USE_ACCELERATE
    vImage_Buffer src = { pix, h, w, w * sizeof(uint8_t) };
    vImage_Buffer dst = { proc, h, w, w * sizeof(uint8_t) };
    vImage_Flags border = ((flags&0xf0)==BORDER_COPY) ? kvImageEdgeExtend : kvImageBackgroundColorFill;
    int okw = kw;
    int okh = kh;
    if (!(okw&1)) okw++;
    if (!(okh&1)) okh++;
    VLA(int16_t, k16, okw*okh);
    memset(k16,0,sizeof(int16_t)*okw*okh);
    float extrema = 0.0;
    for (int i = 0; i < kw*kh; i++){
      float a = fabsf(kern[i]);
      if (a>extrema){
        extrema = a;
      }
    }
    int divisor = 16384 / extrema;
    for (int i = 0; i < kh; i++){
      for (int j = 0; j < kw; j++){
        k16[i*okw+j] = kern[i*kw+j]*divisor;
      }
    }
    vImageConvolve_Planar8(&src,&dst,NULL,0,0,k16,okh,okw,divisor,0,border);
  #else
    CONVOLVE_SIMPLE
  #endif
  if (out == proc) return;
  memcpy(out,proc,w*h*sizeof(uint8_t));
}


void img_impl_convolve_f32(float* pix, int w, int h, float* kern, int kw, int kh, int flags, float* out){
  float* proc = out;
  if (out == pix){
    int sz = w*h*sizeof(float);
    if (tmp_buf_len < sz){
      tmp_buf_len = sz;
      tmp_buf = realloc(tmp_buf,tmp_buf_len);
    }
    proc = (float*)tmp_buf;
  }
  #ifdef USE_ACCELERATE
    vImage_Buffer src = { pix, h, w, w * sizeof(float) };
    vImage_Buffer dst = { proc, h, w, w * sizeof(float) };
    vImage_Flags border = ((flags&0xf0)==BORDER_COPY) ? kvImageEdgeExtend : kvImageBackgroundColorFill;
    int okw = kw;
    int okh = kh;
    if (!(okw&1)) okw++;
    if (!(okh&1)) okh++;
    if (okw == kw && okh == kh){
      vImageConvolve_PlanarF(&src,&dst,NULL,0,0,kern,kh,kw,0.0,border);
    }else{
      VLA(float,kp,okw*okh);
      memset(kp,0,sizeof(float)*okw*okh);
      for (int i = 0; i < kw; i++){
        for (int j = 0; j < kh; j++){
          kp[i*okw+j] = kern[i*kw+j];
        }
      }
      vImageConvolve_PlanarF(&src,&dst,NULL,0,0,kp,okh,okw,0.0,border);
    }
  #else
    CONVOLVE_SIMPLE
  #endif
  if (out == proc) return;
  memcpy(out,proc,w*h*sizeof(float));
}


// https://github.com/LingDong-/PContour

#define N_PIXEL_NEIGHBOR 8
#define BAD_INT -666

typedef struct {
  int i;
  int j;
} IJ;

typedef struct {
  /** Vertices */
  IJ* points;
  int len;
  int cap;
  /** Unique ID, starts from 2 */
  int id;
  /** ID of parent contour, 0 means top-level contour */
  int parent;
  /** Is this contour a hole (as opposed to outline) */
  char isHole;
} Contour;

// give pixel neighborhood counter-clockwise ID's for
// easier access with findContours algorithm
IJ neighborIDToIndex(int i, int j, int id){
  switch (id){
    case 0: {IJ ij = {i  , j+1};return ij;}
    case 1: {IJ ij = {i-1, j+1};return ij;}
    case 2: {IJ ij = {i-1, j  };return ij;}
    case 3: {IJ ij = {i-1, j-1};return ij;}
    case 4: {IJ ij = {i  , j-1};return ij;}
    case 5: {IJ ij = {i+1, j-1};return ij;}
    case 6: {IJ ij = {i+1, j  };return ij;}
    case 7: {IJ ij = {i+1, j+1};return ij;}
  };
  IJ ij = {BAD_INT,BAD_INT};
  return ij;
}
int neighborIndexToID(int i0, int j0, int i, int j){
  int di = i - i0;
  int dj = j - j0;
  if (di == 0 && dj == 1){return 0;}
  if (di ==-1 && dj == 1){return 1;}
  if (di ==-1 && dj == 0){return 2;}
  if (di ==-1 && dj ==-1){return 3;}
  if (di == 0 && dj ==-1){return 4;}
  if (di == 1 && dj ==-1){return 5;}
  if (di == 1 && dj == 0){return 6;}
  if (di == 1 && dj == 1){return 7;}
  return -1;
}

// first counter clockwise non-zero element in neighborhood
IJ ccwNon0(int* F, int w, int h, int i0, int j0, int i, int j, int offset){
  int id = neighborIndexToID(i0,j0,i,j);
  for (int k = 0; k < N_PIXEL_NEIGHBOR; k++){
    int kk = (k+id+offset + N_PIXEL_NEIGHBOR*2) % N_PIXEL_NEIGHBOR;
    IJ ij = neighborIDToIndex(i0,j0,kk);
    if (F[ij.i*w+ij.j]!=0){
      return ij;
    }
  }
  IJ ij = {BAD_INT,BAD_INT};
  return ij;
}

// first clockwise non-zero element in neighborhood
IJ cwNon0(int* F, int w, int h, int i0, int j0, int i, int j, int offset){
  int id = neighborIndexToID(i0,j0,i,j);
  for (int k = 0; k < N_PIXEL_NEIGHBOR; k++){
    int kk = (-k+id-offset + N_PIXEL_NEIGHBOR*2) % N_PIXEL_NEIGHBOR;
    IJ ij = neighborIDToIndex(i0,j0,kk);
    if (F[ij.i*w+ij.j]!=0){
      return ij;
    }
  }
  IJ ij = {BAD_INT,BAD_INT};
  return ij;
}

/**
 * Find contours in a binary image
 * <p>
 * Implements Suzuki, S. and Abe, K.
 * Topological Structural Analysis of Digitized Binary Images by Border Following.
 * <p>
 * See source code for step-by-step correspondence to the paper's algorithm
 * description.
 * @param  F    The bitmap, stored in 1-dimensional row-major form.
 *              0=background, 1=foreground, will be modified by the function
 *              to hold semantic information
 * @param  w    Width of the bitmap
 * @param  h    Height of the bitmap
 * @return      An array of contours found in the image.
 * @see         Contour
 */
Contour* findContours(int* F, int w, int h, int* nret) {
  // Topological Structural Analysis of Digitized Binary Images by Border Following.
  // Suzuki, S. and Abe, K., CVGIP 30 1, pp 32-46 (1985)
  Contour* contours = NULL;
  int lencnt = 0;
  int capcnt = 0;

  int nbd = 1;
  int lnbd = 1;
  
  // Without loss of generality, we assume that 0-pixels fill the frame
  // of a binary picture
  for (int i = 1; i < h-1; i++){
    F[i*w] = 0; F[i*w+w-1]=0;
  }
  for (int i = 0; i < w; i++){
    F[i] = 0; F[w*h-1-i]=0;
  }

  //Scan the picture with a TV raster and perform the following steps
  //for each pixel such that fij # 0. Every time we begin to scan a
  //new row of the picture, reset LNBD to 1.
  for (int i = 1; i < h-1; i++) {
    lnbd = 1;

    for (int j = 1; j < w-1; j++) {
      
      int i2 = 0, j2 = 0;
      if (F[i*w+j] == 0) {
        continue;
      }
      //(a) If fij = 1 and fi, j-1 = 0, then decide that the pixel
      //(i, j) is the border following starting point of an outer
      //border, increment NBD, and (i2, j2) <- (i, j - 1).
      if (F[i*w+j] == 1 && F[i*w+(j-1)] == 0) {
        nbd ++;
        i2 = i;
        j2 = j-1;
        
        
      //(b) Else if fij >= 1 and fi,j+1 = 0, then decide that the
      //pixel (i, j) is the border following starting point of a
      //hole border, increment NBD, (i2, j2) <- (i, j + 1), and
      //LNBD + fij in case fij > 1.
      } else if (F[i*w+j]>=1 && F[i*w+j+1] == 0) {
        nbd ++;
        i2 = i;
        j2 = j+1;
        if (F[i*w+j]>1) {
          lnbd = F[i*w+j];
        }
        
        
      } else {
        //(c) Otherwise, go to (4).
        //(4) If fij != 1, then LNBD <- |fij| and resume the raster
        //scan from pixel (i,j+1). The algorithm terminates when the
        //scan reaches the lower right corner of the picture
        if (F[i*w+j]!=1){lnbd = abs(F[i*w+j]);}
        continue;
        
      }
      //(2) Depending on the types of the newly found border
      //and the border with the sequential number LNBD
      //(i.e., the last border met on the current row),
      //decide the parent of the current border as shown in Table 1.
      // TABLE 1
      // Decision Rule for the Parent Border of the Newly Found Border B
      // ----------------------------------------------------------------
      // Type of border B'
      // \    with the sequential
      //     \     number LNBD
      // Type of B \                Outer border         Hole border
      // ---------------------------------------------------------------
      // Outer border               The parent border    The border B'
      //                            of the border B'
      //
      // Hole border                The border B'      The parent border
      //                                               of the border B'
      // ----------------------------------------------------------------
      
      Contour B = {0};
      IJ ij = {i,j};
      B.points = (IJ*)malloc((B.cap=32)*sizeof(IJ));
      B.points[B.len++] = ij;
      B.isHole = (j2 == j+1);
      B.id = nbd;
      if (capcnt < lencnt + 1){
        int hs = capcnt/2;
        capcnt = lencnt + MAX(16,hs);
        contours = (Contour*)realloc(contours, capcnt * sizeof(Contour));
      }
      contours[lencnt++] = B;

      Contour B1;
      for (int c = 0; c < lencnt; c++){
        if (contours[c].id == lnbd){
          B1 = contours[c];
          break;
        }
      }
      if (B1.isHole){
        if (B.isHole){
          B.parent = B1.parent;
        }else{
          B.parent = lnbd;
        }
      }else{
        if (B.isHole){
          B.parent = lnbd;
        }else{
          B.parent = B1.parent;
        }
      }
      
      //(3) From the starting point (i, j), follow the detected border:
      //this is done by the following substeps (3.1) through (3.5).
      
      //(3.1) Starting from (i2, j2), look around clockwise the pixels
      //in the neigh- borhood of (i, j) and tind a nonzero pixel.
      //Let (i1, j1) be the first found nonzero pixel. If no nonzero
      //pixel is found, assign -NBD to fij and go to (4).
      int i1 = -1, j1 = -1;
      IJ i1j1 = cwNon0(F,w,h,i,j,i2,j2,0);
      if (i1j1.i == BAD_INT){
        F[i*w+j] = -nbd;
        //go to (4)
        if (F[i*w+j]!=1){lnbd = abs(F[i*w+j]);}
        continue;
      }
      i1 = i1j1.i; j1 = i1j1.j;
      
      // (3.2) (i2, j2) <- (i1, j1) ad (i3,j3) <- (i, j).
      i2 = i1;
      j2 = j1;
      int i3 = i;
      int j3 = j;
      
      
      while (true){
        //(3.3) Starting from the next elementof the pixel (i2, j2)
        //in the counterclock- wise order, examine counterclockwise
        //the pixels in the neighborhood of the current pixel (i3, j3)
        //to find a nonzero pixel and let the first one be (i4, j4).
        
        IJ i4j4 = ccwNon0(F,w,h,i3,j3,i2,j2,1);
        int i4 = i4j4.i;
        int j4 = i4j4.j;
        
        if (contours[lencnt-1].cap < contours[lencnt-1].len + 1){
          int hs = contours[lencnt-1].cap/2;
          contours[lencnt-1].cap += MAX(32,hs);
          contours[lencnt-1].points = (IJ*)realloc(contours[lencnt-1].points, contours[lencnt-1].cap * sizeof(IJ));
        }
        contours[lencnt-1].points[contours[lencnt-1].len++] = i4j4;
        
        //(a) If the pixel (i3, j3 + 1) is a O-pixel examined in the
        //substep (3.3) then fi3, j3 <-  -NBD.
        if (F[i3*w+j3+1] == 0){
          F[i3*w+j3] = -nbd;
          
        //(b) If the pixel (i3, j3 + 1) is not a O-pixel examined
        //in the substep (3.3) and fi3,j3 = 1, then fi3,j3 <- NBD.
        }else if (F[i3*w+j3] == 1){
          F[i3*w+j3] = nbd;
        }else{
          //(c) Otherwise, do not change fi3, j3.
        }
        
        //(3.5) If (i4, j4) = (i, j) and (i3, j3) = (i1, j1)
        //(coming back to the starting point), then go to (4);
        if (i4 == i && j4 == j && i3 == i1 && j3 == j1){
          if (F[i*w+j]!=1){lnbd = abs(F[i*w+j]);}
          break;
          
        //otherwise, (i2, j2) + (i3, j3),(i3, j3) + (i4, j4),
        //and go back to (3.3).
        }else{
          i2 = i3;
          j2 = j3;
          i3 = i4;
          j3 = j4;
        }
      }
    }
  }
  *nret = lencnt;
  return contours;
}


float** img_impl_find_contours(uint8_t* pix, int w, int h, int* out_n, int** out_lens){
  int sz = w*h*sizeof(int);
  if (tmp_buf_len < sz){
    tmp_buf_len = sz;
    tmp_buf = realloc(tmp_buf,tmp_buf_len);
  }
  int* F = (int*)tmp_buf;
  for (int i = 0; i < w*h; i++){
    F[i] = !!(pix[i]);
  }
  int nret = 0;
  Contour* contours = findContours(F, w, h, &nret);
  float** out = (float**)malloc(nret*sizeof(float*));
  int* lens = (int*)malloc(nret*sizeof(int));
  static const float eps = 0.001;
  int k = 0;
  for (int i = 0; i < nret; i++){
    int cnt = 0;
    out[k] = (float*)malloc(contours[i].len*sizeof(float)*2);
    for (int j = 0; j < contours[i].len; j++){
      float x1 = contours[i].points[j].j;
      float y1 = contours[i].points[j].i;
      if (j == 0 || j == contours[i].len-1){
        out[k][cnt*2+0] = x1;
        out[k][cnt*2+1] = y1;
        cnt++;
      }else{
        float x0 = out[k][(cnt-1)*2+0];
        float y0 = out[k][(cnt-1)*2+1];
        float x2 = contours[i].points[j+1].j;
        float y2 = contours[i].points[j+1].i;
        float cw = (((x1)-(x0))*((y2)-(y0)) - ((x2)-(x0))*((y1)-(y0)));
        if (cw > eps || cw < -eps){
          out[k][cnt*2+0] = x1;
          out[k][cnt*2+1] = y1;
          cnt++;
        }
      }
    }
    if (cnt >= 3){
      lens[k++] = cnt;
    }else{
      free(out[k]);
    }
  }
  *out_lens = lens;
  *out_n = k;
  return out;
}

int lbl_assoc(int* lbl_eq, int pw, int pn){
  if (pw == pn) return pw;
  if (!pw) return pn;
  if (!pn) return pw;
  int pa = pw;
  int pb = pn;
  if (pa > pb){
    pa = pn;
    pb = pw;
  }
  if (!lbl_eq[pb]){
    lbl_eq[pb] = pa;
    return pa;
  }
  int pc = lbl_eq[pb];
  return (lbl_eq[pb] = lbl_assoc(lbl_eq,pa,pc));
}

void img_impl_label_blobs(uint8_t* pix, int w, int h, int* out){
  int* lbl_eq = (int*)tmp_buf;

  int lbl = 1;
  for (int i = 0; i < h; i++){
    for (int j = 0; j < w; j++){
      if (pix[i*w+j]){
        int pw = j ? out[i*w+j-1] : 0;
        int pn = i ? out[i*w+j-w] : 0;
        if (pw == 0 && pn == 0){
          out[i*w+j] = lbl;
          if (tmp_buf_len <= lbl*sizeof(int)){
            tmp_buf_len = (lbl*2+1)*sizeof(int);
            tmp_buf = realloc(tmp_buf,tmp_buf_len);
            lbl_eq = (int*)tmp_buf;
          }
          lbl_eq[lbl] = 0;
          lbl ++;
        }else{
          out[i*w+j] = lbl_assoc(lbl_eq,pw,pn);
        }
      }else{
        out[i*w+j] = 0;
      }
    }
  }
  int* uniques = (int*)calloc(lbl,sizeof(int));
  int nid = 1;
  for (int i = 1; i < lbl; i++){
    if (!lbl_eq[i]){
      uniques[i] = nid++;
    }
  }
  for (int i = 0; i < h; i++){
    for (int j = 0; j < w; j++){
      if (out[i*w+j]){
        int ini = out[i*w+j];
        int bot = ini;
        while (lbl_eq[bot]){
          bot = lbl_eq[bot];
        }
        if (bot != ini){
          lbl_eq[ini] = bot;
        }
        out[i*w+j] = uniques[bot];
        // out[i*w+j] = bot;
      }
    }
  }
  free(uniques);
  
}