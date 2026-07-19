//

#define LEADING  1
#define TRAILING 2

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

void str__encode(){
  char* __ARG(e);
  char* __ARG(s);
  __list_t* a;

  int sn = strlen(s);

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->w = 1;
  lst->t = VART_U08;
  lst->data = (char*)malloc(sn);
  memcpy(lst->data, s, sn);
  lst->cap = sn;
  lst->n = sn;
  __put_ret(&lst);
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

void str__affixed(){
  int __ARG(end);
  char* __ARG(cs);
  char* __ARG(s);

  int sn = strlen(s);
  int csn = strlen(cs);

  int i0 = 0;
  if (end == TRAILING){
    i0 = sn - csn;
  }
  for (int i = 0; i < csn; i++){
    if (cs[i] != s[i0+i]){
      int r = 0;
      __put_ret(&r);
      return;
    }
  }
  int r = 1;
  __put_ret(&r);
  return;
}

void str__pad(){
  int __ARG(end);
  char* __ARG(cs);
  int __ARG(n);
  char* __ARG(s);

  int sn = strlen(s);
  int csn = strlen(cs);

  if (n < sn) n = sn;

  char* o = __gc_alloc(VART_STR,n+1);

  int i0 = 0;
  int i1 = n-sn;
  int i2 = i1;
  if (end == TRAILING){
    i0 = sn;
    i1 = n;
    i2 = 0;
  }
  memcpy(o + i2, s, sn);
  for (int i = i0; i < i1; i++){
    o[i] = cs[(i-i0)%csn];
  }

  __put_ret(&o);
}