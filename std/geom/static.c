//

#include <stdint.h>

#include "impl.c"

void geom__triangulate(){
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 4;
  a->t = VART_I32;
  a->data =  geom_impl_triangulate_simple(points->n, points->data, &(a->n));
  a->cap = points->n-2;

  __put_ret(&a);
}

void geom__convex_hull(){
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 8;
  a->t = VART_F32;
  a->data =  geom_impl_convex_hull(points->n, points->data, &(a->n));
  a->cap = a->n;

  __put_ret(&a);
}

void geom__pt_in_poly(){
  __list_t* __ARG(points);
  float pt[2];
  __pop_arg(pt, 2*sizeof(float));

  int32_t r = geom_impl_pt_in_poly(pt[0],pt[1],points->n, points->data);
  __put_ret(&r);
}

void geom__poly_resample(){
  int32_t __ARG(flags);
  float __ARG(n);
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 8;
  a->t = VART_F32;
  a->data = geom_impl_poly_resample(points->n,points->data,n,flags,&(a->n));
  a->cap = a->n;
  __put_ret(&a);
}

void geom__poly_simplify(){
  float __ARG(eps);
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 8;
  a->t = VART_F32;
  a->data = geom_impl_poly_simplify(points->data,0,points->n,eps,&(a->n));
  a->cap = a->n;
  __put_ret(&a);
}


void geom__delaunay(){
  __list_t* __ARG(points);

  __list_t* a = __gc_alloc(VART_LST, sizeof(__list_t));
  a->w = 4;
  a->t = VART_I32;
  a->data =  geom_impl_delaunay(points->n, points->data, &(a->n));
  a->cap = a->n*3;

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

  site_t* sites = geom_impl_voronoi(points->n, points->data);
  for (int i = 0; i < points->n; i++){
    free(sites[i].angs);

    __list_t* l = __gc_alloc(VART_LST,sizeof(__list_t));
    l->n = sites[i].nv;
    l->cap = sites[i].nv;
    l->w = 8;
    l->data = sites[i].vs;
    l->t = VART_F32;
    ((__list_t**)(lst->data))[i] = l;
  }
  free(sites);
  __put_ret(&lst);
}