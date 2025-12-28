#define _USE_MATH_DEFINES
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
#define MODE_ORIENTED 16
#define OP_INCLUDE 1
#define OP_EXCLUDE 2
#define TYPE_BEZIER   16
#define TYPE_CATROM   32
#define TYPE_BSPLINE  48
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
    (t <= 1 || !(flags & LHS_CAPR)) && 
    (0 <= s || !(flags & RHS_CAPL)) && 
    (s <= 1 || !(flags & RHS_CAPR))
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
  int n_poly = n_points;
  if (flags & MODE_POLYGON){
    n_poly++;
  }
  if (n_poly+1 > n_acc_len){
    acc_len = (float*)realloc(acc_len,(n_acc_len = n_poly+1)*sizeof(float));
  }
  float tot_len = 0;
  for (int i = 0; i < n_poly-1; i++){
    float dx = points[i*2] - points[((i+1)%n_points)*2];
    float dy = points[i*2+1] - points[((i+1)%n_points)*2+1];
    tot_len += sqrt(dx*dx+dy*dy);
    acc_len[i+1] = tot_len;
  }
  acc_len[0] = 0;
  acc_len[n_poly] = tot_len;
  float spacing = 0;
  int count = 0;
  if (flags & BY_SPACING){
    spacing = n;
    count = ceil(tot_len/spacing);
  }else{ // BY_COUNT
    spacing = tot_len/n;
    count = ceil(n);
  }
  float* out = (float*)malloc(count*sizeof(float)*2);
  int idx = 0;
  *n_out = count;
  int lidx = 0;
  for (float l = 0; l < tot_len; l+= spacing){
    for (int i = lidx; i < n_poly; i++){
      if (acc_len[i] <= l && l < acc_len[i+1]){
        float t = (l-acc_len[i])/(acc_len[i+1]-acc_len[i]);
        float x = points[i*2]*(1-t)+points[((i+1)%n_points)*2]*t;
        float y = points[(i*2)+1]*(1-t)+points[((i+1)%n_points)*2+1]*t;
        out[idx*2]   = x;
        out[idx*2+1] = y;
        idx ++;
        lidx = i;
        if (idx == count){
          goto readout;
        }
        break;
      }
    }
  }
readout:
  if (!(flags & MODE_POLYGON)){
    out[(count-1)*2] = points[(n_poly-1)%n_points*2];
    out[(count-1)*2+1] = points[(n_poly-1)%n_points*2+1];
  }
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
  float param = -1.0;
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
  if (n_points <= 1){
    *n_out = n_points;
    float* dum = malloc(n_points*sizeof(float)*2);
    return memcpy(dum,points,n_points*sizeof(float)*2);
  }
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
  
int32_t* geom_impl_triangulate_simple(int n_points, float* points, int* o_n_ids){
  *o_n_ids = (n_points-2)*3;
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
      *o_n_ids = n_out*3;
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

int circum_circle(float xp,float yp,
  float x1,float y1,float x2,float y2,float x3,float y3,
  float *xc,float *yc,float *rsqr){
  const static float EPSILON = 0.000001;
  float m1,m2,mx1,mx2,my1,my2;
  float dx,dy,drsqr;
  float fabsy1y2 = fabs(y1-y2);
  float fabsy2y3 = fabs(y2-y3);
  if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON) return 0;
  if (fabsy1y2 < EPSILON) {
    m2 = - (x3-x2) / (y3-y2);
    mx2 = (x2 + x3) / 2.0;
    my2 = (y2 + y3) / 2.0;
    *xc = (x2 + x1) / 2.0;
    *yc = m2 * (*xc - mx2) + my2;
  } else if (fabsy2y3 < EPSILON) {
    m1 = - (x2-x1) / (y2-y1);
    mx1 = (x1 + x2) / 2.0;
    my1 = (y1 + y2) / 2.0;
    *xc = (x3 + x2) / 2.0;
    *yc = m1 * (*xc - mx1) + my1;
  } else {
    m1 = - (x2-x1) / (y2-y1);
    m2 = - (x3-x2) / (y3-y2);
    mx1 = (x1 + x2) / 2.0;
    mx2 = (x2 + x3) / 2.0;
    my1 = (y1 + y2) / 2.0;
    my2 = (y2 + y3) / 2.0;
    *xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
    if (fabsy1y2 > fabsy2y3) {
      *yc = m1 * (*xc - mx1) + my1;
    } else {
      *yc = m2 * (*xc - mx2) + my2;
    }
  }
  dx = x2 - *xc;
  dy = y2 - *yc;
  *rsqr = dx*dx + dy*dy;
  dx = xp - *xc;
  dy = yp - *yc;
  drsqr = dx*dx + dy*dy;
  return ((drsqr - *rsqr) <= EPSILON);
}

int cmp_x(const void *a, const void *b){
  float x0 = ((float*)a)[0];
  float x1 = ((float*)b)[0];
  return SIGN(x0-x1);
}

