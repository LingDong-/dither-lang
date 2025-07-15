#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>

uint32_t jsr = 0x5EED;

float rand_impl_random() {
  jsr^=(jsr<<17);
  jsr^=(jsr>>13);
  jsr^=(jsr<<5);
  return jsr/4294967295.0;
}

void rand_impl_seed(uint32_t x){
  jsr = x;
}

#define PERLIN_YWRAPB 4
#define PERLIN_YWRAP (1<<PERLIN_YWRAPB)
#define PERLIN_ZWRAPB 8
#define PERLIN_ZWRAP (1<<PERLIN_ZWRAPB)
#define PERLIN_SIZE 4095

int perlin_octaves = 4;
float perlin_amp_falloff = 0.5;
float* p_perlin;
int perlin_initialized = 0;

float scaled_cosine(float i){
  return 0.5*(1.0-cos(i*M_PI));
}

void rand_impl_noise_reseed(){
  int i;
  if (!perlin_initialized){
    p_perlin = (float*)malloc(sizeof(float)*(PERLIN_SIZE + 1));
    perlin_initialized = 1;
  }
  for (i = 0; i < PERLIN_SIZE + 1; i++) {
    p_perlin[i] = rand_impl_random();
  }
}

void rand_impl_noise_detail(int lod, float falloff){
  perlin_octaves = lod;
  perlin_amp_falloff = falloff;
}

float rand_impl_noise(float x, float y, float z) {
  int xi, yi, zi, o, of;
  float xf, yf, zf, rxf, ryf, r, ampl, n1, n2, n3;
  if (!perlin_initialized){
    rand_impl_noise_reseed();
  }
  if (x<0) { x=-x; } if (y<0) { y=-y; } if (z<0) { z=-z; }
  xi=(int)x; yi=(int)y; zi=(int)z;
  xf = x - xi; yf = y - yi; zf = z - zi;
  r=0; ampl=0.5;
  for (o=0; o<perlin_octaves; o++) {
    of=xi+(yi<<PERLIN_YWRAPB)+(zi<<PERLIN_ZWRAPB);
    rxf = scaled_cosine(xf); ryf = scaled_cosine(yf);
    n1  = p_perlin[of&PERLIN_SIZE];
    n1 += rxf*(p_perlin[(of+1)&PERLIN_SIZE]-n1);
    n2  = p_perlin[(of+PERLIN_YWRAP)&PERLIN_SIZE];
    n2 += rxf*(p_perlin[(of+PERLIN_YWRAP+1)&PERLIN_SIZE]-n2);
    n1 += ryf*(n2-n1);
    of += PERLIN_ZWRAP;
    n2  = p_perlin[of&PERLIN_SIZE];
    n2 += rxf*(p_perlin[(of+1)&PERLIN_SIZE]-n2);
    n3  = p_perlin[(of+PERLIN_YWRAP)&PERLIN_SIZE];
    n3 += rxf*(p_perlin[(of+PERLIN_YWRAP+1)&PERLIN_SIZE]-n3);
    n2 += ryf*(n3-n2);
    n1 += scaled_cosine(zf)*(n2-n1);
    r += n1*ampl;
    ampl *= perlin_amp_falloff;
    xi<<=1; xf*=2; yi<<=1; yf*=2; zi<<=1; zf*=2;
    if (xf>=1.0) { xi++; xf--; }
    if (yf>=1.0) { yi++; yf--; }
    if (zf>=1.0) { zi++; zf--; }
  }
  return r;
}
