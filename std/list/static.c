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