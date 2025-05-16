#ifndef __INTERP_GUARD__
#define __INTERP_GUARD__

#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include <math.h>
#include <inttypes.h>
#include "common.h"

#ifndef DBG
  #define DBG 0
#endif

#define UNIMPL {printf("UNIMPLEMENTED: %s line %d\n",__FILE__,__LINE__); exit(0);}

#define INSC2(x) (((x)[0]<<8)|((x)[1]))
#define INSC3(x) (((x)[0]<<16)|((x)[1]<<8)|((x)[2]))
#define INSC4(x) (((x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|((x)[3]))
#define INSC5(x) (((uint64_t)((x)[0])<<32)|((x)[1]<<24)|((x)[2]<<16)|((x)[3]<<8)|((x)[4]))
#define INSC6(x) (((uint64_t)((x)[0])<<40)|((uint64_t)((x)[1])<<32)|((x)[2]<<24)|((x)[3]<<16)|((x)[4]<<8)|((x)[5]))

#define INSIS2(x) (ins->op==INSC2(x))
#define INSIS3(x) (ins->op==INSC3(x))
#define INSIS4(x) (ins->op==INSC4(x))
#define INSIS5(x) (ins->op==INSC5(x))
#define INSIS6(x) (ins->op==INSC6(x))

#define TYPM_VOID 0
#define TYPM_NUMB 1
#define TYPM_SIMP 2
#define TYPM_CONT 3


#define TERM_IDEN 1
#define TERM_STRL 2
#define TERM_ADDR 3
#define TERM_NUMI 4
#define TERM_NUMU 5
#define TERM_NUMF 6

#define OPRD_TYPE 1
#define OPRD_LABL 2
#define OPRD_TERM 3

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

#define GFLG_NOGC (1L<<0)
#define GFLG_TRGC (1L<<1)

const char* vart_names = "NULU08I08U16I16U32I32U64I64F32F64STRVECARRLSTTUPDICSTTFUNUON";

typedef struct type_st{
  uint8_t tag;
  uint8_t mode;
  uint8_t vart;
  union {
    str_t str;
    list_t elem;
  } u;
} type_t;

typedef struct term_st{
  uint8_t tag;
  uint8_t mode;
  union {
    str_t str;
    double f;
    int64_t i;
    uint64_t u;
    struct {
      str_t base;
      str_t offs;
      int   offi;
    } addr;
  } u;
} term_t;

typedef struct label_st{
  uint8_t tag;
  list_node_t* ptr;
  str_t str;
} label_t;

typedef struct opran_st{
  uint8_t tag;
} opran_t;

typedef struct loc_st{
  int file;
  int pos;
  char* pth;
} loc_t;


typedef struct instr_st{
  uint64_t op;
  opran_t* a;
  opran_t* b;
  opran_t* c;
  loc_t* loc;
} instr_t;

typedef struct layout_field_st{
  uint32_t offs;
  str_t name;
  type_t* type;
} lofd_t;

typedef struct layout_struct_st{
  uint32_t size;
  list_t fields;
} lost_t;

typedef struct obj_st{
  
  char flag;
  type_t* type;
  void* data;
} obj_t;

typedef struct vec_st{
  char flag;
  type_t* type;
  int n;
  uint8_t w;
  char data[];
} vec_t;

typedef struct stn_st{
  char flag;
  type_t* type;
  int n;
  uint8_t w;
  char data[];
} stn_t;

typedef struct lst_st{
  char flag;
  type_t* type;
  int n;
  uint8_t w;
  int cap;
  char* data;
} lst_t;

typedef struct arr_st{
  char flag;
  type_t* type;
  int n;
  uint8_t w;
  int ndim;
  char* data;
  int dims[];
} arr_t;

typedef struct tup_st{
  char flag;
  type_t* type;
  char data[];
} tup_t;

typedef struct dic_st{
  char flag;
  type_t* type;
  map_t map;
} dic_t;

typedef struct fun_st{
  char flag;
  type_t* type;
  void* ptr;
  list_t captr;
} fun_t;

struct var_st;

typedef struct uon_st{
  char flag;
  type_t* type;
  struct var_st* var;
} uon_t;


typedef struct var_st{
  type_t* type;
  union {
    uint8_t u8;
    int8_t i8;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
    uint64_t u64;
    int64_t i64;
    float f32;
    double f64;
    stn_t* str;
    obj_t* obj;
    vec_t* vec;
    tup_t* tup;
    arr_t* arr;
    lst_t* lst;
    dic_t* dic;
    fun_t* fun;
    struct uon_st* uon;
  } u;
} var_t;


typedef struct retp_st{
  map_t* frame;
  list_node_t* addr;
  var_t* v;
} retp_t;

ARR_DEF(retp_t);
ARR_DEF(var_t);



typedef struct mem_node_st {
  struct mem_node_st *prev;
  struct mem_node_st *next;
  int64_t size;
  char data[];
} mem_node_t;

typedef struct mem_list_st {
  mem_node_t *head;
  mem_node_t *tail;
  int len;
} mem_list_t;

void* mem_alloc(mem_list_t* l, int sz){
  mem_node_t* n = (mem_node_t*)calloc( sizeof(mem_node_t)+sz, 1);
  n->size = sz;
  n->next = NULL;
  n->prev = l->tail;
  if (l->head == NULL){
    l->head = n;
  }else{
    l->tail->next = n;
  }
  l->tail = n;
  l->len+=sz;
  return n->data;
}
void mem_free(mem_list_t* l, void* ptr){
  mem_node_t* node = ptr - sizeof(mem_node_t);
  int sz = node->size;
  if (node == l->tail){
    if (node == l->head){
      l->head = NULL;
      l->tail = NULL;
      goto cleanup;
    }else{
      l->tail->prev->next = NULL;
      l->tail = l->tail->prev;
      goto cleanup;
    }
  }else if (node == l->head){
    l->head->next->prev = NULL;
    l->head = l->head->next;
    goto cleanup;
  }
  node->prev->next = node->next;
  node->next->prev = node->prev;
cleanup:
  l->len-=sz;
  free(node);
}

typedef struct{
  uint64_t flags;
  list_t vars;
  map_t layouts;
  mem_list_t objs;
  map_t cfuncs;
  retp_t_arr_t ret_pts;
  uintptr_t_arr_t args;
} gstate_t;

gstate_t _G = {0};

void global_init(){
  memset(&_G,0,sizeof(_G));

  list_init(&_G.vars);
  ARR_INIT(uintptr_t, _G.args);
  ARR_INIT(retp_t, _G.ret_pts);
}

void* gc_alloc_(gstate_t* _g, int sz){
  void* x = mem_alloc(&(_g->objs), sz);
#if DBG
  printf("ALC %d %p\n",sz,x);
#endif
  return x;
}

void* gc_alloc(int sz){
  return gc_alloc_(&(_G),sz);
}

void* re_ptr_at(void* offsp){
  char n[8];
  memcpy(n,offsp,8);
  return (void*)*(void* const*)n;
}

uint8_t str2vart(char* ts){
  if (!strcmp(ts,"u8")){
    return VART_U08;
  }else if (!strcmp(ts,"i8")){
    return VART_I08;
  }else if (!strcmp(ts,"u16")){
    return VART_U16;
  }else if (!strcmp(ts,"i16")){
    return VART_I16;
  }else if (!strcmp(ts,"u32")){
    return VART_U32;
  }else if (!strcmp(ts,"i32")){
    return VART_I32;
  }else if (!strcmp(ts,"u64")){
    return VART_U64;
  }else if (!strcmp(ts,"i64")){
    return VART_I64;
  }else if (!strcmp(ts,"f32")){
    return VART_F32;
  }else if (!strcmp(ts,"f64")){
    return VART_F64;
  }else if (!strcmp(ts,"str")){
    return VART_STR;
  }else if (!strcmp(ts,"func")){
    return VART_FUN;
  }else if (!strcmp(ts,"vec")){
    return VART_VEC;
  }else if (!strcmp(ts,"arr")){
    return VART_ARR;
  }else if (!strcmp(ts,"list")){
    return VART_LST;
  }else if (!strcmp(ts,"dict")){
    return VART_DIC;
  }else if (!strcmp(ts,"union")){
    return VART_UON;
  }else if (!strcmp(ts,"tup")){
    return VART_TUP;
  }else if (!strcmp(ts,"void")){
    return VART_NUL;
  }
  return VART_STT;
}


// void print_type(type_t* a);

opran_t* read_type(FILE* fd){
  char c;
  int lvl = 0;
  type_t* typ = (type_t*)malloc(sizeof(type_t));
  typ->tag = OPRD_TYPE;
  str_t word = str_new();
  int ign = 0;
  while ((c=fgetc(fd))==' '){
  }
  do{
    if (c == '[' ){
      lvl++;
      if ((str_eq(&word,"vec") || str_eq(&word,"arr") || str_eq(&word,"list") || str_eq(&word,"tup") || str_eq(&word,"dict") || str_eq(&word,"func") || str_eq(&word,"union"))  ){
        typ->mode = TYPM_CONT;
        
        typ->vart = str2vart(word.data);
        list_init(&typ->u.elem);
        type_t* t;
        while(1){
          t = (type_t*)read_type(fd);
          if (t->mode != 255){
            list_add(&typ->u.elem,t);
          }
          char d = fgetc(fd);
          if (d == ']'){
            break;
          }else if (d == EOF){
            break;
          }else if (d != ','){

          }
        }
        return (opran_t*)typ;
      }else{
        ign = 1;
        str_addch(&word,c);
      }
    }else if (c == ']'){
      lvl--;
      if (ign){
        str_addch(&word,c);
      }
      if (lvl <= 0){
        if (ign){
          ign = 0;
          goto cleanup2;
        }else{
          goto cleanup;
        }
      }
    }else if (c == ','){
      goto cleanup;
    }else{
      str_addch(&word,c);
    }
  }while ((c=fgetc(fd))!=' ' && c!= EOF && c!= '\n');
cleanup:
  ungetc(c,fd);
cleanup2:
  if (!word.len){
    typ->mode = 255;
  }else if (str_eq(&word,"void")){
    typ->mode = TYPM_VOID;
    typ->vart = VART_NUL;
  }else if (str_eq(&word,"f32") || 
      str_eq(&word,"f64") ||
      str_eq(&word,"i8")  ||
      str_eq(&word,"u8")  ||
      str_eq(&word,"i16") ||
      str_eq(&word,"u16") ||
      str_eq(&word,"i32") ||
      str_eq(&word,"u32") ||
      str_eq(&word,"i64") ||
      str_eq(&word,"u64")
  ){
    typ->mode = TYPM_NUMB;
    typ->vart = str2vart(word.data);
  }else if (str_eq(&word,"str")){
    typ->mode = TYPM_SIMP;
    typ->vart = VART_STR;
  }else{
    typ->mode = TYPM_SIMP;
    typ->vart = VART_STT;
    typ->u.str = word;
  }
  
  if (typ->vart != VART_STT){
    free(word.data);
  }
  // print_type(typ);
  // printf("\n");
  return (opran_t*)typ;
}
void free_type(type_t* typ){
  if (typ->vart == VART_STT){
    free(typ->u.str.data);
  }else if (typ->mode == TYPM_CONT){
    list_node_t* n = typ->u.elem.head;
    while (n){
      list_node_t* next = n->next;
      type_t* t = n->data;
      free_type(t);
      free(n);
      n = next;
    }
  }
  free(typ);
}



int parse_num_maybe(char* s, int64_t* n, uint64_t* u, double* f){
  int m = 0;
  char c;
  if ((m = (s[0] == '-')) || s[0] == '+'){
    int r = parse_num_maybe(s+1,n,u,f);
    if (m){
      (*n) = -(*n);
      (*u) = -(*u);
      (*f) = -(*f);
      if (r == 2) r=1;
    }
    return r;
  }
  if (s[0] == '0'){
    if (s[1] == 'x' || s[1] == 'X'){
      (*u) = strtoull(s+2,NULL,16);
      return 2;
    }else if (s[1] == 'b' || s[1] == 'B'){
      (*u) = strtoull(s+2,NULL,2);
      return 2;
    }
  }
  if (s[0] == '.'){
    if (s[1] < '0' || s[1] > '9'){
      return 0;
    }
  }else if (s[0] < '0' || s[0] > '9'){
    return 0;
  }  
  char* s2 = s;
  int hdf = 0;
  while (( c = *(s2++) )){
    if (c == '.' || c == 'E' || c == 'e'){
      hdf = 1;
      break;
    }
  }
  (*f) = strtod(s,NULL);
  (*u) = strtoull(s,NULL,10);
  if ((*f) != (double)0.0){
    if (!hdf && (*f)==(double)(*u)){
      return 2;
    }else{
      return 3;
    }
  }else if ((*u) != 0){
    // 1e-999
    return 3;
  }
  if (hdf) return 3;
  return 2;
}