int delaunay_bowyer_watson_bourke(int nv, float* pxyz, int32_t* V){
  // based on https://paulbourke.net/papers/triangulate/
  int *complete = NULL;
  int *edges = NULL;
  int nedge = 0;
  int trimax,emax = 200;
  int inside;
  int ntri;
  float xp,yp,x1,y1,x2,y2,x3,y3,xc,yc,r;
  float xmin,xmax,ymin,ymax,xmid,ymid;
  float dx,dy,dmax;
  trimax = 4 * nv;
  complete = malloc(trimax*sizeof(int));
  edges = malloc(emax*(long)sizeof(int)*2);
  xmin = pxyz[0];
  ymin = pxyz[1];
  xmax = xmin;
  ymax = ymin;
  for (int i=1;i<nv;i++) {
    if (pxyz[i*3+0] < xmin) xmin = pxyz[i*3+0];
    if (pxyz[i*3+0] > xmax) xmax = pxyz[i*3+0];
    if (pxyz[i*3+1] < ymin) ymin = pxyz[i*3+1];
    if (pxyz[i*3+1] > ymax) ymax = pxyz[i*3+1];
  }
  dx = xmax - xmin;
  dy = ymax - ymin;
  dmax = (dx > dy) ? dx : dy;
  xmid = (xmax + xmin) / 2.0;
  ymid = (ymax + ymin) / 2.0;
  pxyz[(nv+0)*3+0] = xmid - 20 * dmax;
  pxyz[(nv+0)*3+1] = ymid - dmax;
  pxyz[(nv+1)*3+0] = xmid;
  pxyz[(nv+1)*3+1] = ymid + 20 * dmax;
  pxyz[(nv+2)*3+0] = xmid + 20 * dmax;
  pxyz[(nv+2)*3+1] = ymid - dmax;
  V[0] = nv;
  V[1] = nv+1;
  V[2] = nv+2;
  complete[0] = 0;
  ntri = 1;
  
  for (int i=0;i<nv;i++) {
    xp = pxyz[i*3+0];
    yp = pxyz[i*3+1];
    nedge = 0;
    for (int j=0;j<ntri;j++) {
      if (complete[j]) continue;
      x1 = pxyz[V[j*3+0]*3+0];
      y1 = pxyz[V[j*3+0]*3+1];
      x2 = pxyz[V[j*3+1]*3+0];
      y2 = pxyz[V[j*3+1]*3+1];
      x3 = pxyz[V[j*3+2]*3+0];
      y3 = pxyz[V[j*3+2]*3+1];
      inside = circum_circle(xp,yp,x1,y1,x2,y2,x3,y3,&xc,&yc,&r);
      if (xc < xp && ((xp-xc)*(xp-xc)) > r) complete[j] = 1;
      if (inside) {
        if (nedge+3 >= emax) {
          emax += 100;
          edges = realloc(edges,emax*(long)sizeof(int)*2);
        }
        edges[(nedge+0)*2+0] = V[j*3+0];
        edges[(nedge+0)*2+1] = V[j*3+1];
        edges[(nedge+1)*2+0] = V[j*3+1];
        edges[(nedge+1)*2+1] = V[j*3+2];
        edges[(nedge+2)*2+0] = V[j*3+2];
        edges[(nedge+2)*2+1] = V[j*3+0];
        nedge += 3;
        V[j*3+0] = V[(ntri-1)*3+0];
        V[j*3+1] = V[(ntri-1)*3+1];
        V[j*3+2] = V[(ntri-1)*3+2];
        complete[j] = complete[ntri-1];
        ntri--;
        j--;
      }
    }
    for (int j=0;j<nedge-1;j++) {
      for (int k=j+1;k<nedge;k++) {
        if ((edges[j*2] == edges[k*2+1]) && (edges[j*2+1] == edges[k*2])) {
          edges[j*2+0] = -1;
          edges[j*2+1] = -1;
          edges[k*2+0] = -1;
          edges[k*2+1] = -1;
        }
        if ((edges[j*2] == edges[k*2]) && (edges[j*2+1] == edges[k*2+1])) {
          edges[j*2+0] = -1;
          edges[j*2+1] = -1;
          edges[k*2+0] = -1;
          edges[k*2+1] = -1;
        }
      }
    }
    for (int j=0;j<nedge;j++) {
      if (edges[j*2] < 0 || edges[j*2+1] < 0) continue;
      V[ntri*3+0] = edges[j*2];
      V[ntri*3+1] = edges[j*2+1];
      V[ntri*3+2] = i;
      complete[ntri] = 0;
      ntri++;
    }
  }
  free(edges);
  free(complete);
  return ntri;
}

