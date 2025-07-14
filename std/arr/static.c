//
void arr__shape(){
  __arr_t* a;
  __pop_arg(&a, 8);
  __put_ret(&(a->dims));
}

void arr__reshape(){
  int ndim = __peek_arg_size()/sizeof(int32_t);
  __vla(int32_t,v,ndim);

  __pop_arg(v, ndim*sizeof(int32_t));

  __arr_t* a;
  __pop_arg(&a, 8);

  int n = 1;
  for (int i = 0; i < ndim; i++){
    n *= v[i];
  }
  if (a->n != n){
    a->n = n;
    a->data = realloc(a->data,a->w*a->n);
  }

  memcpy(a->dims, v, ndim*sizeof(int32_t));
}

void arr__make(){
  int t = __peek_arg_type();
  int w = __peek_arg_size();
  __vla(char,e,w);
  __pop_arg(e, w);
  
  int ndim = __peek_arg_size()/sizeof(int32_t);
  __vla(int32_t,v,ndim);
  __pop_arg(v, ndim*sizeof(int32_t));

  int n = 1;
  for (int i = 0; i < ndim; i++){
    n *= v[i];
  }

  __arr_t* a = __gc_alloc(VART_ARR, sizeof(__arr_t)+ndim*4);
  a->ndim = ndim;
  a->n = n;
  a->w = w;
  a->t = t;
  a->data = malloc(w*n);
  memcpy(a->dims, v, ndim*sizeof(int32_t));

  for (int i = 0; i < n; i++){
    if (t == VART_STR){
      char* s = __gc_alloc(VART_STR,strlen(e)+1);
      strcpy(s,e);
      memcpy(a->data + (i*w), &s, w);
    }else{
      memcpy(a->data + (i*w), e, w);
    }
    
  }

  __put_ret(&a);

}