opran_t* read_term(FILE* fd){
  char c;
  int lvl = 0;
  term_t* term = (term_t*)malloc(sizeof(term_t));
  term->tag = OPRD_TERM;
  
  str_t* word;
  while ((c=fgetc(fd))==' '){
    
  }
  if (c == '"'){
    term->mode = TERM_STRL;
    c = fgetc(fd);
    term->u.str = str_new();
    word = &(term->u.str);
    do{
      if (c == '\\'){
        c = fgetc(fd);
        if (c == 'n'){
          c = '\n';
        }else if (c == 'r'){
          c = '\r';
        }else if (c == 't'){
          c = '\t';
        }
      }else if (c == '"'){
        break;
      }
      str_addch(word,c);
    }while ((c=fgetc(fd))!=EOF);
    c = fgetc(fd);

  }else{
    term->mode = TERM_IDEN;
    int snn = (c >= 'A' || c == '*') ? 2 : (c=='.');
    term->u.str = str_new();
    word = &(term->u.str);
    do{
      if (c == '+' && (snn)){
        term->mode = TERM_ADDR;
        term->u.addr.base = *word;
        term->u.addr.offs = str_new();
        word = &(term->u.addr.offs);
        c = fgetc(fd);
      }
      str_addch(word,c);
      c=fgetc(fd);
      if (snn == 1){
        if ('0' <= c && c <= '9'){
          snn = 0;
        }else{
          snn = 2;
        }
      }
    }while (c!=' ' && c!= EOF && c!= '\n');
  }
  ungetc(c,fd);
  if (term->mode == TERM_IDEN){
    uint64_t u;
    int64_t i;
    double f;
    int r = parse_num_maybe(word->data,&i,&u,&f);
    if (r == 1){
      
      term->mode = TERM_NUMI;
      term->u.i = i;
    }else if (r == 2){
      term->mode = TERM_NUMU;
      term->u.u = u;
    }else if (r == 3){
      term->mode = TERM_NUMF;
      term->u.f = f;
    }
  }else if (term->mode == TERM_ADDR){
    term->u.addr.offi = atoi(term->u.addr.offs.data);
  }

  return (opran_t*)term;
}

opran_t* read_label(FILE* fd){
  char c;
  int lvl = 0;
  label_t* lbl = (label_t*)malloc(sizeof(label_t));
  lbl->tag = OPRD_LABL;
  lbl->str = str_new();
  
  while ((c=fgetc(fd))==' '){
    
  }
  do{
    str_addch(&(lbl->str),c);
  }while ((c=fgetc(fd))!=' ' && c!= EOF && c!= '\n');
  ungetc(c,fd);
  return (opran_t*)lbl;
}

void begin_color(int c){
  printf("\033[%dm",c);
}
void end_color(){
  printf("\033[0m");
}


void print_type(type_t* a){
  if (!a){
    printf("?");
    return;
  }
  if (a->mode == TYPM_VOID){
    begin_color(2);
    printf("void");
    end_color();
  }else if (a->mode == TYPM_NUMB){
    begin_color(34);
    printf("%.03s",vart_names + (a->vart*3));
    end_color();
  }else if (a->mode == TYPM_SIMP){
    if (a->vart == VART_STR){
      begin_color(34);
      printf("%.03s",vart_names + (a->vart*3));
      end_color();
    }else{
      begin_color(34);
      printf("%s",a->u.str.data);
      end_color();
    }
  }else if (a->mode == TYPM_CONT){
    begin_color(34);
    printf("%.03s",vart_names + (a->vart*3));
    end_color();
    printf("[");
    list_node_t* n = a->u.elem.head;
    while (n){
      type_t* t = (type_t*)n->data;
      print_type(t);
      n = n->next;
      if (n){
        printf(",");
      }
    }
    printf("]");
    
  }else{
    printf("?%d?",a->mode);
  }
}

int type_eq(type_t* a, type_t* b){
  if (!a || !b){
    return 0;
  }
  if (a->mode != b->mode) return 0;
  if (a->vart != b->vart) return 0;
  
  if (a->mode == TYPM_SIMP){
    if (a->vart == VART_STR){

    }else{
      return strcmp(a->u.str.data, b->u.str.data)==0;
    }
  }else if (a->mode == TYPM_CONT){
    if (a->u.elem.len != b->u.elem.len) return 0;

    list_node_t* n = a->u.elem.head;
    list_node_t* m = b->u.elem.head;
    while (n){
      type_t* t = (type_t*)n->data;
      type_t* s = (type_t*)m->data;
      if (!type_eq(t,s)){
        return 0;
      }
      n = n->next;
      m = m->next;
    }
  }
  return 1;
}




void print_opran(opran_t* a){
  
  uint32_t tag = a->tag;
  if (tag == OPRD_TERM){
    if (((term_t*)a)->mode == TERM_IDEN){
      begin_color(32);
      printf("%s",((term_t*)a)->u.str.data);
      end_color();
    }else if (((term_t*)a)->mode == TERM_STRL){
      begin_color(33);
      printf("%s",((term_t*)a)->u.str.data);
      end_color();
    }else if (((term_t*)a)->mode == TERM_ADDR){
      // begin_color(31);
      // printf("%s+%s",((term_t*)a)->u.addr.base.data,((term_t*)a)->u.addr.offs.data);
      // end_color();

      begin_color(31);
      printf("%s",((term_t*)a)->u.addr.base.data);
      end_color();
      begin_color(2);
      printf("+");
      end_color();
      begin_color(31);
      printf("%s",((term_t*)a)->u.addr.offs.data);
      end_color();

    }else if (((term_t*)a)->mode == TERM_NUMI){
      begin_color(35);
      printf("%" PRId64,((term_t*)a)->u.i);
      end_color();
    }else if (((term_t*)a)->mode == TERM_NUMU){
      begin_color(35);
      printf("%" PRIu64,((term_t*)a)->u.u);
      end_color();
    }else if (((term_t*)a)->mode == TERM_NUMF){
      begin_color(35);
      printf("%f",((term_t*)a)->u.f);
      end_color();
    }
  }else if (tag == OPRD_TYPE){
    print_type((type_t*)a);
  }else if (tag == OPRD_LABL){
    begin_color(36);
    printf("%s",((label_t*)a)->str.data);
    end_color();

    begin_color(2);
    printf("=%05lx",(uintptr_t)(((label_t*)a)->ptr)&0xfffff);
    end_color();
  }else{
    printf("?");
  }
}

void print_instr(instr_t* ins){
  // printf("%p\n",ins);
  // printf("%llu",ins->op);

  if (ins->loc){
    begin_color(2);
    printf("(%d:%d)",ins->loc->file, ins->loc->pos);
    end_color(); 
    printf("\t");
  }

  for (int i = 7; i >= 0; i--){
    printf("%c",(char)((ins->op)>>(i*8)));
  }
  printf("\t");
  if (ins->a) print_opran(ins->a);
  printf("\t");
  if (ins->b) print_opran(ins->b);
  printf("\t");
  if (ins->c) print_opran(ins->c);
  

  printf("\n");
}

void print_instrs(list_t* l){
  list_node_t* n = l->head;
  int idx = 0;
  while (n){
    instr_t* ins = (instr_t*)n->data;
    begin_color(2);
    // printf("%3d ",idx);
    printf("%05lx ",((uintptr_t)n)&0xfffff );
    end_color();
    print_instr(ins);
    n = n->next;
    idx++;
  }
}

str_t read_ir_label(FILE* fd){
  char c;
  str_t lbl = str_new();
  while ((c=fgetc(fd))!=EOF){
    if (c == ':'){
      break;
    }else{
      str_addch(&lbl,c);
    }
  }

  return lbl;
}

instr_t* read_ir_line(FILE* fd){
  
  instr_t* ins = NULL;
  char c;
  int state = 0;

  while ((c=fgetc(fd)) != EOF){

    if (c == '\n'){

      if (ins){
        return ins;
      }
      ins = NULL;
      state = 0;
      break;
    }else if (c==' '){

      if (state & 1){
        state ++;
      }
    }else{

      if (!(state & 1)){

        state++;
        if (ins == NULL){
          ins = (instr_t*)calloc(sizeof(instr_t),1);
        }
        ungetc(c,fd);
      }
      if (state == 1){
        ins->op = 0;
        while ((c=fgetc(fd))!=' ' && c!= '\n' && c!=EOF){
          ins->op = (ins->op)<<8|c;
        }
        ungetc(c,fd);
      }else if (state == 3){
        if (INSIS3("mov") || INSIS4("cast") || INSIS4("bnot") || INSIS4("lnot")){
          ins->a = read_term(fd);
          ins->b = read_term(fd);
        }else if (INSIS4("utag")){
          ins->a = read_term(fd);
          ins->b = read_term(fd);
          ins->c = read_type(fd);

        }else if (INSIS4("decl") || INSIS4("argr") || INSIS4("argw") || INSIS4("dcap")){

          ins->a = read_term(fd);
          ins->b = read_type(fd);
          
        }else if (INSIS5("alloc")){
          ins->a = read_term(fd);
          ins->b = read_type(fd);
          ins->c = read_term(fd);

        }else if (INSIS4("call") || INSIS4("jeqz")){
          ins->a = read_term(fd);
          ins->b = read_label(fd);
        }else if (INSIS4("fpak")){
          ins->a = read_term(fd);
          ins->b = read_type(fd);
          ins->c = read_label(fd);
        }else if (INSIS3("cap")){
          ins->a = read_term(fd);
          ins->b = read_term(fd);
          ins->c = read_type(fd);
        }else if (INSIS3("ret")){
          ins->a = read_term(fd);
        }else if (INSIS5("ccall") || INSIS5("rcall")){ 
          ins->a = read_term(fd);
          ins->b = read_term(fd); 
        }else if (INSIS3("jmp")){
          ins->a = read_label(fd);
        }else if (INSIS4("incl")){
          ins->a = read_term(fd);
        }else if (
          INSIS3("add" ) ||
          INSIS3("sub" ) ||
          INSIS3("mul" ) ||
          INSIS3("div" ) ||
          INSIS3("mod" ) ||
          INSIS3("pow" ) ||
          INSIS3("shl" ) ||
          INSIS3("shr" ) ||
          INSIS3("bor" ) ||
          INSIS3("xor" ) ||
          INSIS4("band") ||
          INSIS2("gt"  ) ||
          INSIS2("lt"  ) ||
          INSIS3("geq" ) ||
          INSIS3("leq" ) ||
          INSIS2("eq"  ) ||
          INSIS3("neq" ) ||
          INSIS6("matmul" )
        ){
          ins->a = read_term(fd);
          ins->b = read_term(fd);
          ins->c = read_term(fd);
        }else{
          
          for (int i = 7; i >= 0; i--){
            printf("%c",(char)((ins->op)>>(i*8)));
          }
          printf(" %" PRIu64 "\n",ins->op);
        }

      }
    }
  }

  return ins;
}

list_node_t* find_label(map_t* lblmap, char* s){
  str_t t;
  t.len = strlen(s);
  t.data = s;
  return map_get(lblmap,&t);
}

list_t read_ir(FILE* fd){
  char c;
  
  map_t lblmap = {0};

  list_t l;
  list_init(&l);

  int32_t m0 = 0;
  int32_t m1 = -1;
  int32_t m2 = -1;
  int32_t idx = 0;
  int q = 0;
  int wbs = 0;
  while ((c=fgetc(fd))|1){
    if (c=='\n' || c == EOF){
      // printf("%d %d %d %d\n",m0,m1,m2,idx);
      str_t lbl = {0};
      if (m0 >= 0){
        if (m1 >= 0){
          fseek(fd,m0,SEEK_SET);
          // fgetc(fd);
          lbl = read_ir_label(fd);
          // printf("%s\n",lbl.data);
          
        }else{
          m1 = m0;
        }
      }
      if (m1 >= 0){
        fseek(fd,m1,SEEK_SET);
        instr_t* ins = read_ir_line(fd);
        if (ins){
          // print_instr(ins);
          list_add(&l,ins);
          if (lbl.len){
            
            map_add(&lblmap,&lbl,l.tail);
          }
          if (INSIS4("eoir")){
            break;
          }
        }
      }
      m0 = ftell(fd);
      m1 = -1;
      if (c==EOF)break;
    }else if (c==':' && !q){
      while (((c=fgetc(fd))==' ') || (c=='\n')){
        
      }
      ungetc(c,fd);
      m1 = ftell(fd);
    }else if (c == '"'){
      if (!wbs){
        q = !q;
      }
    }else if (c == '\\'){
      wbs = !wbs;
    }else{
      wbs = 0;
    }
  }
  
  list_node_t* n = l.head;
  while (n){
    instr_t* ins = (instr_t*)n->data;
    if (ins->a && ins->a->tag == OPRD_LABL){
      ((label_t*)(ins->a))->ptr = find_label( &lblmap, ((label_t*)(ins->a))->str.data  );
    }
    if (ins->b && ins->b->tag == OPRD_LABL){
      ((label_t*)(ins->b))->ptr = find_label( &lblmap, ((label_t*)(ins->b))->str.data  );
    }
    if (ins->c && ins->c->tag == OPRD_LABL){
      ((label_t*)(ins->c))->ptr = find_label( &lblmap, ((label_t*)(ins->c))->str.data  );
    }
    n = n->next;
  }

#if DBG
  print_instrs(&l);
#endif

  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    for (int i = 0; i < lblmap.slots[k].len;i++){
      pair_t p = lblmap.slots[k].data[i];
      #if DBG
      printf("%s %p\n",p.key,p.val);
      #endif
    }
    free(lblmap.slots[k].data);
  }

  return l;
}