int32_t* geom_impl_delaunay(int n_points, float* points, int* n_out){
  float* pxyz = malloc(sizeof(float)*3*(n_points+3));
  for (int i = 0; i < n_points; i++){
    pxyz[i*3] = points[i*2];
    pxyz[i*3+1] = points[i*2+1];
    ((int32_t*)pxyz)[i*3+2] = i;
  }
  qsort(pxyz, n_points, sizeof(float)*3, cmp_x);
  int32_t* V = malloc(n_points*3*sizeof(int32_t)*3);
  int ntri = delaunay_bowyer_watson_bourke(n_points, pxyz, V);
  for (int i=0;i<ntri;i++) {
    if (V[i*3+0] >= n_points || V[i*3+1] >= n_points || V[i*3+2] >= n_points) {
      V[i*3+0] = V[(ntri-1)*3+0];
      V[i*3+1] = V[(ntri-1)*3+1];
      V[i*3+2] = V[(ntri-1)*3+2];
      ntri--;
      i--;
    }
  }
  for (int i = 0; i < ntri*3; i++){
    V[i] = ((int32_t*)pxyz)[V[i]*3+2];
  }
  *n_out = ntri*3;
  free(pxyz);
  return V;
}

typedef struct site_st {
  int mv;
  int nv;
  float* vs;
  float* angs;
} site_t;

void site_add_vertex(float* points, site_t* sites, int idx, float x, float y){
  if (sites[idx].nv >= sites[idx].mv){
    sites[idx].vs = realloc(sites[idx].vs, (sites[idx].mv+=8)*2*sizeof(float));
    sites[idx].angs = realloc(sites[idx].angs, (sites[idx].mv)*2*sizeof(float));
  }
  float ang = atan2(y-points[idx*2+1], x-points[idx*2]);
  int ii;
  for (ii = 0; ii < sites[idx].nv; ii++){
    if (ang < sites[idx].angs[ii]){
      memmove(sites[idx].angs+(ii+1), sites[idx].angs+ii, (sites[idx].nv-ii)*sizeof(float) );
      memmove(sites[idx].vs+((ii+1)*2), sites[idx].vs+(ii*2), (sites[idx].nv-ii)*sizeof(float)*2);
      break;
    }
  }
  sites[idx].vs[ii*2  ] = x;
  sites[idx].vs[ii*2+1] = y;
  sites[idx].angs[ii] = ang;
  sites[idx].nv++;
}

site_t* geom_impl_voronoi(int n_points, float* points){

  float* pxyz = malloc(sizeof(float)*3*(n_points+3));
  for (int i = 0; i < n_points; i++){
    pxyz[i*3] = points[i*2];
    pxyz[i*3+1] = points[i*2+1];
    ((int32_t*)pxyz)[i*3+2] = i;
  }
  qsort(pxyz, n_points, sizeof(float)*3, cmp_x);
  int32_t* V = malloc(n_points*3*sizeof(int32_t)*3);
  int ntri = delaunay_bowyer_watson_bourke(n_points, pxyz, V);

  site_t* sites = calloc(n_points,sizeof(site_t));

  for (int i = 0; i < ntri; i++){
    int i0 = V[i*3];
    int i1 = V[i*3+1];
    int i2 = V[i*3+2];
    float xc,yc,rsqr;
    circum_circle(0,0, 
      pxyz[i0*3], pxyz[i0*3+1],
      pxyz[i1*3], pxyz[i1*3+1],
      pxyz[i2*3], pxyz[i2*3+1],
      &xc,&yc,&rsqr
    );
    if (i0 < n_points) site_add_vertex(points,sites,((int32_t*)pxyz)[i0*3+2], xc,yc);
    if (i1 < n_points) site_add_vertex(points,sites,((int32_t*)pxyz)[i1*3+2], xc,yc);
    if (i2 < n_points) site_add_vertex(points,sites,((int32_t*)pxyz)[i2*3+2], xc,yc);
  }
  free(V);
  free(pxyz);
  return sites;
}

void aabb_2d(int n_points, float* points, float* out){
  float minX = INFINITY, maxX = -INFINITY;
  float minY = INFINITY, maxY = -INFINITY;
  for (int i = 0; i < n_points; i++){
    float x = points[i*2];
    float y = points[i*2+1];
    minX = fmin(minX, x);
    maxX = fmax(maxX, x);
    minY = fmin(minY, y);
    maxY = fmax(maxY, y);
  }
  float w = maxX - minX;
  float h = maxY - minY;
  out[0] = (minX+maxX)*0.5;
  out[1] = (minY+maxY)*0.5;
  out[2] = w/2;
  out[3] = h/2;
  out[4] = 1;
  out[5] = 0;
  out[6] = 0;
  out[7] = 1;
}

