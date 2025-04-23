const path = require('path');

let collectible = ["VART_STR","VART_LST","VART_TUP","VART_DIC","VART_STT","VART_FUN","VART_ARR"]

let lib = `
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
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
#define OP_ADD(a,b) ((a)+(b))
#define OP_SUB(a,b) ((a)-(b))
#define OP_MUL(a,b) ((a)*(b))
#define OP_DIV(a,b) ((a)/(b))
#define OP_BAND(a,b) ((a)&(b))
#define OP_BOR(a,b) ((a)|(b))
#define OP_MOD(a,b) ((a)%(b))
#define OP_MODF(a,b) (fmod(a,b))
#define OP_POW(a,b)  (pow(a,b))
typedef struct ret_st {
  void* var;
  int n;
} ret_t;
ret_t* __retpts;
int __retpts_cap = 256;
int __retpts_idx = 0;
char* __args;
int __args_siz = 256;
int __args_top = 0;
void __push_retpt(void* var, int n){
  if (__retpts_idx>=__retpts_cap){
    __retpts_cap = __retpts_cap*2+1;
    __retpts = realloc(__retpts, __retpts_cap*sizeof(ret_t));
  }
  ret_t r;
  r.var = var;
  r.n = n;
  __retpts[__retpts_idx++] = r;
}
void __put_ret(void* addr){
  memcpy(__retpts[__retpts_idx-1].var, addr, __retpts[__retpts_idx-1].n);
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
  if(DBG_GC)printf("alloc %d %p\\n",(char)n->flag,n->data);
  return n->data;
}
void* __gc_realloc(void* ptr, int sz){
  __mem_node_t* node = ptr - sizeof(__mem_node_t);
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
  __mem_node_t* node = ptr - sizeof(__mem_node_t);
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

uintptr_t* __vars;
int __vars_cap = 256;
int __vars_top = 0;
int __vars_stack = 0;

int* __stack;
int __stack_cap = 256;
int __stack_top = 0;


void __push_stack(){
  __vars_stack = __vars_top;
  if (__stack_top+1>=__stack_cap){
    __stack_cap = (__stack_cap)*2+1;
    __stack = realloc(__stack,__stack_cap);
  }
  __stack[__stack_top++] = __vars_stack;
}
void __pop_stack(){
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
  __mem_node_t* node = (__mem_node_t*) ((void*)ptr-sizeof(__mem_node_t));
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
  }else if (vt == VART_STT){
    int n = ((int*)ptr)[0];
    for (int i = 0; i < n; i++){
      int ofs = ((int*)ptr)[i+1];
      __gc_mark(ptr + ofs);
    }
  }else if (vt == VART_DIC){
    __dict_t* dic = (__dict_t*) ptr;
    if (${collectible.map(x=>"dic->kt=="+x).join('||')}){
      for (int i = 0; i < __NUM_DICT_SLOTS; i++){
        for (int j = 0; j < dic->slots[i].n; j++){
          __gc_mark( dic->slots[i].data + (dic->kw+dic->vw) * j );
        }
      }
    }
    if (${collectible.map(x=>"dic->vt=="+x).join('||')}){
      for (int i = 0; i < __NUM_DICT_SLOTS; i++){
        for (int j = 0; j < dic->slots[i].n; j++){
          __gc_mark( dic->slots[i].data + (dic->kw+dic->vw) * j + dic->kw );
        }
      }
    }
  }
}
void __gc_run(){
  if (DBG_GC) {printf("stack "); for (int i = 0; i < __vars_top; i++) printf("%p %d ",(void*)__vars[i], (char)(((__mem_node_t*)(((void*)__vars[i])-sizeof(__mem_node_t)))->flag )); printf("\\n");}
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
  if (DBG_GC) printf("put %d %p %d\\n", idx, ptr, (char)(((__mem_node_t*)(ptr-sizeof(__mem_node_t)))->flag));
  if (__vars_stack+idx+1>__vars_cap){
    __vars = realloc(__vars,(__vars_stack+idx+1)*2);
  }
  __vars[__vars_stack+idx] = (uintptr_t)ptr;
  if (__vars_stack+idx+1 > __vars_top){
    __vars_top = __vars_stack+idx+1;
  }
}
char* __to_str(void* ptr, int vart){
  char* o;
  if (vart == VART_LST){
    __list_t* lst = (__list_t*)ptr;
    o = calloc(2,1);
    o[0] = '{';
    for (int i = 0; i < lst->n; i++){
      char* a = __to_str(lst->data + (i*lst->w), lst->t);
      int no = strlen(o);
      int na = strlen(a);
      o = realloc(o, no+na+2);
      strcpy(o+no, a);
      o[no+na] = (i == lst->n-1) ? '}' : ',';
      o[no+na+1] = 0;
    }
  }else if (vart == VART_F32) {
    o = malloc(32);
    float f = *((float*) ptr);
    sprintf(o, "%f", f);
  }else if (vart == VART_I32) {
    o = malloc(32);
    int32_t f = *((int32_t*) ptr);
    sprintf(o, "%" PRId32, f);
  }
  return o;
}
int __hash(void* x, int n){
  int k = 0;
  for (int i = 0; i < n; i++){
    k ^= ((char*)x)[i];
  }
  return k % __NUM_DICT_SLOTS;
}
void* __dict_get(__dict_t* dic, void* idx){
  int s = __hash(idx, dic->kw);
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
      return k + dic->kw;
    }
  }
  if (dic->slots[s].n >= dic->slots[s].cap){
    dic->slots[s].cap = (dic->slots[s].cap)*2+1;
    dic->slots[s].data = realloc(dic->slots[s].data, dic->slots[s].cap);
  }
  void* k = dic->slots[s].data + (dic->kw+dic->vw) * dic->slots[s].n;
  memcpy( k,  idx, dic->kw);
  dic->slots[s].n++;
  dic->n++;
  return k + dic->kw;
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
  'void':'int',
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
      let nf = [];
      for (let i = 1; i < lo[ca].fields.length; i++){
        let vt = vart(lo[ca].fields[i][2]);
        if (collectible.includes(vt)){
          nf.push(lo[ca].fields[0]);
        }
      }
      lo[ca].collect = nf;
    }
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
function read_type(s){
  let acc = "";
  let cstk = [];
  let cptr = [];
  cstk.push(cptr);
  function proc(x){
    let y = typmap;
    if (y[x]) return y[x];
    return clean(x)+"";
  }
  for (let i = 0; i < s.length; i++){
    if (s[i] == '['){
      cptr.push({con:proc(acc),elt:[]});
      acc = "";
      cptr = cptr.at(-1).elt;
      cstk.push(cptr);
    }else if (s[i] == ']'){
      if (acc.length){
        cptr.push(proc(acc));
        acc = "";
      }
      cstk.pop();
      cptr = cstk.at(-1);
    }else if (s[i] == ','){
      if (acc.length){
        cptr.push(proc(acc));
        acc = "";
      }
    }else{
      acc += s[i];
    }
  }
  if (acc.length){
    o = proc(acc);
  }else{
    o = cstk[0][0];
  }
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
    return `(sizeof(${x.elt[0]})*${x.elt[1]})`
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
  }[v] ?? "VART_STT";
  return z;
}

function transpile_c(instrs,layout){
  let varcnt = 0;
  let o = [];
  let lookup = {};
  let cflags = [];

  function cast(a,b){
    let ta = lookup[a];
    let tb = lookup[b];
    if (ta == 'char*' && intpam[tb]){
      o.push(`${a}=__gc_alloc(VART_STR,32);`);
      o.push(`__put_var(${varcnt++},${a});`);
      if (tb[0] == 'u'){
        o.push(`sprintf(${a},"%" PRIu64,(uint64_t)${b});`);
      }else{
        o.push(`sprintf(${a},"%" PRId64,(int64_t)${b});`);
      }
    }else if (ta == 'char*' && (tb == 'float' || tb == 'double')){
      o.push(`${a}=__gc_alloc(VART_STR,32);`);
      o.push(`__put_var(${varcnt++},${a});`);
      o.push(`sprintf(${a},"%f",(double)${b});`);
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
      df = new Array(tb.elt[1]).fill(df).join(` "," `);
      let ar = new Array(tb.elt[1]).fill(0).map((x,i)=>`${cst}${b}[${i}]`).join(",");
      o.push(`${a}=__gc_alloc(VART_STR,snprintf(NULL, 0, "{" ${df} "}", ${ar})+1);`);
      o.push(`__put_var(${varcnt++},${a});`);
      o.push(`sprintf(${a}, "{" ${df} "}", ${ar});`);
    }else if (numpam[ta] && typeof b == 'number'){
      o.push(`${a} = ${b};`)
    }else if (numpam[ta] && numpam[tb]){
      o.push(`${a} = ${b};`);
    }else if (ta.con == 'vec' && (numpam[tb] || typeof b == 'number')){
      o.push(`{for (int i = 0; i < ${ta.elt[1]}; i++){`);
      o.push(`${a}[i] = ${b};`);
      o.push(`}}`);
    }else if (ta == "char*" && tb.con == 'list'){
      let tmp = shortid();
      o.push(`char* ${tmp} = __to_str(${b}, VART_LST);`);
      o.push(`${a} = __gc_alloc(VART_STR, strlen(${tmp})+1);`);
      o.push(`__put_var(${varcnt++},${a});`);
      o.push(`strcpy(${a},${tmp});`);
      o.push(`free(${tmp});`);
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
      o.push(`{for (int i = 0; i < ${typ.elt[1]}; i++){`);
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
      // o.push(`printf("%f ${os} %f\\n",(float)${b},(float)${c});`);
      o.push(`${a} = ${os}(${b},${c});`);
    }
  }

  function compare(op,a,b,c){
    let os = {
      leq:'<=',geq:'>=',lt:'<',gt:'>',eq:'==',neq:'!='
    }[op]
    if (lookup[a].con == "vec"){
      UNIMPL();
    }else if (lookup[a] == "char*"){
      UNIMPL();
    }else{
      o.push(`${a} = ${b} ${os} ${c};`);
    }
  }

  function tup_size(t,idx){
    let nc = '4';
    for (let i = 0; i < t.elt.length; i++){
      if (collectible.includes(vart(t.elt[i]))){
        nc += '+4';
      }
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
        if (t.elt[0] == 'char*' && n !== null){
          return [`((  ((${tt}*)((${v})->data))[${idx}*${tn}] = __gc_alloc(VART_STR, ${n})) , (((${tt}*)((${v})->data)) + (${idx}*${tn})))`, t.elt[0], type_size(t.elt[0])]
        }else{
          return [`(((${tt}*)((${v})->data)) + (${idx}*${tn}))`, t.elt[0], type_size(t.elt[0])]
        }
      }else if (t.con == 'dict'){
        idx = maybenum(idx);
        if (typeof idx == 'number' || idx[0] == '"'){
          let tmp = shortid();
          o.push(`${t.elt[0]} ${tmp} = ${idx};`);
          idx = tmp;
        }
        return [`__dict_get(${v}, &(${idx}))`, t.elt[1], type_size(t.elt[1])];
      }else if (t.con == 'tup'){
        idx = Number(idx);
        let nc = tup_size(t, idx);
        if (t.elt[idx] == 'char*' && n !== null){
          return [`( ((((char**) (((void*)${v}) + (${nc})) ))[0] = __gc_alloc(VART_STR, ${n})), (char**) ((((void*)${v}) + (${nc})) ))`, t.elt[idx], type_size(t.elt[idx])]
        }else{
          return [`(char**) ((((void*)${v}) + (${nc})))`, t.elt[idx], type_size(t.elt[idx])]
        }
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
        return [`(${tt}*)((void*)${v} + ${ofs}) `, typ, type_size(typ)];
      }else{
        // console.log(v,t,idx)
        UNIMPL();
      }
    }else{
      let t = lookup[x];
      if (t == 'char*'){
        if (n == null){
          return [`&(${x})`, 'char*', `(strlen(${x})+1)`];
        }else{
          return [`((${x})=__gc_alloc(VART_STR,${n}), __put_var(${varcnt++},${x}), (char**)&(${x}))`, 'char*', null];
        }
      }else{
        let t = lookup[x];
        return [`&(${x})`, t, type_size(t)];
      }
      
    }
  }
  function writable_type(typ){
    if (typ.con == 'list'){
      return [`__list_t*`,1];
    }else if (typ.con == 'dict'){
      return [`__dict_t*`,1];
    }else if (typ.con == 'vec'){
      return typ.elt;
    }else{
      return [typ,1];
    }
  }
  function write_decl(typ,nom){
    if (typ.con == 'list'){
      o.push(`__list_t* ${nom};`)
    }else if (typ.con == 'dict'){
      o.push(`__dict_t* ${nom};`)
    }else if (typ.con == 'vec'){
      o.push(`${typ.elt[0]} ${nom}[${typ.elt[1]}];`)
    }else if (typ.con == 'tup'){
      o.push(`void* ${nom};`);
    }else{
      o.push(`${typ} ${nom};`);
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
        o.push(`memset(${nom},0,${type_size(typ)});`);
      }else if (typ.con == 'tup'){
        let nc = tup_size(typ,Infinity);
        o.push(`${nom} = __gc_alloc(VART_TUP,(${nc}));`);
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
        o.push(`void* ${nom} = __gc_alloc(VART_STT,${lo.size+nf.length*4+4});`);
        o.push(`__put_var(${varcnt++},${nom});`);
        o.push(`((int*)${nom})[0] = ${nf.length};`);
        for (let i = 0; i < nf.length; i++){
          o.push(`((int*)(${nom}+${4+i*4}))[0] = ${nf[i]+nf.length*4+4};`);
        }
        // UNIMPL();
      }else{
        UNIMPL();
      }
    }else if (ins[0] == 'dcap'){
      let nom = clean(ins[1]);
      let typ = read_type(ins[2]);
      lookup[nom] = typ;

      if (typ.con == 'vec'){
        o.push(`${typ.elt[0]} (*_${nom})[${typ.elt[1]}];`);
      }else if (typ.con == 'list'){
        o.push(`__list_t** _${nom};`);
      }else{
        // console.log(typ,nom);
        o.push(`${typ} *_${nom};`);
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
        let [pa,ta,na] = get_ptr(a,`(strlen(${b})+1)`);
        o.push(`strcpy(*${pa}, ${b});`)
      }else{
        let [pb,tb,nb] = get_ptr(b,null);
        let [pa,ta,na] = get_ptr(a,nb);
  
        o.push(`memcpy(${pa}, ${pb}, ${nb});`);
      }



      // if (a.includes('+')){
      //   let [v,i] = a.split('+');
      //   if (lookup[v].con == 'vec'){
      //     o.push(`${v}[${i}] = ${clean(ins[2])};`);
      //   }else{
      //     UNIMPL();
      //   }
      // }else if (typeof b == 'number'){
      //   o.push(`${a} = ${b};`);
      // }else if (lookup[a] == 'char*'){
      //   // if (b.startsWith('"')){
      //     o.push(`${a} = __gc_alloc(strlen(${b})+1);`);
      //     o.push(`strcpy(${a}, ${b});`);
      //   // }else{
      //   //   UNIMPL();
      //   // }
      // }else{
      //   o.push(`memcpy(&(${a}), &(${b}), ${type_size(lookup[a])});`);
      // }
      
    }else if (['add','sub','mul','div','mod','pow'].includes(ins[0])){
      math(ins[0], clean(ins[1]), clean(ins[2]), clean(ins[3]));
    }else if (['band','bor'].includes(ins[0])){
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
    }else if (ins[0] == 'lt'){
      o.push(`${clean(ins[1])} = ${clean(ins[2])} < ${clean(ins[3])};`);
    }else if (ins[0] == 'eq'){
      o.push(`${clean(ins[1])} = ${clean(ins[2])} == ${clean(ins[3])};`);
    }else if (ins[0] == 'cast'){
      cast(clean(ins[1]),clean(ins[2]));
    }else if (ins[0] == 'ccall'){
      let v = clean(ins[1])
      let n = type_size(lookup[v]);
      o.push(`__push_retpt(&${v}, ${n});`);
      o.push(ins[2].replace(/\./g,'__')+`();`);
      o.push(`__POP_RETPT;`);
      o.push(`memcpy(&${v}, __retpts[__retpts_idx].var, ${n});`);
      if (collectible.includes(vart(lookup[v]))){
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
    }else if (ins[0] == 'call'){
      // let mk= `__retpt_${shortid()}`
      let v = clean(ins[1])
      let n = type_size(lookup[v]);

      let funname = clean(ins[2]);

      if (funcs[funname]){
        for (let i = funcs[funname].dcap.length-1; i>=0; i--){
          let {typ,nom} = funcs[funname].dcap[i];
          let nam = `_p_${nom}_${shortid()}`
          o.push(`void* ${nam} = &(${nom});`);
          o.push(`__push_arg(&(${nam}),${vart(typ)},8);`);
        }
        o.push(`__push_retpt(&${v}, ${n});`);
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
        o.push("{")
        for (let i = 0; i < typds[funname].instrs.length; i++){
          if (typds[funname].instrs[i][0] == 'ret'){
            o.push(`${v} = ${clean(typds[funname].instrs[i][1])};`);
          }else{
            trans_instr(typds[funname].instrs[i]);
          }
        }
        o.push("}")

      }else{
        UNIMPL();
      }
      
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
      let f = path.join(ins[1].slice(1,-1),"static.c");
      o.unshift(`#include "${f}"`);
      cflags.push(`eval "$(head -n 1 "${f}" | cut -c 3-)" && CFLAGS+=" " `);
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



if (typeof module !== 'undefined'){
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
  let [ir,layout] = parse_ir(txt);
  // console.dir(layout,{depth:Infinity})
  fs.writeFileSync(out_pth,transpile_c(ir,layout));

}