lofd_t* read_layout_line(FILE* fd){
  char c=fgetc(fd);
  if (c != '\t'){
    ungetc(c,fd);
    return NULL;
  }
  lofd_t* f = (lofd_t*)malloc(sizeof(lofd_t));
  f->name = str_new();
  int mode = 0;
  char szs[8];
  char szi = 0;
  int sz;
  while ((c=fgetc(fd))!=EOF) {
    if (mode == 0){
      if (c == '\t'){
        szs[szi] = 0;
        sz = atoi(szs);
        f->offs = sz;
        mode++;
      }else{
        szs[szi++] = c;
      }
    }else if (mode == 1){
      if (c == '\t'){
        mode++;
      }else{
        str_addch(&(f->name),c);
      }
    }else if (mode == 2){
      ungetc(c,fd);
      f->type = (type_t*)read_type(fd);
      fgetc(fd);
      return f;
    }
  }
  return f;
}

map_t read_layout(FILE* fd){
   
  char c;
  int mode = 0;
  map_t layout = {0};
  str_t sname = str_new();
  char szs[8];
  char szi = 0;
  int sz;

  while ((c=fgetc(fd))!=EOF){

    if (mode == 0){
      if (c == '\t'){
        mode++;
        szi = 0;
      }else{
        str_addch(&sname,c);
      }
    }else if (mode == 1){
      if (c == '\n'){
        szs[szi] = 0;
        sz = atoi(szs);
        mode++;
      }else{
        szs[szi++] = c;
      }
    }else if (mode == 2){
      ungetc(c,fd);
      lost_t* ll = (lost_t*)malloc(sizeof(lost_t));
      list_init(&(ll->fields));
      ll->size = sz;
      lofd_t* l = NULL;
      while(1){
        l = read_layout_line(fd);
        if (!l) break;
        list_add(&(ll->fields),l);
      }
      map_add(&layout,&sname,ll);
      sname.len=0;
      mode = 0;
      szi = 0;
    }
  }
  return layout;
}

loc_t* read_srcmap_line(FILE* fd){
  char c0 = fgetc(fd);
  if (c0 == EOF) return NULL;
  int mode = 0;
  char buf[512];
  int bui = 0;
  loc_t* ll = NULL;
  while (1){
    char c = fgetc(fd);
    if (c == ' ' || c == '\n' || c == EOF){
      if (bui){
        if (ll == NULL) ll = (loc_t*)malloc(sizeof(loc_t));
        buf[bui] = 0;
        if (mode == 0){
          ll->file = atoi(buf);
        }else{
          if (c0 == 'P'){
            ll->pos = atoi(buf);
          }else if (c0 == 'F'){
            ll->pth = malloc(strlen(buf)+1);
            strcpy(ll->pth, buf);
            ll->pos = -1;
          }
        }
        bui = 0;
        mode++;
      }
      if (c != ' '){
        return ll;
      }
    }else{
      buf[bui++] = c;
    }
  }
}

void read_srcmap(list_t* instrs, FILE* fd){

  uintptr_t_arr_t fmap;
  ARR_INIT(uintptr_t, fmap);

  list_node_t* node = instrs->head;

  char c;
  while ((c=fgetc(fd)) != EOF){
    ungetc(c,fd);
    loc_t* ll = read_srcmap_line(fd);
    if (ll){
      if (ll->pos == -1){
        while (fmap.len <= ll->file){
          ARR_PUSH(uintptr_t, fmap, NULL);
        }
        fmap.data[ll->file] = (uintptr_t)(ll->pth);
      }else{
        ll->pth = (char*)(fmap.data[ll->file]);
        ((instr_t*)(node->data))->loc = ll;
        node = node->next;
      }
    }
  }
  free(fmap.data);
}

void print_layouts(map_t* m){
  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    for (int i = 0; i < m->slots[k].len;i++){
      pair_t p = m->slots[k].data[i];
      lost_t* layout = (lost_t*)p.val;
      begin_color(34);
      printf("%s\t",p.key);
      end_color();
      begin_color(35);
      printf("%d\n",layout->size);
      end_color();
      list_node_t* it = layout->fields.head;
      while(it){
        lofd_t* l = (lofd_t*)(it->data);
        begin_color(35);
        printf("\t%d\t",l->offs);
        end_color();
        begin_color(32);
        printf("%s\t",l->name.data);
        end_color();
        
        print_type(l->type);
        printf("\n");
        it = it->next;
      }
    }
  }
}

// list_t vars;

int type_size(type_t* t){
  if (t->mode == TYPM_NUMB){
    if (t->vart == VART_U08 || t->vart == VART_I08) return 1;
    if (t->vart == VART_U16 || t->vart == VART_I16) return 2;
    if (t->vart == VART_U32 || t->vart == VART_I32 || t->vart == VART_F32) return 4;
    if (t->vart == VART_U64 || t->vart == VART_I64 || t->vart == VART_F64) return 8;
  }
  
  return 8;
  
}

var_t* find_var(str_t* name){
  list_node_t* e = _G.vars.tail;
  var_t* v = NULL;
  while (e){
    v = (var_t*) map_get( (map_t*)(e->data), name);
    if (v){
      return v;
    }
    e = e->prev;
  }
  return v;
}

int elem_vart(void* v){
  return ((type_t*)(((vec_t*)v)->type->u.elem.head->data))->vart;
}



void to_str(int vart, void* u, str_t* s){
  if (vart == VART_NUL){
    null_case:
    str_add(s,"null");
    return;
  }else if (vart <= VART_F64){
    char cs[64];
         if (vart == VART_F64) snprintf(cs,64,"%f",  *((double*)u));
    else if (vart == VART_F32) snprintf(cs,64,"%f",  *((float*)u));
    else if (vart == VART_I64) snprintf(cs,64,"%" PRId64,*(( int64_t*)u));
    else if (vart == VART_U64) snprintf(cs,64,"%" PRIu64,*((uint64_t*)u));
    else if (vart == VART_I32) snprintf(cs,64,"%" PRId32,  *(( int32_t*)u));
    else if (vart == VART_U32) snprintf(cs,64,"%" PRIu32,  *((uint32_t*)u));
    else if (vart == VART_I16) snprintf(cs,64,"%" PRId16,  *(( int16_t*)u));
    else if (vart == VART_U16) snprintf(cs,64,"%" PRIu16,  *((uint16_t*)u));
    else if (vart == VART_I08) snprintf(cs,64,"%" PRId8,  *((  int8_t*)u));
    else if (vart == VART_U08) snprintf(cs,64,"%" PRIu8,  *(( uint8_t*)u));
    str_add(s,cs);
    return;
  }else if (vart == VART_STR){
    stn_t* q = (*((stn_t**)u));
    if (!q) goto null_case;
    str_add(s,q->data);
    return;
  }else if (vart == VART_VEC){
    vec_t* v = (*((vec_t**)u));
    if (!v) goto null_case;
    str_addch(s,'{');
    char* ptr = v->data;
    for (int i = 0; i < v->n; i++){
      
      to_str(elem_vart(v), ptr, s);
      ptr += v->w;
      if (i<v->n-1)str_addch(s,',');
    }
    str_addch(s,'}');
    return;
  }else if (vart == VART_LST){
    lst_t* v = (*((lst_t**)u));
    if (!v) goto null_case;
    str_addch(s,'{');
    char* ptr = v->data;
    for (int i = 0; i < v->n; i++){
      to_str(elem_vart(v), ptr, s);
      ptr += v->w;
      if (i<v->n-1)str_addch(s,',');
    }
    str_addch(s,'}');
    return;
  }else if (vart == VART_ARR){
    arr_t* v = (*((arr_t**)u));
    if (!v) goto null_case;
    str_addch(s,'[');
    for (int i = 0; i < v->ndim; i++){
      if (i)str_addch(s,'x');
      char n[50];
      sprintf(n,"%d",v->dims[i]);
      str_add(s,n);
    }
    str_addch(s,']');
    str_addch(s,'{');
    char* ptr = v->data;
    for (int i = 0; i < v->n; i++){
      to_str(elem_vart(v), ptr, s);
      ptr += v->w;
      if (i<v->n-1)str_addch(s,',');
    }
    str_addch(s,'}');
    return;
  }else if (vart == VART_TUP){
    tup_t* tup = (*((tup_t**)u));
    if (!tup) goto null_case;
    list_node_t* q = tup->type->u.elem.head;
    str_addch(s,'[');
    int ofs = 0;
    while (q){
      type_t* t = (type_t*)(q->data);

      to_str(t->vart,tup->data + ofs,s);

      q = q->next;
      ofs += type_size(t);
      if (q)str_addch(s,',');
    }
    str_addch(s,']');
  }else if (vart == VART_DIC){
    
    dic_t* dic = (*((dic_t**)u));
    if (!dic) goto null_case;

    str_addch(s,'{');

    type_t* ta = (type_t*)(dic->type->u.elem.head->data);
    type_t* tb = (type_t*)(dic->type->u.elem.tail->data);

    int n = 0;
    for (int k = 0; k < NUM_MAP_SLOTS; k++){
      if (dic->map.slots[k].cap){
        for (int i = 0; i < dic->map.slots[k].len; i++){
          pair_t p = dic->map.slots[k].data[i];
          if (ta->vart <= VART_F64){
            to_str(ta->vart, p.key, s);
          }else{
            str_add(s,p.key);
          }
          str_addch(s,':');
          to_str(tb->vart, p.val, s);
          n++;
          if (n != dic->map.len) str_addch(s,',');
        }
      }
    }

    str_addch(s,'}');
  }else if (vart == VART_STT){
    obj_t* o = (*((obj_t**)u));
    if (!o) goto null_case;
    lost_t* lost = (lost_t*)map_get(&_G.layouts,&(o->type->u.str));
    list_node_t* n = lost->fields.head;
    str_addch(s,'{');
    while (n){
      lofd_t* lofd = n->data;
      void* offsp = (void*)( ((char*)(o->data)) + (lofd->offs) );
      str_add(s,lofd->name.data);
      str_addch(s,':');
      if (lofd->type->mode == TYPM_SIMP && lofd->type->vart != VART_STR){
        obj_t* ptr = (obj_t*)re_ptr_at(offsp);
        char cs[32];
        // sprintf(cs,"[object@%p]",ptr);
        sprintf(cs,"[object@%05lx]",((unsigned long)ptr) & 0xFFFFF);
        str_add(s,cs);
      }else{
        to_str(lofd->type->vart,offsp,s);
      }
      n = n->next;
      if (n) str_addch(s,',');
    }
    str_addch(s,'}');
  }else if (vart == VART_FUN){
    fun_t* f = (*((fun_t**)u));
    if (!f) goto null_case;
    char cs[64];
    //sprintf(cs,"[func@%p->%p]",f,f->ptr);
    sprintf(cs,"[func@%05lx->%05lx]",((unsigned long)f) & 0xFFFFF,((unsigned long)(f->ptr)) & 0xFFFFF);
    str_add(s,cs);
  }else if (vart == VART_UON){
    uon_t* v = (*((uon_t**)u));
    if (!v || !v->var) goto null_case;
    to_str(v->var->type->vart, &(v->var->u), s);
  }else{
    UNIMPL
  }
}

void to_key(int vart, void* u, str_t* s){
  return to_str(vart,u,s);
}


void print_vars(){
  printf("===============BEGIN===============\n");
  list_node_t* e = _G.vars.head;
  while (e){
    if (e != _G.vars.head) printf("-----------------------------------\n");
    map_t* m = (map_t*)(e->data);
    for (int k = 0; k < NUM_MAP_SLOTS; k++){
      for (int i = 0; i < m->slots[k].len; i++){
        pair_t p = m->slots[k].data[i];
        var_t* v = (var_t*)(p.val);
        printf("%s\t%.03s\t",p.key,vart_names+(v->type->vart*3));
        // printf("%s\t",key.data);
        print_type(v->type);
        printf("\t");
        str_t s = str_new();
        if (v->type->mode == TYPM_NUMB || v->type->mode == TYPM_VOID){
          to_str(v->type->vart, &(v->u), &s);
        }else{
          printf("%p\t",v->u.obj);
          to_str(v->type->vart, &(v->u), &s);
        }
        printf("%s",s.data);
        free(s.data);
        printf("\n");
      }
    }
    e = e->next;
  }
  printf("================END================\n");
}

