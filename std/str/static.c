//
void str__length(){
  char* __ARG(s); 
  int32_t l = strlen(s);
  __put_ret(&(l));
}

void str__chr(){
  int32_t __ARG(a); 
  char* o = __gc_alloc(VART_STR,2);

  o[0] = a;
    
  __put_ret(&(o));
}

void str__decode(){
  char* __ARG(e);
  __list_t* a;
  __pop_arg(&a, 8);

  char* o = __gc_alloc(VART_STR,a->n+1);
  memcpy(o,a->data,a->n);
  __put_ret(&(o));
}


void str__slice(){
  int __ARG(j);
  int __ARG(i);
  char* __ARG(a);

  int n = j-i;

  char* o = __gc_alloc(VART_STR,n+1);
  memcpy(o,a+i,n);
  __put_ret(&o);
}

void str__split(){
  char* __ARG(b);
  char* __ARG(a);

  char** ss = NULL;
  int an = strlen(a);
  int bn = strlen(b);
  int sn = 0;
  int start = 0;
  for (int i = 0; i < an+1-bn+1; i++){
    if (i < an+1-bn){
      for (int j = 0; j < bn; j++){
        if (a[i+j] != b[j]){
          goto nextchar;
        }
      }
    }
    int n = i-start;
    char* s = __gc_alloc(VART_STR,n+1);
    memcpy(s, a+start, n);
    ss = (char**)realloc(ss,(sn+1)*sizeof(char*));
    ss[sn++] = s;
    start = i+bn;
    nextchar: continue;
  }

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->w = sizeof(char*);
  lst->t = VART_STR;
  lst->data = (char*)ss;
  lst->cap = sn;
  lst->n = sn;
  __put_ret(&lst);
}

void str__trim(){
  int __ARG(mode);
  char* __ARG(b);
  char* __ARG(a);

  int an = strlen(a);
  int bn = strlen(b);

  int start = 0;
  int end = an;
  int i;
  if (mode & 1){
    for (i = 0; i < end; i++){
      for (int j = 0; j < bn; j++){
        if (a[i] == b[j]){
          goto nextchar;
        }
      }
      break;
      nextchar: continue;
    }
    start = i;
  }
  if (mode & 2){
    for (i = end-1; i >= start; i--){
      for (int j = 0; j < bn; j++){
        if (a[i] == b[j]){
          goto prevchar;
        }
      }
      break;
      prevchar: continue;
    }
    end = i+1;
  }
  int n = end-start;
  char* s = __gc_alloc(VART_STR,n+1);
  memcpy(s, a+start, n);
  __put_ret(&s);
}


void str__join(){
  __list_t* a = NULL;
  __pop_arg(&a, 8);

  char* __ARG(s);

  int n = 0;
  int sl = strlen(s);
  for (int i = 0; i < a->n; i++){
    n += strlen(((char**)(a->data))[i])+sl;
  }
  
  char* o = __gc_alloc(VART_STR,n+1);
  n = 0;
  for (int i = 0; i < a->n; i++){
    int l = strlen((((char**)(a->data))[i]));
    memcpy(o+n, (((char**)(a->data))[i]), l);
    n += l;
    if (i < a->n-1){
      memcpy(o+n, s, sl);
      n += sl;
    }
  }
  o[n] = 0;
  __put_ret(&o);

}