#include <stdio.h>

#include "../../src/interp.c"

#include "impl.c"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x

float* copy_list_vecf_pack(lst_t* lst, int d){
  if (!lst->n) return NULL;
  void* data = malloc(lst->n*d*sizeof(float));
  for (int i = 0; i < lst->n; i++){
    vec_t* v = ((vec_t**)(lst->data))[i];
    memcpy((char*)data + (i*d*sizeof(float)), v->data, d*sizeof(float));
  }
  return data;
}

lst_t* copy_list_vecf_unpack(gstate_t* _g, type_t* typ, float* data, int n, int d){
  lst_t* l = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  l->n = n;
  l->cap = n;
  l->w = sizeof(vec_t**);
  l->type = typ;
  l->data = malloc(sizeof(vec_t**)*n);
  for (int i = 0; i < n; i++){
    vec_t* v = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(d*sizeof(float)));
    v->n = d;
    v->w = sizeof(float);
    v->type = (type_t*)(l->type->u.elem.head->data);
    memcpy(v->data, data + (i*d), d*sizeof(float));
    ((vec_t**)(l->data))[i] = v;
  }
  return l;
}

EXPORTED void geom_line_intersect(var_t* ret, gstate_t* _g){
  int32_t flags = ARG_POP(_g, i32);
  vec_t* q1 = ARG_POP(_g,vec);
  vec_t* q0 = ARG_POP(_g,vec);
  vec_t* p1 = ARG_POP(_g,vec);
  vec_t* p0 = ARG_POP(_g,vec);

  int nd = q1->n;

  vec_t* v = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(nd*4));
  v->n = nd;
  v->w = 4;
  v->type = (type_t*)(ret->type->u.elem.tail->data);

  int b = 0;

  if (nd == 2){
    b = geom_impl_line_intersect_2d(
      ((float*)(p0->data))[0],((float*)(p0->data))[1],
      ((float*)(p1->data))[0],((float*)(p1->data))[1],
      ((float*)(q0->data))[0],((float*)(q0->data))[1],
      ((float*)(q1->data))[0],((float*)(q1->data))[1],
      flags, (float*)(v->data), ((float*)(v->data))+1
    );
  }else if (nd == 3){
    b = geom_impl_line_intersect_3d(
      ((float*)(p0->data))[0],((float*)(p0->data))[1],((float*)(p0->data))[2],
      ((float*)(p1->data))[0],((float*)(p1->data))[1],((float*)(p1->data))[2],
      ((float*)(q0->data))[0],((float*)(q0->data))[1],((float*)(q0->data))[2],
      ((float*)(q1->data))[0],((float*)(q1->data))[1],((float*)(q1->data))[2],
      flags, (float*)(v->data), ((float*)(v->data))+1, ((float*)(v->data))+2
    );
  }

  tup_t* tup = gc_alloc_(_g,sizeof(tup_t)+24);
  tup->type = ret->type;
  ((int32_t*)(tup->data))[0] = b;
  ((vec_t**)(((char*)(tup->data))+4))[0] = v;
  ret->u.tup = tup;
}

EXPORTED void geom_triangulate(var_t* ret, gstate_t* _g){
  lst_t* points = ARG_POP(_g,lst);
  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  lst->w = sizeof(int32_t);
  lst->type = ret->type;

  float* data = copy_list_vecf_pack(points, 2);
  lst->data = (char*)geom_impl_triangulate_simple(points->n, data, &(lst->n));
  lst->cap = lst->n;

  ret->u.lst = lst;
  free(data); 
}

EXPORTED void geom_convex_hull(var_t* ret, gstate_t* _g){
  lst_t* points = ARG_POP(_g,lst);

  float* data = copy_list_vecf_pack(points, 2);
  int n_out;
  float* out = geom_impl_convex_hull(
    points->n, data, &n_out
  );

  lst_t* lst = copy_list_vecf_unpack(_g,ret->type,out,n_out,2);
  ret->u.lst = lst;
  free(data);
}

EXPORTED void geom_pt_in_poly(var_t* ret, gstate_t* _g){
  lst_t* points = ARG_POP(_g,lst);
  vec_t* pt = ARG_POP(_g,vec);

  float* data = copy_list_vecf_pack(points, 2);
  int r = geom_impl_pt_in_poly( 
    ((float*)(pt->data))[0],((float*)(pt->data))[1],
    points->n, data
  );
  free(data);

  ret->u.i32 = r;
}

EXPORTED void geom_poly_resample(var_t* ret, gstate_t* _g){
  int32_t flags = ARG_POP(_g, i32);
  float n = ARG_POP(_g, f32);
  lst_t* points = ARG_POP(_g,lst);

  float* data = copy_list_vecf_pack(points, 2);
  int n_out;
  float* out = geom_impl_poly_resample(points->n,data,n,flags,&(n_out));
  lst_t* lst = copy_list_vecf_unpack(_g,ret->type,out,n_out,2);

  ret->u.lst = lst;
  free(data);
}

EXPORTED void geom_poly_simplify(var_t* ret, gstate_t* _g){
  float eps = ARG_POP(_g, f32);
  lst_t* points = ARG_POP(_g,lst);

  float* data = copy_list_vecf_pack(points, 2);
  int n_out;
  float* out = geom_impl_poly_simplify(data,0,points->n,eps,&(n_out));
  lst_t* lst = copy_list_vecf_unpack(_g,ret->type,out,n_out,2);

  ret->u.lst = lst;
  free(data);
}


