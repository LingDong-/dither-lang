//
void list__insert(){
  int sz = __peek_arg_size();
#ifdef _WIN32
  char* item = (char*)_alloca(sz*sizeof(char));
#else
  char item[sz];
#endif
  __pop_arg(item, sz);
  
  int i;
  __pop_arg(&i, 4);
  __list_t* a = NULL;
  __pop_arg(&a, 8);
  // printf("%d %d %p\n",sz,i,a);
  // printf("%d %d %d\n",sz,a->w,a->n);

  if (a->n + 1 > a->cap){
    a->cap = (a->cap+1)*2;
    a->data = realloc(a->data,(a->cap)*(a->w));
  }
  memmove(a->data + ((i+1)*a->w), a->data + (i*a->w), (a->n-i)*a->w);


  for (int j = 0; j < a->w; j++){
    ((char*)(a->data))[i*a->w+j] = item[j];
  }
  a->n ++;
  __put_ret(&(a->n));
}

void list__length(){
  __list_t* a;
  __pop_arg(&a, 8);
  __put_ret(&(a->n));
}

void list__erase(){  
  int32_t __ARG(j);
  int32_t __ARG(i);

  __list_t* a = NULL;
  __pop_arg(&a, 8);


  memmove(a->data+((i)*a->w), a->data + (j*a->w), (a->n-j)*a->w);

  a->n-=(j-i);

  __put_ret(&(a->n));
}

void list__slice(){
  int32_t __ARG(j);
  int32_t __ARG(i);
  __list_t* a = NULL;
  __pop_arg(&a, 8);

  if (j<0) j+=a->n;
  if (i<0) i+=a->n;

  int n = j-i;

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->w = a->w;
  lst->t = a->t;
  lst->data = malloc(a->w*n);
  memcpy(lst->data, (char*)(a->data) + (a->w*i), n*(a->w));
  lst->cap = n;
  lst->n = n;
  __put_ret(&lst);
}

int _list_cmp(const void *a, const void *b){
  float c = (*(float *)a - *(float *)b);
  if (c < 0) return -1;
  if (c > 0) return 1;
  return 0;
}

void list___sort(){
  __list_t* b = NULL;
  __pop_arg(&b, 8);

  __list_t* a = NULL;
  __pop_arg(&a, 8);
  char* tmp = malloc(a->n*(a->w+4));
  for (int i = 0; i < a->n; i++){
    ((float*)(tmp + (i*(a->w+4)))) [0] = ((float*)b->data)[i];
    memcpy(tmp + (i*(a->w+4)+4), a->data + (i*a->w), a->w);
  }
  qsort(tmp, a->n, a->w+4, _list_cmp);
  for (int i = 0; i < a->n; i++){
    memcpy(a->data + (i*a->w), tmp + (i*(a->w+4)+4), a->w);
  }
  free(tmp);
}
