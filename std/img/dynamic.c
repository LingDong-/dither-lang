//

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

EXPORTED void img_decode(var_t* ret, gstate_t* _g){
  lst_t* l = ARG_POP(_g,lst);

  int w,h,chan;
  uint8_t* data = img_impl_decode((uint8_t*)(l->data), l->n, &w, &h, &chan);

  arr_t* arr = (arr_t*)gc_alloc_(_g,sizeof(arr_t)+12);

  arr->ndim = 3;
  arr->w = 1;
  arr->type = ret->type;
  arr->n = w*h*chan;
  arr->dims[0] = h;
  arr->dims[1] = w;
  arr->dims[2] = chan;

  arr->data = (char*)data;
  ret->u.arr = arr;
}

EXPORTED void img_encode(var_t* ret, gstate_t* _g){
  arr_t* a = ARG_POP(_g,arr);
  stn_t* s = ARG_POP(_g,str);

  int n;
  void* data = img_impl_encode(s->data, (uint8_t*)(a->data), a->dims[1], a->dims[0], a->dims[2], &n);

  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));

  lst->n = n;
  lst->cap = n;
  lst->w = 1;
  lst->type = ret->type;
  lst->data = data;
  ret->u.lst = lst;
}

EXPORTED void img_dist_transform(var_t* ret, gstate_t* _g){
  arr_t* out = ARG_POP(_g,arr);
  int flags = ARG_POP(_g,i32);
  arr_t* pix = ARG_POP(_g,arr);
  if (out->dims[0]*out->dims[1]<pix->dims[0]*pix->dims[1]){
    out->n = pix->dims[0]*pix->dims[1];
    out->data = realloc(out->data, out->n*sizeof(float));
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  img_impl_dist_transform((void*)(pix->data), pix->dims[1], pix->dims[0], pix->w, flags, (float*)(out->data));
}

EXPORTED void img_convert(var_t* ret, gstate_t* _g){
  arr_t* out = ARG_POP(_g,arr);
  int flags = ARG_POP(_g,i32);
  arr_t* pix = ARG_POP(_g,arr);

  type_t* it = (type_t*)(pix->type->u.elem.head->data);
  type_t* ot = (type_t*)(out->type->u.elem.head->data);
  if (out->ndim == 3 && (out->dims[2] == 0 || (out->dims[0]==0 && out->dims[1]==0))){
    out->dims[2] = 1;
    if (pix->ndim == 3){
      out->dims[2] = pix->dims[2];
    }
    if ((out->dims[2] == 4 || out->dims[2] == 2) && (flags&MASK_ALPHA) == ALPHA_DROP){
      out->dims[2] --;
    }
  }
  int oc = out->ndim == 3 ? out->dims[2] : 1;
  int ic = pix->ndim == 3 ? pix->dims[2] : 1;
  if (out->dims[0]*out->dims[1]<pix->dims[0]*pix->dims[1]){
    out->n = pix->dims[0]*pix->dims[1]*oc;
    out->data = realloc(out->data, out->n*type_size( ot ));
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  
  void (*Fs[])(void*,int,int,int,int,void*,int) = {
    img_impl_convert_uint8_t_uint8_t,
    img_impl_convert_uint8_t_float,
    img_impl_convert_float_uint8_t,
    img_impl_convert_float_float,
  };
  int idx = ((it->vart == VART_F32)<<1)|(ot->vart == VART_F32);
  
  Fs[idx](pix->data,pix->dims[1],pix->dims[0],ic,flags,out->data,oc);
}

EXPORTED void img_threshold(var_t* ret, gstate_t* _g){
  int flags = ARG_POP(_g,i32);
  int thresh = ARG_POP(_g,i32);
  arr_t* pix = ARG_POP(_g,arr);

  img_impl_threshold((uint8_t*)(pix->data), pix->dims[1], pix->dims[0], thresh, flags);
}

EXPORTED void img_morphology(var_t* ret, gstate_t* _g){
  arr_t* out = ARG_POP(_g,arr);
  int flags = ARG_POP(_g,i32);
  int rad = ARG_POP(_g,i32);
  arr_t* pix = ARG_POP(_g,arr);
  if (out->dims[0]*out->dims[1]<pix->dims[0]*pix->dims[1]){
    out->n = pix->dims[0]*pix->dims[1];
    out->data = realloc(out->data, out->n*sizeof(float));
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  img_impl_morphology((uint8_t*)(pix->data), pix->dims[1], pix->dims[0], rad, flags, (uint8_t*)(out->data));
}

EXPORTED void img_convolve(var_t* ret, gstate_t* _g){
  arr_t* out = ARG_POP(_g,arr);
  int flags = ARG_POP(_g,i32);
  arr_t* kern = ARG_POP(_g,arr);
  arr_t* pix = ARG_POP(_g,arr);
  if (out->dims[0]*out->dims[1]<pix->dims[0]*pix->dims[1]){
    out->n = pix->dims[0]*pix->dims[1];
    out->data = realloc(out->data, out->n*sizeof(float));
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  if (pix->w == 4){
    img_impl_convolve_f32((float*)(pix->data), pix->dims[1], pix->dims[0], (float*)(kern->data), kern->dims[1], kern->dims[0], flags, (float*)(out->data));
  }else{
    img_impl_convolve_u8((uint8_t*)(pix->data), pix->dims[1], pix->dims[0], (float*)(kern->data), kern->dims[1], kern->dims[0], flags, (uint8_t*)(out->data));
  }
}

EXPORTED void img_find_contours(var_t* ret, gstate_t* _g){
  arr_t* pix = ARG_POP(_g,arr);
  int* lens;
  int nret;
  float** contours = img_impl_find_contours((uint8_t*)(pix->data),pix->dims[1],pix->dims[0],&nret, &lens);

  lst_t* out = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  out->n = nret;
  out->cap = nret;
  out->w = sizeof(lst_t**);
  out->type = ret->type;
  out->data = malloc(sizeof(lst_t**)*nret);

  for (int k = 0; k < nret; k++){
    lst_t* l = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
    l->n = lens[k];
    l->cap = lens[k];
    l->w = sizeof(vec_t**);
    l->type = (type_t*)(ret->type->u.elem.head->data);
    l->data = malloc(sizeof(vec_t**)*lens[k]);
    for (int i = 0; i < lens[k]; i++){
      vec_t* v = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(2*sizeof(float)));
      v->n = 2;
      v->w = sizeof(float);
      v->type = (type_t*)(l->type->u.elem.head->data);
      memcpy(v->data, contours[k] + (i*2), 2*sizeof(float));
      ((vec_t**)(l->data))[i] = v;
    }
    ((lst_t**)(out->data))[k] = l;
    free(contours[k]);
  }
  free(lens);
  ret->u.lst = out;
}


EXPORTED void img_label_blobs(var_t* ret, gstate_t* _g){
  arr_t* out = ARG_POP(_g,arr);
  arr_t* pix = ARG_POP(_g,arr);
  if (out->dims[0]*out->dims[1]<pix->dims[0]*pix->dims[1]){
    out->n = pix->dims[0]*pix->dims[1];
    out->data = realloc(out->data, out->n*sizeof(int));
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  img_impl_label_blobs((void*)(pix->data), pix->dims[1], pix->dims[0], (int*)(out->data));
}



#define QK_REG(name) register_cfunc(&(_g->cfuncs), "img." QUOTE(name), img_ ## name);


EXPORTED void lib_init_img(gstate_t* _g){
  QK_REG(decode)
  QK_REG(encode)
  QK_REG(dist_transform)
  QK_REG(convert)
  QK_REG(threshold)
  QK_REG(morphology)
  QK_REG(convolve)
  QK_REG(find_contours)
  QK_REG(label_blobs)
}