void* get_addr(term_t* a, int* nbytes){

  var_t* v = find_var(&(a->u.addr.base));

  if (v->type->vart == VART_STT){

    // print_vars();

    lost_t* lost = (lost_t*)map_get(&_G.layouts,&(v->type->u.str));

    

    list_node_t* n = lost->fields.head;
    
    while (n){
      lofd_t* lofd = n->data;
      // printf("%s %s\n",lofd->name.data,a->u.addr.offs.data);

      if (str_eq(&(lofd->name), a->u.addr.offs.data)){
        uint32_t offs = lofd->offs;
        if (nbytes){
          // if (n->next){
          //   *nbytes = ((lofd_t*)(n->next->data))->offs - lofd->offs;
          // }else{
          //   *nbytes = lost->size - lofd->offs;
          // }
          *nbytes = type_size(lofd->type);
        }
        // printf("O %d\n",offs);
        return (void*)((char*)(v->u.obj->data) + offs);
       
      }
      n = n->next;
    }
  }else if (v->type->vart == VART_VEC){
    
    if (nbytes){
      (*nbytes) = v->u.vec->w;
    }
    int idx = a->u.addr.offi;
    if (idx == 0 && a->u.addr.offs.data[0] != '0'){
      var_t* u = find_var(&(a->u.addr.offs));
      idx = u->u.i32;
    }
    return (void*)(v->u.vec->data + (v->u.vec->w * idx));

  }else if (v->type->vart == VART_LST){
    if (nbytes){
      (*nbytes) = v->u.lst->w;
    }
    int idx = a->u.addr.offi;
    if (idx == 0 && a->u.addr.offs.data[0] != '0'){
      var_t* u = find_var(&(a->u.addr.offs));
      idx = u->u.i32;
    }
    return (void*)(v->u.lst->data + (v->u.lst->w * idx));

  }else if (v->type->vart == VART_ARR){
    if (nbytes){
      (*nbytes) = v->u.arr->w;
    }
    int idx = a->u.addr.offi;
    if (idx == 0 && a->u.addr.offs.data[0] != '0'){
      var_t* u = find_var(&(a->u.addr.offs));
      if (u->type->vart == VART_VEC){
        vec_t* vec = u->u.vec;
        idx = 0;
        int stride = 1;
        for (int i = v->u.arr->ndim-1; i>=0; i--){
          idx += ((int*)vec->data)[i] * stride;
          stride *= (v->u.arr->dims)[i];
        }
      }else{
        idx = u->u.i32;
      }
    }
    return (void*)(v->u.arr->data + (v->u.arr->w * idx));

  }else if (v->type->vart == VART_TUP){
    
    int idx = a->u.addr.offi;
    if (idx == 0 && a->u.addr.offs.data[0] != '0'){
      var_t* u = find_var(&(a->u.addr.offs));
      idx = u->u.i32;
    }

    int ofs = 0;
    list_node_t* q = v->type->u.elem.head;

    for (int i = 0; i < idx; i++){
      ofs += type_size((type_t*)(q->data));
      q = q->next;
    }
    
    type_t* t = (type_t*)(q->data);
    // print_type(t);
    // printf(" %d \n",type_size(t));
    if (nbytes){
      *nbytes = type_size(t);
    }
    return (void*)(v->u.tup->data + ofs);

  }else if (v->type->vart == VART_DIC){
    type_t* ta = (type_t*)(v->type->u.elem.head->data);
    type_t* tb = (type_t*)(v->type->u.elem.tail->data);
    int ds = type_size(tb);
    if (nbytes){
      *nbytes = ds;
    }
    char* bs = NULL;
    int bl;

    if (ta->vart == VART_STR){
      char* idx = a->u.addr.offs.data;
      if (idx[0] == '"'){
        bl = a->u.addr.offs.len-2;
        bs = malloc(bl+1);
        memcpy(bs,idx+1,bl);
        bs[bl] = 0;
      }else{
        var_t* u = find_var(&(a->u.addr.offs));
        bl = u->u.str->n;
        bs = malloc(bl+1);
        memcpy(bs,u->u.str->data,bl);
        bs[bl] = 0;
      }
    }else if (ta->vart < VART_F32){
      int idx = a->u.addr.offi;
      if (idx == 0 && a->u.addr.offs.data[0] != '0'){
        var_t* u = find_var(&(a->u.addr.offs));
        if (ta->vart == VART_I08 || ta->vart == VART_U08){
          bs = malloc(bl=1);
          bs[0] = u->u.u8;
        }else if (ta->vart == VART_I16 || ta->vart == VART_U16){
          bs = malloc(bl=2);
          ((uint16_t*)bs)[0] = u->u.u16;
        }else if (ta->vart == VART_I32 || ta->vart == VART_U32){
          bs = malloc(bl=4);
          ((uint32_t*)bs)[0] = u->u.u32;
        }else{
          bs = malloc(bl=8);
          ((uint64_t*)bs)[0] = u->u.u64;
        }
      }else{
        if (ta->vart == VART_I08 || ta->vart == VART_U08){
          bs = malloc(bl=1);
          bs[0] = ((char*)(&idx))[0];
        }else if (ta->vart == VART_I16 || ta->vart == VART_U16){
          bs = malloc(bl=2);
          ((uint16_t*)bs)[0] = ((uint16_t*)(&idx))[0];
        }else if (ta->vart == VART_I32 || ta->vart == VART_U32){
          bs = malloc(bl=4);
          ((uint32_t*)bs)[0] = ((uint32_t*)(&idx))[0];
        }else if (ta->vart == VART_I64){
          int64_t lidx = idx;
          bs = malloc(bl=8);
          ((uint64_t*)bs)[0] = ((uint64_t*)(&lidx))[0];
        }else{
          uint64_t lidx = idx;
          bs = malloc(bl=8);
          ((uint64_t*)bs)[0] = ((uint64_t*)(&lidx))[0];
        }
      }
    }else if (ta->vart == VART_F32){
      float idx = atof(a->u.addr.offs.data);
      bs = malloc(bl=4);
      if (idx == 0 && a->u.addr.offs.data[0] != '0' && !(a->u.addr.offs.data[0]=='-' && a->u.addr.offs.data[1]=='0')){
        var_t* u = find_var(&(a->u.addr.offs));
        ((float*)bs)[0] = u->u.f32;
      }else{
        ((float*)bs)[0] = idx;
      }
    }else if (ta->vart == VART_F64){
      double idx = atof(a->u.addr.offs.data);
      bs = malloc(bl=8);
      if (idx == 0 && a->u.addr.offs.data[0] != '0' && !(a->u.addr.offs.data[0]=='-' && a->u.addr.offs.data[1]=='0')){
        var_t* u = find_var(&(a->u.addr.offs));
        ((double*)bs)[0] = u->u.f64;
      }else{
        ((double*)bs)[0] = idx;
      }
    }else{
      str_t s = str_new();
      var_t* u = find_var(&(a->u.addr.offs));
      to_key(u->type->vart, &(u->u), &s);
      bs = s.data;
      bl = s.len;
    }

    // printf("%s\n",bs);
    void* ptr = map_addr_raw(&(v->u.dic->map), bs, bl, ds, 1);
    free(bs);

    // printf("%p\n",ptr);
    return ptr;

  }else if (v->type->vart == VART_STR){
    
    if (nbytes){
      (*nbytes) = v->u.str->w;
    }
    int idx = a->u.addr.offi;
    if (idx == 0 && a->u.addr.offs.data[0] != '0'){
      var_t* u = find_var(&(a->u.addr.offs));
      idx = u->u.i32;
    }
    return (void*)(v->u.vec->data + (v->u.vec->w * idx));

  }else{
    UNIMPL
  }

  return NULL;
}

uint64_t get_val_int(term_t* a){
  if (a->mode == TERM_IDEN){
    var_t* v = find_var(&(a->u.str));
    return v->u.u64;
  }else if (a->mode == TERM_ADDR){
    return *((uint64_t*)get_addr(a,NULL));
  }
  return a->u.u;
}

double get_val_f32(term_t* a){
  if (a->mode == TERM_IDEN){
    var_t* v = find_var(&(a->u.str));
    return v->u.f32;
  }else if (a->mode == TERM_ADDR){
    
    return *((float*)get_addr(a,NULL));
  }
  return a->u.f;
}

double get_val_f64(term_t* a){
  if (a->mode == TERM_IDEN){
    var_t* v = find_var(&(a->u.str));
    return v->u.f64;
  }else if (a->mode == TERM_ADDR){
    return *((double*)get_addr(a,NULL));
  }
  return a->u.f;
}

#define GVN_IDEN_VAL_INT(VART,CTYPE)\
  else if (v->type->vart == VART_ ## VART){\
    uint64_t q = v->u.u64;\
    return *(CTYPE*)(&q);\
  }

double get_val_num(term_t* a){
  if (a->mode == TERM_IDEN){
    var_t* v = find_var(&(a->u.str));
  tryagain:
    if (v->type->vart == VART_F32){
      return v->u.f32;
    }else if (v->type->vart == VART_F64){
      return v->u.f64;
    }else if (v->type->vart == VART_UON){
      v = v->u.uon->var;
      goto tryagain;
      
    }
      GVN_IDEN_VAL_INT(I64,int64_t)
      GVN_IDEN_VAL_INT(U64,uint64_t)
      GVN_IDEN_VAL_INT(I32,int32_t)
      GVN_IDEN_VAL_INT(U32,uint32_t)
      GVN_IDEN_VAL_INT(I16,int16_t)
      GVN_IDEN_VAL_INT(U16,uint16_t)
      GVN_IDEN_VAL_INT(I08,int8_t)
      GVN_IDEN_VAL_INT(U08,uint8_t)
    else{
      UNIMPL;
    }
  }else if (a->mode == TERM_ADDR){
    UNIMPL;
  }else if (a->mode == TERM_NUMF){
    return a->u.f;
  }else if (a->mode == TERM_NUMI){
    return a->u.i;
  }else if (a->mode == TERM_NUMU){
    return a->u.u;
  }else{
    UNIMPL;
  }
}

type_t _strl_type;
type_t* strl_type(){
  _strl_type.mode = TYPM_SIMP;
  _strl_type.vart = VART_STR;
  return &_strl_type;
}


stn_t* stn_copy_(gstate_t* _g, stn_t* u){
  stn_t* s = (stn_t*)gc_alloc_(_g,sizeof(stn_t)+u->n+1);
  memcpy(s,u,sizeof(stn_t)+u->n+1);
  return s;
}

stn_t* stn_copy(stn_t* u){
  return stn_copy_(&_G,u);
}

stn_t* get_val_stn(term_t* a){
  
  if (a->mode == TERM_STRL){
    // return &(a->u.str);
    stn_t* s = (stn_t*)gc_alloc(sizeof(stn_t)+a->u.str.len+1);
    s->n = a->u.str.len;
    s->w = 1;
    s->type = strl_type();
    memcpy(s->data, a->u.str.data, s->n);
    return s;
  }else if (a->mode == TERM_IDEN){
    
    var_t* v = find_var(&(a->u.str));
    return stn_copy(v->u.str);
  }else if (a->mode == TERM_ADDR){
    stn_t* s = *((stn_t**)get_addr(a,NULL));
    return s;
  }else{
    UNIMPL;
  }
}

vec_t* vec_copy_(gstate_t* _g, vec_t* u){
  vec_t* vec = (vec_t*)gc_alloc_(_g,sizeof(vec_t)+(u->n*u->w));
  memcpy(vec,u,sizeof(vec_t)+u->n*u->w);
  return vec;
}

vec_t* vec_copy(vec_t* u){
  return vec_copy_(&_G,u);
}

vec_t* get_val_vec(term_t* a){
  if (a->mode == TERM_IDEN){
    var_t* v = find_var(&(a->u.str));
    return vec_copy(v->u.vec);
  }else if (a->mode == TERM_ADDR){
    vec_t* v = *((vec_t**)get_addr(a,NULL));
    return vec_copy(v);
  }else{
    UNIMPL;
  }
  return NULL;
}

vec_t* get_ref_vec(term_t* a){
  if (a->mode == TERM_IDEN){
    var_t* v = find_var(&(a->u.str));
    return v->u.vec;
  }else if (a->mode == TERM_ADDR){
    vec_t* v = *((vec_t**)get_addr(a,NULL));
    return v;
  }else{
    UNIMPL;
  }
  return NULL;
}

obj_t* get_ref_obj(term_t* a){
  if (a->mode == TERM_IDEN){
    var_t* v = find_var(&(a->u.str));
    return v->u.obj;
  }else if (a->mode == TERM_ADDR){
    obj_t* v = *((obj_t**)get_addr(a,NULL));
    return v;
  }else{
    UNIMPL;
  }
  return NULL;
}


