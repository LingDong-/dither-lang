//

// #define DBG 1
#include "../../src/interp.c"

#include "impl_json.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

#define OBJ_THIS(o) (((void**)((o)->data))[0])
#define OBJ_UON(o) (((void**)((o)->data))[1])
#define OBJ_UONV(o) ((uon_t*)(((void**)((o)->data))[1]))

type_t* jt_uon;
type_t* jt_obj;
type_t* jt_lst;
type_t* jt_dic;
type_t* jt_str;
type_t* jt_num;
gstate_t* __g;
list_t jstack = {0};
obj_t* root;
char* tmp_key = NULL;

void jencode(FILE* fd, obj_t* o){
  uon_t* u = OBJ_UONV(o);
  if (u->var->type->vart == VART_LST){
    fprintf(fd,"[");
    lst_t* l = u->var->u.lst;
    for (int i = 0; i < l->n; i++){
      jencode(fd,((obj_t**)(l->data))[i]);
      if (i != l->n-1){
        fprintf(fd,",");
      }
    }
    fprintf(fd,"]");
  }else if (u->var->type->vart == VART_DIC){
    fprintf(fd,"{");
    dic_t* dic = OBJ_UONV(o)->var->u.dic;
    int n = 0;
    for (int k = 0; k < NUM_MAP_SLOTS; k++){
      if (dic->map.slots[k].cap){
        for (int i = 0; i < dic->map.slots[k].len; i++){
          pair_t p = dic->map.slots[k].data[i];
          fprintf(fd,"\"%s\":",p.key);
          jencode(fd,*((obj_t**)p.val));
          n++;
          if (n != dic->map.len) fprintf(fd,",");
        }
      }
    }
    fprintf(fd,"}");
  }else if (u->var->type->vart == VART_F32){
    fprintf(fd,"%g",u->var->u.f32);
  }else if (u->var->type->vart == VART_STR){
    char* s = u->var->u.str->data;
    fprintf(fd,"\"");
    for (int i = 0; i < u->var->u.str->n; i++){
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
  }
}

EXPORTED void exch__encode_json(var_t* ret,  gstate_t* _g){
  obj_t* obj = ARG_POP(_g,obj);

  char *buf;
  size_t len;
  FILE *fd = open_memstream(&buf, &len);
  jencode(fd, obj);
  fflush(fd);
  fclose(fd);
  
  stn_t* ss = (stn_t*)gc_alloc(sizeof(stn_t)+len+1);
  ss->n = len;
  ss->w = 1;
  ss->type = jt_str;
  memcpy(ss->data, buf, len);
  ss->data[len] = 0;

  free(buf);

  ret->u.str = ss;
}

void jinsert(obj_t* o){
  if (!jstack.len)return;
  obj_t* par = (obj_t*)(jstack.tail->data);
  if (OBJ_UONV(par)->var->type->vart == VART_LST){
    lst_t* l = OBJ_UONV(par)->var->u.lst;
    if (l->n + 1 > l->cap){
      l->cap = l->cap*2+1;
      l->data = realloc(l->data,l->cap*l->w);
    }
    ((obj_t**)(l->data))[l->n++] = o;
  }else if (OBJ_UONV(par)->var->type->vart == VART_DIC){
    
    map_t* m = &(OBJ_UONV(par)->var->u.dic->map);
    obj_t** ptr = map_addr_raw(m, tmp_key, strlen(tmp_key), 8, 1);
    memcpy(ptr,&o,8);
  }
}
uon_t* jinit(obj_t* o){
  o->type = jt_obj;
  o->data = calloc(16,1);
  OBJ_THIS(o) = o;
  OBJ_UON(o) = gc_alloc(sizeof(uon_t));
  uon_t* u = OBJ_UONV(o);
  u->type = jt_uon;
  return u;
}

void exch_impl__json_enter_dic(){
  obj_t* o = jstack.len ? gc_alloc(sizeof(obj_t)) : root;
  uon_t* u = jinit(o);
  u->var = var_new_alloc(jt_dic,0);
  jinsert(o);
  list_add(&jstack, o);
}
void exch_impl__json_exit_dic(){ 
  list_pop(&jstack);
}

void exch_impl__json_enter_lst(){
  obj_t* o = jstack.len ? gc_alloc(sizeof(obj_t)) : root;
  uon_t* u = jinit(o);
  u->var = var_new_alloc(jt_lst,0);
  jinsert(o);
  list_add(&jstack, o);

}
void exch_impl__json_exit_lst(){
  list_pop(&jstack);
}


void exch_impl__json_key(char* s,int n){
  tmp_key = realloc(tmp_key,n+1);
  exch_impl__json_copy_esc(tmp_key,s,n);
  tmp_key[n] = 0;
}

void exch_impl__json_num(float x){
  obj_t* o = jstack.len ? gc_alloc(sizeof(obj_t)) : root;
  uon_t* u = jinit(o);

  u->var = var_new(jt_num);
  u->var->u.f32 = x;

  jinsert(o);
}

void exch_impl__json_str(char* s,int n){
  obj_t* o = jstack.len ? gc_alloc(sizeof(obj_t)) : root;
  uon_t* u = jinit(o);

  u->var = var_new(jt_str);
  stn_t* ss = (stn_t*)gc_alloc(sizeof(stn_t)+n+1);
  ss->n = n;
  ss->w = 1;
  ss->type = jt_str;
  exch_impl__json_copy_esc(ss->data, s, n);

  u->var->u.str = ss;
  jinsert(o);
}


EXPORTED void exch__decode_json(var_t* ret,  gstate_t* _g){
  
  stn_t* s = ARG_POP(_g,str);
  obj_t* obj = ARG_POP(_g,obj);

  jt_obj = obj->type;
  jt_uon = OBJ_UONV(obj)->type;
  jt_lst = jt_uon->u.elem.head->data;
  jt_dic = jt_uon->u.elem.head->next->data;
  jt_str = jt_uon->u.elem.tail->prev->data;
  jt_num = jt_uon->u.elem.tail->data;

  root = obj;
  exch_impl__decode_json(s->data);

}


EXPORTED void lib_init_exch(gstate_t* _g){
  _G.objs = _g->objs;
  register_cfunc(&(_g->cfuncs), "exch._encode_json", exch__encode_json);
  register_cfunc(&(_g->cfuncs), "exch._decode_json", exch__decode_json);
}


