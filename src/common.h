#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#define no_segfault_yet printf("no segfault yet on line %d %s!\n",__LINE__,__FILE__);

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif


#define ARR_DEF(dtype) \
  typedef struct { int len; int cap; dtype* data; } dtype ## _arr_t;

#define ARR_INIT(dtype,name) \
  name.len = 0;  \
  name.cap = 8; \
  name.data = (dtype*) malloc((name.cap)*sizeof(dtype));

#define ARR_PUSH(dtype,name,item) \
  if (name.cap < name.len+1){ \
    int hs = name.cap/2; \
    name.cap = name.len+MAX(1,hs); \
    name.data = (dtype*)realloc(name.data, (name.cap)*sizeof(dtype) ); \
  }\
  name.data[name.len] = (dtype)item;\
  name.len += 1;

#define ARR_POP(dtype,name) (name.data[--name.len])

#define ARR_CLEAR(dtype,name) {name.len = 0;}


typedef struct str_st {
  int len;
  int cap;
  char* data;
} str_t;

typedef struct list_node_st {
  struct list_node_st *prev;
  struct list_node_st *next;
  void* data;
} list_node_t;

typedef struct list_st {
  list_node_t *head;
  list_node_t *tail;
  int len;
} list_t;

typedef struct pair_st {
  char* key;
  void* val;
} pair_t;

ARR_DEF(pair_t)
ARR_DEF(uint64_t)
ARR_DEF(int8_t)
ARR_DEF(list_t)
ARR_DEF(uintptr_t)

#define NUM_MAP_SLOTS 32
typedef struct map_st {
  int len;
  pair_t_arr_t slots[NUM_MAP_SLOTS];
} map_t;

void list_init(list_t *l){
  l->head = NULL;
  l->tail = NULL;
  l->len = 0;
}

list_t* list_new_on_heap(){
  list_t* l = (list_t*)malloc(sizeof(list_t));
  l->head = NULL;
  l->tail = NULL;
  l->len = 0;
  return l;
}


void list_add(list_t* l, void* dataptr){
  list_node_t* n = (list_node_t*)malloc( sizeof(list_node_t) );
  n->data = dataptr;
  n->next = NULL;
  n->prev = l->tail;
  if (l->head == NULL){
    l->head = n;
  }else{
    l->tail->next = n;
  }
  l->tail = n;
  l->len ++;
}

void list_insert_l(list_t* l, list_node_t* node, void* dataptr){
  if (l->head == NULL){
    list_add(l,dataptr);
    return;
  }
  list_node_t* n = (list_node_t*)malloc( sizeof(list_node_t) );
  n->data = dataptr;

  n->prev = node->prev;
  n->next = node;

  if (l->head == node){
    l->head = n;
  }else{
    node->prev->next = n;
  }
  node->prev = n;

  l->len ++;
}

void list_insert_r(list_t* l, list_node_t* node, void* dataptr){
  if (node == l->tail || l->head == NULL){
    list_add(l,dataptr);
    return;
  }
  list_node_t* n = (list_node_t*)malloc( sizeof(list_node_t) );
  n->data = dataptr;
  n->prev = node;
  n->next = node->next;
  node->next->prev = n;
  node->next = n;

  l->len ++;
}

void list_pop(list_t* l){
  list_node_t* n = l->tail;
  if (l->tail == l->head){
    l->head = NULL;
    l->tail = NULL;
    l->len --;
    if (n) free(n);
    return;
  }
  l->tail->prev->next = NULL;
  l->tail = l->tail->prev;
  l->len --;
  if (n) free(n);
}

void list_pophead(list_t* l){
  list_node_t* n = l->head;
  if (l->tail == l->head){
    l->head = NULL;
    l->tail = NULL;
    l->len --;
    if (n) free(n);
    return;
  }
  l->head->next->prev = NULL;
  l->head = l->head->next;
  l->len --;
  if (n) free(n);
}

void list_rem(list_t* l, list_node_t* node){
  if (node == l->tail){
    return list_pop(l);
  }else if (node == l->head){
    return list_pophead(l);
  }
  node->prev->next = node->next;
  node->next->prev = node->prev;
  l->len--;
  if (node) free(node);
}

void list_nuke(list_t* l){
  list_node_t* it = l->head;
  while (it){
    free(it->data);
    list_node_t* nxt = it->next;
    free(it);
    it = nxt;
  }
  free(l);
}

str_t str_new(){
  str_t s;
  s.cap = 31;
  s.data = (char*) malloc((s.cap+1)*sizeof(char));
  s.data[0] = 0;
  s.len = 0;
  return s;
}

str_t str_from(const char* cs, int l){
  str_t s;
  s.cap = l;
  s.len = l;
  s.data = (char*)malloc((l+1)*sizeof(char));
  memcpy(s.data,cs,l*sizeof(char));
  s.data[l] = 0;
  return s;
}