tup_t* get_val_tup(term_t* a){
  if (a->mode == TERM_IDEN){
    var_t* v = find_var(&(a->u.str));
    int n = 0;
    list_node_t* p = v->type->u.elem.head;
    while (p){
      type_t* t = (type_t*)(p->data);
      n += type_size(t);
      p = p->next;
    }
    tup_t* tup = (tup_t*)gc_alloc(sizeof(tup_t)+n);
    tup->type = v->type;
    p = v->type->u.elem.head;
    n = 0;
    while (p){
      type_t* t = (type_t*)(p->data);
      int ds = type_size(t);
    
           if (t->vart == VART_VEC) ((vec_t**)  (tup->data+n))[0] = vec_copy(((vec_t**)(v->u.tup->data+n))[0]);
      else if (t->vart == VART_STR) ((stn_t**)  (tup->data+n))[0] = stn_copy(((stn_t**)(v->u.tup->data+n))[0]);
      else{
        memcpy(tup->data+n, v->u.tup->data+n, ds);
      }
      n += ds;
      p = p->next;
    }
    return tup;
  }else{
    UNIMPL;
  }
  return NULL;
}

var_t* var_new(type_t* typ);

uon_t* get_val_uon(term_t* a){
  uon_t* uon = (uon_t*)gc_alloc(sizeof(uon_t));
  uon_t* von;
  if (a->mode == TERM_IDEN){
    von = find_var(&(a->u.str))->u.uon;
  }else if (a->mode == TERM_ADDR){
    von = *((uon_t**)get_addr(a,NULL));
  }else{
    UNIMPL;
  }
  type_t* t = von->var->type;
  uon->type = von->type;
  uon->var = var_new(von->var->type);
  if (t->vart == VART_STR){
    uon->var->u.vec = vec_copy(von->var->u.vec);
  }else if (t->vart == VART_VEC){
    uon->var->u.str = stn_copy(von->var->u.str);
  }else{
    uon->var->u.obj = von->var->u.obj;
  }
  return uon;
}


fun_t* get_ref_fun(term_t* a){
  if (a->mode == TERM_IDEN){
    // printf("%s\n",a->u.str.data);
    var_t* v = find_var(&(a->u.str));
    // printf("%p\n",v);
    return v->u.fun;
  }else{
    UNIMPL
  }
}

void gc_mark(obj_t* o);

void gc_mark_stt(obj_t* o){
  lost_t* lost = (lost_t*)map_get(&_G.layouts,&(o->type->u.str));
  list_node_t* n = lost->fields.head;
  
  while (n){
    lofd_t* lofd = n->data;
    if (lofd->type->mode == TYPM_SIMP || lofd->type->mode == TYPM_CONT){
      void* offsp = (void*)( ((char*)(o->data)) + (lofd->offs) );

      obj_t* ptr = (obj_t*)re_ptr_at(offsp);
      gc_mark(ptr);
    }
    n = n->next;
  }
}

void gc_mark_vec(vec_t* o){
  type_t* t = (type_t*)(o->type->u.elem.head->data);
  if (t->mode == TYPM_SIMP || t->mode == TYPM_CONT){
    for (int i = 0; i < o->n; i++){
      obj_t* ptr = (obj_t*)re_ptr_at((o->data + (i * o->w)));
      gc_mark(ptr);
    }
  }
}

void gc_mark_lst(lst_t* o){
  type_t* t = (type_t*)(o->type->u.elem.head->data);
  if (t->mode == TYPM_SIMP || t->mode == TYPM_CONT){
    for (int i = 0; i < o->n; i++){
      obj_t* ptr = (obj_t*)re_ptr_at((o->data + (i * o->w)));
      gc_mark(ptr);
    }
  }
}

void gc_mark_arr(arr_t* o){
  type_t* t = (type_t*)(o->type->u.elem.head->data);
  if (t->mode == TYPM_SIMP || t->mode == TYPM_CONT){
    for (int i = 0; i < o->n; i++){
      obj_t* ptr = (obj_t*)re_ptr_at((o->data + (i * o->w)));
      gc_mark(ptr);
    }
  }
}

void gc_mark_tup(tup_t* o){
  list_node_t* p = o->type->u.elem.head;
  int n = 0;
  while (p){
    type_t* t = (type_t*)(p->data);

    if (t->mode == TYPM_SIMP || t->mode == TYPM_CONT){
      obj_t* ptr =  *((obj_t**)(o->data+n));
      gc_mark(ptr);
    }
    p = p->next;
    n += type_size(t);
  }

}
void gc_mark_uon(uon_t* o){
  if (!o->var) return;
  if (o->var->type->mode == TYPM_SIMP || o->var->type->mode == TYPM_CONT){
    gc_mark(o->var->u.obj);
  }
}

void gc_mark_fun(fun_t* o){
  list_node_t* p = o->captr.head;
  while (p){
    var_t* v = ((pair_t*)(p->data))->val;
    type_t* t = v->type;
    if (t->mode == TYPM_SIMP || t->mode == TYPM_CONT){
      gc_mark(v->u.obj);
    }
    p = p->next;
  }
}

void gc_mark_dic(dic_t* o){
  type_t* tb = (type_t*)(o->type->u.elem.tail->data);
  if (tb->mode == TYPM_SIMP || tb->mode == TYPM_CONT){
    for (int k = 0; k < NUM_MAP_SLOTS; k++){
      if (o->map.slots[k].cap){
        for (int i = 0; i < o->map.slots[k].len;i++){
          pair_t p = o->map.slots[k].data[i];

          gc_mark(*((obj_t**)p.val));
        }
      }
    }
  }
}



void gc_mark(obj_t* o){
  if (!o){
    return;
  }
  if (o->flag & 1){
    return;
  }
  o->flag |= 1;
#if DBG
  printf("MRK %p\n",o);
#endif
  type_t* typ = o->type;
  if (typ->mode == TYPM_SIMP){
    // printf("%s\n",typ->u.sm.data);
    if (typ->vart!=VART_STR){
      gc_mark_stt(o);
    }
  }else if (typ->mode == TYPM_CONT){
    // printf("%s\n",typ->u.cn.cont.data);
    if (typ->vart == VART_VEC){
      gc_mark_vec((vec_t*)o);
    }else if (typ->vart == VART_LST){
      gc_mark_lst((lst_t*)o);
    }else if (typ->vart == VART_ARR){
      gc_mark_arr((arr_t*)o);
    }else if (typ->vart == VART_TUP){
      gc_mark_tup((tup_t*)o);
    }else if (typ->vart == VART_FUN){
      gc_mark_fun((fun_t*)o);
    }else if (typ->vart == VART_DIC){
      gc_mark_dic((dic_t*)o);
    }else if (typ->vart == VART_UON){
      gc_mark_uon((uon_t*)o);
    }
  }
}


void gc_sweep(){
  mem_node_t* n = _G.objs.head;
  while (n){
    obj_t* obj = (obj_t*)(n->data);
    mem_node_t* n1 = n->next;
    
    if (!obj->flag){
#if DBG
      printf("SWP %p\n",obj);
#endif
      if (obj->type->mode == TYPM_SIMP){

      }else if (obj->type->mode == TYPM_CONT){

        if (obj->type->vart == VART_TUP){
          
        }else if (obj->type->vart == VART_FUN){
          list_node_t* it = (((fun_t*)obj)->captr).head;
          while (it){
            list_node_t* nxt = it->next;
            pair_t* kv = (pair_t*)(it->data);
            free(kv->key);
            free(kv->val);
            free(it);
            it = nxt;
          }
        }else if (obj->type->vart == VART_VEC){

        }else if (obj->type->vart == VART_LST){
          free(((lst_t*)obj)->data);
        }else if (obj->type->vart == VART_ARR){
          free(((arr_t*)obj)->data);
        }else if (obj->type->vart == VART_DIC){
          map_nuke(&(((dic_t*)obj)->map));
        }else if (obj->type->vart == VART_UON){
          free(((uon_t*)obj)->var);
        }
      }else{
        free(obj->data);
      }
      mem_free((&_G.objs),obj);
    }else{
      obj->flag = 0;
    }
    n = n1;
  }
}


void gc_run(){
  // return;
  // print_vars();
  
  list_node_t* e = _G.vars.head;

  while (e){
    // printf("%d\n",_G.vars.len);

    map_t* m = (map_t*)(e->data);
    for (int k = 0; k < NUM_MAP_SLOTS; k++){
      for (int i = 0; i < m->slots[k].len; i++){
        pair_t p = m->slots[k].data[i];
        var_t* v = (var_t*)(p.val);
        if (v->type->mode == TYPM_CONT || v->type->mode == TYPM_SIMP){
          // printf("MSR %s\n",key.data);
          gc_mark(v->u.obj);
        }
      }
    }
    e = e->next;
  }

  for (int i = 0; i < _G.args.len; i++){
    var_t* v = (var_t*)(_G.args.data[i]);
    if (v->type->mode == TYPM_CONT || v->type->mode == TYPM_SIMP){
      gc_mark(v->u.obj);
    }
  }
  
  gc_sweep();
}



map_t* frame_start(){
  map_t* frame = (map_t*)calloc(1,sizeof(map_t));
  list_add(&_G.vars,frame);
  return frame;
}

map_t* frame_end(){
  // printf("end%d\n",_G.vars.len);
  
  #if DBG
    if (_G.vars.len<=2){
      print_vars();
    }
  #endif

  map_t* frame = _G.vars.tail->data;

  map_nuke(frame);
  free(frame);
  list_pop(&_G.vars);
  if (_G.vars.len == 0){
    return NULL;
  }
  
  if (!(_G.flags & GFLG_NOGC)){
    if (_G.objs.len > 16384){
      gc_run();
    }
  }
  return (map_t*)(_G.vars.tail->data);
}


var_t* var_new(type_t* typ){
  var_t* v = (var_t*)calloc(1,sizeof(var_t));
  v->type = typ;
  int m = typ->mode;
  if (m == TYPM_VOID){

  }else if (m == TYPM_NUMB){

  }else if (m == TYPM_SIMP){
    if (typ->vart == VART_STR){
      v->u.str = (stn_t*)gc_alloc(sizeof(stn_t)+1);
      v->u.str->type = typ;
      v->u.str->n = 0;
      v->u.str->w = 1;
      v->u.str->data[0] = 0;
    }else{
      v->type = typ;
    }
  }else{

    if (typ->vart == VART_VEC){
      int n = 1;
      list_node_t* q = typ->u.elem.head->next;
      while(q){
        type_t* tb = (type_t*)(q->data);
        n *= atoi(tb->u.str.data);
        q = q->next;
      }
      type_t* ta = (type_t*)(typ->u.elem.head->data);
      int ds = type_size(ta);

      v->type = typ;
      v->u.vec = (vec_t*)gc_alloc(sizeof(vec_t)+n*ds);
      
      v->u.vec->n = n;
      v->u.vec->w = ds;
      v->u.vec->type = typ;

    }else if (typ->vart == VART_LST){

      v->type = typ;
      v->u.lst = NULL;

    }else if (typ->vart == VART_ARR){
      v->type = typ;
      v->u.arr = NULL;
    }else if (typ->vart == VART_TUP){

      v->type = typ;
      // print_type(typ);
      int n = 0;
      list_node_t* p = typ->u.elem.head;
      while (p){
        type_t* t = (type_t*)(p->data);
        n += type_size(t);
        p = p->next;
      }

      v->u.tup = gc_alloc(sizeof(tup_t)+n);
      v->u.tup->type = typ;

      // list_add(&_G.objs,(obj_t*)(v->u.tup));

    }else if (typ->vart == VART_DIC){

      v->type = typ;
    }else if (typ->vart == VART_FUN){

      v->type = typ;
      v->u.fun = gc_alloc(sizeof(fun_t));
      v->u.fun->type = typ;
      list_init(&(v->u.fun->captr));

    }else if (typ->vart == VART_UON){
      v->type = typ;
      v->u.uon = gc_alloc(sizeof(uon_t));
      v->u.uon->type = typ;
      v->u.uon->var = NULL;
    }else{
      UNIMPL
    }
  }
  return v;
}


