#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#define LHS_CAPL   1
#define RHS_CAPL   4
#define LHS_CAPR   2
#define RHS_CAPR   8
#define RET_POINTS 0
#define RET_PARAMS 16
#define BY_COUNT   0
#define BY_SPACING 1
#define MODE_POLYLINE   0
#define MODE_POLYGON  128
#define MODE_ALIGNED   0
#define MODE_ORIENTED  1
#define MODE_BEZIER   16
#define MODE_CATROM   17
#define MODE_BSPLINE  18
#define ORD_QUADRATIC  2
#define ORD_CUBIC      3

#define DET3(a,b,c,d,e,f,g,h,i) ((a)*(e)*(i) + (b)*(f)*(g) + (c)*(d)*(h) - (c)*(e)*(g) - (b)*(d)*(i) - (a)*(f)*(h))

int geom_impl_line_intersect_3d(
  float p0x, float p0y, float p0z,
  float p1x, float p1y, float p1z,
  float q0x, float q0y, float q0z,
  float q1x, float q1y, float q1z,
  int flags, float* o0, float* o1, float* o2
){
  float d0x = p1x - p0x;
  float d0y = p1y - p0y;
  float d0z = p1z - p0z;
  float d1x = q1x - q0x;
  float d1y = q1y - q0y;
  float d1z = q1z - q0z;

  float vcx = d0y*d1z-d0z*d1y;
  float vcy = d0z*d1x-d0x*d1z;
  float vcz = d0x*d1y-d0y*d1x;

  float vcn = vcx*vcx+vcy*vcy+vcz*vcz;
  if (vcn == 0){
    return 0;
  }

  float q0x_p0x = q0x - p0x;
  float q0y_p0y = q0y - p0y;
  float q0z_p0z = q0z - p0z;

  float t = DET3(q0x_p0x,q0y_p0y,q0z_p0z,d1x,d1y,d1z,vcx,vcy,vcz)/vcn;
  float s = DET3(q0x_p0x,q0y_p0y,q0z_p0z,d0x,d0y,d0z,vcx,vcy,vcz)/vcn;

  if (flags & RET_PARAMS){
    *o0 = t;
    *o1 = s;
    *o2 = 0;
  }else{
    *o0 = p0x * (1-t) + p1x * t;
    *o1 = p0y * (1-t) + p1y * t;
    *o2 = p0z * (1-t) + p1z * t;
  }
  if (
    (0 <= t || !(flags & LHS_CAPL)) && 
    (t < 1 || !(flags & LHS_CAPR)) && 
    (0 <= s || !(flags & RHS_CAPL)) && 
    (s < 1 || !(flags & RHS_CAPR))
  ) {
    return 1;
  }
  return 0;
}

int geom_impl_line_intersect_2d(
  float p0x, float p0y, 
  float p1x, float p1y,
  float q0x, float q0y,
  float q1x, float q1y,
  int flags, float* o0, float* o1
){
  float d0x = p1x - p0x;
  float d0y = p1y - p0y;
  float d1x = q1x - q0x;
  float d1y = q1y - q0y;
  float vc = d0x * d1y - d0y * d1x;
  if (vc == 0) {
    return 0;
  }
  float vcn = vc * vc;
  float q0x_p0x = q0x - p0x;
  float q0y_p0y = q0y - p0y;
  float vc_vcn = vc / vcn;
  float t = (q0x_p0x * d1y - q0y_p0y * d1x) * vc_vcn;
  float s = (q0x_p0x * d0y - q0y_p0y * d0x) * vc_vcn;
  if (flags & RET_PARAMS){
    *o0 = t;
    *o1 = s;
  }else{
    *o0 = p0x * (1-t) + p1x * t;
    *o1 = p0y * (1-t) + p1y * t;
  }
  if (
    (0 <= t || !(flags & LHS_CAPL)) && 
    (t < 1 || !(flags & LHS_CAPR)) && 
    (0 <= s || !(flags & RHS_CAPL)) && 
    (s < 1 || !(flags & RHS_CAPR))
  ) {
    return 1;
  }
  return 0;
}

