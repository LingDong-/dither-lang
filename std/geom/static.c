//

#include <stdint.h>

#include "impl.c"

void geom__triangulate(){
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 4;
  a->t = VART_I32;
  a->data = (char*)geom_impl_triangulate_simple(points->n,(float*)(points->data), &(a->n));
  a->cap = points->n-2;

  __put_ret(&a);
}

void geom__convex_hull(){
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 8;
  a->t = VART_F32;
  a->data =  (char*)geom_impl_convex_hull(points->n, (float*)(points->data), &(a->n));
  a->cap = a->n;

  __put_ret(&a);
}

void geom__pt_in_poly(){
  __list_t* __ARG(points);
  float pt[2];
  __pop_arg(pt, 2*sizeof(float));

  int32_t r = geom_impl_pt_in_poly(pt[0],pt[1],points->n, (float*)(points->data));
  __put_ret(&r);
}

void geom__poly_resample(){
  int32_t __ARG(flags);
  float __ARG(n);
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 8;
  a->t = VART_F32;
  a->data = (char*)geom_impl_poly_resample(points->n,(float*)(points->data),n,flags,&(a->n));
  a->cap = a->n;
  __put_ret(&a);
}

void geom__poly_simplify(){
  float __ARG(eps);
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 8;
  a->t = VART_F32;
  a->data = (char*)geom_impl_poly_simplify((float*)(points->data),0,points->n,eps,&(a->n));
  a->cap = a->n;
  __put_ret(&a);
}


void geom__delaunay(){
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 4;
  a->t = VART_I32;
  a->data =  (char*)geom_impl_delaunay(points->n, (float*)(points->data), &(a->n));
  a->cap = a->n;

  __put_ret(&a);
}


void geom__voronoi(){
  __list_t* __ARG(points);

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->w = 8;
  lst->n = points->n;
  lst->cap = lst->n+1;
  lst->t = VART_LST;
  lst->data = malloc(lst->cap*lst->w);

  site_t* sites = geom_impl_voronoi(points->n, (float*)(points->data));
  for (int i = 0; i < points->n; i++){
    free(sites[i].angs);

    __list_t* l = __gc_alloc(VART_LST,sizeof(__list_t));
    l->n = sites[i].nv;
    l->cap = sites[i].nv;
    l->w = 8;
    l->data = (char*)(sites[i].vs);
    l->t = VART_F32;
    ((__list_t**)(lst->data))[i] = l;
  }
  free(sites);
  __put_ret(&lst);
}

void geom__bbox(){
  int32_t __ARG(flags);
  __list_t* __ARG(points);

  if (points->w == 8){
    char* tup = __gc_alloc(VART_TUP,59);
    ((char*)tup)[0]  = VART_F32;
    ((char*)tup)[5]  = VART_F32;
    ((char*)tup)[10] = VART_F32;
    ((char*)tup)[15] = 0;

    *(int32_t*)(tup+1)  = 20;
    *(int32_t*)(tup+6)  = 28;
    *(int32_t*)(tup+11) = 36;
    *(int32_t*)(tup+16) = 52;

    float* out = (float*)(tup+20);
    geom_impl_bbox_2d(points->n, (float*)(points->data),flags,out);
    __put_ret(&tup);
  }else if (points->w == 12){
    char* tup = __gc_alloc(VART_TUP,87);
    ((char*)tup)[0]  = VART_F32;
    ((char*)tup)[5]  = VART_F32;
    ((char*)tup)[10] = VART_F32;
    ((char*)tup)[15] = 0;

    *(int32_t*)(tup+1)  = 20;
    *(int32_t*)(tup+6)  = 32;
    *(int32_t*)(tup+11) = 44;
    *(int32_t*)(tup+16) = 80;

    float* out = (float*)(tup+20);
    geom_impl_bbox_3d(points->n, (float*)(points->data),flags,out);
    __put_ret(&tup);
  }

}

void geom__clip(){
  int32_t __ARG(flags);
  __list_t* __ARG(polygon);
  __list_t* __ARG(polyline);

  int n_out;
  int* l_out;
  float** out = geom_impl_clip(
    polyline->n, (float*)(polyline->data),
    polygon->n, (float*)(polygon->data),
    flags, &n_out, &l_out
  );

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->w = 8;
  lst->n = n_out;
  lst->cap = lst->n+1;
  lst->t = VART_LST;
  lst->data = malloc(lst->cap*lst->w);

  for (int i = 0; i < n_out; i++){    
    __list_t* l = __gc_alloc(VART_LST,sizeof(__list_t));
    l->n = l_out[i];
    l->cap = l_out[i];
    l->w = 8;
    l->data = (char*)(out[i]);
    l->t = VART_F32;
    ((__list_t**)(lst->data))[i] = l;
  }

  __put_ret(&lst);
}

void geom__curve(){
  int32_t __ARG(flags);
  float __ARG(t);
  __list_t* __ARG(params);
  __list_t* __ARG(ps);

  float fparams[4] = {1.0,1.0,1.0,1.0};
  memcpy(fparams, params->data, sizeof(float)*params->n);

  int nd = ps->w/4;
  __vla(float,v,nd);
  geom_impl_curve((float*)(ps->data),ps->w/4,fparams,t,flags,v);
  __put_ret(v);
}