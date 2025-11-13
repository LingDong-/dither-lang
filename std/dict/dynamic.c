//

// #define DBG 1
#include "../../src/interp.c"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

EXPORTED void dict_keys(var_t* ret,  gstate_t* _g){

  dic_t* dic = ARG_POP(_g,dic);

  type_t* ta = (type_t*)(dic->type->u.elem.head->data);
  type_t* tb = (type_t*)(dic->type->u.elem.tail->data);

  int ds = type_size(ta);

  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  lst->n = dic->map.len;
  lst->cap = lst->n+1;
  lst->type = ret->type;
  lst->w = ds;
  lst->data = calloc(lst->cap,ds);

  map_t* m = &(dic->map);
  int idx = 0;
  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    if (m->slots[k].cap){
      for (int i = 0; i < m->slots[k].len;i++){
        pair_t p = m->slots[k].data[i];
        if (ta->vart == VART_VEC){
          ((vec_t**)(lst->data))[idx++] = vec_copy_(_g, (void*) (((pair_t*)p.val)->key) );
        }else if (ta->vart == VART_TUP){
          ((tup_t**)(lst->data))[idx++] = tup_copy_(_g, (void*) (((pair_t*)p.val)->key) );
        }else if (ta->vart == VART_STR){
          int l = strlen(p.key);
          stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+l+1);
          s->n = l;
          s->w = 1;
          s->type = ta;
          memcpy(s->data, p.key, ds);
          ((stn_t**)(lst->data))[idx++] = s;
        }else{
          memcpy((char*)(lst->data) + (idx++)*ds, p.key, ds);
        }
      }
    }
  }
  ret->u.lst = lst;
}




EXPORTED void dict_values(var_t* ret,  gstate_t* _g){

  dic_t* dic = ARG_POP(_g,dic);

  type_t* ta = (type_t*)(dic->type->u.elem.head->data);
  type_t* tb = (type_t*)(dic->type->u.elem.tail->data);

  int ds = type_size(tb);

  lst_t* lst = (lst_t*)gc_alloc_(_g,sizeof(lst_t));
  lst->n = dic->map.len;
  lst->cap = lst->n+1;
  lst->type = ret->type;
  lst->w = ds;
  lst->data = calloc(lst->cap,ds);

  map_t* m = &(dic->map);
  int idx = 0;
  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    if (m->slots[k].cap){
      for (int i = 0; i < m->slots[k].len;i++){
        pair_t p = m->slots[k].data[i];
        void* ptr = p.val;
        if (ta->vart == VART_VEC || ta->vart == VART_TUP){
          ptr = & (((pair_t*)(p.val))->val);
        }
        if (tb->vart == VART_VEC){
          ((vec_t**)(lst->data))[idx++] = vec_copy_(_g, *(void**)ptr );
        }else if (tb->vart == VART_TUP){
          ((tup_t**)(lst->data))[idx++] = tup_copy_(_g, *(void**)ptr );
        }else if (tb->vart == VART_STR){
          ((stn_t**)(lst->data))[idx++] = stn_copy_(_g, *(void**)ptr );
        }else{
          memcpy((char*)(lst->data) + (idx++)*ds, ptr, ds);
        }
      }
    }
  }
  
  ret->u.lst = lst;
}


EXPORTED void lib_init_dict(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "dict.keys", dict_keys);
  register_cfunc(&(_g->cfuncs), "dict.values", dict_values);
}