float* acc_len = NULL;
int n_acc_len = 0;
float* geom_impl_poly_resample(int n_points, float* points, float n, int flags, int* n_out){
  if (n_points+1 > n_acc_len){
    acc_len = (float*)realloc(acc_len,(n_acc_len = n_points+1)*sizeof(float));
  }
  float tot_len = 0;
  for (int i = 0; i < n_points-1; i++){
    float dx = points[i*2] - points[(i+1)*2];
    float dy = points[i*2+1] - points[(i+1)*2+1];
    tot_len += sqrt(dx*dx+dy*dy);
    acc_len[i+1] = tot_len;
  }
  acc_len[0] = 0;
  acc_len[n_points] = tot_len;
  float spacing = 0;
  int count = 0;
  if (flags & BY_COUNT){
    spacing = tot_len/n;
    count = ceil(n);
  }else{ // BY_SPACING
    spacing = n;
    count = ceil(tot_len/spacing);
  }
  float* out = (float*)malloc(count*sizeof(float)*2);
  int idx = 0;
  *n_out = count;
  int lidx = 0;
  for (float l = 0; l < tot_len; l+= spacing){
    for (int i = lidx; i < n_points; i++){
      if (acc_len[i] <= l && l < acc_len[i+1]){
        float t = (l-acc_len[i])/(acc_len[i+1]-acc_len[i]);
        float x = points[i*2]*(1-t)+points[(i+1)*2]*t;
        float y = points[(i*2)+1]*(1-t)+points[(i+1)*2+1]*t;
        out[idx*2]   = x;
        out[idx*2+1] = y;
        idx ++;
        lidx = i;
        if (idx == count){
          free(acc_len);
          return out;
        }
        break;
      }
    }
  }
  free(acc_len);
  return out;
}

#define CWISE(x0,y0,x1,y1,x2,y2) (((x1)-(x0))*((y2)-(y0)) - ((x2)-(x0))*((y1)-(y0)))

int geom_impl_pt_in_poly(float x, float y, int n_points, float* points){
  int wn = 0;
  for (int i = 0, j = n_points-1; i < n_points; j = i++){
    float xi = points[i*2];
    float yi = points[i*2+1];
    float xj = points[j*2];
    float yj = points[j*2+1];
    if (yj <= y){
      if (yi > y){
        if (CWISE(xj,yj,xi,yi,x,y)>0){
          wn++;
        }
      }
    }else{
      if (yi <= y){
        if (CWISE(xj,yj,xi,yi,x,y)<0){
          wn--;
        }
      }
    }
  }
  return wn != 0;
}

float dist_pt_seg(float x0,float y0,float x1,float y1,float x2,float y2){
  float A = x0-x1;
  float B = y0-y1;
  float C = x2-x1;
  float D = y2-y1;
  float dot = A*C+B*D;
  float len_sq = C*C+D*D;
  float param = 01;
  if (len_sq != 0){
    param = dot/len_sq;
  }
  float xx,yy;
  if (param < 0){
    xx = x1; yy = y1;
  }else if (param > 1){
    xx = x2; yy = y2;
  }else{
    xx = x1 + param*C;
    yy = y1 + param*D;
  }
  float dx = x0-xx;
  float dy = y0-yy;
  return sqrt(dx*dx+dy*dy);
}

float* geom_impl_poly_simplify(float* points, int start, int end, float eps, int* n_out){
  int n_points = end - start;
  float* out;
  if (n_points <= 2){
    *n_out = n_points;
    out = (float*)malloc(sizeof(float)*n_points*2);
    memcpy(out, points+(start*2), n_points*sizeof(float)*2);
    return out;
  }
  float dmax = 0;
  int argmax = -1;
  for (int i = start+1; i < end-1; i++){
    float d = dist_pt_seg(
      points[i*2],points[i*2+1],
      points[start*2],points[start*2+1],
      points[(end-1)*2],points[(end-1)*2+1]
    );
    if (d > dmax){
      dmax = d;
      argmax = i;
    }
  }
  if (dmax > eps){
    int nL,nR,nO;
    float* L = geom_impl_poly_simplify(points, start, argmax+1, eps, &nL);
    float* R = geom_impl_poly_simplify(points, argmax, end, eps, &nR);
    nL--;
    nO = nL+nR;
    out = (float*)realloc(L,sizeof(float)*nO*2);
    memcpy(out + (nL*2), R, nR*sizeof(float)*2);
    *n_out = nO;
    free(R);
  }else{
    out = (float*)malloc(sizeof(float)*2*2);
    out[0] = points[start*2];
    out[1] = points[start*2+1];
    out[2] = points[(end-1)*2];
    out[3] = points[(end-1)*2+1];
    *n_out = 2;
  }
  return out;
}

#define SIGN(x) (((x)>0)-((x)<0))

float cur_px;
float cur_py;
int cmp_angle(const void *a, const void *b){
  float x0 = ((float*)a)[0] - cur_px;
  float y0 = ((float*)a)[1] - cur_py;
  float x1 = ((float*)b)[0] - cur_px;
  float y1 = ((float*)b)[1] - cur_py;
  float vc = x0*y1 - y0*x1;
  if (vc == 0){
    vc = x0+y0-x1-y1;
    return SIGN(vc);
  }
  return -SIGN(vc);
}

