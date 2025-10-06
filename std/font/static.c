//

#include <stdint.h>

#include "impl.c"

void font___lookup(){
  uint32_t __ARG(flag);
  int t = __peek_arg_type();
  int n = __peek_arg_size();
  int gid;
  if (t == VART_I32){
    int32_t __ARG(code);
    int32_t __ARG(id);
    gid = font_impl__lookup(id,code,flag);
  }else if (t == VART_LST){
    __list_t* lst;
    __pop_arg(&lst, 8);
    int32_t __ARG(id);
    int n = lst->n;
    gid = font_impl__ligature(id,&n,(int32_t*)(lst->data),flag);
    memmove(lst->data, ((int32_t*)(lst->data)) + n, (lst->n - n)*sizeof(int32_t) );
    lst->n -= n;
  }
  __put_ret(&gid);
}

void font___advance(){
  uint32_t __ARG(flag);
  int32_t __ARG(hidx);
  int32_t __ARG(gidx);
  int32_t __ARG(id);
  float f = font_impl__advance(id,gidx,hidx,flag);
  __put_ret(&f);
}

void font___glyph(){
  int32_t __ARG(reso);
  int32_t __ARG(gidx);
  int32_t __ARG(id);
  int n;
  int* m;
  float* polys = font_impl__glyph(id,gidx,reso,&n,&m);
  __list_t* lst = __gc_alloc(VART_LST,sizeof(__list_t));;
  lst->cap = n+1;
  lst->n = n;
  lst->w = 8;
  lst->t = VART_LST;
  lst->data = malloc(lst->cap*lst->w);

  int pidx = 0;
  for (int i = 0; i < n; i++){
    __list_t* l = __gc_alloc(VART_LST,sizeof(__list_t));;
    l->n = m[i];
    l->cap = m[i]+1;
    l->w = 8;
    l->data = calloc(l->w,l->cap);
    l->t = VART_VEC;
    for (int j = 0; j < m[i]; j++){
      ((float*)(l->data))[j*2]   = polys[pidx++];
      ((float*)(l->data))[j*2+1] = polys[pidx++];
    }
    ((__list_t**)(lst->data))[i] = l;
  }
  __put_ret(&lst);
}

void font__decode(){
  __push_stack();

  __list_t* l = NULL;
  __pop_arg(&l, 8);
 
  int id = font_impl_decode(l->n,(char*)(l->data));

  void* o = __gc_alloc(VART_STT,16);
  __put_var(0,o);
  void* oo = (char*)o+4;
  ((void**)(oo))[0] = o;
  ((int32_t*)(oo))[2] = id;
  __put_ret(&(o));
  __pop_stack();
}