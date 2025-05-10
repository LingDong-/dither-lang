//

// #define DBG 1
#include "../../src/interp.c"

#include "impl_json.c"

#ifndef EXPORTED
#define EXPORTED __attribute__ ((visibility ("default")))
#endif

EXPORTED void exch__encode_json(var_t* ret,  gstate_t* _g){
  
}

uon_t* jnode;
type_t* jt_uon;
type_t* jt_obj;
type_t* jt_lst;
type_t* jt_dic;
type_t* jt_str;
type_t* jt_num;
gstate_t* __g;

void exch_impl__json_enter_dic(){
}
void exch_impl__json_exit_dic(){ 
}

void exch_impl__json_enter_lst(){
  jnode->var = var_new_alloc(jt_lst,0);
}
void exch_impl__json_exit_lst(){
}

void exch_impl__json_key_len(int n){
}
void exch_impl__json_key(char* s){
}

void exch_impl__json_num(float x){
  uon_t* onode = jnode;
  obj_t* o = gc_alloc(sizeof(obj_t));
  o->flag = 0;
  o->type = jt_obj;
  o->data = calloc(16,1);
  ((void**)(o->data))[0] = o;
  ((void**)(o->data))[1] = gc_alloc(sizeof(uon_t));
  jnode = ((void**)(o->data))[1];
  jnode->type = jt_uon;
  jnode->var = var_new(jt_num);
  jnode->var->u.f32 = x;
  jnode = onode;

  lst_t* l = jnode->var->u.lst;
  
  if (l->n + 1 > l->cap){
    l->cap = l->cap*2+1;
    l->data = realloc(l->data,l->cap*l->w);
  }
  ((obj_t**)(l->data))[l->n++] = o;
}
void exch_impl__json_str_len(int n){
}
void exch_impl__json_str(char* s){
}


EXPORTED void exch__decode_json(var_t* ret,  gstate_t* _g){
  
  stn_t* s = ARG_POP(_g,str);
  obj_t* obj = ARG_POP(_g,obj);

  jt_obj = obj->type;
  jt_uon = ((uon_t*)(((void**)(obj->data))[1]))->type;
  jt_lst = jt_uon->u.elem.head->data;
  jt_dic = jt_uon->u.elem.head->next->data;
  jt_str = jt_uon->u.elem.tail->prev->data;
  jt_num = jt_uon->u.elem.tail->data;

  uon_t* u = gc_alloc(sizeof(uon_t));
  u->type = jt_uon;
  ((void**)(obj->data))[1] = u;

  jnode = u;
  exch_impl__decode_json(s->data);

}


EXPORTED void lib_init_exch(gstate_t* _g){
  _G.objs = _g->objs;
  register_cfunc(&(_g->cfuncs), "exch._encode_json", exch__encode_json);
  register_cfunc(&(_g->cfuncs), "exch._decode_json", exch__decode_json);
}