float* geom_impl_convex_hull(int n_points, float* points, int* n_out){
  int mi = 0;
  float my = INFINITY;
  float mx = INFINITY;
  for (int i = 0; i < n_points; i++){
    if (points[i*2+1]<my || (points[i*2+1]==my && points[i*2]<mx)){
      mx = points[i*2];
      my = points[i*2+1];
      mi = i;
    }
  }
  float px = points[mi*2];
  float py = points[mi*2+1];
  float* sorted = (float*)malloc(sizeof(float)*2*(n_points-1));
  cur_px = px;
  cur_py = py;
  memcpy(sorted, points, mi*2*sizeof(float));
  memcpy(sorted + (mi*2), points + ((mi+1)*2), (n_points-mi-1)*2*sizeof(float) );

  qsort(sorted, n_points-1, sizeof(float)*2, cmp_angle);

  // *n_out = n_points-1;
  // return sorted;

  int cap_stack = 4+n_points/4;
  float* stack = (float*)malloc(sizeof(float)*2*cap_stack);
  int n_stack = 2;
  stack[0] = px;
  stack[1] = py;
  stack[2] = sorted[0];
  stack[3] = sorted[1];
  for (int i = 1; i < n_points-1; i++){
    while (n_stack >= 2 && CWISE(
      stack[(n_stack-2)*2],stack[(n_stack-2)*2+1],
      stack[(n_stack-1)*2],stack[(n_stack-1)*2+1],
      sorted[i*2],sorted[i*2+1]
    )<=0){
      n_stack --;
    }
    if (n_stack >= cap_stack){
      cap_stack=cap_stack*2+1;
      if (cap_stack > n_points) cap_stack = n_points;
      stack = (float*)realloc(stack,sizeof(float)*2*cap_stack);
    }
    stack[n_stack*2] =  sorted[i*2];
    stack[n_stack*2+1] =sorted[i*2+1];
    n_stack++;
  }
  free(sorted);
  *n_out = n_stack;
  return stack;
}

#define PT_IN_TRI(p0x,p0y,p1x,p1y,p2x,p2y,p3x,p3y) (\
  CWISE(p0x,p0y, p1x,p1y, p2x,p2y)>=0 && \
  CWISE(p0x,p0y, p2x,p2y, p3x,p3y)>=0 && \
  CWISE(p0x,p0y, p3x,p3y, p1x,p1y)>=0)
  
int32_t* geom_impl_triangulate_simple(int n_points, float* points, int* o_n_tri){
  *o_n_tri = (n_points-2)*3;
  int32_t* out = (int32_t*)malloc(3*(n_points-2)*sizeof(int32_t));
  int* skips = calloc(n_points,sizeof(int));
  int n_out = 0;
  int i = -1;
  int skipped = -1;
  while (n_out < n_points-2){
    i++;
    skipped++;
    if (skipped > n_points){
      free(skips);
      *o_n_tri = n_out*3;
      return out;
    }
    int i0 = i % n_points;
    i0 = (i0+skips[i0])%n_points;

    int i1 = (i0+1) % n_points;
    i1 = (i1+skips[i1])%n_points;

    int i2 = (i1+1) % n_points;
    i2 = (i2+skips[i2])%n_points;

    if (CWISE(
      points[i0*2],points[i0*2+1],
      points[i1*2],points[i1*2+1],
      points[i2*2],points[i2*2+1] 
    )<=0) continue;
    int ok = 1;
    for (int j = 0; j < n_points; j++){
      int j0 = j % n_points;
      j0 = (j0+skips[j0])%n_points;

      int j1 = (j0+1) % n_points;
      j1 = (j1+skips[j1])%n_points;

      int j2 = (j1+1) % n_points;
      j2 = (j2+skips[j2])%n_points;

      if (j1 == i0 || j1 == i1 || j1 == i2) continue;
      if (CWISE(
        points[j0*2],points[j0*2+1],
        points[j1*2],points[j1*2+1],
        points[j2*2],points[j2*2+1] 
      )>=0) continue;
      if (PT_IN_TRI(
        points[j1*2],points[j1*2+1],
        points[i0*2],points[i0*2+1],
        points[i1*2],points[i1*2+1],
        points[i2*2],points[i2*2+1] 
      )){
        ok = 0;
        break;
      }
    }
    if (!ok) continue;
    out[n_out*3+0] = i0;
    out[n_out*3+1] = i1;
    out[n_out*3+2] = i2;
    skips[i1] = skips[(i1+1)%n_points]+1;
    int k = i1;
    while (k = (k-1+n_points)%n_points, skips[k]){
      skips[k] += skips[i1];
    }
    i--;
    n_out++;
    skipped = 0;
  }
  free(skips);
  return out;
}