void obb_2d_pca(int n_points, float* points, float* out){
  float meanX = 0;
  float meanY = 0;
  for (int i = 0; i < n_points; i++){
    meanX += points[i*2];
    meanY += points[i*2+1];
  }
  meanX /= n_points;
  meanY /= n_points;
  float covXX = 0, covXY = 0, covYY = 0;
  for (int i = 0; i < n_points; i++){
    float dx = points[i*2] - meanX;
    float dy = points[i*2+1] - meanY;
    covXX += dx*dx;
    covXY += dx*dy;
    covYY += dy*dy;
  }
  covXX /= n_points;
  covXY /= n_points;
  covYY /= n_points;
  float trace = covXX + covYY;
  float det = covXX*covYY - covXY*covXY;
  float temp = sqrtf((trace*trace)/4 - det);
  float eig1X = trace/2 + temp;
  float eig1Y = trace/2 - temp;
  float eigvec1X = 1;
  float eigvec1Y = 0;
  if (fabs(covXY) > 1e-6){
    eigvec1X = eig1X - covYY;
    eigvec1Y = covXY;
  }
  float mag = hypot(eigvec1X,eigvec1Y);
  eigvec1X /= mag;
  eigvec1Y /= mag;
  float eigvec2X = -eigvec1Y;
  float eigvec2Y = eigvec1X;
  float minX = INFINITY, maxX = -INFINITY;
  float minY = INFINITY, maxY = -INFINITY;
  for (int i = 0; i < n_points; i++){
    float dx = points[i*2] - meanX;
    float dy = points[i*2+1] - meanY;
    float projX = dx*eigvec1X + dy*eigvec1Y;
    float projY = dx*eigvec2X + dy*eigvec2Y;
    minX = fmin(minX, projX);
    maxX = fmax(maxX, projX);
    minY = fmin(minY, projY);
    maxY = fmax(maxY, projY);
  }
  float w = maxX - minX;
  float h = maxY - minY;
  float cx = (maxX+minX)/2;
  float cy = (maxY+minY)/2;
  float ccx = cx*eigvec1X + cy*eigvec2X;
  float ccy = cx*eigvec1Y + cy*eigvec2Y;
  out[0] = meanX+ccx;
  out[1] = meanY+ccy;
  out[2] = w/2;
  out[3] = h/2;
  out[4] = eigvec1X;
  out[5] = eigvec2X;
  out[6] = eigvec1Y;
  out[7] = eigvec2Y;
}

#define RCHULL_PROJ(ux,uy,idx) ((ux)*hull[((idx)%nh)*2] + (uy)*hull[((idx)%nh)*2+1])
#define RCHULL_EDGE(ux,uy,idx) (ux) = (hull[(((idx)+1)%nh)*2]-hull[((idx)%nh)*2]); (uy) = (hull[(((idx)+1)%nh)*2+1]-hull[((idx)%nh)*2+1]); {float l = hypot(ux,uy); if (l){ux/=l; uy/=l;}}

void obb_2d_rotcal(int n_points, float* points, float* out){
  int nh;
  float* hull = geom_impl_convex_hull(n_points, points, &nh);
  int kMinX,kMinY,kMaxX,kMaxY;
  float u1x,u1y,u2x,u2y;
  RCHULL_EDGE(u1x,u1y,0);
  u2x = -u1y, u2y = u1x;
  float maxX=-INFINITY, minX=INFINITY, maxY=-INFINITY, minY=INFINITY;
  for(int t=0;t<nh;t++){
    float px = RCHULL_PROJ(u1x,u1y,t);
    float py = RCHULL_PROJ(u2x,u2y,t);
    if(px>maxX){maxX=px; kMaxX=t;}
    if(px<minX){minX=px; kMinX=t;}
    if(py>maxY){maxY=py; kMaxY=t;}
    if(py<minY){minY=py; kMinY=t;}
  }
  float bestArea = INFINITY;
  for (int i = 0; i < nh; i++){
    float maxX=-INFINITY, minX=INFINITY, maxY=-INFINITY, minY=INFINITY;
    float u1x,u1y,u2x,u2y;
    RCHULL_EDGE(u1x,u1y,i);
    u2x = -u1y, u2y = u1x;
    while(RCHULL_PROJ(u1x,u1y,kMaxX+1) > (maxX=RCHULL_PROJ(u1x,u1y,kMaxX))) kMaxX++; 
    while(RCHULL_PROJ(u1x,u1y,kMinX+1) < (minX=RCHULL_PROJ(u1x,u1y,kMinX))) kMinX++; 
    while(RCHULL_PROJ(u2x,u2y,kMaxY+1) > (maxY=RCHULL_PROJ(u2x,u2y,kMaxY))) kMaxY++; 
    while(RCHULL_PROJ(u2x,u2y,kMinY+1) < (minY=RCHULL_PROJ(u2x,u2y,kMinY))) kMinY++; 
    float width = (maxX-minX);
    float height = (maxY-minY);
    float area = width*height;
    if (area < bestArea){
      bestArea = area;
      float cx = (minX+maxX)*0.5;
      float cy = (minY+maxY)*0.5;
      out[0] = u1x*cx + u2x*cy;
      out[1] = u1y*cx + u2y*cy;
      out[2] = width*0.5;
      out[3] = height*0.5;
      out[4] = u1x;
      out[5] = u2x;
      out[6] = u1y;
      out[7] = u2y;
    }
  }
}


