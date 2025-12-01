//

// #include <assert.h>

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

void dict__has(){
  int sz = __peek_arg_size();
#ifdef _WIN32
  char* item = (char*)_alloca(sz*sizeof(char));
#else
  char item[sz];
#endif
  __pop_arg(item, sz);

  __dict_t* dic;
  __pop_arg(&dic, 8);

  // assert(dic->kw == sz);

  int32_t found = 0;

  for (int i = 0; i < __NUM_DICT_SLOTS; i++){
    for (int j = 0; j < dic->slots[i].n; j++){
      int r = memcmp(((char*)(dic->slots[i].data) + (dic->kw+dic->vw) * j), item, sz);
      if (r == 0){
        found = 1;
        goto done;
      }
    }
  }
done:
  __put_ret(&found);

}