//
void dict__keys(){
  __dict_t* dic;
  __pop_arg(&dic, 8);

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->n = dic->n;
  lst->w = dic->kw;
  lst->t = dic->kt;
  lst->cap = dic->n+1;
  lst->data = malloc(lst->w*lst->cap);

  int idx = 0;
  for (int i = 0; i < __NUM_DICT_SLOTS; i++){
    for (int j = 0; j < dic->slots[i].n; j++){
      memcpy(
        ((char*)(lst->data))+ (idx++)*(lst->w), 
        ((char*)(dic->slots[i].data) + (dic->kw+dic->vw) * j),
        lst->w
      );
    }
  }
  __put_ret(&lst);
}


void dict__values(){
  __dict_t* dic;
  __pop_arg(&dic, 8);

  __list_t* lst = __gc_alloc(VART_LST, sizeof(__list_t));
  lst->n = dic->n;
  lst->w = dic->vw;
  lst->t = dic->vt;
  lst->cap = dic->n+1;
  lst->data = malloc(lst->w*lst->cap);

  int idx = 0;
  for (int i = 0; i < __NUM_DICT_SLOTS; i++){
    for (int j = 0; j < dic->slots[i].n; j++){
      memcpy(
        ((char*)(lst->data))+ (idx++)*(lst->w), 
        ((char*)(dic->slots[i].data) + (dic->kw+dic->vw) * j + dic->kw),
        lst->w
      );
    }
  }
  __put_ret(&lst);
}