void power_iter(
  float a00, float a01, float a02,
  float a10, float a11, float a12,
  float a20, float a21, float a22,
  float* o0, float* o1, float* o2
){
  float v0 = M_PI;
  float v1 = M_E;
  float v2 = M_SQRT2;
  for (int k = 0; k < 12; k++){
    float x0 = a00*v0+a01*v1+a02*v2;
    float x1 = a10*v0+a11*v1+a12*v2;
    float x2 = a20*v0+a21*v1+a22*v2;
    float n = sqrt(x0*x0+x1*x1+x2*x2);
    if (n < 1e-12) break;
    v0 = x0/n;
    v1 = x1/n;
    v2 = x2/n;
  }
  *o0 = v0;
  *o1 = v1;
  *o2 = v2;
}


void obb_3d_pca(int n_points, float* points, float* out){
  float meanX = 0, meanY = 0, meanZ = 0;
  for (int i = 0; i < n_points; i++){
    meanX += points[i*3];
    meanY += points[i*3+1];
    meanZ += points[i*3+2];
  }
  meanX /= n_points;
  meanY /= n_points;
  meanZ /= n_points;
  float cxx=0, cxy=0, cxz=0, cyy=0, cyz=0, czz=0;
  for (int i = 0; i < n_points; i++){
    float dx = points[i*3+0] - meanX;
    float dy = points[i*3+1] - meanY;
    float dz = points[i*3+2] - meanZ;
    cxx += dx*dx; cxy += dx*dy; cxz += dx*dz; 
    cyy += dy*dy; cyz += dy*dz; czz += dz*dz;
  }
  cxx/=n_points; cxy/=n_points; cxz/=n_points;
  cyy/=n_points; cyz/=n_points; czz/=n_points;

  float e1x, e1y, e1z;
  power_iter(
    cxx,cxy,cxz, 
    cxy,cyy,cyz, 
    cxz,cyz,czz, &e1x,&e1y,&e1z);

  float lambda =
    e1x*(cxx*e1x+cxy*e1y+cxz*e1z)+
    e1y*(cxy*e1x+cyy*e1y+cyz*e1z)+
    e1z*(cxz*e1x+cyz*e1y+czz*e1z);
  
  float e2x, e2y, e2z;
  power_iter(
    cxx-lambda*e1x*e1x, cxy-lambda*e1x*e1y, cxz-lambda*e1x*e1z,
    cxy-lambda*e1y*e1x, cyy-lambda*e1y*e1y, cyz-lambda*e1y*e1z,
    cxz-lambda*e1z*e1x, cyz-lambda*e1z*e1y, czz-lambda*e1z*e1z,
    &e2x,&e2y,&e2z
  );

  float e3x = e1y*e2z-e1z*e2y;
  float e3y = e1z*e2x-e1x*e2z;
  float e3z = e1x*e2y-e1y*e2x;

  float minX = INFINITY, minY = INFINITY, minZ = INFINITY;
  float maxX =-INFINITY, maxY =-INFINITY, maxZ =-INFINITY;

  for (int i = 0; i < n_points; i++){
    float dx = points[i*3] - meanX;
    float dy = points[i*3+1] - meanY;
    float dz = points[i*3+2] - meanZ;
    float projX = dx*e1x+dy*e1y+dz*e1z;
    float projY = dx*e2x+dy*e2y+dz*e2z;
    float projZ = dx*e3x+dy*e3y+dz*e3z;
    minX = fmin(minX, projX);
    maxX = fmax(maxX, projX);
    minY = fmin(minY, projY);
    maxY = fmax(maxY, projY);
    minZ = fmin(minZ, projZ);
    maxZ = fmax(maxZ, projZ);
  }
  float cx = (minX+maxX)*0.5;
  float cy = (minY+maxY)*0.5;
  float cz = (minZ+maxZ)*0.5;

  out[0] = meanX + cx*e1x + cy*e2x + cz*e3x;
  out[1] = meanY + cx*e1y + cy*e2y + cz*e3y;
  out[2] = meanZ + cx*e1z + cy*e2z + cz*e3z;
  out[3] = (maxX-minX)*0.5;
  out[4] = (maxY-minY)*0.5;
  out[5] = (maxZ-minZ)*0.5;
  out[6] = e1x; out[7] = e2x; out[8] = e3x;
  out[9] = e1y; out[10]= e2y; out[11]= e3y;
  out[12]= e1z; out[13]= e2z; out[14]= e3z;
}

void rotate_axes(float* a, float* b, float th, float* a1, float* b1){
  float c=cos(th), s=sin(th);
  float o[6] = { c*a[0]+s*b[0], c*a[1]+s*b[1], c*a[2]+s*b[2],
                -s*a[0]+c*b[0],-s*a[1]+c*b[1],-s*a[2]+c*b[2]};
  memcpy(a1,o,sizeof(float)*3);
  memcpy(b1,o+3,sizeof(float)*3);
}