str_t str_from2(const char* cs0, int l0, const char* cs1, int l1){
  str_t s;
  s.cap = l0+l1;
  s.len = l0+l1;
  s.data = (char*)malloc((l0+l1+1)*sizeof(char));
  memcpy(s.data,cs0,l0*sizeof(char));
  memcpy(s.data+l0,cs1,l1*sizeof(char));
  s.data[l0+l1] = 0;
  return s;
}

str_t str_fromch(char c){
  str_t s;
  s.cap = 1;
  s.len = 1;
  s.data = (char*)malloc(sizeof(char)*2);
  s.data[0] = c;
  s.data[1] = 0;
  return s;
}

str_t str_fromnum(double n){
  char buf[32];
  snprintf(buf,32,"%g",n);
  size_t l = strlen(buf);
  str_t s;
  s.cap = l;
  s.len = l;
  s.data = (char*)malloc((l+1)*sizeof(char));
  memcpy(s.data,buf,l*sizeof(char));
  s.data[l] = 0;
  return s;
}

void str_add (str_t* s, const char* cs){
  int l = strlen(cs);
  if (s->cap < s->len + l){
    int hs = s->cap/2;
    s->cap = s->len+MAX(l,hs);
    s->data = (char*)realloc(s->data, (s->cap+1)*sizeof(char) );
  }
  memcpy(&s->data[s->len],cs, l*sizeof(char));
  s->len += l;
  s->data[s->len] = 0;
}

void str_addch (str_t* s, char c){
 
  if (s->cap < s->len + 1){
    int hs = s->cap/2;
    s->cap = s->len+MAX(1,hs);
    s->data = (char*)realloc(s->data, (s->cap+1)*sizeof(char) );
  }
  s->data[s->len] = c;
  s->len += 1;
  s->data[s->len] = 0;
}

int str_eq(str_t* s, const char* cs){
  if (strncmp(s->data,cs,s->len) == 0){
    return 1;
  }
  return 0;
}

int map_hash_bk(char* s, int l){
  uint64_t x = 0;
  // l = MIN(l,32);
  for (int i = 0; i < l; i++){
    char y = s[i];
    x ^= y;
  }
  x %= NUM_MAP_SLOTS;
  return x;
}

int map_hash(str_t* s){
  return map_hash_bk(s->data,s->len);
}

void map_add(map_t* m, str_t* s, void* dataptr){
  int k = map_hash(s);
  if (!(m->slots[k].cap)){
    ARR_INIT(pair_t,m->slots[k]);
  }
  pair_t kv;
  kv.key = malloc(s->len+1);
  kv.key[s->len]=0;
  memcpy(kv.key, s->data, s->len);
  kv.val = dataptr;
  ARR_PUSH(pair_t,m->slots[k],kv);
  m->len++;
}

void* map_get(map_t* m, str_t* s){
  int k = map_hash(s);
  if (m->slots[k].len){
    for (int i = 0; i < m->slots[k].len; i++){
      pair_t p = m->slots[k].data[i];
      if (!memcmp(p.key, s->data, s->len)){
        return p.val;
      }
    }
  }
  return NULL;
}

void* map_overwrite(map_t* m, str_t* s, void* dataptr){
  int k = map_hash(s);
  if (!(m->slots[k].cap)){
    ARR_INIT(pair_t,m->slots[k]);
  }
  if (m->slots[k].len){
    for (int i = 0; i < m->slots[k].len; i++){
      if (!memcmp(m->slots[k].data[i].key, s->data, s->len)){
        void* old = m->slots[k].data[i].val;
        m->slots[k].data[i].val = dataptr;
        return old;
      }
    }
  }
  pair_t kv;
  kv.key = malloc(s->len+1);
  kv.key[s->len]=0;
  memcpy(kv.key, s->data, s->len);
  kv.val = dataptr;
  ARR_PUSH(pair_t,m->slots[k],kv);
  m->len++;
  return NULL;
}

void* map_addr_raw(map_t* m, void* s, int n, int nbytes, int is_lval){
  int k = map_hash_bk(s,n);
  if (m->slots[k].len){
    for (int i = 0; i < m->slots[k].len; i++){
      if (!memcmp( m->slots[k].data[i].key, s, n)){
        return m->slots[k].data[i].val;
      }
    }
  }
  if (!is_lval){
    return NULL;
  }
  if (!(m->slots[k].cap)){
    ARR_INIT(pair_t,m->slots[k]);
  }
  pair_t kv;
  kv.key = malloc(n+1);
  kv.key[n] = 0;
  kv.val = calloc(nbytes,1);
  memcpy(kv.key, s, n);
  ARR_PUSH(pair_t,m->slots[k],kv);
  m->len++;
  return kv.val;
}


void map_nuke(map_t* m){
  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    if (m->slots[k].cap){
      // printf("%d ",m->slots[k].len);
      for (int i = 0; i < m->slots[k].len;i++){
        pair_t p = m->slots[k].data[i];
        free(p.key);
        free(p.val);
      }
      free(m->slots[k].data);
    }
  }
  // printf("\n");
}
