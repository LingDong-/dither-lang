#include <stdio.h>

#include "impl_json.c"

void** _jstack = NULL;
int _jcap = 0;
int _jlen = 0;
void* _jroot;

char* _jkey = NULL;

void _jpush(void* o){
  if (_jlen+1 > _jcap){
    _jcap = (_jcap+2)*2;
    _jstack = realloc(_jstack,sizeof(void*)*_jcap);
  }
  _jstack[_jlen++] = o;
}
void _jpop(){
  _jlen--;
}

__union_t* _jinit(void* o){
  ((int32_t*) o)[0] = 1;
  ((int32_t*) o)[1] = 16;
  ((void**) o)[1] = o;
  __union_t* u = __gc_alloc(VART_UON,20);
  ((void**) o)[2] = u;
  return u;
}

void _jinsert(void* o){
  if (!_jlen) return;
  void* par = _jstack[_jlen-1];
  __union_t* u = ((void**) par)[2];
  if (u->sel == 0){
    __list_t* l = *((void**)(u->data));
    if (l->n + 1 > l->cap){
      l->cap = l->cap*2+1;
      l->data = realloc(l->data, l->cap*l->w);
    }
    ((void**)l->data) [l->n++] = o;
  }else if (u->sel == 1){
    __dict_t* m = *((void**)(u->data));
    void** ptr = __dict_get(m, &_jkey);
    *ptr = o;
  }
}

void exch_impl__json_enter_dic(){
  void* o = _jlen ? __gc_alloc(VART_STT,24) : _jroot;
  __union_t* u = _jinit(o);
  u->sel = 1;
  u->w = 8;
  u->t = VART_DIC;
  __dict_t* m = __gc_alloc(VART_DIC,sizeof(__dict_t));
  *((void**)(u->data)) = m;
  m->n = 0;
  m->kt = VART_STR;
  m->vt = VART_STT;
  m->kw = 8;
  m->vw = 8;
  _jinsert(o);
  _jpush(o);
}
void exch_impl__json_exit_dic(){
  _jpop();
}
void exch_impl__json_enter_lst(){
  void* o = _jlen ? __gc_alloc(VART_STT,24) : _jroot;
  __union_t* u = _jinit(o);
  u->sel = 0;
  u->w = 8;
  u->t = VART_LST;
  __list_t* l = __gc_alloc(VART_LST,sizeof(__list_t));;
  *((void**)(u->data)) = l;
  l->cap = 8;
  l->n = 0;
  l->w = 8;
  l->t = VART_STT;
  l->data = malloc(l->cap*l->w);
  _jinsert(o);
  _jpush(o);
}
void exch_impl__json_exit_lst(){
  _jpop(); 
}

void exch_impl__json_key(char* s, int n){
  _jkey = __gc_alloc(VART_STR,n+1);
  exch_impl__json_copy_esc(_jkey,s,n);
  _jkey[n] = 0;
}

void exch_impl__json_num(float x){
  void* o = _jlen ? __gc_alloc(VART_STT,24) : _jroot;
  __union_t* u = _jinit(o);
  u->sel = 3;
  u->w = 4;
  u->t = VART_F32;
  *((float*)(u->data)) = x;
  _jinsert(o);
}



void exch_impl__json_str(char* s, int n){
  void* o = _jlen ? __gc_alloc(VART_STT,24) : _jroot;
  __union_t* u = _jinit(o);
  u->sel = 2;
  u->w = 8;
  u->t = VART_STR;
  char* ss = __gc_alloc(VART_STR,n+1);
  exch_impl__json_copy_esc(ss,s,n);
  ss[n]=0;
  *((char**)(u->data)) = ss;
  _jinsert(o);
}

void exch___decode_json(){
  char* __ARG(src);
  void* obj;
  __pop_arg(&obj, sizeof(obj));

  _jroot = obj;
  
  exch_impl__decode_json(src);
}


void _jencode(FILE* fd, void* o){
  __union_t* u = ((void**) o)[2];
  if (u->sel == 0){
    fprintf(fd,"[");
    __list_t* l = *((void**)(u->data));
    for (int i = 0; i < l->n; i++){
      _jencode(fd, ((void**)(l->data)) [i]);
      if (i != l->n-1){
        fprintf(fd,",");
      }
    }
    fprintf(fd,"]");
  }else if (u->sel == 1){
    fprintf(fd,"{");
    __dict_t* m = *((void**)(u->data));
    int n = 0;
    for (int i = 0; i < __NUM_DICT_SLOTS; i++){
      for (int j = 0; j < m->slots[i].n; j++){
        fprintf(fd,"\"%s\":",*(char**)(m->slots[i].data + (m->kw+m->vw) * j));
        _jencode(fd,  *(void**)(m->slots[i].data + (m->kw+m->vw) * j + m->kw)  );
        n++;
        if (n != m->n) fprintf(fd,",");
      }
    }
    fprintf(fd,"}");
  }else if (u->sel == 2){
    char* s = *((char**)(u->data));
    int n = strlen(s);
    fprintf(fd,"\"");
    for (int i = 0; i < n; i++){
      if (s[i] == '\n'){
        fprintf(fd,"\\n");
      }else if (s[i] == '\t'){
        fprintf(fd,"\\t");
      }else if (s[i] == '\r'){
        fprintf(fd,"\\r");
      }else if (s[i] == '\\'){
        fprintf(fd,"\\\\");
      }else if (s[i] == '\"'){
        fprintf(fd,"\\\"");
      }else{
        fprintf(fd,"%c",s[i]);
      }
    }
    fprintf(fd,"\"");
  }else if (u->sel == 3){
    fprintf(fd,"%g", *((float*)(u->data)) );
  }
}



void exch___encode_json(){
  void* obj;
  __pop_arg(&obj, sizeof(obj));

#ifdef _WIN32
  FILE *fd = tmpfile();
  _jencode(fd, obj);
  size_t len = ftell(fd);
  char* buf = malloc(len);
  rewind(fd);
  fread(buf, 1, len, fd);
  fclose(fd);
#else
  char *buf;
  size_t len;
  FILE *fd = open_memstream(&buf, &len);
  _jencode(fd, obj);
  fflush(fd);
  fclose(fd);
#endif
  
  char* ss = __gc_alloc(VART_STR,len+1);
  memcpy(ss, buf, len);

  free(buf);

  __put_ret(&ss);

}