void obb_3d_refine(int n_points, float* points, float* out){
  float best_vol = out[3]*out[4]*out[5]*8;
  float best_axes[9] = {
    out[6], out[9], out[12],
    out[7], out[10],out[13],
    out[8], out[11],out[14]
  };
  float angle = 0.15;
  float cx,cy,cz;
  for (int iter = 0; iter < 2; iter++){
    for (int i = 0; i < 3; i++){
      int j = (i+1)%3;
      int k = (i+2)%3;
      for (int s = -1; s <=1; s+=2){
        float try_axes[9];
        rotate_axes(best_axes + (i*3), best_axes + (j*3), s*angle, try_axes + (i*3), try_axes + (j*3));
        memcpy(try_axes + (k*3), best_axes + (k*3), sizeof(float)*3);

        float minX = INFINITY, minY = INFINITY, minZ = INFINITY;
        float maxX =-INFINITY, maxY =-INFINITY, maxZ =-INFINITY;
        for (int p = 0; p < n_points; p++){
          float dx = points[p*3] -  out[0];
          float dy = points[p*3+1] -out[1];
          float dz = points[p*3+2] -out[2];
          float projX = dx*try_axes[0]+dy*try_axes[1]+dz*try_axes[2];
          float projY = dx*try_axes[3]+dy*try_axes[4]+dz*try_axes[5];
          float projZ = dx*try_axes[6]+dy*try_axes[7]+dz*try_axes[8];
          minX = fmin(minX, projX);
          maxX = fmax(maxX, projX);
          minY = fmin(minY, projY);
          maxY = fmax(maxY, projY);
          minZ = fmin(minZ, projZ);
          maxZ = fmax(maxZ, projZ);
        }
        float vol = (maxX-minX)*(maxY-minY)*(maxZ-minZ);
        if (vol < best_vol){
          best_vol = vol;
          memcpy(best_axes, try_axes, sizeof(float)*9);
          cx = (minX+maxX)*0.5;
          cy = (minY+maxY)*0.5;
          cz = (minZ+maxZ)*0.5;
          out[3] = (maxX-minX)*0.5;
          out[4] = (maxY-minY)*0.5;
          out[5] = (maxZ-minZ)*0.5;
        }
      }
    }
    angle *= 0.5;
  }

  out[0] += cx*best_axes[0] + cy*best_axes[3] + cz*best_axes[6];
  out[1] += cx*best_axes[1] + cy*best_axes[4] + cz*best_axes[7];
  out[2] += cx*best_axes[2] + cy*best_axes[5] + cz*best_axes[8];
  out[6] = best_axes[0]; out[7] = best_axes[3]; out[8] = best_axes[6];
  out[9] = best_axes[1]; out[10]= best_axes[4]; out[11]= best_axes[7];
  out[12]= best_axes[2]; out[13]= best_axes[5]; out[14]= best_axes[8];
}

void aabb_3d(int n_points, float* points, float* out){
  float minX = INFINITY, maxX = -INFINITY;
  float minY = INFINITY, maxY = -INFINITY;
  float minZ = INFINITY, maxZ = -INFINITY;
  for (int i = 0; i < n_points; i++){
    float x = points[i*3];
    float y = points[i*3+1];
    float z = points[i*3+2];
    minX = fmin(minX, x);
    maxX = fmax(maxX, x);
    minY = fmin(minY, y);
    maxY = fmax(maxY, y);
    minZ = fmin(minZ, z);
    maxZ = fmax(maxZ, z);
  }
  out[0] = (minX+maxX)*0.5;
  out[1] = (minY+maxY)*0.5;
  out[2] = (minZ+maxZ)*0.5;
  out[3] = (maxX-minX)*0.5;
  out[4] = (maxY-minY)*0.5;
  out[5] = (maxZ-minZ)*0.5;
  out[6] = 1; out[7] = 0; out[8] = 0;
  out[9] = 0; out[10]= 1; out[11]= 0;
  out[12]= 0; out[13]= 0; out[13]= 1;
}

void geom_impl_bbox_2d(int n_points, float* points, int flags, float* out){
  if ((flags&0xf0)==MODE_ORIENTED){
    if (flags&0xf){
      obb_2d_rotcal(n_points,points,out);
    }else{
      obb_2d_pca(n_points,points,out);
    }
  }else if ((flags&0xf0)==MODE_ALIGNED){
    aabb_2d(n_points,points,out);
  }
}


void geom_impl_bbox_3d(int n_points, float* points, int flags, float* out){
  if ((flags&0xf0)==MODE_ORIENTED){
    obb_3d_pca(n_points,points,out);
    if (flags&0xf){
      obb_3d_refine(n_points,points,out);
    }
  }else if ((flags&0xf0)==MODE_ALIGNED){
    aabb_3d(n_points,points,out);
  }
}

float** clipped = NULL;
int n_clipped = 0;
int m_clipped = 0;
int* l_clipped = 0;
int m_clipping = 0;

