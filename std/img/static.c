//
#include <stdio.h>

#include "impl.c"

void img__decode(){
  __list_t* l = NULL;
  __pop_arg(&l, 8);


  __arr_t* a = __gc_alloc(VART_ARR, sizeof(__arr_t)+12);
  a->ndim = 3;

  a->data = (char*)img_impl_decode((uint8_t*)(l->data),l->n,&(a->dims[1]),&(a->dims[0]),&(a->dims[2]));
  a->n = a->dims[0]*a->dims[1]*a->dims[2];
  a->w = 1;
  a->t = VART_U08;

  __put_ret(&a);
}

void img__encode(){
  __arr_t* a = NULL;
  __pop_arg(&a, 8);

  char* __ARG(s);

  __list_t* l = __gc_alloc(VART_LST, sizeof(__list_t));
  l->w = 1;
  l->t = VART_U08;
  int n;
  l->data = (char*)img_impl_encode(s,(uint8_t*)(a->data),a->dims[1],a->dims[0],a->dims[2],&n);
  l->n = n;
  l->cap = n;

  __put_ret(&l);
}

void img__dist_transform(){
  __arr_t* __ARG(out);
  int32_t __ARG(flags);
  __arr_t* __ARG(pix);

  if (out->dims[0]*out->dims[1]<pix->dims[0]*pix->dims[1]){
    out->n = pix->dims[0]*pix->dims[1];
    out->data = realloc(out->data, out->n*sizeof(float));
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  img_impl_dist_transform((void*)(pix->data), pix->dims[1], pix->dims[0], pix->w, flags, (float*)(out->data));

}

void img__convert(){
  __arr_t* __ARG(out);
  int32_t __ARG(flags);
  __arr_t* __ARG(pix);

  int it = pix->t;
  int ot = out->t;
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
    out->data = realloc(out->data, out->n*out->w);
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  void (*Fs[])(void*,int,int,int,int,void*,int) = {
    img_impl_convert_uint8_t_uint8_t,
    img_impl_convert_uint8_t_float,
    img_impl_convert_float_uint8_t,
    img_impl_convert_float_float,
  };
  int idx = ((it == VART_F32)<<1)|(ot == VART_F32);
  Fs[idx](pix->data,pix->dims[1],pix->dims[0],ic,flags,out->data,oc);
}

void img__threshold(){
  int32_t __ARG(flags);
  int32_t __ARG(thresh);
  __arr_t* __ARG(pix);

  img_impl_threshold((uint8_t*)(pix->data), pix->dims[1], pix->dims[0], thresh, flags);
}

void img__morphology(){
  __arr_t* __ARG(out);
  int32_t __ARG(flags);
  int32_t __ARG(rad);
  __arr_t* __ARG(pix);
  if (out->dims[0]*out->dims[1]<pix->dims[0]*pix->dims[1]){
    out->n = pix->dims[0]*pix->dims[1];
    out->data = realloc(out->data, out->n*sizeof(float));
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  img_impl_morphology((uint8_t*)(pix->data), pix->dims[1], pix->dims[0], rad, flags, (uint8_t*)(out->data));
}

void img__convolve(){
  __arr_t* __ARG(out);
  int32_t __ARG(flags);
  __arr_t* __ARG(kern);
  __arr_t* __ARG(pix);

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


void img__find_contours(){
  __arr_t* __ARG(pix);
  int* lens;
  int nret;

  float** contours = img_impl_find_contours((uint8_t*)(pix->data),pix->dims[1],pix->dims[0],&nret, &lens);

  __list_t* out = __gc_alloc(VART_LST, sizeof(__list_t));
  out->n = nret;
  out->cap = nret;
  out->w = sizeof(__list_t**);
  out->t = VART_LST;
  out->data = malloc(sizeof(__list_t**)*nret);

  for (int k = 0; k < nret; k++){
    __list_t* l = __gc_alloc(VART_LST, sizeof(__list_t));
    l->n = lens[k];
    l->cap = lens[k];
    l->w = sizeof(float)*2;
    l->t = VART_F32;
    l->data = (void*)(contours[k]);
    ((__list_t**)(out->data))[k] = l;
  }
  free(lens);
  __put_ret(&out);
}


void img__label_blobs(){
  __arr_t* __ARG(out);
  __arr_t* __ARG(pix);

  if (out->dims[0]*out->dims[1]<pix->dims[0]*pix->dims[1]){
    out->n = pix->dims[0]*pix->dims[1];
    out->data = realloc(out->data, out->n*sizeof(int));
  }
  out->dims[0] = pix->dims[0];
  out->dims[1] = pix->dims[1];
  img_impl_label_blobs((uint8_t*)(pix->data), pix->dims[1], pix->dims[0], (int*)(out->data));

}