var_t* var_new_alloc(type_t* typ,int cnt){
  type_t* b = typ;

  if (b->mode == TYPM_SIMP){
    lost_t* lost = (lost_t*)map_get(&_G.layouts, &(b->u.str));
    
    obj_t* obj = (obj_t*)gc_alloc(sizeof(obj_t));

    obj->data = calloc(lost->size,1);
  
    obj->flag = 0;
    obj->type = b;

    var_t* v = var_new( typ );
    v->type = b;
    v->u.obj = obj;
    return v;
  }else{

    var_t* v = (var_t*)calloc(1,sizeof(var_t));

    if (typ->vart == VART_LST){
      
      v->type = typ;

      lst_t* arr = (lst_t*)gc_alloc(sizeof(lst_t));

      type_t* ta = (type_t*)(typ->u.elem.head->data);

      arr->n = cnt;

      arr->cap = (cnt+1)*2;

      arr->w = type_size(ta);
      arr->type = typ;

      arr->data = calloc(arr->w,arr->cap);

      v->u.lst = arr;

    }else if (typ->vart == VART_ARR){
      int is2d = cnt & (1<<30);
      int n = cnt;
      int d0 = cnt;
      int d1 = 1;
      if (is2d){
        d0 = ((cnt >> 15) & 0x7fff);
        d1 = (cnt & 0x7fff);
        n = d0 * d1;
      }
      int ndim = atoi(((type_t*)(typ->u.elem.tail->data))->u.str.data);

      v->type = typ;
      arr_t* arr = (arr_t*)gc_alloc(sizeof(arr_t)+ndim*sizeof(int));
      type_t* ta = (type_t*)(typ->u.elem.head->data);
      arr->n = n;
      arr->w = type_size(ta);
      arr->type = typ;
      arr->data = calloc(arr->w,arr->n);
      arr->ndim = ndim;
      
      for (int i = 0; i < ndim; i++){
        if (i == 0){
          arr->dims[i] = d0;
        }else if (i == 1){
          arr->dims[i] = d1;
        }else{
          arr->dims[i] = 1;
        }
      }
      v->u.arr= arr;

    }else if (typ->vart == VART_DIC){
      v->type = typ;
      dic_t* dic = (dic_t*)gc_alloc(sizeof(dic_t));

      dic->type = typ;
      // type_t* ta = (type_t*)(typ->u.elem.head->data);
      // type_t* tb = (type_t*)(typ->u.elem.tail->data);

      v->u.dic = dic;
    }
    return v;
  }
}

void var_assign(var_t* v, term_t* b){
  if (v->type->vart < VART_F32){
    v->u.u64 = get_val_int(b);
  }else if (v->type->vart == VART_F32){
    v->u.f32 = get_val_f32(b);
  }else if (v->type->vart == VART_F64){
    v->u.f64 = get_val_f64(b);
  }else if (v->type->vart == VART_STT){
    v->u.obj = get_ref_obj(b);
  }else if (v->type->vart == VART_STR){
    v->u.str = get_val_stn(b);
  }else if (v->type->vart == VART_VEC){
    v->u.vec = get_val_vec(b);
  }else if (v->type->vart == VART_LST){
    if (b->mode == TERM_IDEN){
      var_t* u = find_var(&(b->u.str));
      v->u.lst = u->u.lst;

    }else{
      UNIMPL;
    }
  }else if (v->type->vart == VART_ARR){
    if (b->mode == TERM_IDEN){
      var_t* u = find_var(&(b->u.str));
      v->u.arr = u->u.arr;
    }else{
      UNIMPL;
    }
  }else if (v->type->vart == VART_TUP){
    v->u.tup = get_val_tup(b);
  }else if (v->type->vart == VART_FUN){
    v->u.fun = get_ref_fun(b);
  }else if (v->type->vart == VART_DIC){
    if (b->mode == TERM_IDEN){
      var_t* u = find_var(&(b->u.str));
      v->u.dic = u->u.dic;
    }else if (b->mode == TERM_ADDR){
      dic_t* u = *((dic_t**)get_addr(b,NULL));
      v->u.dic = u;
    }else{
      UNIMPL;
    }
  }else if (v->type->vart == VART_UON){
    v->u.uon = get_val_uon(b);
  }else{
    UNIMPL
  }
}



void addr_assign(void* v, int nbytes, term_t* b){
  // printf("%p\n",v);
  if (b->mode == TERM_IDEN){
    
    var_t* u = find_var(&(b->u.str));
    char* x = (char*)(void*)(&(u->u));
    for (int i = 0; i < nbytes; i++){
      ((char*)v)[i] = x[i];
    }
  }else if (b->mode == TERM_NUMI || b->mode == TERM_NUMU){
    
    char* x = (char*)(void*)(&(b->u));
    for (int i = 0; i < nbytes; i++){
      
      ((char*)v)[i] = x[i];
    }
  }else if (b->mode == TERM_NUMF){
    
    if (nbytes == 4){
      float y = b->u.f;
      char* x = (char*)(&y);
      for (int i = 0; i < nbytes; i++){
        ((char*)v)[i] = x[i];
      }
    }else{
      UNIMPL
    }
  }else if (b->mode == TERM_STRL){
    stn_t* sp = get_val_stn(b);

    char x[8];
    // printf("%p %s\n",sp->type,sp->str.data);
    memcpy(x,(void*)(&sp),8);
    for (int i = 0; i < nbytes; i++){
      ((char*)v)[i] = x[i];
    }
    // stn_t* q = *((stn_t**)v);
    // printf("%p %p %p\n",q,sp,v);
    // print_type(q->type);
    // printf(" %s\n",q->str.data);
    // print_vars();
    // list_add(&_G.objs,(obj_t*)sp);
  }else if (b->mode == TERM_ADDR){
    
    int nb;
    char* x = get_addr(b,&nb);
    for (int i = 0; i < nbytes; i++){
      ((char*)v)[i] = x[i];
    }
  }else{
    UNIMPL
  }
}