void clip_add_seg(float ls0x, float ls0y, float ls1x, float ls1y){
  if (!n_clipped){
    float* clipping = (float*)malloc(sizeof(float)*2*(m_clipping = 8));
    if (!m_clipped){
      clipped = (float**)malloc(sizeof(float*)*(m_clipped = 8));
      l_clipped = (int*)malloc(sizeof(float*)*m_clipped);
    }
    l_clipped[n_clipped] = 0;
    clipped[n_clipped++] = clipping;
  }
  if (!l_clipped[n_clipped-1]){
    clipped[n_clipped-1][0] = ls0x;
    clipped[n_clipped-1][1] = ls0y;
    clipped[n_clipped-1][2] = ls1x;
    clipped[n_clipped-1][3] = ls1y;
    l_clipped[n_clipped-1]=2;
    return;
  }
  if (clipped[n_clipped-1][(l_clipped[n_clipped-1]-1)*2  ] == ls0x && 
      clipped[n_clipped-1][(l_clipped[n_clipped-1]-1)*2+1] == ls0y){
    if (l_clipped[n_clipped-1] >= m_clipping){
      clipped[n_clipped-1] = (float*)realloc(clipped[n_clipped-1], sizeof(float)*2*(m_clipping=m_clipping*2+1));
    }
    clipped[n_clipped-1][l_clipped[n_clipped-1]*2  ] = ls1x;
    clipped[n_clipped-1][l_clipped[n_clipped-1]*2+1] = ls1y;
    l_clipped[n_clipped-1]++;
  }else{
    if (n_clipped >= m_clipped){
      clipped = (float**)realloc(clipped,sizeof(float*)*(m_clipped=m_clipped*2+1));
      l_clipped = (int*)realloc(l_clipped,sizeof(float*)*m_clipped);
    }
    clipped[n_clipped] = (float*)malloc(sizeof(float)*2*(m_clipping=8));
    clipped[n_clipped][0] = ls0x;
    clipped[n_clipped][1] = ls0y;
    clipped[n_clipped][2] = ls1x;
    clipped[n_clipped][3] = ls1y;
    l_clipped[n_clipped]=2;
    n_clipped ++;
  }
}


float** geom_impl_clip(int n_polyline, float* polyline, int n_polygon, float* polygon, int flags, int* o_n_clipped, int** o_l_clipped){
  int do_diff = flags == OP_EXCLUDE;
  n_clipped = 0;

  float* isx = malloc(n_polygon*sizeof(float));
  int n_isx = 0;
  for (int i = 0; i < n_polyline-1; i++){
    float ls0x = polyline[i*2];
    float ls0y = polyline[i*2+1];
    float ls1x = polyline[(i+1)*2];
    float ls1y = polyline[(i+1)*2+1];
    
    n_isx = 0;
    for (int j = 0; j < n_polygon; j++){
      float t, s;
      int ret = geom_impl_line_intersect_2d(
        ls0x,ls0y,ls1x,ls1y,
        polygon[j*2], polygon[j*2+1], polygon[((j+1)%n_polygon)*2], polygon[((j+1)%n_polygon)*2+1],
        LHS_CAPL|LHS_CAPR|RHS_CAPL|RHS_CAPR|RET_PARAMS, &t, &s
      );
      if (ret){
        isx[n_isx++] = t;
      }
    };
    if (!n_isx){
      if (do_diff == !geom_impl_pt_in_poly(ls0x,ls0y,n_polygon,polygon)){
        clip_add_seg(ls0x,ls0y,ls1x,ls1y);
      }
    }else{
      isx[n_isx++] = 0;
      isx[n_isx++] = 1;

      qsort(isx, n_isx, sizeof(float), cmp_x);
      
      float dx = ls1x-ls0x;
      float dy = ls1y-ls0y;
      float td = dx*dx+dy*dy;
      for (int k = 0; k < n_isx-1; k++){
        float_t t0 = isx[k];
        float_t t1 = isx[k+1];
        float x0 = ls0x*(1-t0)+ls1x*(t0);
        float y0 = ls0y*(1-t0)+ls1y*(t0);        
        float x1 = ls0x*(1-t1)+ls1x*(t1);
        float y1 = ls0y*(1-t1)+ls1y*(t1);
        float ds = (t1-t0)*td;
        if (ds >= 0.001){
          if (do_diff == !geom_impl_pt_in_poly((x0+x1)*0.5,(y0+y1)*0.5,n_polygon,polygon)){
            clip_add_seg(x0,y0,x1,y1);
          }
        }
      }
    }
  }
  free(isx);
  *o_n_clipped = n_clipped;
  *o_l_clipped = l_clipped;
  return clipped;
}


