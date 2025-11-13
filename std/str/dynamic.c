//

#include "../../src/interp.c"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

#define QUOTED(x) QUOTE(x)
#define QUOTE(x) #x

EXPORTED void str_length(var_t* ret,  gstate_t* _g){
  stn_t* a = ARG_POP(_g,str);
  ret->u.i32 = a->n;
}

EXPORTED void str_chr(var_t* ret,  gstate_t* _g){
  uint32_t a = ARG_POP(_g,u32);

  stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+1+1);
  s->n = 1;
  s->w = 1;
  s->type = ret->type;
  s->data[0] = a;
  ret->u.str = s;
}

EXPORTED void str_decode(var_t* ret,  gstate_t* _g){
  stn_t* e = ARG_POP(_g,str);
  lst_t* a = ARG_POP(_g,lst);

  stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+a->n+1);
  s->n = a->n;
  s->w = 1;
  s->type = ret->type;
  memcpy(s->data, a->data, a->n);
  ret->u.str = s;
}

EXPORTED void str_slice(var_t* ret,  gstate_t* _g){
  int j = ARG_POP(_g,i32);
  int i = ARG_POP(_g,i32);
  stn_t* a = ARG_POP(_g,str);

  int n = j-i;

  stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+n+1);
  s->n = n;
  s->w = 1;
  s->type = ret->type;
  memcpy(s->data, ((char*)(a->data))+i, n);
  ret->u.str = s;
}

EXPORTED void str_split(var_t* ret,  gstate_t* _g){
  stn_t* b = ARG_POP(_g,str);
  stn_t* a = ARG_POP(_g,str);
  stn_t** ss = NULL;
  int sn = 0;
  int start = 0;
  for (int i = 0; i < a->n+1-b->n+1; i++){
    if (i < a->n+1-b->n){
      for (int j = 0; j < b->n; j++){
        if (a->data[i+j] != b->data[j]){
          goto nextchar;
        }
      }
    }
    int n = i-start;
    stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+n+1);
    s->n = n;
    s->w = 1;
    s->type = a->type;
    memcpy(s->data, ((char*)(a->data))+start, n);
    ss = (stn_t**)realloc(ss,(sn+1)*sizeof(stn_t*));
    ss[sn++] = s;
    start = i+b->n;
    nextchar: continue;
  }

  ret->u.lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  ret->u.lst->data = (char*)ss;
  ret->u.lst->n = sn;
  ret->u.lst->w = 8;
  ret->u.lst->type = ret->type;
  ret->u.lst->cap = sn; 
}


EXPORTED void str_trim(var_t* ret,  gstate_t* _g){
  int mode = ARG_POP(_g,i32);
  stn_t* b = ARG_POP(_g,str);
  stn_t* a = ARG_POP(_g,str);

  int start = 0;
  int end = a->n;
  int i;
  if (mode & 1){
    for (i = 0; i < end; i++){
      for (int j = 0; j < b->n; j++){
        if (a->data[i] == b->data[j]){
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
      for (int j = 0; j < b->n; j++){
        if (a->data[i] == b->data[j]){
          goto prevchar;
        }
      }
      break;
      prevchar: continue;
    }
    end = i+1;
  }

  int n = end-start;
  stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+n+1);
  s->n = n;
  s->w = 1;
  s->type = ret->type;
  memcpy(s->data, ((char*)(a->data))+start, n);
  ret->u.str = s;
}


EXPORTED void str_join(var_t* ret,  gstate_t* _g){
  lst_t* a = ARG_POP(_g,lst);
  stn_t* s = ARG_POP(_g,str);
 
  int n = 0;
  int sl = s->n;
  for (int i = 0; i < a->n; i++){
    n += ((stn_t**)(a->data))[i]->n;
  }
  
  stn_t* o = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+n+1);
  n = 0;
  for (int i = 0; i < a->n; i++){
    int l = ((stn_t**)(a->data))[i]->n;
    memcpy(((char*)(o->data))+n, ((stn_t**)(a->data))[i]->data, l);
    n += l;
    if (i < a->n-1){
      memcpy(((char*)(o->data))+n, s->data, sl);
      n += sl;
    }
  }
  o->data[n] = 0;
  
  o->n = n;
  o->w = 1;
  o->type = ret->type;
  ret->u.str=o;
}

#define QK_REG(name) register_cfunc(&(_g->cfuncs), "str." QUOTE(name), str_ ## name);

EXPORTED void lib_init_str(gstate_t* _g){
  QK_REG(length);
  QK_REG(chr);
  QK_REG(decode);
  QK_REG(slice);
  QK_REG(split);
  QK_REG(trim);
  QK_REG(join);
}


