var TO_C = function(cfg){
  let collectible = ["VART_STR","VART_LST","VART_TUP","VART_DIC","VART_STT","VART_FUN","VART_ARR","VART_UON"];

  let vartnummap = {
    'VART_I08':'int8_t',
    'VART_U08':'uint8_t',
    'VART_I16':'int16_t',
    'VART_U16':'uint16_t',
    'VART_I32':'int32_t',
    'VART_U32':'uint32_t',
    'VART_I64':'int64_t',
    'VART_U64':'uint64_t',
    'VART_F32':'float',
    'VART_F64':'double',
  }

  let lib = `
  #include <stdint.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
  #define PRIf32 "f"
  #define PRIf64 "f"
  #define DBG_GC 0
  #define VART_NUL 0
  #define VART_U08 1
  #define VART_I08 2
  #define VART_U16 3
  #define VART_I16 4
  #define VART_U32 5
  #define VART_I32 6
  #define VART_U64 7
  #define VART_I64 8
  #define VART_F32 9
  #define VART_F64 10
  #define VART_STR 11
  #define VART_VEC 12
  #define VART_ARR 13
  #define VART_LST 14
  #define VART_TUP 15
  #define VART_DIC 16
  #define VART_STT 17
  #define VART_FUN 18
  #define VART_UON 19
  #define OP_ADD(a,b) ((a)+(b))
  #define OP_SUB(a,b) ((a)-(b))
  #define OP_MUL(a,b) ((a)*(b))
  #define OP_DIV(a,b) ((a)/(b))
  #define OP_BAND(a,b) ((a)&(b))
  #define OP_BOR(a,b) ((a)|(b))
  #define OP_XOR(a,b) ((a)^(b))
  #define OP_MOD(a,b) ((a)%(b))
  #define OP_MODF(a,b) (fmod(a,b))
  #define OP_POW(a,b)  (pow(a,b))
  #define VOID_T int
  #if _WIN32
  #define __vla(dtype,name,n) dtype* name = (dtype*)_alloca((n)*sizeof(dtype));
  #else
  #define __vla(dtype,name,n) dtype name[n];
  #endif
  typedef struct ret_st {
    void* var;
    int16_t n;
    int16_t vart;
  } ret_t;
  ret_t* __retpts;
  int __retpts_cap = 256;
  int __retpts_idx = 0;
  char* __args;
  int __args_siz = 256;
  int __args_top = 0;
  void __push_retpt(void* var, int vart, int n){
    if (__retpts_idx>=__retpts_cap){
      __retpts_cap = __retpts_cap*2+1;
      __retpts = realloc(__retpts, __retpts_cap*sizeof(ret_t));
    }
    ret_t r;
    r.var = var;
    r.n = n;
    r.vart = vart;
    __retpts[__retpts_idx++] = r;
  }
  void __put_ret(void* addr){
    memcpy(__retpts[__retpts_idx-1].var, addr, __retpts[__retpts_idx-1].n);
  }
  int __peek_ret_type(){
    return __retpts[__retpts_idx-1].vart;
  }
  int __peek_ret_size(){
    return __retpts[__retpts_idx-1].n;
  }
  #define __POP_RETPT --__retpts_idx;
  void __push_arg(void* ptr, char typ, int siz){
    if (__args_top + siz+sizeof(int)+sizeof(char) > __args_siz){
      __args_siz = __args_siz * 2 + siz+sizeof(int)+sizeof(char);
      __args = realloc(__args, __args_siz);
    }
    memcpy(__args + __args_top, ptr, siz);
    memcpy(__args + __args_top+siz, &siz, sizeof(int));
    memcpy(__args + __args_top+siz+sizeof(int), &typ, sizeof(char));
    __args_top += siz+sizeof(int)+sizeof(char);
  }
  int __peek_arg_size(){
    return * ((int*)(__args+(__args_top-sizeof(int)-sizeof(char))));
  }
  int __peek_arg_type(){
    return * ((char*)(__args+(__args_top-sizeof(char))));
  }
  void __pop_arg(void* ptr, int siz){
    memcpy(ptr, __args + (__args_top-siz-sizeof(int)-sizeof(char)), siz);
    __args_top -= siz+sizeof(int)+sizeof(char);
  }
  #define __ARG(nom) nom; __pop_arg(&nom,sizeof(nom));

  typedef struct __mem_node_st {
    struct __mem_node_st *prev;
    struct __mem_node_st *next;
    int64_t flag;
    char data[];
  } __mem_node_t;
  typedef struct __mem_list_st {
    __mem_node_t *head;
    __mem_node_t *tail;
    int len;
  } __mem_list_t;

  __mem_list_t __gc_list = {0};

  void* __gc_alloc(char flag, int sz){
    __mem_node_t* n = (__mem_node_t*)calloc( sizeof(__mem_node_t)+sz, 1);
    n->next = NULL;
    n->prev = __gc_list.tail;
    n->flag = flag;
    if (__gc_list.head == NULL){
      __gc_list.head = n;
    }else{
      __gc_list.tail->next = n;
    }
    __gc_list.tail = n;
    __gc_list.len ++;
    if(DBG_GC)printf("alloc %d %p sz:%d\\n",(char)n->flag,n->data,sz);
    return n->data;
  }
  void* __gc_realloc(void* ptr, int sz){
    __mem_node_t* node = (__mem_node_t*)((char*)ptr - sizeof(__mem_node_t));
    int h = (__gc_list.head == node);
    int t = (__gc_list.tail == node);
    node = realloc(node,sz+sizeof(__mem_node_t));
    if (node->prev) node->prev->next = node;
    if (node->next) node->next->prev = node;
    if (t) __gc_list.tail = node;
    if (h) __gc_list.head = node;
    return node->data;
  }
  void __gc_free(void* ptr){
    __mem_node_t* node = (__mem_node_t*)((char*)ptr - sizeof(__mem_node_t));
    if (node == __gc_list.tail){
      if (node == __gc_list.head){
        __gc_list.head = NULL;
        __gc_list.tail = NULL;
        goto cleanup;
      }else{
        __gc_list.tail->prev->next = NULL;
        __gc_list.tail = __gc_list.tail->prev;
        goto cleanup;
      }
    }else if (node == __gc_list.head){
      __gc_list.head->next->prev = NULL;
      __gc_list.head = __gc_list.head->next;
      goto cleanup;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
  cleanup:
    __gc_list.len--;
    if (node) free(node);
  }
  typedef struct{
    int cap;
    int n;
    int w;
    int t;
    char* data;
  } __list_t;

  typedef struct{
    int n;
    int w;
    int t;
    int ndim;
    char* data;
    int dims[];
  } __arr_t;

  typedef struct{
    int32_t sel;
    int16_t w;
    int16_t t;
    char data[];
  } __union_t;

  #define __NUM_DICT_SLOTS 64

  typedef struct{
    int cap;
    int n;
    char* data;
  } __dict_slot_t;

  typedef struct{
    int n;
    int kt;
    int vt;
    int kw;
    int vw;
    __dict_slot_t slots[__NUM_DICT_SLOTS];
  } __dict_t;


  typedef struct{
    void (*funptr)(void);
    void* captr;
    int cnt;
    int siz;
  } __func_t;

  uintptr_t* __vars;
  int __vars_cap = 256;
  int __vars_top = 0;
  int __vars_stack = 0;

  int* __stack;
  int __stack_cap = 256;
  int __stack_top = 0;


  void __push_stack(){
    if (DBG_GC) printf("push stack\\n");
    __vars_stack = __vars_top;
    if (__stack_top+1>=__stack_cap){
      __stack_cap = (__stack_cap)*2+1;
      __stack = realloc(__stack,__stack_cap);
    }
    __stack[__stack_top++] = __vars_stack;
  }
  void __pop_stack(){
    if (DBG_GC) printf("pop stack\\n");
    __vars_top = __vars_stack;
    __stack_top--;
    __vars_stack = __stack[__stack_top-1];
  }
  void __init_g(){
    __retpts = malloc(__retpts_cap*sizeof(ret_t));
    __args = malloc(__args_siz);
    __vars = malloc(__vars_cap*sizeof(uintptr_t));
    __stack = malloc(__stack_cap*sizeof(int));
  }
  void __gc_mark(void* ptr){
    if (ptr == NULL) return;
    __mem_node_t* node = (__mem_node_t*) ((char*)ptr-sizeof(__mem_node_t));
    if (node->flag & 0x80){
      return;
    }
    char vt = node->flag;
    node->flag |= 0x80;
    if(DBG_GC)printf("mark %p\\n",ptr);
    if (vt == VART_LST){
      __list_t* lst = (__list_t*) ptr;
      if (${collectible.map(x=>"lst->t=="+x).join('||')}){
        for (int i = 0; i < lst->n; i++){
          void* ptr = ((void**)(lst->data + lst->w * i))[0];
          __gc_mark( ptr );
        }
      }
    }else if (vt == VART_ARR){
      __arr_t* arr = (__arr_t*) ptr;
      if (${collectible.map(x=>"arr->t=="+x).join('||')}){
        for (int i = 0; i < arr->n; i++){
          void* ptr = ((void**)(arr->data + arr->w * i))[0];
          __gc_mark( ptr );
        }
      }
    }else if (vt == VART_STT){
      int n = ((int*)ptr)[0];
      for (int i = 0; i < n; i++){
        int ofs = ((int*)ptr)[i+1];
        __gc_mark( *((void**)((char*)ptr + ofs)) );
      }
    }else if (vt == VART_TUP){
      int i = 0;
      while (1){
        char typ = ((char*)ptr + i*5)[0];
        if (typ == 0) break;
        if (${collectible.map(x=>"typ=="+x).join('||')}){
          int ofs = ((int*)((char*)ptr + i*5 +1))[0];
          __gc_mark( *(void**)((char*)ptr + ofs));
        }
        i++;
      }
    }else if (vt == VART_DIC){
      __dict_t* dic = (__dict_t*) ptr;
      if (${collectible.map(x=>"dic->kt=="+x).join('||')}){
        for (int i = 0; i < __NUM_DICT_SLOTS; i++){
          for (int j = 0; j < dic->slots[i].n; j++){
            __gc_mark( *(void**)((char*)(dic->slots[i].data) + (dic->kw+dic->vw) * j) );
          }
        }
      }
      if (${collectible.map(x=>"dic->vt=="+x).join('||')}){
        for (int i = 0; i < __NUM_DICT_SLOTS; i++){
          for (int j = 0; j < dic->slots[i].n; j++){
            __gc_mark( *(void**)((char*)(dic->slots[i].data) + (dic->kw+dic->vw) * j + dic->kw) );
          }
        }
      }
    }else if (vt == VART_FUN){
      __func_t* fun = (__func_t*) ptr;
      char* top = (char*)(fun->captr) + fun->siz;
      while ((void*)top > fun->captr){
        char vvt = *(top-=1);
        int sz;
        memcpy(&sz, (top-=4), 4);
        void* p = (top-=sz);
        if (${collectible.map(x=>"vvt=="+x).join('||')}){
          __gc_mark(p);
        }
      }
    }else if (vt == VART_UON){
      __union_t* uon = (__union_t*) ptr;
      if (${collectible.map(x=>"uon->t=="+x).join('||')}){
        __gc_mark(*(void**)(uon->data));
      }
    }
  }
  int __gc_off = 0;
  void __gc_run(){
    if (__gc_off) return;
    if (DBG_GC) {printf("stack "); for (int i = 0; i < __vars_top; i++) printf("%p %d ",(char*)__vars[i], (char)(((__mem_node_t*)(((char*)__vars[i])-sizeof(__mem_node_t)))->flag )); printf("\\n");}
    for (int i = 0; i < __vars_top; i++){
      __gc_mark((void*)(__vars[i]));
    }
    __mem_node_t* node = __gc_list.head;
    __mem_node_t* next;
    while (node){
      next = node->next;
      if (node->flag & 0x80){
        node->flag &= ~0x80;
      }else{
        void* ptr = node->data;
        if(DBG_GC)printf("free %p\\n",ptr);
        if (node->flag == VART_LST){
          free( ((__list_t*)ptr)->data );
        }else if (node->flag == VART_ARR){
          free( ((__arr_t*)ptr)->data );
        }else if (node->flag == VART_DIC){
          __dict_t* dic = (__dict_t*)ptr;
          for (int i = 0; i < __NUM_DICT_SLOTS; i++){
            if (dic->slots[i].cap) free(dic->slots[i].data);
          }
        }
        __gc_free((void*)ptr);
      }
      node = next;
    }
  }
  void __put_var(int idx, void* ptr){
    if (DBG_GC) printf("put %d %p %d\\n", idx, ptr, (char)(((__mem_node_t*)((char*)ptr-sizeof(__mem_node_t)))->flag));
    if (__vars_stack+idx+1>__vars_cap){
      __vars = realloc(__vars,(__vars_stack+idx+1)*2);
    }
    __vars[__vars_stack+idx] = (uintptr_t)ptr;
    if (__vars_stack+idx+1 > __vars_top){
      __vars_top = __vars_stack+idx+1;
    }
  }
  char* __to_str(void* ptr, int vart, int w){
    char* o;
    if (*(void**)ptr == NULL && (vart > VART_VEC || vart == VART_STR)){
      o = malloc(5);
      strcpy(o,"null");
    }else if (vart == VART_LST){
      
      __list_t* lst = *(__list_t**)ptr;
      o = calloc(3,1);
      o[0] = '{';
      for (int i = 0; i < lst->n; i++){
        char* a = __to_str(lst->data + (i*lst->w), lst->t, lst->w);
        int no = strlen(o);
        int na = strlen(a);
        o = realloc(o, no+na+2);
        strcpy(o+no, a);
        o[no+na] = (i == lst->n-1) ? '}' : ',';
        o[no+na+1] = 0;
        free(a);
      }
      if (!lst->n) o[1] = '}';
    }else if (vart == VART_ARR){
      __arr_t* arr = *(__arr_t**)ptr;
      o = calloc(3,1);
      o[0] = '[';
      for (int i = 0; i < arr->ndim; i++){
        char* a = __to_str(arr->dims + i, VART_I32, 4);
        int no = strlen(o);
        int na = strlen(a);
        o = realloc(o, no+na+3);
        strcpy(o+no, a);
        if (i == arr->ndim-1){
          o[no+na] = ']';
          o[no+na+1] = '{';
          o[no+na+2] = 0;
        }else{
          o[no+na] = 'x';
          o[no+na+1] = 0;
        }
        free(a);
      }
      for (int i = 0; i < arr->n; i++){
        char* a = __to_str(arr->data + (i*arr->w), arr->t, arr->w);
        int no = strlen(o);
        int na = strlen(a);
        o = realloc(o, no+na+2);
        strcpy(o+no, a);
        o[no+na] = (i == arr->n-1) ? '}' : ',';
        o[no+na+1] = 0;
        free(a);
      }
      if (!arr->n) o[1] = '}';
    }else if (vart == VART_TUP) {
      char* tup = *(char**)ptr;
      o = calloc(2,1);
      o[0] = '[';
      int i = 0;
      while (1){
        char typ = ((char*)(tup + i*5))[0];
        if (typ == 0) break;
        int ofs = ((int*)(tup + i*5 +1))[0];
        int nofs = ((int*)(tup + (i+1)*5 +1))[0];

        char* a = __to_str(tup + ofs, typ, nofs-ofs);
        int no = strlen(o);
        int na = strlen(a);
        o = realloc(o, no+na+2);
        strcpy(o+no, a);
        o[no+na] = ',';
        o[no+na+1] = 0;
        free(a);
        i++;
      }
      o[strlen(o)-1] = ']';
    }else if (vart == VART_DIC) {
      __dict_t* dic = *(__dict_t**)ptr;
      o = calloc(3,1);
      o[0] = '{';
      for (int i = 0; i < __NUM_DICT_SLOTS; i++){
        for (int j = 0; j < dic->slots[i].n; j++){
          char* a = __to_str( (void**)(dic->slots[i].data + (dic->kw+dic->vw) * j), dic->kt, dic->kw);
          int no = strlen(o);
          int na = strlen(a);
          o = realloc(o, no+na+2);
          strcpy(o+no, a);
          o[no+na] = ':';
          o[no+na+1] = 0;
          free(a);

          a = __to_str(dic->slots[i].data + (dic->kw+dic->vw) * j + dic->kw, dic->vt, dic->vw);
          no = strlen(o);
          na = strlen(a);
          o = realloc(o, no+na+2);
          strcpy(o+no, a);
          o[no+na] = ',';
          o[no+na+1] = 0;
          free(a);
        }
      }
      if (o[1] == 0){
        o[1] = '}';
      }else{
        o[strlen(o)-1] = '}';
      }
    }else if (vart == VART_UON) {
      __union_t* uon = *(__union_t**)ptr;
      o = __to_str( &(uon->data), uon->t, uon->w);
    }else if (vart == VART_STR) {
      o = strdup(*(char**)ptr);
    }else if (vart == VART_STT) {
      o = malloc(32);
      sprintf(o,"[object@%p]",ptr);
    }${Object.keys(vartnummap).map(vart=>`
      else if (vart == ${vart}){
        int dw = ${Number(vart.slice(-2))/8};
        int n = w/dw;
        if (n == 1){
          o = malloc(32);
          sprintf(o, "%" PRI${{F:'f',U:'u',I:'d'}[vart.at(-3)]}${Number(vart.slice(-2))}, (*(${vartnummap[vart]}*)ptr) );
        }else{
          o = calloc(2,1);
          o[0] = '{';
          for (int i = 0; i < n; i++){
            char* a = __to_str((char*)ptr + (i*dw), ${vart}, dw);
            int no = strlen(o);
            int na = strlen(a);
            o = realloc(o, no+na+2);
            strcpy(o+no, a);
            o[no+na] = (i == n-1) ? '}' : ',';
            o[no+na+1] = 0;
            free(a);
          }
        }
      }`).join("")}
    return o;
  }
  int __hash(void* x, int n){
    uint32_t k = 0;
    for (int i = 0; i < n; i++){
      k ^= ((char*)x)[i];
    }
    return k % __NUM_DICT_SLOTS;
  }
  void* __dict_get(__dict_t* dic, void* idx){
    int s;
    if (dic->kt == VART_STR){
      s = __hash(*(char**)idx, strlen(*(char**)idx));
    }else{
      s = __hash(idx, dic->kw);
    }
    if (! dic->slots[s].cap){
      dic->slots[s].cap = 4;
      dic->slots[s].n = 0;
      dic->slots[s].data = malloc( (dic->kw + dic->vw)*dic->slots[s].cap );
    }
    for (int i = 0; i < dic->slots[s].n; i++){
      void* k = dic->slots[s].data + ((dic->kw+dic->vw)*i);
      int ok = 0;
      if (dic->kt == VART_STR){
        ok = strcmp(* ((char**)k), * ((char**)idx)) == 0;
      }else{
        ok = memcmp(k,idx,dic->kw) == 0;
      }
      if (ok){
        return (char*)k + dic->kw;
      }
    }
    if (dic->slots[s].n >= dic->slots[s].cap){
      dic->slots[s].cap = (dic->slots[s].cap)*2+1;
      dic->slots[s].data = realloc(dic->slots[s].data, (dic->kw + dic->vw)*dic->slots[s].cap);
    }
    void* k = dic->slots[s].data + (dic->kw+dic->vw) * dic->slots[s].n;
    memcpy( k,  idx, dic->kw);
    memset( (char*)k + dic->kw, 0, dic->vw);
    dic->slots[s].n++;
    dic->n++;
    return (char*)k + dic->kw;
  }
  `
  let intmap = {
    'i8':'int8_t',
    'u8':'uint8_t',
    'i16':'int16_t',
    'u16':'uint16_t',
    'i32':'int32_t',
    'u32':'uint32_t',
    'i64':'int64_t',
    'u64':'uint64_t',
  }
  let intpam = Object.fromEntries(Object.entries(intmap).map(x=>[x[1],x[0]]))

  let nummap = {
    ...intmap,
    'f32':'float',
    'f64':'double',
  }
  let numpam = Object.fromEntries(Object.entries(nummap).map(x=>[x[1],x[0]]))

  let typmap = {
    ...nummap,
    'void':'VOID_T',
    'str':'char*',
  }

  let typpam = Object.fromEntries(Object.entries(typmap).map(x=>[x[1],x[0]]))

  function UNIMPL(){
    console.error("UNIMPLEMENTED");
    console.trace();
    process.exit();
  }


  function parse_layout(ls){
    let lo = {};
    let ca = "";
    for (let i = 0; i < ls.length; i++){
      if (ls[i][0] == '\t'){
        let vs = ls[i].trim().split('\t');
        vs[0] = Number(vs[0]);
        vs[1] = clean(vs[1]);
        vs[2] = read_type(vs[2]);
        lo[ca].fields.push(vs);
      }else{
        let [a,b] = ls[i].trim().split("\t");

        lo[ca = read_type(a)] = {
          size: Number(b),
          fields:[],
        }
      }
    }
    for (let ca in lo){
      let nf = [];
      for (let i = 1; i < lo[ca].fields.length; i++){
        let vt = vart(lo[ca].fields[i][2]);
        if (collectible.includes(vt)){
          nf.push(lo[ca].fields[i][0]);
        }
      }
      lo[ca].collect = nf;
    }
    return lo;
  }

  function parse_ir(txt){
    
    let ls = txt.split("\n").filter(x=>x.trim().length);
    let o = [];
    let lo = {};
    for (let i = 0; i < ls.length; i++){
      if (ls[i].trim() == "eoir"){
        lo = parse_layout(ls.slice(i+1));
        break;
      }
      ls[i] = ls[i].trim();
      let lbl = "";
      let ln = ls[i];
      if (ls[i].includes(':')){
        let q = ls[i].split(':')
        if (!q[0].includes('"')){
          lbl = q[0];
          ln = q.splice(1).join(':');
        }
      }
      let lx = [];
      let ac = "";
      let st = 0;
      for (let j = 0; j < ln.length; j++){
        if (st == 0 && ln[j] == " "){
          lx.push(ac);
          ac = "";
        }else if (ln[j] == '"'){
          if (st == 0){
            st = 1;
          }else if (st == 1){
            st = 0;
          }else{
            ac += '\\'
            st = 1;
          }
          ac += ln[j];
        }else if (st && ln[j] == '\\'){
          if (st == 1){
            st = 2;
          }else{
            ac += '\\\\'
            st = 1;
          }
        }else{
          if (st == 2){
            ac += '\\'
            st = 1;
          }
          ac += ln[j];
        }
      }
      lx.push(ac);
      
      lx = lx.map(x=>x.trim()).filter(x=>x.length);
      // console.log(ln,lx)
      o.push([lbl,lx])
    }
    // console.log(o);
    return [o,lo];
  }
  function proc_type(o){
    function proc(x){
      let y = typmap;
      if (y[x]) return y[x];
      return clean(x);
    }
    function flat(o){
      if (o.con){
        if (o.con.includes('.')){
          return `${o.con}[${o.elt.map(flat).join(',')}]`;
        }else{
          return {con:o.con,elt:o.elt.map(flat)}
        }
      }
      return o;
    }
    function doit(o){
      if (o.con){
        return {con:proc(o.con),elt:o.elt.map(doit)}
      }
      return proc(o);
    }
    return doit(flat(o));
  }
  function read_type(s){
    let acc = "";
    let cstk = [];
    let cptr = [];
    cstk.push(cptr);

    for (let i = 0; i < s.length; i++){
      if (s[i] == '['){
        cptr.push({con:acc,elt:[]});
        acc = "";
        cptr = cptr.at(-1).elt;
        cstk.push(cptr);
      }else if (s[i] == ']'){
        if (acc.length){
          cptr.push(acc);
          acc = "";
        }
        cstk.pop();
        cptr = cstk.at(-1);
      }else if (s[i] == ','){
        if (acc.length){
          cptr.push(acc);
          acc = "";
        }
      }else{
        acc += s[i];
      }
    }
    if (acc.length){
      o = acc;
    }else{
      o = cstk[0][0];
    }
    o = proc_type(o);
    return o;
  }


  function maybenum(x){
    let re = /^(?:0[xX][0-9A-Fa-f]+|[+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)$/;
    if (re.test(x)){
      let y = parseFloat(x);
      if (y==0){
        return parseInt(x);
      }
      return y;
    }
    return x;
  }
  function clean(x){
    if (x[0] == '"') return x;
    x = maybenum(x);
    if (typeof x == 'string'){
      return x
        .replace(/\[/g,'__L_')
        .replace(/\]/g,'__7_')
        .replace(/\,/g,'__9_')
        .replace(/\./g,'__o_')
        .replace(/^(\d)/, '__N_$1')
    }
    return x;
  }
  function shortid(){
    var id = "";
    for (var i = 0; i < 6; i++){
      id+=String.fromCharCode(~~(Math.random()*26)+0x41);
    }
    return id;
  }
  function type_size(x){
    // console.log(x,typpam[x],typmap[x])
    if (typpam[x]){
      return `sizeof(${x})`
    }else if (typmap[x]){
      return `sizeof(${typmap[x]})`
    }else if (x.con == 'vec'){
      return `(sizeof(${x.elt[0]})*${x.elt.slice(1).join('*')})`
    }
    return 8;
  }
  function vart(x){
    let y = x.con??x;
    let v = typpam[y]??y;
    if (v == 'vec'){
      v = typpam[x.elt[0]];
    }
    let z = {
      void:"VART_NUL",
      u8:"VART_U08",
      i8:"VART_I08",
      u16:"VART_U16",
      i16:"VART_I16",
      u32:"VART_U32",
      i32:"VART_I32",
      u64:"VART_U64",
      i64:"VART_I64",
      f32:"VART_F32",
      f64:"VART_F64",
      str:"VART_STR",
      vec:"VART_VEC",
      arr:"VART_ARR",
      list:"VART_LST",
      tup:"VART_TUP",
      dict:"VART_DIC",
      fun:"VART_FUN",
      union:"VART_UON",
    }[v] ?? "VART_STT";
    return z;
  }

  function transpile_c(instrs,layout){
    let varcnt = 0;
    let o = [];
    let lookup = {};
    let cflags = [];

    function vec_type_flat_n(tb){
      return eval(tb.elt.slice(1).join('*'))
    }
    function cast(a,b,ins){
      let ta = lookup[a];
      let tb = lookup[b];
      // console.log(a,b,ta,tb)
      if (ta == 'char*' && intpam[tb]){
        o.push(`${a}=__gc_alloc(VART_STR,32);`);
        o.push(`__put_var(${varcnt++},${a});`);
        if (tb[0] == 'u'){
          o.push(`sprintf(${a},"%" PRIu64,(uint64_t)${b});`);
        }else{
          o.push(`sprintf(${a},"%" PRId64,(int64_t)${b});`);
        }
      }else if (ta == 'char*' && (tb == 'float' || tb == 'double')){
        o.push(`${a}=__gc_alloc(VART_STR,64);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`sprintf(${a},"%f",(double)${b});`);
      }else if (ta == 'VOID_T'){
        o.push(`${a}=0;`);
      }else if (ta == 'char*' && tb.con == 'vec'){
        let df;
        let cst = "";
        if (intpam[tb.elt[0]]){
          if (tb.elt[0] == 'u'){
            df = `"%" PRIu64 `;
            cst = `(uint64_t)`
          }else{
            df = `"%" PRId64 `
            cst = `(int64_t)`
          }
        }else if (tb.elt[0] == 'float' || tb.elt[0] == 'double'){
          df = `"%f" `;
          cst = `(double)`
        }
        let n = vec_type_flat_n(tb);
        df = new Array(n).fill(df).join(` "," `);
        let ar = new Array(n).fill(0).map((x,i)=>`${cst}${b}[${i}]`).join(",");
        o.push(`${a}=__gc_alloc(VART_STR,snprintf(NULL, 0, "{" ${df} "}", ${ar})+1);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`sprintf(${a}, "{" ${df} "}", ${ar});`);
      }else if (numpam[ta] && typeof b == 'number'){
        o.push(`${a} = ${b};`)
      }else if (numpam[ta] && numpam[tb]){
        o.push(`${a} = ${b};`);
      }else if (ta.con == 'vec' && (numpam[tb] || typeof b == 'number')){
        o.push(`{for (int i = 0; i < ${ta.elt.slice(1).join('*')}; i++){`);
        o.push(`${a}[i] = ${b};`);
        o.push(`}}`);
      }else if (ta == "char*" && tb.con == 'list'){
        let tmp = shortid();
        o.push(`char* ${tmp} = __to_str(&(${b}), VART_LST, 8);`);
        o.push(`${a} = __gc_alloc(VART_STR, strlen(${tmp})+1);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`strcpy(${a},${tmp});`);
        o.push(`free(${tmp});`);
      }else if (ta == "char*" && tb.con == 'arr'){
        let tmp = shortid();
        o.push(`char* ${tmp} = __to_str(&(${b}), VART_ARR, 8);`);
        o.push(`${a} = __gc_alloc(VART_STR, strlen(${tmp})+1);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`strcpy(${a},${tmp});`);
        o.push(`free(${tmp});`);
      }else if (ta == "char*" && tb.con == 'tup'){
        let tmp = shortid();
        o.push(`char* ${tmp} = __to_str(&(${b}), VART_TUP, 8);`);
        o.push(`${a} = __gc_alloc(VART_STR, strlen(${tmp})+1);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`strcpy(${a},${tmp});`);
        o.push(`free(${tmp});`);
      }else if (ta == "char*" && tb.con == 'dict'){
        let tmp = shortid();
        o.push(`char* ${tmp} = __to_str(&(${b}), VART_DIC, 8);`);
        o.push(`${a} = __gc_alloc(VART_STR, strlen(${tmp})+1);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`strcpy(${a},${tmp});`);
        o.push(`free(${tmp});`);
      }else if (ta == "char*" && tb.con == "union"){
        let tmp = shortid();
        o.push(`char* ${tmp} = __to_str(&(${b}), VART_UON, 8);`);
        o.push(`${a} = __gc_alloc(VART_STR, strlen(${tmp})+1);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`strcpy(${a},${tmp});`);
        o.push(`free(${tmp});`);
      }else if (ta == "char*" && vart(tb)=="VART_STT"){
        let tmp = shortid();
        let tmp1 = shortid();
        let no = shortid();
        let na = shortid();
        o.push(`char* ${tmp} = calloc(1,2); ${tmp}[0] = '{';`);
        o.push(`if (${b}){`);
        o.push(`char* ${tmp1}; int ${no},${na};`);
        let lo = layout[tb];
        for (let i = 0; i < lo.fields.length; i++){
          let ofs = lo.fields[i][0];
          let typ = lo.fields[i][2];
          let last = (i==lo.fields.length-1);
          let ds = (last ? lo.size : lo.fields[i+1][0])-ofs;
          ofs += lo.collect.length*4+4;
          o.push(`\
            ${tmp1} = __to_str((char*)${b} + ${ofs}, ${vart(typ)}, ${ds});
            ${no} = strlen(${tmp});
            ${na} = strlen(${tmp1});
            ${tmp} = realloc(${tmp}, ${no}+${na}+${lo.fields[i][1].length+3});
            strcpy(${tmp}+${no}, "${lo.fields[i][1]}");
            strcpy(${tmp}+${no}+${lo.fields[i][1].length+1}, ${tmp1});
            ${tmp}[${no}+${lo.fields[i][1].length}]=':';
            ${tmp}[${no}+${lo.fields[i][1].length+1}+${na}] = '${last?"}":","}';
            ${tmp}[${no}+${lo.fields[i][1].length+2}+${na}] = 0;\
          `)
        };
        o.push(`}else{`);
        o.push(`${tmp} = realloc(${tmp},5); strcpy(${tmp},"null");`);
        o.push('}');
        o.push(`${a} = __gc_alloc(VART_STR, strlen(${tmp})+1);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`strcpy(${a},${tmp});`);
        o.push(`free(${tmp});`);
      }else if (ta.con == 'union'){
        if (b[0] == '"'){
          o.push(`${a}->sel = ${ta.elt.indexOf("char*")};`);
          o.push(`${a}->t = VART_STR;`);
          o.push(`${a}->w = 8;`);
          o.push(`* ((char**) (${a}->data)) = __gc_alloc(VART_STR, ${b.length-1});`);
          o.push(`strcpy(  *((char**) (${a}->data)), ${b}  );`);
        }else if (typeof b == 'number'){
          if (ins[2].includes('.')){
            o.push(`${a}->sel = ${ta.elt.indexOf("float")};`);
            o.push(`${a}->t = VART_F32;`);
            o.push(`${a}->w = 4;`);
            o.push(`* ((float*) (${a}->data)) = ${b};`);
          }else{
            o.push(`${a}->sel = ${ta.elt.indexOf("int32_t")};`);
            o.push(`${a}->t = VART_I32;`);
            o.push(`${a}->w = 4;`);
            o.push(`* ((int32_t*) (${a}->data)) = ${b};`);
          }
        }else if (tb == 'char*'){
          o.push(`${a}->sel = ${ta.elt.indexOf("char*")};`);
          o.push(`${a}->t = VART_STR;`);
          o.push(`${a}->w = 8;`);
          o.push(`* ((char**) (${a}->data)) = __gc_alloc(VART_STR, strlen(${b})+1);`);
          o.push(`strcpy(  *((char**) (${a}->data)), ${b}  );`);
        }else{
          o.push(`${a}->sel = ${ ta.elt.map(q=>JSON.stringify(q)).indexOf(JSON.stringify(tb)) };`);
          o.push(`${a}->t = ${vart(tb)};`);
          o.push(`${a}->w = ${type_size(tb)};`);
          o.push(`memcpy( (void*) (${a}->data),  &${b}, 8);`);
        }
      }else if (tb.con == 'union'){
        o.push(`memcpy( &(${a}), &(${b}->data), ${type_size(ta)} );`);
      }else if (ta.con == 'vec' && tb.con == 'vec'){
        for (let i = 0; i < vec_type_flat_n(ta); i++){
          o.push(`${a}[${i}] = ${b}[${i}];`);
        }
      }else if (ta == 'char*' && tb == 'VOID_T'){
        o.push(`${a}=__gc_alloc(VART_STR,5);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`sprintf(${a},"null");`);
      }else if (intpam[ta] && tb == 'char*'){
        o.push(`${a} = atoll(${b});`);
      }else if (numpam[ta] && tb == 'char*'){
        o.push(`${a} = atof(${b});`);
      }else{
        console.log(a,b,ta,tb);
        UNIMPL();
      }
    }

    function math(op,a,b,c){

      let os = "OP_"+op.toUpperCase();

      let typ = lookup[a];
      if (typ.con == "vec"){
        if (op == "mod" && (typ.elt[0] == 'float' || typ.elt[0] == 'double')){
          os+="F";
        }
        o.push(`{for (int i = 0; i < ${typ.elt.slice(1).join('*')}; i++){`);
        o.push(`${a}[i] = ${os}(${b}[i],${c}[i]);`);
        o.push(`}}`);
      }else if (typ == "char*"){
        let bb = shortid();
        let cc = shortid();
        o.push(`char* ${bb} = ${b}; char* ${cc} = ${c};`);
        o.push(`${a} = __gc_alloc(VART_STR,strlen(${bb})+strlen(${cc})+1);`);
        o.push(`__put_var(${varcnt++},${a});`);
        o.push(`strcpy(${a},${bb});`);
        o.push(`strcpy(${a}+strlen(${bb}),${cc});`);
      }else{
        if (op == "mod" && (typ == 'float' || typ == 'double')){
          os+="F";
        }
        if (op == "div" && (typ == 'float' || typ == 'double')){
          o.push(`${a} = ${os}((${typ})${b},(${typ})${c});`);
        }else{
          // o.push(`printf("%f ${os} %f\\n",(float)${b},(float)${c});`);
          o.push(`${a} = ${os}(${b},${c});`);
        }
      }
    }

    function compare(op,a,b,c){
      let os = {
        leq:'<=',geq:'>=',lt:'<',gt:'>',eq:'==',neq:'!='
      }[op];
      let t = lookup[b]??lookup[c];
      if (!t){
        o.push(`${a} = ${b} ${os} ${c};`);
      }else if (t.con == "vec"){
        let s = [];
        let n = vec_type_flat_n(t);
        for (let i = 0; i < n; i++){
          s.push(`${b}[${i}]==${c}[${i}]`);
        }
        o.push(`${a} = (${s.join('&&')})${os}1;`);
      }else if (t == "char*"){
        o.push(`${a} = strcmp(${b},${c})${os}0;`);
      }else{
        o.push(`${a} = ${b} ${os} ${c};`);
      }
    }

    function tup_size(t,idx){
      let nc = '5';
      for (let i = 0; i < t.elt.length; i++){
        nc += '+5';
        if (i < idx){
          nc += '+'+type_size(t.elt[i]);
        }
      }
      return nc;
    }

    function get_ptr(x,n){
      if (x.includes('+')){
        let [v,idx] = x.split('+');
        let t = lookup[v];
        if (t.con == 'vec'){
          return [`((${v}) + (${idx}))`, t.elt[0], type_size(t.elt[0])]
        }else if (t.con == 'list'){
          let [tt,tn] = writable_type(t.elt[0]);
          return [`(((${tt}*)((${v})->data)) + (${idx}*${tn}))`, t.elt[0], type_size(t.elt[0])]
          
        }else if (t.con == 'arr'){
          let [tt,tn] = writable_type(t.elt[0]);
          if (lookup[idx] && lookup[idx].con == 'vec'){
            let ii = "0";
            let stride = "1";
            for (let i = t.elt[1]-1; i >= 0; i--){
              ii += `+(${idx}[${i}] * ${stride})`;
              stride += `*(${v}->dims[${i}])`;
            }
            idx = `(${ii})`;
          }
          return [`(((${tt}*)((${v})->data)) + (${idx}*${tn}))`, t.elt[0], type_size(t.elt[0])]
          
          
        }else if (t.con == 'dict'){
          idx = maybenum(idx);
          if (typeof idx == 'number'){
            let tmp = shortid();
            o.push(`${t.elt[0]} ${tmp} = ${idx};`);
            idx = tmp;
          }else if (idx[0] == '"'){
            let tmp = shortid();
            o.push(`${t.elt[0]} ${tmp} = __gc_alloc(VART_STR,${idx.length-1});`);
            o.push(`strcpy(${tmp},${idx});`);
            idx = tmp;
          }
          return [`__dict_get(${v}, &(${idx}))`, t.elt[1], type_size(t.elt[1])];
        }else if (t.con == 'tup'){
          idx = Number(idx);
          let nc = tup_size(t, idx);

          return [`(char**) ((((char*)${v}) + (${nc})))`, t.elt[idx], type_size(t.elt[idx])]
          
        }else if (t == 'char*'){
          return [`((char*)${v}+${idx})`, 'char', 1];
        }else if (typeof t == 'string'){
          let lo = layout[t];
          let ofs,typ;
          for (let i = 0; i < lo.fields.length; i++){
            if (lo.fields[i][1] == idx){
              ofs = lo.fields[i][0];
              typ = lo.fields[i][2];
              break;
            }
          }
          ofs += lo.collect.length*4+4;
          let [tt,tn] = writable_type(typ);
          return [`(${tt}*)((char*)${v} + ${ofs}) `, typ, type_size(typ)];
        }else{
          // console.log(v,t,idx)
          UNIMPL();
        }
      }else{
        let t = lookup[x];
        if (t == 'char*'){
          return [`&(${x})`, 'char*', `8`];
        }else{
          let t = lookup[x];
          return [`&(${x})`, t, type_size(t)];
        }
        
      }
    }
    function writable_type(typ){
      if (typ.con == 'list'){
        return [`__list_t*`,1];
      }else if (typ.con == 'arr'){
        return [`__arr_t*`,1];
      }else if (typ.con == 'dict'){
        return [`__dict_t*`,1];
      }else if (typ.con == 'vec'){
        return [typ.elt[0],vec_type_flat_n(typ)];
      }else if (typ.con == 'func'){
        return [`__func_t*`,1];
      }else if (typ.con == 'tup'){
        return [`void*`,1];
      }else if (typ.con == 'union'){
        return [`__union_t*`,1];
      }else{
        return [typ,1];
      }
    }
    function write_decl(typ,nom){
      if (typ.con == 'list'){
        o.push(`__list_t* ${nom} = 0;`)
      }else if (typ.con == 'arr'){
        o.push(`__arr_t* ${nom} = 0;`)
      }else if (typ.con == 'dict'){
        o.push(`__dict_t* ${nom} = 0;`)
      }else if (typ.con == 'vec'){
        o.push(`${typ.elt[0]} ${nom}[${typ.elt.slice(1).join('*')}];`)
      }else if (typ.con == 'tup'){
        o.push(`void* ${nom};`);
      }else if (typ.con == 'func'){
        o.push(`__func_t* ${nom};`);
      }else if (typ.con == 'union'){
        o.push(`__union_t* ${nom};`);
      }else{
        o.push(`${typ} ${nom} = 0;`);
      }
    }

    let funcs = {};
    let typds = {};
    let curfun;
    let curtypd;
    let liftdecl = [];
    let inmain = 0;
    for (let i = 1; i < instrs.length; i++){
      let [lbl, ins] = instrs[i];
      if (lbl.length){
        if (lbl == '__main__'){
          inmain = 1;
        }else if (lbl.startsWith("__func_ovld_")){
          let funname = clean(lbl);
          funcs[funname] = curfun = {dcap:[]};
          o.push(`void ${funname}();`);
        }else if (lbl.startsWith("__typd_")){
          typds[clean(lbl)] = curtypd = {instrs:[]};
          o.push(`#define ${clean(lbl.slice(7))} void*`);
          inmain = -inmain;
        }else if (lbl.startsWith("end__typd_")){
          curtypd = null;
          inmain = -inmain;
        }else{
          
        }
      }else{
      }
      if (curtypd && ins.length){
        curtypd.instrs.push(ins);
        instrs[i][1] = ["nop"];
      }else if (ins[0] == 'dcap'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        curfun.dcap.push({nom,typ});
      }else if (ins[0] == 'decl' && inmain == 1 && ins[1][0] != "_"){
        liftdecl.push(["",ins]);
        instrs[i][1] = ["nop"];
      }
    }
    instrs.shift();
    instrs = liftdecl.concat(instrs);
    
    let tmpdefs = [];
    let infun = 0;
    inmain = 0;
    
    for (let i = 0; i < instrs.length; i++){
      let [lbl, ins] = instrs[i];
      
      // console.log(lbl,ins)
      if (lbl.length){
        if (lbl == '__main__'){
          for (let j = 0; j < tmpdefs.length; j++){
            o.push(`#undef ${tmpdefs[j]}`);
          }
          tmpdefs.splice(0,Infinity);

          varcnt = 0;
          if (infun){
            o.push(`}`)
          }
          o.push(`int main(){`);
          o.push(`__init_g();`);
          o.push(`__push_stack();`);
          infun = 0;
          inmain = 1;
        }else if (lbl.startsWith("__func_ovld_")){
          varcnt = 0;
          for (let j = 0; j < tmpdefs.length; j++){
            o.push(`#undef ${tmpdefs[j]}`);
          }
          tmpdefs.splice(0,Infinity);

          if (infun){
            o.push(`}`)
          }
          let funname = clean(lbl);
          o.push(`void ${funname}(){`);
          o.push(`__push_stack();`);
          infun = 1;

        }else{
          o.push(`${clean(lbl)}:;`);
        }
      }else{
        if (infun || inmain){
          // o.push(`printf("no segfault yet on line %d %s!\\n",__LINE__,__FILE__);`);
        }
      }
      trans_instr(ins);
    }
    function trans_instr(ins){
      if (!ins.length || ins[0] == 'nop'){

      }else if (ins[0] == 'decl'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        lookup[nom] = typ;
        write_decl(typ,nom);
        if (typ.con == 'vec'){
          if (infun){
            o.push(`memset(${nom},0,${type_size(typ)});`);
          }
        }else if (typ.con == 'tup'){
          let nc = tup_size(typ,Infinity);
          o.push(`${nom} = __gc_alloc(VART_TUP,(${nc}));`);
          for (let i = 0; i < typ.elt.length; i++){
            o.push(`((char*)${nom} + ${i}*5)[0] = ${vart(typ.elt[i])};`);
            o.push(`((int*)((char*)${nom} + ${i}*5 + 1))[0] = ${tup_size(typ,i)};`);
          }
          o.push(`((char*)${nom} + ${typ.elt.length}*5)[0] = 0;`);
          o.push(`((int*)((char*)${nom} + ${typ.elt.length}*5 + 1))[0] = ${tup_size(typ,Infinity)};`);

          o.push(`__put_var(${varcnt++},${nom});`);
        }else if (typ.con == 'union'){
          let ts = type_size(typ.elt[0]);
          for (let i = 1; i < typ.elt.length; i++){
            ts = `(((${type_size(typ.elt[i])})>(${ts}))?(${type_size(typ.elt[i])}):(${ts}))`;
          }
          o.push(`${nom} = __gc_alloc(VART_UON,sizeof(__union_t)+${ts});`);
          o.push(`${nom}->sel = -1;`);
          o.push(`__put_var(${varcnt++},${nom});`);
        }
      }else if (ins[0] == 'alloc'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        lookup[nom] = typ;
        if (typ.con == 'list'){
          let w = type_size(typ.elt[0]);
          let cap = Number(ins[3])+1;
          o.push(`__list_t* ${nom} = __gc_alloc(VART_LST,sizeof(__list_t));`);
          o.push(`${nom}->cap = ${cap};`);
          o.push(`${nom}->n = ${ins[3]};`);
          o.push(`${nom}->w = ${w};`);
          o.push(`${nom}->t = ${vart(typ.elt[0])};`);
          o.push(`${nom}->data = malloc(${w}*${cap});`);
          o.push(`__put_var(${varcnt++},${nom});`);
        }else if (typ.con == 'arr'){
          let w = type_size(typ.elt[0]);
          let ndim = typ.elt[1];
          let cnt = ins[3];

          let is2d = cnt & (1<<30);
          let n = cnt;
          let d0 = cnt;
          let d1 = 1;
          if (is2d){
            d0 = ((cnt >> 15) & 0x7fff);
            d1 = (cnt & 0x7fff);
            n = d0 * d1;
          }
          let ds = [d0,d1];
          while (ds.length < ndim) ds.push(1);

          o.push(`__arr_t* ${nom} = __gc_alloc(VART_ARR,sizeof(__arr_t)+${ndim}*4);`);
          o.push(`${nom}->n = ${n};`);
          o.push(`${nom}->ndim = ${ndim};`);
          o.push(`${nom}->w = ${w};`);
          o.push(`${nom}->t = ${vart(typ.elt[0])};`);
          o.push(`${nom}->data = malloc(${w}*${n});`);
          for (let i = 0; i < ndim; i++){
            o.push(`${nom}->dims[${i}] = ${ds[i]};`);
          }
          o.push(`__put_var(${varcnt++},${nom});`);
        }else if (typ.con == 'dict'){

          let wk = type_size(typ.elt[0]);
          let wv = type_size(typ.elt[1]);
          o.push(`__dict_t* ${nom} = __gc_alloc(VART_DIC,sizeof(__dict_t));`);
          o.push(`${nom}->n = 0;`);
          o.push(`${nom}->kw = ${wk};`);
          o.push(`${nom}->vw = ${wv};`);
          o.push(`${nom}->kt = ${vart(typ.elt[0])};`);
          o.push(`${nom}->vt = ${vart(typ.elt[1])};`);
          o.push(`{for (int i=0; i<__NUM_DICT_SLOTS; i++){`);
          o.push(`${nom}->slots[i].n = 0;`);
          o.push(`${nom}->slots[i].cap = 0;`);
          o.push(`${nom}->slots[i].data = NULL;`);
          o.push(`}}`);
          o.push(`__put_var(${varcnt++},${nom});`);

        }else if (typeof typ == 'string'){
          let lo = layout[typ];
          let nf = lo.collect;
          // console.log(lo.size,nf,lo.size+nf.length*4+4)
          o.push(`void* ${nom} = __gc_alloc(VART_STT,${lo.size+nf.length*4+4});`);
          o.push(`__put_var(${varcnt++},${nom});`);
          o.push(`((int*)${nom})[0] = ${nf.length};`);
          for (let i = 0; i < nf.length; i++){
            // console.log(nf[i])
            o.push(`((int*)((char*)${nom}+${4+i*4}))[0] = ${nf[i]+nf.length*4+4};`);
          }
        }else{
          UNIMPL();
        }
      }else if (ins[0] == 'dcap'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        lookup[nom] = typ;

        if (typ.con == 'vec'){
          o.push(`${typ.elt[0]} (*_${nom})[${typ.elt.slice(1).join('*')}];`);
        }else{
          // console.log(typ,nom);
          o.push(`${writable_type(typ)[0]} *_${nom};`);
        }
        o.push(`#define ${nom} (*_${nom})`);
        tmpdefs.push(nom);
        o.push(`__pop_arg(&_${nom}, 8);`);

      }else if (ins[0] == 'jmp'){
        o.push(`goto ${clean(ins[1])};`);
      }else if (ins[0] == 'jeqz'){
        o.push(`if (!${clean(ins[1])}) goto ${clean(ins[2])};`);
      }else if (ins[0] == 'mov'){
        let a = clean(ins[1])
        let b = clean(ins[2]);

        if (typeof b == 'number'){
          let [pa,ta,na] = get_ptr(a,null);
          o.push(`((${ta}*)${pa})[0] = ${b};`);
        }else if (b[0] == '"'){
          // let [pa,ta,na] = get_ptr(a,`(strlen(${b})+1)`);
          // o.push(`strcpy(*(char**)${pa}, ${b});`)
          // o.push(`*(char**)${pa} = ${b};`);
          let [pa,ta,na] = get_ptr(a,8);
          let tmp = shortid();
          o.push(`char* ${tmp} = __gc_alloc(VART_STR, strlen(${b})+1 );`);
          o.push(`strcpy(${tmp}, ${b} );`);
          o.push(`memcpy(${pa}, &${tmp}, 8);`);
          o.push(`__put_var(${varcnt++},${tmp});`);
        }else{
          let [pb,tb,nb] = get_ptr(b,null);
          let [pa,ta,na] = get_ptr(a,nb);
          if (tb == 'char*'){
            let tmp = shortid();
            o.push(`char* ${tmp} = __gc_alloc(VART_STR, strlen(*(${pb}))+1 );`);
            o.push(`strcpy(${tmp}, *(${pb}) );`)
            o.push(`memcpy(${pa}, &${tmp}, ${nb});`);
            o.push(`__put_var(${varcnt++},${tmp});`);
          }else{
            o.push(`memcpy(${pa}, ${pb}, ${nb});`);
          }
        }
      }else if (['add','sub','mul','div','mod','pow'].includes(ins[0])){
        math(ins[0], clean(ins[1]), clean(ins[2]), clean(ins[3]));
      }else if (['band','bor','xor'].includes(ins[0])){
        math(ins[0], clean(ins[1]), clean(ins[2]), clean(ins[3]));
      }else if (ins[0] == 'bnot'){
        o.push(`${clean(ins[1])} = ~${clean(ins[2])};`);
      }else if (ins[0] == 'lnot'){
        o.push(`${clean(ins[1])} = !${clean(ins[2])};`);
      }else if (ins[0] == 'shl'){
        o.push(`${clean(ins[1])} = ${clean(ins[2])} << ${clean(ins[3])};`);
      }else if (ins[0] == 'shr'){
        o.push(`${clean(ins[1])} = ${clean(ins[2])} >> ${clean(ins[3])};`);
      }else if (['leq','geq','lt','gt','neq','eq'].includes(ins[0])){
        compare(ins[0], clean(ins[1]), clean(ins[2]), clean(ins[3]));
      }else if (ins[0] == 'matmul'){
        let c = clean(ins[1]);
        let a = clean(ins[2]);
        let b = clean(ins[3]);
        let ta = lookup[a];
        let tb = lookup[b];
        let nr0 = ta.elt[1];
        let nc0 = 1;
        let nr1 = tb.elt[1];
        let nc1 = 1;
        if (ta.elt.length == 3){
          nc0 = ta.elt[2];
        }
        if (tb.elt.length == 3){
          nc1 = tb.elt[2];
        }
        let nr = nr0;
        let nc = nc1;
        for (let i = 0; i < nr; i++){
          for (let j = 0; j < nc; j++){
            let s = "0";
            for (let k = 0; k < nc0; k++){
              s += `+${a}[${i*nc0+k}]*${b}[${k*nc1+j}]`;
            }
            o.push(`${c}[${i*nc1+j}]=${s};`);
          }
        }
      }else if (ins[0] == 'utag'){
        let a = clean(ins[1]);
        let b = clean(ins[2]);
        let typ = read_type(ins[3]);
        let tb = lookup[b];
        let idx = tb.elt.map(x=>JSON.stringify(x)).indexOf(JSON.stringify(typ));
        o.push(`${a} = ${idx} == (${b})->sel;`);
      }else if (ins[0] == 'lt'){
        o.push(`${clean(ins[1])} = ${clean(ins[2])} < ${clean(ins[3])};`);
      }else if (ins[0] == 'cast'){
        cast(clean(ins[1]),clean(ins[2]),ins);
      }else if (ins[0] == 'ccall'){
        let v = clean(ins[1])
        let luv = lookup[v];
        let n = type_size(luv);
        o.push(`__push_retpt(&${v}, ${vart(luv)}, ${n});`);
        o.push(ins[2].replace(/\./g,'__')+`();`);
        o.push(`__POP_RETPT;`);
        o.push(`memcpy(&${v}, __retpts[__retpts_idx].var, ${n});`);
        if (collectible.includes(vart(luv))){
          o.push(`__put_var(${varcnt++},${v});`);
        }
      }else if (ins[0] == 'argw'){
        let a = clean(ins[1]);
        if (typeof a == 'number'){
          let r = shortid()
          o.push(`${read_type(ins[2])} ${r} = ${a};`);
          a = r;
        }else if (a.startsWith('"')){
          let r = shortid()
          o.push(`char* ${r} = ${a};`);
          a = r;
        }
        let typ = read_type(ins[2]);
        o.push(`__push_arg(&(${a}),${vart(typ)},${type_size(typ)});`);
      }else if (ins[0] == 'argr'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        lookup[nom] = typ;
        write_decl(typ,nom)
        o.push(`__pop_arg(&(${clean(ins[1])}),${type_size(read_type(ins[2]))});`);
      }else if (ins[0] == 'fpak'){
        let nom = clean(ins[1]);
        let ptr = clean(ins[3]);
        lookup[nom] = read_type(ins[2]);
        
        let {dcap} = funcs[ptr];
        let n = "0";
        for (let i = 0; i < dcap.length; i++){
          n += "+5+" + type_size(dcap[i].typ);
        }
        o.push(`__func_t* ${nom} = __gc_alloc(VART_FUN,sizeof(__func_t));`);
        o.push(`${nom}->funptr = ${ptr};`);
        o.push(`${nom}->captr = malloc(${n});`);
        o.push(`${nom}->cnt = ${dcap.length};`);
        o.push(`${nom}->siz = ${n};`);
        o.push(`__put_var(${varcnt++},${nom});`);

        n = "0";
        for (let i = 0; i < dcap.length; i++){
          let tmp = shortid();
          o.push(`int ${tmp} = ${type_size(dcap[i].typ)};`);
          o.push(`memcpy((char*)(${nom}->captr)+(${n}), &(${dcap[i].nom}), ${tmp});`);
          n += "+"+type_size(dcap[i].typ);
          o.push(`memcpy((char*)(${nom}->captr)+(${n}), &${tmp}, 4);`);
          n += "+4";
          o.push(`((char*)(${nom}->captr))[${n}] = ${vart(dcap[i].typ)};`);
          n += "+1";
        }
      }else if (ins[0] == 'cap'){
        // pass
      }else if (ins[0] == 'call'){
        // let mk= `__retpt_${shortid()}`
        let v = clean(ins[1])
        let luv = lookup[v];
        let n = type_size(luv);

        let funname = clean(ins[2]);

        if (funcs[funname]){
          for (let i = funcs[funname].dcap.length-1; i>=0; i--){
            let {typ,nom} = funcs[funname].dcap[i];
            let nam = `_p_${nom}_${shortid()}`
            o.push(`void* ${nam} = &(${nom});`);
            o.push(`__push_arg(&(${nam}),${vart(typ)},8);`);
          }
          o.push(`__push_retpt(&${v}, ${vart(luv)}, ${n});`);
          o.push(`${funname}();`);
          // o.push(`${mk}:;`);
          // console.log(v,lookup[v],n)
          o.push(`__POP_RETPT;`);
          o.push(`memcpy(&${v}, __retpts[__retpts_idx].var, ${n});`);
          if (collectible.includes(vart(lookup[v]))){
            o.push(`__put_var(${varcnt++},${v});`);
          }
          // o.push(`__gc_run();`);
        }else if (typds[funname]){
          let oldlookup = Object.assign({},lookup);
          o.push("{")
          for (let i = 0; i < typds[funname].instrs.length; i++){
            if (typds[funname].instrs[i][0] == 'ret'){
              o.push(`${v} = ${clean(typds[funname].instrs[i][1])};`);
            }else{
              trans_instr(typds[funname].instrs[i]);
            }
          }
          Object.assign(lookup,oldlookup);
          o.push("}")

        }else{
          UNIMPL();
        }
        
      }else if (ins[0] == 'rcall'){
        let v = clean(ins[1])
        let luv = lookup[v];
        let n = type_size(luv);
        let fun = clean(ins[2]);
        // o.push(`printf("%p\\n",${funname});`);
        let tmp = shortid();
        let top = shortid();
        
        o.push(`void* ${tmp} = malloc(${fun}->siz);`);
        o.push(`memcpy(${tmp},${fun}->captr,${fun}->siz);`);
        o.push(`char* ${top} = (char*)${tmp} + ${fun}->siz;`);
        o.push(`while (${top} > (char*)${tmp}){`);
        o.push(`char vt = *((char*)(${top}-=1));`);
        o.push(`int sz;`);
        o.push(`memcpy(&sz, (${top}-=4), 4);`);
        o.push(`void* ptr = (${top}-=sz);`);
        o.push(`__push_arg(&ptr,vt,8);`);
        o.push("}");
        o.push(`__push_retpt(&${v}, ${vart(luv)}, ${n});`);
        o.push(`${fun}->funptr();`);
        o.push(`__POP_RETPT;`);
        o.push(`memcpy(&${v}, __retpts[__retpts_idx].var, ${n});`);
        if (collectible.includes(vart(lookup[v]))){
          o.push(`__put_var(${varcnt++},${v});`);
        }
        o.push(`free(${tmp});`);
      }else if (ins[0] == 'ret'){
        if (ins[1]){
          let v = clean(ins[1]);
          if (typeof v == 'number'){
            let u = shortid();
            o.push(`${v<0?'':'u'}int64_t ${u} = ${v};`);
            o.push(`__put_ret(&${u});`);
          }else{
            o.push(`__put_ret(&${v});`);
          }
        }
        o.push(`__pop_stack();`);
        o.push(`return;`);
      }else if (ins[0] == 'incl'){
        let p = ins[1].slice(1,-1);
        let f = p+"/static.c";
        o.unshift(`#include "${f}"`);
        cflags.push(`eval "$(cat ${p}/cflags.txt)"`);
      }else{
        console.log(ins)
      }
    }
    o.push(`__gc_run();`);
    o.push(`__pop_stack();`);
    //o.push(`__gc_run();`);
    o.push(`return 0;`);
    o.push(`}`);
    o.unshift(lib);
    o.unshift('//'+cflags.join(' && '));
    return o.join('\n')
    return o.join(`\nprintf("L%d %s!\\n",__LINE__,__FILE__);\n`)
  }
  this.parse_ir = parse_ir;
  this.transpile = transpile_c;
}


if (typeof module !== 'undefined'){
  if (require.main !== module){
    module.exports = TO_C;
  }else{
    let to_c = new TO_C({});
    let inp_pth;
    let out_pth;
    for (let i = 2; i < process.argv.length; i++){
      if (process.argv[i] == '-o' || process.argv[i] == '--output'){
        out_pth = process.argv[++i];
      }else{
        inp_pth = process.argv[i];
      }
    }
    const fs = require('fs');
    let txt = fs.readFileSync(inp_pth).toString();
    let [ir,layout] = to_c.parse_ir(txt);
    // console.dir(layout,{depth:Infinity})
    fs.writeFileSync(out_pth,to_c.transpile(ir,layout));
  }
}