void quadratic_rational_bezier(float* p, int nd, float* w, float t, float* o){
  float tt = t*t;
  float l_tl_t = (1-t)*(1-t);
  float ztl_tw = 2*t*(1-t)*w[0];
  float u = l_tl_t+ztl_tw+tt;
  for (int i = 0; i < nd; i++){
    o[i] = (l_tl_t*p[i]+ztl_tw*p[nd+i]+tt*p[nd*2+i])/u;
  }
}
void cubic_rational_bezier(float* p, int nd, float* w, float t, float* o){
  float tt = t*t;
  float ttt = tt*t;
  float l_t2 = (1-t)*(1-t);
  float l_t3 = l_t2 * (1-t);
  float tl_t2w3 = t*l_t2*w[0]*3;
  float ttl_tw3 = tt*(1-t)*w[1]*3;
  float u = l_t3 + tl_t2w3 + ttl_tw3 + ttt;
  for (int i = 0; i < nd; i++){
    o[i] = (l_t3*p[i]+tl_t2w3*p[nd+i]+ttl_tw3*p[nd*2+i]+ttt*p[nd*3+i])/u;
  }
}

float catrom_getT(float* p, int nd, float alpha){
  float d = 0;
  for (int i = 0; i < nd; i++){
    float dx = p[i]-p[nd+i];
    d += dx*dx;
  }
  if (d < 1e-4) d = 1e-4;
  return powf(d, 0.5*alpha);
}

void catrom(float* p, int nd, float* alpha, float t, float* o){
  float t0 = 0.0;
  float t1 = t0 + catrom_getT(p, nd, alpha[0]);
  float t2 = t1 + catrom_getT(p+nd, nd, alpha[0]);
  float t3 = t2 + catrom_getT(p+(nd*2), nd, alpha[0]);
  float t_ = t1 * (1-t) + t2 * t;
  for (int i = 0; i < nd; i++){
    float A1 = ( t1-t_ )/( t1-t0 )*p[0+i] + ( t_-t0 )/( t1-t0 )*p[nd+i];
    float A2 = ( t2-t_ )/( t2-t1 )*p[nd+i] + ( t_-t1 )/( t2-t1 )*p[nd*2+i];
    float A3 = ( t3-t_ )/( t3-t2 )*p[nd*2+i] + ( t_-t2 )/( t3-t2 )*p[nd*3+i];
    float B1 = ( t2-t_ )/( t2-t0 )*A1 + ( t_-t0 )/( t2-t0 )*A2;
    float B2 = ( t3-t_ )/( t3-t1 )*A2 + ( t_-t1 )/( t3-t1 )*A3;
    float C  = ( t2-t_ )/( t2-t1 )*B1 + ( t_-t1 )/( t2-t1 )*B2;
    o[i] = C;
  }
}

void cubic_bspline(float* p, int nd, float* w, float t, float* o){
  float t2 = t * t;
  float t3 = t2 * t;
  float b0 = (-t3 + 3*t2 - 3*t + 1) / 6*w[0];
  float b1 = ( 3*t3 - 6*t2 + 4) / 6*w[1];
  float b2 = (-3*t3 + 3*t2 + 3*t + 1) / 6*w[2];
  float b3 = ( t3 ) / 6*w[3];
  float denom = b0+b1+b2+b3;
  for (int i = 0; i < nd; i++){
    o[i] = (b0*p[i] + b1*p[nd+i] + b2*p[nd*2+i] + b3*p[nd*3+i])/denom;
  }
}

void quadratic_bspline(float* p, int nd, float* w, float t, float* o){
  float t2 = t * t;
  float b0 = 0.5 * (t2 - 2*t + 1)*w[0];
  float b1 = 0.5 * (-2*t2 + 2*t + 1)*w[1];
  float b2 = 0.5 * (t2)*w[2];
  float denom = b0+b1+b2;
  for (int i = 0; i < nd; i++){
    o[i] = (b0*p[i] + b1*p[nd+i] + b2*p[nd*2+i])/denom;
  }
}

void geom_impl_curve(float* p, int nd, float* a, float t, int flags, float* o){
  if ((flags & 0xf0) == TYPE_BEZIER){
    if ((flags & 0xf) == ORD_QUADRATIC){
      quadratic_rational_bezier(p,nd,a,t,o);
    }else if ((flags & 0xf) == ORD_CUBIC){
      cubic_rational_bezier(p,nd,a,t,o);
    }
  }else if ((flags & 0xf0) == TYPE_CATROM){
    catrom(p,nd,a,t,o);
  }else if ((flags & 0xf0) == TYPE_BSPLINE){
    if ((flags & 0xf) == ORD_QUADRATIC){
      quadratic_bspline(p,nd,a,t,o);
    }else if ((flags & 0xf) == ORD_CUBIC){
      cubic_bspline(p,nd,a,t,o);
    }
  }
}

float geom_impl_poly_area(int n_points, float* points){
  int n = n_points;
  float a = 0;
  for (int p=n-1,q=0; q<n; p=q++){
    a += points[p*2]*points[q*2+1] - points[q*2]*points[p*2+1];
  }
  return a*0.5;
}