EXPORTED void geom_delaunay(var_t* ret, gstate_t* _g){
  lst_t* points = ARG_POP(_g,lst);
  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  lst->w = sizeof(int32_t);
  lst->type = ret->type;

  float* data = copy_list_vecf_pack(points, 2);
  lst->data = (char*)geom_impl_delaunay(points->n, data, &(lst->n));
  lst->cap = lst->n;

  ret->u.lst = lst;
  free(data);
}


EXPORTED void geom_voronoi(var_t* ret, gstate_t* _g){
  lst_t* points = ARG_POP(_g,lst);

  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  lst->n = points->n;
  lst->cap = lst->n;
  lst->w = 8;
  lst->type = ret->type;
  lst->data = malloc(lst->w*lst->cap);
  ret->u.lst = lst;

  float* data = copy_list_vecf_pack(points, 2);

  site_t* sites = geom_impl_voronoi(points->n, data);
  for (int i = 0; i < points->n; i++){
    free(sites[i].angs);

    lst_t* l = copy_list_vecf_unpack(_g, 
      (type_t*)(ret->type->u.elem.head->data), sites[i].vs, sites[i].nv, 2
    );

    free(sites[i].vs);
    ((lst_t**)(lst->data))[i] = l;
  }
  free(sites);
  free(data);

}

EXPORTED void geom_bbox(var_t* ret, gstate_t* _g){
  int flags = ARG_POP(_g,i32);
  lst_t* points = ARG_POP(_g,lst);

  int nd;
  if (points->n){
    nd = ((vec_t**)(points->data))[0]->n;
  }else{
    nd = atoi(((type_t*)(((type_t*)(points->type->u.elem.head->data))->u.elem.tail->data))->u.str.data);
  }

  vec_t* v0 = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(nd*4));
  v0->n = nd;
  v0->w = 4;
  v0->type = (type_t*)(ret->type->u.elem.head->data);

  vec_t* v1 = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(nd*4));
  v1->n = nd;
  v1->w = 4;
  v1->type = (type_t*)(ret->type->u.elem.head->next->data);

  vec_t* v2 = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(nd*nd*4));
  v2->n = nd*nd;
  v2->w = 4;
  v2->type = (type_t*)(ret->type->u.elem.tail->data);

  tup_t* tup = gc_alloc_(_g,sizeof(tup_t)+24);
  tup->type = ret->type;
  ((vec_t**)(tup->data))[0] = v0;
  ((vec_t**)(tup->data))[1] = v1;
  ((vec_t**)(tup->data))[2] = v2;

  float* data = copy_list_vecf_pack(points, nd);

  if (nd == 2){
    float out[8];
    geom_impl_bbox_2d(points->n, data, flags, out);
    memcpy(v0->data, out, sizeof(float)*2);
    memcpy(v1->data, out+2, sizeof(float)*2);
    memcpy(v2->data, out+4, sizeof(float)*4);
  }else if (nd == 3){
    float out[15];
    geom_impl_bbox_3d(points->n, data, flags, out);
    memcpy(v0->data, out, sizeof(float)*3);
    memcpy(v1->data, out+3, sizeof(float)*3);
    memcpy(v2->data, out+6, sizeof(float)*9);
  }

  ret->u.tup = tup;
  free(data);
}

EXPORTED void geom_clip(var_t* ret, gstate_t* _g){
  int flags = ARG_POP(_g,i32);
  lst_t* polygon = ARG_POP(_g,lst);
  lst_t* polyline = ARG_POP(_g,lst);

  float* dat0 = copy_list_vecf_pack(polyline, 2);
  float* dat1 = copy_list_vecf_pack(polygon, 2);

  int n_out;
  int* l_out;
  float** out = geom_impl_clip(
    polyline->n, dat0, polygon->n, dat1, 
    flags, &n_out, &l_out
  );

  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  lst->n = n_out;
  lst->cap = lst->n;
  lst->w = 8;
  lst->type = ret->type;
  lst->data = malloc(lst->w*lst->cap);
  ret->u.lst = lst;

  for (int i = 0; i < n_out; i++){
    lst_t* l = copy_list_vecf_unpack(_g, 
      (type_t*)(ret->type->u.elem.head->data), out[i], l_out[i], 2
    );
    free(out[i]);
    ((lst_t**)(lst->data))[i] = l;
  }

  free(dat0);
  free(dat1);
}


EXPORTED void geom_curve(var_t* ret, gstate_t* _g){
  int flags = ARG_POP(_g,i32);
  float t = ARG_POP(_g,f32);
  lst_t* params = ARG_POP(_g,lst);
  lst_t* ps = ARG_POP(_g,lst);

  float fparams[4] = {1.0,1.0,1.0,1.0};
  memcpy(fparams, params->data, sizeof(float)*params->n);

  int nd;
  if (ps->n){
    nd = ((vec_t**)(ps->data))[0]->n;
  }else{
    nd = atoi(((type_t*)(ret->type->u.elem.tail->data))->u.str.data);
  }

  vec_t* v = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(nd*4));
  v->n = nd;
  v->w = 4;
  v->type = ret->type;

  float* data = copy_list_vecf_pack(ps, nd);
  geom_impl_curve(data,nd,fparams,t,flags,(float*)(v->data));

  ret->u.vec = v;
  free(data);
}


#define QK_REG(name) register_cfunc(&(_g->cfuncs), "geom." QUOTE(name), geom_ ## name);

EXPORTED void lib_init_geom(gstate_t* _g){
  QK_REG(curve);
  QK_REG(clip);
  QK_REG(bbox);
  QK_REG(voronoi);
  QK_REG(delaunay);
  QK_REG(poly_simplify);
  QK_REG(poly_resample);
  QK_REG(pt_in_poly);
  QK_REG(convex_hull);
  QK_REG(triangulate);
  QK_REG(line_intersect);
}