void cast(term_t* a, term_t* b){
  int st0 = 0;
  int st1 = 0;
  int st2 = 0;

  var_t* v;
  if (a->mode == TERM_IDEN){
  
    v = find_var(&(a->u.str));

    if (v->type->vart < VART_F32){
      
      if (b->mode == TERM_NUMU){
        v->u.u64 = b->u.u;
      }else if (b->mode == TERM_IDEN){
        v->u.u64 = get_val_num(b);
      }else{
        UNIMPL
      }
    }else if (v->type->vart == VART_F32){

      if (b->mode == TERM_NUMU){
        v->u.f32 = (float)(b->u.u);
      }else if (b->mode == TERM_NUMI){
        v->u.f32 = (float)(b->u.i);
      }else if (b->mode == TERM_NUMF){
        v->u.f32 = (float)(b->u.f);
      }else if (b->mode == TERM_IDEN){
        
        // var_t* u = find_var(&(b->u.str));
        // if (u->type->vart == VART_I32){
        //   v->u.f32 = u->u.i32;
        // }else{
        //   printf("%d\n",u->type->vart);
        //   UNIMPL
        // }
        v->u.f32 = get_val_num(b);
      }else{
        UNIMPL
      }
    }else if (v->type->vart == VART_F64){
      if (b->mode == TERM_NUMU){
        v->u.f64 = (double)(b->u.u);
      }else if (b->mode == TERM_NUMI){
        v->u.f64 = (double)(b->u.i);
      }else if (b->mode == TERM_NUMF){
        v->u.f64 = (double)(b->u.f);
      }else{
        UNIMPL
      }
    }else if (v->type->vart == VART_STR){
      if (b->mode == TERM_IDEN){
        var_t* u = find_var(&(b->u.str));
        
        str_t s = str_new();
        to_str(u->type->vart, &(u->u), &s);

        stn_t* sp = gc_alloc(sizeof(stn_t)+s.len+1);
        sp->type = v->type;
        sp->n = s.len;
        sp->w = 1;
        memcpy(sp->data, s.data, s.len);

        free(s.data);
        
        v->u.str = sp;
      }else if ((st0=(b->mode == TERM_NUMU)) || (st1=(b->mode == TERM_NUMI)) || b->mode == TERM_NUMF){
        char s[32];
        if (st0){
          sprintf(s, "%" PRIu64, b->u.u);
        }else if (st1){
          sprintf(s, "%" PRId64, b->u.i);
        }else{
          sprintf(s, "%f", b->u.f);
        }
        int l = strlen(s);
        stn_t* sp = gc_alloc(sizeof(stn_t)+l+1);
        sp->type = v->type;
        sp->n = l;
        sp->w = 1;
        memcpy(sp->data, s, l);

        v->u.str = sp;
      }else{
        UNIMPL;
      }
    }else if (v->type->vart == VART_VEC){
      if (b->mode == TERM_IDEN){
        var_t* u = find_var(&(b->u.str));
        if (u->type->vart == VART_VEC){
          int vm = ((type_t*)(v->type->u.elem.head->data))->vart;
          int um = ((type_t*)(u->type->u.elem.head->data))->vart;

          if (vm == um){
            v->u.vec = u->u.vec;
          }else{
            for (int i = 0; i < v->u.vec->n; i++){
              double bv;
                   if (um == VART_U08) bv = ((uint8_t*) (u->u.vec->data))[i] ;
              else if (um == VART_I08) bv = (( int8_t*) (u->u.vec->data))[i] ;
              else if (um == VART_U16) bv = ((uint16_t*)(u->u.vec->data))[i];
              else if (um == VART_I16) bv = (( int16_t*)(u->u.vec->data))[i];
              else if (um == VART_U32) bv = ((uint32_t*)(u->u.vec->data))[i];
              else if (um == VART_I32) bv = (( int32_t*)(u->u.vec->data))[i];
              else if (um == VART_U64) bv = ((uint64_t*)(u->u.vec->data))[i];
              else if (um == VART_I64) bv = (( int64_t*)(u->u.vec->data))[i];
              else if (um == VART_F32) bv = ((float*)   (u->u.vec->data))[i];
              else if (um == VART_F64) bv = ((double*)  (u->u.vec->data))[i];    

                   if (vm == VART_U08) ((uint8_t*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_I08) (( int8_t*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_U16) ((uint16_t*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_I16) (( int16_t*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_U32) ((uint32_t*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_I32) (( int32_t*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_U64) ((uint64_t*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_I64) (( int64_t*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_F32) ((float*)(v->u.vec->data))[i] = bv;
              else if (vm == VART_F64) ((double*)(v->u.vec->data))[i] = bv;
            }
          }
        }else if (u->type->vart <= VART_F64){
          int vm = ((type_t*)(v->type->u.elem.head->data))->vart;
          int um = u->type->vart;

          for (int i = 0; i < v->u.vec->n; i++){
            double bv;
                 if (um == VART_U08) bv = u->u.u8;
            else if (um == VART_I08) bv = u->u.i8;
            else if (um == VART_U16) bv = u->u.u16;
            else if (um == VART_I16) bv = u->u.i16;
            else if (um == VART_U32) bv = u->u.u32;
            else if (um == VART_I32) bv = u->u.i32;
            else if (um == VART_U64) bv = u->u.u64;
            else if (um == VART_I64) bv = u->u.i64;
            else if (um == VART_F32) bv = u->u.f32;
            else if (um == VART_F64) bv = u->u.f64;    

                 if (vm == VART_U08) ((uint8_t*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_I08) (( int8_t*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_U16) ((uint16_t*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_I16) (( int16_t*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_U32) ((uint32_t*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_I32) (( int32_t*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_U64) ((uint64_t*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_I64) (( int64_t*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_F32) ((float*)(v->u.vec->data))[i] = bv;
            else if (vm == VART_F64) ((double*)(v->u.vec->data))[i] = bv;
          }
        }else if (u->type->vart == VART_UON){
          v->u.vec = vec_copy(u->u.uon->var->u.vec);
        }else{
          UNIMPL
        }
      }else if (b->mode == TERM_NUMU || b->mode == TERM_NUMF || b->mode == TERM_NUMI){
        double bv = get_val_num(b);
        int vm = ((type_t*)(v->type->u.elem.head->data))->vart;
             if (vm == VART_U08) for (int i = 0; i < v->u.vec->n; i++) ((uint8_t*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_I08) for (int i = 0; i < v->u.vec->n; i++) (( int8_t*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_U16) for (int i = 0; i < v->u.vec->n; i++) ((uint16_t*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_I16) for (int i = 0; i < v->u.vec->n; i++) (( int16_t*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_U32) for (int i = 0; i < v->u.vec->n; i++) ((uint32_t*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_I32) for (int i = 0; i < v->u.vec->n; i++) (( int32_t*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_U64) for (int i = 0; i < v->u.vec->n; i++) ((uint64_t*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_I64) for (int i = 0; i < v->u.vec->n; i++) (( int64_t*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_F32) for (int i = 0; i < v->u.vec->n; i++) ((float*)(v->u.vec->data))[i] = bv;
        else if (vm == VART_F64) for (int i = 0; i < v->u.vec->n; i++) ((double*)(v->u.vec->data))[i] = bv;
        
        
      }else{
        UNIMPL;
      }
    }else if (v->type->vart == VART_UON){
      if (v->u.uon->var == NULL){
        if (b->mode == TERM_IDEN){
          var_t* u = find_var(&(b->u.str));
          v->u.uon->var = var_new(u->type);
        }else if (b->mode == TERM_NUMU || b->mode == TERM_NUMI || b->mode == TERM_NUMF || b->mode == TERM_STRL){
          v->u.uon->var = (var_t*)calloc(1,sizeof(var_t));
          list_node_t* it = v->type->u.elem.head;
          while (it){
            type_t* t = (type_t*)(it->data);
            if (t->vart == VART_I32 && (b->mode == TERM_NUMI || b->mode == TERM_NUMU)){
              v->u.uon->var->type = t;
              break;
            }else if (t->vart == VART_F32 && b->mode == TERM_NUMF){
              v->u.uon->var->type = t;
              break;
            }else if (t->vart == VART_STR && b->mode == TERM_STRL){
              v->u.uon->var->type = t;
              break;
            }
            it = it->next;
          }
        }else{
          UNIMPL
        }
      }
      var_assign(v->u.uon->var, b);
    }else{
      if (b->mode == TERM_IDEN){
        var_t* u = find_var(&(b->u.str));
        if (u->type->vart == VART_UON){
          v->u.obj = u->u.uon->var->u.obj;
        }else{
          UNIMPL
        }
      }else{
        UNIMPL
      }
    }
  }else{
    UNIMPL
  }
}

double fadd(double a, double b){return a+b;}
double fsub(double a, double b){return a-b;}
double fmul(double a, double b){return a*b;}
double fdiv(double a, double b){return a/b;}

#define MAKE_VEC_MATH(NAME,OP,FF) \
  vec_t* vec_math_ ## NAME (vec_t* u, vec_t* v){ \
    vec_t* z = (vec_t*)gc_alloc(sizeof(vec_t)+u->n*u->w); \
    z->n = u->n; z->type = u->type; z->w = u->w; \
    type_t* t = ((type_t*)(u->type->u.elem.head->data)); \
    if ( t->mode == TYPM_NUMB ){ \
      int vm = t->vart; \
      for (int i = 0; i < z->n; i++){ \
        if (vm == VART_U08){ \
          ((uint8_t*)(z->data))[i] = ((uint8_t*)(u->data))[i] OP ((uint8_t*)(v->data))[i]; \
        }else if (vm == VART_U16){ \
          ((uint16_t*)(z->data))[i] = ((uint16_t*)(u->data))[i] OP ((uint16_t*)(v->data))[i]; \
        }else if (vm == VART_U32){ \
          ((uint32_t*)(z->data))[i] = ((uint32_t*)(u->data))[i] OP ((uint32_t*)(v->data))[i]; \
        }else if (vm == VART_U64){ \
          ((uint64_t*)(z->data))[i] = ((uint64_t*)(u->data))[i] OP ((uint64_t*)(v->data))[i]; \
        }else if (vm == VART_I08){ \
          ((int8_t*)(z->data))[i] = ((int8_t*)(u->data))[i] OP ((int8_t*)(v->data))[i]; \
        }else if (vm == VART_I16){ \
          ((int16_t*)(z->data))[i] = ((int16_t*)(u->data))[i] OP ((int16_t*)(v->data))[i]; \
        }else if (vm == VART_I32){ \
          ((int32_t*)(z->data))[i] = ((int32_t*)(u->data))[i] OP ((int32_t*)(v->data))[i]; \
        }else if (vm == VART_I64){ \
          ((int64_t*)(z->data))[i] = ((int64_t*)(u->data))[i] OP ((int64_t*)(v->data))[i]; \
        }else if (vm == VART_F32){ \
          ((float*)(z->data))[i] = FF ( ((float*)(u->data))[i] , ((float*)(v->data))[i] ); \
        }else if (vm == VART_F64){ \
          ((double*)(z->data))[i] = FF ( ((double*)(u->data))[i] , ((double*)(v->data))[i] ); \
        } \
      } \
    }else{ \
      UNIMPL; \
    } \
    return z; \
  }

MAKE_VEC_MATH(add,+,fadd)
MAKE_VEC_MATH(sub,-,fsub)
MAKE_VEC_MATH(mul,*,fmul)
MAKE_VEC_MATH(dvi,/,fdiv)
MAKE_VEC_MATH(mod,%,fmod)

int vec_eq(vec_t* u, vec_t* v){
  type_t* t = ((type_t*)(u->type->u.elem.head->data)); 
  int vm = t->vart;
  if (vm == VART_F32){ //-0 == 0
    for (int i = 0; i < v->n; i++){
      if (((float*)(v->data))[i] != ((float*)(u->data))[i]){
        return 0;
      }
    }
  }else if (vm == VART_F64){
    for (int i = 0; i < v->n; i++){
      if (((double*)(v->data))[i] != ((double*)(u->data))[i]){
        return 0;
      }
    }
  }else{
    for (int i = 0; i < v->n*v->w; i++){
      if (((char*)(v->data))[i] != ((char*)(u->data))[i]){
        return 0;
      }
    }
  }
  return 1;
}

#define VEC_MATMUL_CORE(T) {\
  for (int i = 0; i < nr; i++){ \
    for (int j = 0; j < nc; j++){ \
      ((T*)z->data)[ i*nc1+j ] = 0; \
      for (int k = 0; k < nc0; k++){ \
        ((T*)z->data)[ i*nc1+j ] += ((T*)u->data)[ i*nc0+k ] * ((T*)v->data)[ k*nc1+j ]; \
      } \
    } \
  } \
}

vec_t* vec_matmul(type_t* rt, vec_t* u, vec_t* v){
  type_t* t = ((type_t*)(u->type->u.elem.head->data));

  int nr0 = atoi(((type_t*)(u->type->u.elem.head->next->data))->u.str.data);
  int nc0 = 1;
  int nr1 = atoi(((type_t*)(v->type->u.elem.head->next->data))->u.str.data);
  int nc1 = 1;
  if (u->type->u.elem.len == 3){
    nc0 = atoi(((type_t*)(u->type->u.elem.tail->data))->u.str.data);
  }
  if (v->type->u.elem.len == 3){
    nc1 = atoi(((type_t*)(v->type->u.elem.tail->data))->u.str.data);
  }
  int vm = t->vart;
  int nr = nr0;
  int nc = nc1;

  vec_t* z = (vec_t*)gc_alloc(sizeof(vec_t)+(nr*nc)*u->w);
  z->n = nr*nc;
  z->w = u->w;
  z->type = rt;
       if (vm == VART_F32) VEC_MATMUL_CORE(float)
  else if (vm == VART_F64) VEC_MATMUL_CORE(double)
  else if (vm == VART_U08) VEC_MATMUL_CORE(uint8_t)
  else if (vm == VART_I08) VEC_MATMUL_CORE( int8_t)
  else if (vm == VART_U16) VEC_MATMUL_CORE(uint16_t)
  else if (vm == VART_I16) VEC_MATMUL_CORE( int16_t)
  else if (vm == VART_U32) VEC_MATMUL_CORE(uint32_t)
  else if (vm == VART_I32) VEC_MATMUL_CORE( int32_t)
  else if (vm == VART_U64) VEC_MATMUL_CORE(uint64_t)
  else if (vm == VART_I64) VEC_MATMUL_CORE( int64_t)
  else{
    UNIMPL;
  }
  return z;
}

// #define ARG_POP(ARGL, FIELD)\
//   ((var_t*)(ARGL->tail->data))->u.FIELD;\
//   free(ARGL->tail->data);\
//   list_pop(ARGL);\

#define ARG_POP(G, FIELD)\
 (((var_t*)ARR_POP(uintptr_t, G->args))->u.FIELD);\
 free((void*)(G->args.data[G->args.len]));

#define ARG_POP_VAR_NO_FREE(G)\
  (((var_t*)ARR_POP(uintptr_t, G->args)))

list_node_t* execute_instr(list_node_t* ins_node){
  map_t* frame = (map_t*)(_G.vars.tail->data);
  instr_t* ins = (instr_t*)((ins_node)->data);

  #if DBG
    begin_color(2);
    printf("%05lx ",((uintptr_t)ins_node)&0xfffff );
    end_color();
    print_instr(ins);
  #endif

  int st0 = 0;
  int st1 = 0;
  int st2 = 0;

  if (INSIS3("nop") || INSIS4("eoir")){
    
  }else if (INSIS4("bloc")){
    frame = frame_start();
  }else if (INSIS3("end")){
    frame = frame_end();
  }else if (INSIS3("jmp")){
    return ((label_t*)(ins->a))->ptr;

  }else if (INSIS4("decl")){
    var_t* v = var_new( (type_t*)(ins->b) );
    free(map_overwrite(frame, &(((term_t*)(ins->a))->u.str), v));
  }else if (INSIS3("mov")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    // printf("%d\n",a->mode);
    // printf("%d\n",b->mode);
    
    
    if (a->mode == TERM_IDEN){
      var_t* v;
      v = find_var(&(a->u.str));
      // printf("%p\n",v);
      var_assign(v, b);
    }else if (a->mode == TERM_ADDR){
      int nb;
      
      void* x = get_addr(a,&nb);
      // printf("ADR %p %d\n",x,nb);
      addr_assign(x,nb,b);

    }else{
      UNIMPL
    }

    // print_vars();
  }else if (INSIS4("dcap")){
    // print_vars();
    // term_t* a = ((term_t*)(ins->a));

    // var_t* v = var_new( (type_t*)(ins->b) );
    
    // var_assign(v, a);
    
    // free(map_overwrite(frame, &(((term_t*)(ins->a))->u.str), v));
    // print_vars();

  }else if (INSIS4("fpak")){
    var_t* v = (var_t*)calloc(1,sizeof(var_t));
    v->type = ((type_t*)(ins->b));
    v->u.fun = gc_alloc(sizeof(fun_t));
    v->u.fun->type = v->type;
    v->u.fun->ptr = ((label_t*)(ins->c))->ptr;
    list_init(&(v->u.fun->captr));

    free(map_overwrite(frame, &(((term_t*)(ins->a))->u.str), v));

  }else if (INSIS3("cap")){
    var_t* f = find_var(&(((term_t*)(ins->a))->u.str));

    str_t vname = (((term_t*)(ins->b))->u.str);
    // var_t* v = find_var(&vname);

    type_t* t = (type_t*)(ins->c);

    var_t* v = var_new(t);

    var_assign(v, (term_t*)(ins->b));

    pair_t* kv = (pair_t*)malloc(sizeof(pair_t));
    kv->key = malloc(vname.len+1);
    kv->key[vname.len]=0;
    memcpy(kv->key, vname.data, vname.len);
    kv->val = v;

    list_add(&(f->u.fun->captr), kv);

  }else if (INSIS3("add")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      
      if (v->type->vart < VART_F32){
        v->u.u64 = get_val_int(b)+get_val_int(c);
      }else if (v->type->vart == VART_F32){
        v->u.f32 = get_val_f32(b)+get_val_f32(c);
      }else if (v->type->vart == VART_F64){
        v->u.f64 = get_val_f64(b)+get_val_f64(c);
      }else if (v->type->vart == VART_VEC){
        v->u.vec = vec_math_add(get_ref_vec(b),get_ref_vec(c));
      }else if (v->type->vart == VART_STR){
        stn_t* s0 = get_val_stn(b);
        stn_t* s1 = get_val_stn(c);

        stn_t* sp = gc_alloc(sizeof(stn_t)+s0->n+s1->n+1);
        sp->type = v->type;
        sp->n = s0->n+s1->n;
        sp->w = 1;
        memcpy(sp->data, s0->data, s0->n);
        memcpy(sp->data + (s0->n), s1->data, s1->n);

        v->u.str = sp;
      }
    }
  }else if (INSIS3("mul")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      
      if (v->type->vart < VART_F32){
        v->u.u64 = get_val_int(b)*get_val_int(c);
      }else if (v->type->vart == VART_F32){
        v->u.f32 = get_val_f32(b)*get_val_f32(c);
      }else if (v->type->vart == VART_F64){
        v->u.f64 = get_val_f64(b)*get_val_f64(c);
      }else if (v->type->vart == VART_VEC){
        v->u.vec = vec_math_mul(get_ref_vec(b),get_ref_vec(c));
      }else{
        UNIMPL
      }
    }else{
      UNIMPL
    }
  }else if (INSIS3("sub")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      
      if (v->type->vart < VART_F32){
        v->u.u64 = get_val_int(b)-get_val_int(c);
      }else if (v->type->vart == VART_F32){
        v->u.f32 = get_val_f32(b)-get_val_f32(c);
      }else if (v->type->vart == VART_F64){
        v->u.f64 = get_val_f64(b)-get_val_f64(c);
      }else if (v->type->vart == VART_VEC){
        v->u.vec = vec_math_sub(get_ref_vec(b),get_ref_vec(c));
      }else{
        UNIMPL
      }
    }else{
      UNIMPL
    }
  }else if (INSIS3("mod")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      
      if (v->type->vart < VART_F32){
        uint64_t bv = get_val_int(b);
        uint64_t cv = get_val_int(c);
        if (v->type->vart % 2 == VART_U08){
          v->u.u64 = bv % cv;
        }else{
          int64_t vv = (* ((int64_t*)(&bv))) % (* ((int64_t*)(&cv)));
          v->u.i64 = vv;
        }
      }else if (v->type->vart == VART_F32){
        v->u.f32 = fmod(get_val_f32(b),get_val_f32(c));
      }else if (v->type->vart == VART_F64){
        v->u.f64 = fmod(get_val_f64(b),get_val_f64(c));
      }else{
        UNIMPL
      }
    }else{
      UNIMPL
    }
  }else if (INSIS3("div")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      
      if (v->type->vart < VART_F32){
        uint64_t bv = get_val_int(b);
        uint64_t cv = get_val_int(c);
        if (v->type->vart % 2 == VART_U08){
          v->u.u64 = bv / cv;
        }else{
          int64_t vv = (* ((int64_t*)(&bv))) / (* ((int64_t*)(&cv)));
          v->u.i64 = vv;
        }
      }else if (v->type->vart == VART_F32){
        v->u.f32 = get_val_f32(b)/get_val_f32(c);
      }else if (v->type->vart == VART_F64){
        v->u.f64 = get_val_f64(b)/get_val_f64(c);
      }else if (v->type->vart == VART_VEC){
        v->u.vec = vec_math_dvi(get_ref_vec(b),get_ref_vec(c));
      }else{
        UNIMPL
      }
    }else{
      UNIMPL
    }
  }else if (INSIS3("pow")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      
      if (v->type->vart < VART_F32){
        v->u.u64 = pow(get_val_int(b),get_val_int(c));
      }else if (v->type->vart == VART_F32){
        v->u.f32 = powf(get_val_f32(b),get_val_f32(c));
      }else if (v->type->vart == VART_F64){
        v->u.f64 = pow(get_val_f64(b),get_val_f64(c));
      }else if (v->type->vart == VART_VEC){
        UNIMPL
      }else{
        UNIMPL
      }
    }else{
      UNIMPL
    }
  }else if (INSIS6("matmul")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));

      v->u.vec = vec_matmul(v->type, get_ref_vec(b),get_ref_vec(c));
      
    }else{
      UNIMPL
    }
  }else if ((st0 = INSIS3("shl")) || (st1 = INSIS4("band")) || (st2 = INSIS3("bor")) || INSIS3("xor")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      if (st0){
        v->u.u64 = get_val_int(b)<<get_val_int(c);
      }else if (st1){
        v->u.u64 = get_val_int(b)&get_val_int(c);
      }else if (st2){
        v->u.u64 = get_val_int(b)|get_val_int(c);
      }else{
        v->u.u64 = get_val_int(b)^get_val_int(c);
      }
    }else{
      UNIMPL
    }
  }else if (INSIS3("shr")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      uint64_t bv = get_val_int(b);
      uint64_t cv = get_val_int(c);
      if ( (v->type->vart & 1) == VART_I08){
        v->u.i64 = (*((int64_t*)(&bv))) >> (*((int64_t*)(&cv)));
      }else{
        v->u.u64 = bv>>cv;
      }
    }else{
      UNIMPL
    }
  }else if (INSIS4("lnot")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      v->u.u64 = !get_val_int(b);
    }else{
      UNIMPL
    }
  }else if (INSIS4("bnot")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      v->u.u64 = ~get_val_int(b);
    }else{
      UNIMPL
    }
  }else if (INSIS2("lt") || (st0 = INSIS2("gt")) || (st1 = INSIS3("geq")) || (st2 = INSIS3("leq"))){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      double bv = get_val_num(b);
      double cv = get_val_num(c);
      if (st0){
        v->u.i32 = bv>cv;
      }else if (st1){
        v->u.i32 = bv>=cv;
      }else if (st2){
        v->u.i32 = bv<=cv;
      }else{
        v->u.i32 = bv<cv;
      }
    }else{
      UNIMPL
    }

  }else if (INSIS2("eq") || (st0 = INSIS3("neq"))){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    term_t* c = ((term_t*)(ins->c));

    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      var_t* u = NULL;
      if (b->mode == TERM_IDEN){
        u = find_var(&(b->u.str));
      }else if (c->mode == TERM_IDEN){
        u = find_var(&(c->u.str));
      }
      if (u && u->type->vart == VART_VEC){
        v->u.i32 = vec_eq(get_ref_vec(b),get_ref_vec(c));
      }else{
        v->u.i32 = get_val_num(b)==get_val_num(c);
      }
      if (st0){
        v->u.i32 = 1 - (v->u.i32);
      }
    }else{
      UNIMPL
    }
  }else if (INSIS4("utag")){
    term_t* a = ((term_t*)(ins->a));
    term_t* b = ((term_t*)(ins->b));
    type_t* typ = (type_t*)(ins->c);
    if (a->mode == TERM_IDEN && b->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      var_t* u = find_var(&(b->u.str));
      v->u.i32 = type_eq(u->u.uon->var->type, typ);
    }else{
      UNIMPL
    }

  }else if (INSIS4("argw")){
    term_t* a = ((term_t*)(ins->a));
    type_t* typ = ((type_t*)(ins->b));
    
    var_t* v = var_new(typ);
    // print_vars();
    var_assign(v,a);
    // print_vars();
    // list_add(args,v);
    // printf("+ %d\n",args->len);
    ARR_PUSH(uintptr_t, _G.args, v);

  }else if (INSIS4("argr")){
    term_t* a = ((term_t*)(ins->a));
    type_t* typ = ((type_t*)(ins->b));
    // var_t* v = (var_t*)(args->tail->data);
    // list_pop(args);
    var_t* v = (var_t*)ARR_POP(uintptr_t, _G.args);
    free(map_overwrite(frame, &(a->u.str), v));

    // printf("- %d\n",_G.args.len);
  }else if (INSIS4("call")){

    retp_t r;
    r.addr = ins_node;
    r.frame = frame;
    r.v = find_var(&(((term_t*)(ins->a))->u.str));
    // list_add(ret_pts,r);
    ARR_PUSH(retp_t, _G.ret_pts, r);

    frame = frame_start();

    return ((label_t*)(ins->b))->ptr;

  }else if (INSIS5("rcall")){

    retp_t r;// = (retp_t*)calloc(1,sizeof(retp_t));
    r.addr = ins_node;
    r.frame = frame;
    r.v = find_var(&(((term_t*)(ins->a))->u.str));
    // list_add(ret_pts,r);
    ARR_PUSH(retp_t, _G.ret_pts, r);

    fun_t* f = get_ref_fun((term_t*)(ins->b));
    
    void* n = f->ptr;
    frame = frame_start();

    list_node_t* q = f->captr.head;
    while (q){
      pair_t* kv = (pair_t*)(q->data);
      var_t* v = (var_t*)(kv->val);

      var_t* u = var_new(v->type);
      u->u = v->u;
      // printf("%f\n",v->u.f32);
      str_t t;
      t.len = strlen(kv->key);
      t.data = kv->key;
      free(map_overwrite(frame, &t, u));
      q = q->next;
    }
    return n;

  }else if (INSIS3("ret")){
    // retp_t* r = (retp_t*)(ret_pts->tail->data);
    retp_t r = ARR_POP(retp_t, _G.ret_pts);

    if (ins->a){
      
      term_t* a = ((term_t*)(ins->a));
      var_assign(r.v,a);
      
    }
    list_node_t* n = r.addr;
    
    while (frame != r.frame){
      frame = frame_end();
    }
    
    return n->next;

  }else if (INSIS4("jeqz")){
    term_t* a = ((term_t*)(ins->a));

    int pre = 0;
    if (a->mode == TERM_IDEN){
      var_t* v = find_var(&(a->u.str));
      if (v->type->vart < VART_F32){
        int gvi = get_val_int(a);
        pre = gvi ==0;
      }else if (v->type->vart == VART_F32){
        pre = get_val_f32(a)==0;
      }else if (v->type->vart == VART_F64){
        pre = get_val_f64(a)==0;
      }
    }else if (a->mode == TERM_NUMI || a->mode == TERM_NUMU){
      pre = get_val_int(a)==0;
    }else if (a->mode == TERM_NUMF){
      pre = get_val_f64(a)==0;
    }
    // printf("pred: %d\n",pre);

    if (pre){
      return ((label_t*)(ins->b))->ptr;
    }

  }else if (INSIS5("alloc")){
    type_t* b = (type_t*)(ins->b);

    int c = ((term_t*)(ins->c))->u.u;

    var_t* v = var_new_alloc(b,c);
    
    free(map_overwrite(frame, &(((term_t*)(ins->a))->u.str), v));
    
    // print_vars();
  }else if (INSIS4("cast")){

    cast((term_t*)(ins->a),(term_t*)(ins->b));
    // print_vars();

  }else if (INSIS4("incl")){
    char* name = ((term_t*)(ins->a))->u.str.data;
    char* lid = strrchr(name, '/');
    if (!lid){
      lid=name;
    }else{
      lid++;
    }
    int l = strlen(name);
    char *nname;
    if (name[l-1] == 'o' && name[l-2] == 's' && name[l-3] == '.'){ 
      nname = name;
    }else{
      nname = malloc(l+16);
      strcpy(nname,name);
      strcpy(nname+l,"/dynamic.so");
    }
    
    char buf[32] = "lib_init_";
    int i = 9;
    while (lid[0] != '.' && i < 31 && lid[0] != 0){
      char c = (*(lid++));
      buf[i++] = (('a' <= c && c <= 'z') || ('0' <= c && c <= '9')) ? c : '_';
    }
    buf[i] = 0;

    void* handle = dlopen(nname, RTLD_LAZY);
    if (nname != name){
      free(nname);
    }
    if (!handle){
      printf("%s\n",dlerror());
    }

    void ((*l_init)(gstate_t*)) = (void ((*)(gstate_t*)))dlsym(handle, buf);
    
    // printf("%s %s %p %p\n",name, buf, handle, l_init);

    // printf("%p %p\n",&_G,&(_G.cfuncs));

    l_init(&_G);


  }else if (INSIS5("ccall")){
      
    var_t* v = find_var(&(((term_t*)(ins->a))->u.str));

    str_t name = ((term_t*)(ins->b))->u.str;

    void (*cfun)(var_t* ret, gstate_t* _g) = (void (*)(var_t*, gstate_t*))map_get(&_G.cfuncs, &name);
    // printf("%p %s %d %p\n",&_G.cfuncs,name.data,_G.args.len,cfun);
    
    // print_vars();
    cfun(v,&_G);
    // printf(".\n");
  }else{
    UNIMPL;
  }

  if (_G.flags & GFLG_TRGC){
    _G.flags &= ~GFLG_TRGC;
    gc_run();
  }

  return ins_node->next;
}

void execute(list_t* instrs){
  map_t* frame = frame_start();

  list_node_t* n = instrs->head;
  while (n){
    n = execute_instr(n);
  }

  frame = frame_end();
  if (!(_G.flags & GFLG_NOGC)){
    gc_run();
  }
  
#if DBG
  printf("RMN %d\n",_G.objs.len);
#endif
}


void register_cfunc(map_t* g_cfuncs, char* name, void (*fun)(var_t* ret, gstate_t* _g)){
  // no_segfault_yet
  str_t s;
  s.len = strlen(name);
  s.cap = s.len;
  s.data = name;
  #if DBG
  printf("registering: %s %p\n",name,g_cfuncs);
  #endif
  // free(
    map_overwrite(g_cfuncs, &s, (void*)fun)
  // )
  ;
  
}


void free_opran(opran_t* a){
  uint32_t tag = a->tag;  
  if (tag == OPRD_TERM){
    if (((term_t*)a)->mode == TERM_IDEN || ((term_t*)a)->mode == TERM_STRL){
      free(((term_t*)a)->u.str.data);
    }else if (((term_t*)a)->mode == TERM_ADDR){
      free(((term_t*)a)->u.addr.base.data);
      free(((term_t*)a)->u.addr.offs.data);
    }
    free(a);
  }else if (tag == OPRD_TYPE){
    free_type((type_t*)a);
  }else if (tag == OPRD_LABL){
    free(((label_t*)a)->str.data);
    free(a);
  }else{
    free(a);
  }
}

void free_instrs(list_t* instrs){
  list_node_t* it = instrs->head;
  while (it){
    list_node_t* next = it->next;
    instr_t* ins = it->data;
    if (ins->a){
      free_opran(ins->a);
    }
    if (ins->b){
      free_opran(ins->b);
    }
    if (ins->c){
      free_opran(ins->c);
    }
    if (ins->loc){
      free(ins->loc);
    }
    free(ins);
    free(it);
    it = next;
  }
  memset(instrs,0,sizeof(list_t));
}

void global_exit(){
  // uint64_t flags;
  // list_t vars;
  // map_t layouts;
  // mem_list_t objs;
  // map_t cfuncs;
  // retp_t_arr_t ret_pts;
  // uintptr_t_arr_t args;

  while (_G.vars.len){
    frame_end();
  }
  gc_sweep();

  map_t* m = &(_G.layouts);
  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    for (int i = 0; i < m->slots[k].len;i++){
      pair_t p = m->slots[k].data[i];
      lost_t* layout = (lost_t*)p.val;
      list_node_t* it = layout->fields.head;
      while(it){
        list_node_t* next = it->next;
        lofd_t* l = (lofd_t*)(it->data);
        free(l->name.data);
        free_type(l->type);
        free(it);
        it = next;
      }
    }
  }
  map_nuke(m);

  m = &(_G.cfuncs);
  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    if (m->slots[k].cap){
      for (int i = 0; i < m->slots[k].len;i++){
        pair_t p = m->slots[k].data[i];
        free(p.key);
      }
      free(m->slots[k].data);
    }
  }

  free(_G.ret_pts.data);
  free(_G.args.data);

  memset(&_G,0,sizeof(_G));
}


#endif


