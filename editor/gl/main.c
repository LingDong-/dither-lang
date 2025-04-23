#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
// #include <time.h>

#include "../../src/interp.c"
#include "../../std/win/platform/windowing.h"

char* keywords[] = {
  "namespace","continue","typedef","include","return","break","while","const",
  "else","func","func","dict","list",
  "for","i16","u16","i32","u32","i64","u64","f32","f64","tup","vec","arr","str",
  "if","do","as","i8","u8"
};

char* asm_keywords[] = {
  "matmul","ccall","rcall",
  "cast","bnot","lnot","decl","argr","argw","alloc","call","jeqz","fpak","incl","band","bloc","dcap","eoir",
  "mov","cap","ret","jmp","add","sub","mul","div","mod","pow","shl","shr","bor","xor","geq","leq","eq","neq","nop","end",
  "gt","lt",
  "I08","U08","I16","U16","I32","U32","I64","U64","F32","F64","TUP","LST","VEC","ARR","DIC","STR","FUN","NUL","VOD",
};

#define FONT_W 8
#define FONT_H 16
#define FONT_COLS 32
#define FONT_ROWS 8
#define FONT_N 256
#define FONT_TEX_W (FONT_COLS * FONT_W)
#define FONT_TEX_H (FONT_ROWS * FONT_H)
#define FONT_TEX_HH (FONT_TEX_H*2)

#define WIN_COLS 160
#define WIN_ROWS 50

#define WIN_W (FONT_W*WIN_COLS)
#define WIN_H (FONT_H*WIN_ROWS)

#define LIN_COLS 4
#define EDT_COLS 60
#define ASM_COLS 48
#define VAR_COLS 48
#define USR_COLS 48
#define USR_ROWS 24

#define TOP_ROWS 1
#define MID_ROWS 40
#define BOT_ROWS 1
#define OUT_ROWS 8

#define HL_COMMENT 0x6d6daa
#define HL_KEYWORD 0xff6dff
#define HL_SIGIL   0xffdb00
#define HL_NUMBER  0x6ddb55
#define HL_STRING  0xdb6d55
#define HL_BASE    0xdbffff
#define HL_FUNC    0x00dbff
#define HL_ADDR    0xb64955


#include "font_bitmap.c"

uint8_t cons[WIN_H][WIN_W] = {0};
int cons_rgb[WIN_H][WIN_W] = {0};

GLuint font_texture;
GLuint text_xy_vbo;
GLuint text_uv_vbo;
GLuint text_rg_vbo;

float text_xy[WIN_COLS*WIN_ROWS*8];
float text_uv[WIN_COLS*WIN_ROWS*8];
float text_rg[WIN_COLS*WIN_ROWS*12];

void build_font_texture() {
  unsigned char tex_data[FONT_TEX_HH][FONT_TEX_W];
  memset(tex_data, 0, FONT_TEX_W*FONT_TEX_H);
  memset(tex_data+FONT_TEX_H, 0xff, FONT_TEX_W*FONT_TEX_H);
  for (int ch = 0; ch < FONT_N; ++ch) {
    int gx = (ch % FONT_COLS) * FONT_W;
    int gy = (ch / FONT_COLS) * FONT_H;
    for (int row = 0; row < FONT_H; ++row) {
      unsigned char bits = font_bitmap[ch*FONT_H+row];
      for (int col = 0; col < FONT_W; ++col) {
        if (bits & (1 << (7 - col))) {
          tex_data[gy + row][gx + col] = 0xff;
          tex_data[gy + row + FONT_TEX_H][gx + col] = 0;
        }
      }
    }
  }
  // for (int i = 0; i < FONT_TEX_HH; i++){
  //   for (int j = 0; j < FONT_TEX_W; j++){
  //     printf("%c",tex_data[i][j]?'#':'.');
  //   }
  //   printf("\n");
  // }

  glGenTextures(1, &font_texture);
  glBindTexture(GL_TEXTURE_2D, font_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, FONT_TEX_W, FONT_TEX_HH, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tex_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void build_text_buffer() {
  glGenBuffers(1, &text_xy_vbo);
  glGenBuffers(1, &text_uv_vbo);
  glGenBuffers(1, &text_rg_vbo);
  int idx = 0;
  for (int y = 0; y < WIN_ROWS; ++y) {
    for (int x = 0; x < WIN_COLS; ++x) {
      float px = x * FONT_W;
      float py = y * FONT_H;
      text_xy[idx++] = px;           text_xy[idx++] = py;
      text_xy[idx++] = px + FONT_W;  text_xy[idx++] = py;
      text_xy[idx++] = px + FONT_W;  text_xy[idx++] = py + FONT_H;
      text_xy[idx++] = px;           text_xy[idx++] = py + FONT_H;
    }
  }
  glBindBuffer(GL_ARRAY_BUFFER, text_xy_vbo);
  glBufferData(GL_ARRAY_BUFFER, idx * sizeof(float), text_xy, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, text_uv_vbo);
  glBufferData(GL_ARRAY_BUFFER, WIN_COLS*WIN_ROWS * 8 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, text_rg_vbo);
  glBufferData(GL_ARRAY_BUFFER, WIN_COLS*WIN_ROWS * 12 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render_text() {
  int uv_idx = 0;
  int rg_idx = 0;

  for (int i = 0; i < WIN_ROWS; ++i) {
    for (int j = 0; j < WIN_COLS; ++j){
      int ch = cons[i][j];
      int inv = !!(cons_rgb[i][j]>>24);

      float r = ((cons_rgb[i][j]>>16)&0xff)/255.0;
      float g = ((cons_rgb[i][j]>>8) &0xff)/255.0;
      float b = ((cons_rgb[i][j])    &0xff)/255.0;

      int cx = ch % FONT_COLS;
      int cy = ch / FONT_COLS + inv*FONT_ROWS;

      float u0 = cx * (FONT_W / (float)FONT_TEX_W);
      float v0 = cy * (FONT_H / (float)FONT_TEX_HH);
      float u1 = u0 + (FONT_W / (float)FONT_TEX_W);
      float v1 = v0 + (FONT_H / (float)FONT_TEX_HH);

      text_uv[uv_idx++] = u0; text_uv[uv_idx++] = v0;
      text_uv[uv_idx++] = u1; text_uv[uv_idx++] = v0;
      text_uv[uv_idx++] = u1; text_uv[uv_idx++] = v1;
      text_uv[uv_idx++] = u0; text_uv[uv_idx++] = v1;

      for (int k = 0; k < 4; k++){
        text_rg[rg_idx++] = r;
        text_rg[rg_idx++] = g;
        text_rg[rg_idx++] = b;
      }
    }
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, text_uv_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, uv_idx*sizeof(float), text_uv);

  glBindBuffer(GL_ARRAY_BUFFER, text_rg_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, rg_idx * sizeof(float), text_rg);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glBindBuffer(GL_ARRAY_BUFFER, text_xy_vbo);
  glVertexPointer(2, GL_FLOAT, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, text_uv_vbo);
  glTexCoordPointer(2, GL_FLOAT, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, text_rg_vbo);
  glColorPointer(3, GL_FLOAT, 0, 0);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, font_texture);
  glDrawArrays(GL_QUADS, 0, WIN_COLS*WIN_ROWS*4);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisable(GL_TEXTURE_2D);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, h, 0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

char** read_file_lines(FILE* fd, char** lines, int* out_num_lines) {
  int lines_cap = *out_num_lines;
  if (!lines){
    lines_cap = 64;
    lines = malloc(lines_cap * sizeof(char*));
  }
  int num_lines = *out_num_lines;
  while (!feof(fd)) {
    int line_cap = 16;
    int line_len = 0;
    char* line = malloc(line_cap);
    int c;
    while ((c = fgetc(fd)) != EOF) {
      if (c == '\n') break;
      if (line_len+1 >= line_cap) {
        line_cap = line_cap*2+1;
        line = realloc(line, line_cap);
      }
      line[line_len++] = (char)c; 
    }
    if (line_len == 0 && c == EOF) {
      break;
    }
    line[line_len] = 0;
    if (num_lines >= lines_cap) {
      lines_cap = lines_cap*2+1;
      lines = realloc(lines, lines_cap * sizeof(char*));
    }
    lines[num_lines++] = line;
    
  }
  *out_num_lines = num_lines;
  return lines;
}

void write_lines_file(FILE* fd, char** lines, int n_lines){
  for (int i = 0; i < n_lines; i++){
    fputs(lines[i],fd);
    fputc('\n',fd);
  }
}


typedef struct{
  int left;
  int top;
  int right;
  int bottom;
  int roll_x;
  int roll_y;
  int n_lines;
  char** lines;
} textarea_t;

textarea_t ta_edt = {LIN_COLS,TOP_ROWS,LIN_COLS+EDT_COLS-1,TOP_ROWS+MID_ROWS};
textarea_t ta_asm = {LIN_COLS+EDT_COLS,TOP_ROWS,LIN_COLS+EDT_COLS+ASM_COLS-1,TOP_ROWS+MID_ROWS};
textarea_t ta_out = {0,TOP_ROWS+MID_ROWS+BOT_ROWS,WIN_COLS-1,WIN_ROWS};

int cur_x = 0;
int cur_y = 0;

int mouse_x = 0;
int mouse_y = 0;

int in_textarea(textarea_t* ta, int x, int y){
  return ta->left <= x && x < ta->right && ta->top <= y && y < ta->bottom;
}

void scroll_textarea(textarea_t* ta, int x, int y){
  ta->roll_y -= y;
  if (ta->roll_y > ta->n_lines - (ta->bottom-ta->top)) ta->roll_y = ta->n_lines-(ta->bottom-ta->top);
  if (ta->roll_y < 0) ta->roll_y = 0;
  ta->roll_x -= x;
  if (x < 0){
    int maxlen = 0;
    for (int i = 0; i < ta->n_lines; i++){
      maxlen = MAX(maxlen,strlen(ta->lines[i]));
    }
    if (ta->roll_x > maxlen-(ta->right-ta->left)) ta->roll_x = maxlen-(ta->right-ta->left);
  }
  if (ta->roll_x < 0) ta->roll_x = 0;
}

#define IS_SIGIL(c) ((c)<'0' || ((c)>'9'&&(c)<'A') || ((c)>'Z'&&(c)<'_') || (c)>'z')


void draw_textarea(textarea_t* ta, char** keywords, int n_keywords, int plain){
  int blcm = 0;
  for (int i = ta->top; i < ta->bottom; i++){
    for (int j = ta->left; j < ta->right; j++){
      cons_rgb[i][j] = 0;
    }
  }
  for (int i = 0; i < ta->bottom-ta->top; i++){
    int ty = ta->roll_y+i;
    int cy = ta->top+i;
    if (ty >= ta->n_lines) continue;
    int len = strlen(ta->lines[ty]);

    for (int j = 0; j < ta->right-ta->left; j++){
      int tx = ta->roll_x+j;
      int cx = ta->left + j;
      if (tx > len) continue;
      char c = ta->lines[ty][tx];
      cons[cy][cx] = c;

      if (plain){
        cons_rgb[cy][cx] = HL_BASE;
      }else{
        if (cons_rgb[cy][cx] == 0){
          if (blcm){
            cons_rgb[cy][cx] = HL_COMMENT;
            if (c == '*' && ta->lines[ty][tx+1] == '/'){
              cons_rgb[cy][cx+1] = HL_COMMENT;
              blcm = 0;
            }
          }else if (c == '/' && ta->lines[ty][tx+1] == '*'){
            blcm = 1;
            cons_rgb[cy][cx] = HL_COMMENT;
          }else if (c == '/' && ta->lines[ty][tx+1] == '/'){
            int q = 0;
            do{
              cons_rgb[cy][cx+q] = HL_COMMENT;
            }while (ta->lines[ty][tx+(q++)]);
          }else if(c == '"'){
            int q = 0;
            int esc = 0;
            do{
              cons_rgb[cy][cx+q] = HL_STRING;
              if (ta->lines[ty][tx+q] == '\\'){
                esc = 1;
              }else{
                if (q && ta->lines[ty][tx+q] == '"' && !esc){
                  break;
                }
                esc = 0;
              }
            }while (ta->lines[ty][tx+(q++)]);
          }else if(c == '\''){
            int q = 0;
            int esc = 0;
            do{
              cons_rgb[cy][cx+q] = HL_STRING;
              if (ta->lines[ty][tx+q] == '\\'){
                esc = 1;
              }else{
                if (q && ta->lines[ty][tx+q] == '\'' && !esc){
                  break;
                }
                esc = 0;
              }
            }while (ta->lines[ty][tx+(q++)]);
          }else if (c == '@'){
            int q = 0;
            cons_rgb[cy][cx] = HL_ADDR;
            do{
              if (q && IS_SIGIL(ta->lines[ty][tx+q])){
                break;
              }
              cons_rgb[cy][cx+q] = HL_ADDR;
            }while (ta->lines[ty][tx+(q++)]);
          }else if ('0'<=c && c<='9'){
            if (!tx || IS_SIGIL(ta->lines[ty][tx-1])){
              int q = 0;
              do{
                
                int b = ta->lines[ty][tx+q];
                if (
                    b == 'x' || b == 'X' || b == 'b' || b == 'B' || b == 'e' || b == 'E' ||
                    
                    // ('0'<=b && b<='9') ||
                    !IS_SIGIL(b) ||
                    b == '.' || b == '+' || b == '-'
                    
                    ){
                }else{
                  cons_rgb[cy][cx+q] = 0;
                  break;
                }
                cons_rgb[cy][cx+q] = HL_NUMBER;
              }while (ta->lines[ty][tx+(q++)]);
            }else{
              cons_rgb[cy][cx] = HL_BASE;
            }
          }else if (c == '(' && tx > 0){
            cons_rgb[cy][cx] = HL_SIGIL;
            int q = 0;
            int sp = 0;
            while (tx+(--q) >= 0){
              int b = ta->lines[ty][tx+q];
              if (b == ' '){
                if (q == -1 || sp){
                  sp = 1;
                }else{
                  break;
                }
              }else{
                sp = 0;
                if ((cons_rgb[cy][cx+q]&0xffffff) != HL_BASE){
                  break;
                }
                if (IS_SIGIL(b)){
                  break;
                }
              }
              cons_rgb[cy][cx+q] = (cons_rgb[cy][cx+q]&0xff000000) | HL_FUNC;
            }
          }else if (IS_SIGIL(c)){
            cons_rgb[cy][cx] = HL_SIGIL;
          }else{ 
            
            for (int k = 0; k < n_keywords; k++){
              int ok = 1;
              int l = strlen(keywords[k]);
              for (int q = 0; q < l; q++){
                int a = keywords[k][q];
                int b = ta->lines[ty][tx+q];
                if (a != b){
                  ok = 0;
                  break;
                }
              }
              if (ok){
                int b = ta->lines[ty][tx+l];
                if (! IS_SIGIL(b)){
                  ok = 0;
                }
                if (tx > 0){
                  b = ta->lines[ty][tx-1];
                  if (!IS_SIGIL(b)){
                    ok = 0;
                  }
                }
              }
              if (ok){
                for (int q = 0; q < l; q++){
                  cons_rgb[cy][cx+q] = HL_KEYWORD;
                }
                break;
              }
            }
            if (cons_rgb[cy][cx] == 0){
              cons_rgb[cy][cx] = HL_BASE;
            }
          }
        }
      }
    }
  }
  float pct_top = (float)ta->roll_y / (float) ta->n_lines;
  float pct_bot = fmin(1,(float)(ta->roll_y+(ta->bottom-ta->top)) / (float) ta->n_lines);
  int i_top = ta->top + (ta->bottom-ta->top) * pct_top;
  int i_bot = ta->top + (ta->bottom-ta->top) * pct_bot;
  if (i_bot == i_top) i_bot++;
  if (i_bot > ta->bottom){
    i_top--;
    i_bot--;
  }
  for (int i = ta->top; i < ta->bottom; i++){
    int y = i-ta->top;
    if (i_top <= i && i < i_bot){
      cons[i][ta->right]= 178;
      cons_rgb[i][ta->right] = HL_BASE;
    }else{
      cons[i][ta->right]= 176;
      cons_rgb[i][ta->right] = HL_COMMENT;
    }
    
  }
}

char** put_line(char** lines, int* n_lines, char* line){
  line = strdup(line);
  if (lines){
    lines = realloc(lines,(*n_lines+1)*sizeof(char**));
  }else{
    lines = malloc((*n_lines+1)*sizeof(char**));
  }
  lines[*n_lines] = line;
  *n_lines = *n_lines+1;
  return lines;
}

list_t instrs;
uintptr_t* line_to_instr = NULL;

list_node_t* ins_node = NULL;

int sprint_type(char* str, type_t* a){
  if (!a) return 0;
  if (a->mode == TYPM_VOID){
    return sprintf(str, "VOD");
  }else if (a->mode == TYPM_NUMB){
    return sprintf(str,"%.03s",vart_names + (a->vart*3));
  }else if (a->mode == TYPM_SIMP){
    if (a->vart == VART_STR){
      return sprintf(str,"%.03s",vart_names + (a->vart*3));
    }else{
      return sprintf(str,"%s",a->u.str.data);
    }
  }else if (a->mode == TYPM_CONT){
    int cnt = sprintf(str,"%.03s[",vart_names + (a->vart*3));
    list_node_t* n = a->u.elem.head;
    while (n){
      type_t* t = (type_t*)n->data;
      cnt += sprint_type(str+cnt,t);
      n = n->next;
      if (n){
        cnt += sprintf(str+cnt,",");
      }
    }
    cnt += sprintf(str+cnt,"]");
    return cnt;
  }else{
    return 0;
  }
}

int sprint_opran(char* str, opran_t* a){
  uint32_t tag = a->tag;
  if (tag == OPRD_TERM){
    if (((term_t*)a)->mode == TERM_IDEN){
      return sprintf(str,"%s",((term_t*)a)->u.str.data);
    }else if (((term_t*)a)->mode == TERM_STRL){
      return sprintf(str,"\"%s\"",((term_t*)a)->u.str.data);
    }else if (((term_t*)a)->mode == TERM_ADDR){
      return sprintf(str,"%s+%s",((term_t*)a)->u.addr.base.data,((term_t*)a)->u.addr.offs.data);
    }else if (((term_t*)a)->mode == TERM_NUMI){
      return sprintf(str,"%" PRId64,((term_t*)a)->u.i);
    }else if (((term_t*)a)->mode == TERM_NUMU){
      return sprintf(str,"%" PRIu64,((term_t*)a)->u.u);
    }else if (((term_t*)a)->mode == TERM_NUMF){
      return sprintf(str,"%f",((term_t*)a)->u.f);
    }
  }else if (tag == OPRD_TYPE){
    return sprint_type(str,(type_t*)a);
  }else if (tag == OPRD_LABL){
    return sprintf(str,"%s=@%05lx",((label_t*)a)->str.data,(uintptr_t)(((label_t*)a)->ptr)&0xfffff);
  }
  return 0;
}

void clear_textarea(textarea_t* ta){
  if (ta->lines){
    for (int i = 0; i < ta->n_lines; i++){
      free(ta->lines[i]);
    }
    free(ta->lines);
    ta->n_lines = 0;
    ta->lines = NULL;
  }
}

void btn_asm(){
  FILE* fd;
  fd = fopen("/tmp/source.dh","w");
  write_lines_file(fd, ta_edt.lines, ta_edt.n_lines);
  fclose(fd);

  clear_textarea(&ta_out);
  ta_out.lines = put_line(ta_out.lines, &(ta_out.n_lines), "running parser...");
  fd = popen("node src/parser.js /tmp/source.dh -I editor/gl -o build/ir.dsm --map build/ir.map 2>&1; echo parser exited with status $?", "r");
  ta_out.lines = read_file_lines(fd, ta_out.lines, &(ta_out.n_lines));
  pclose(fd);


  global_init();
  fd = fopen("build/ir.dsm","r");
  instrs = read_ir(fd);
  _G.layouts = read_layout(fd);
  fclose(fd);

  clear_textarea(&ta_asm);
  if (line_to_instr){
    line_to_instr = realloc(line_to_instr,instrs.len*sizeof(uintptr_t));
  }else{
    line_to_instr = malloc(instrs.len*sizeof(uintptr_t));
  }

  int idx = 0;
  list_node_t* n = instrs.head;
  while (n){
    instr_t* ins = (instr_t*)n->data;
    char line[256] = {0};
    int cnt = 0;

    cnt += sprintf(line+cnt,"@%05lx ",((uintptr_t)n)&0xfffff );

    for (int i = 7; i >= 0; i--){
      int c = (char)((ins->op)>>(i*8));
      if (c){
        cnt += sprintf(line+cnt,"%c",c);
      }
    }
    while (cnt < 14){
      cnt += sprintf(line+cnt," ");
    }
    if (ins->a){
      cnt += sprint_opran(line+cnt,ins->a);
      cnt += sprintf(line+cnt,"  ");
    }
    if (ins->b){
      cnt += sprint_opran(line+cnt,ins->b);
      cnt += sprintf(line+cnt,"  ");
    }
    if (ins->c){
      cnt += sprint_opran(line+cnt,ins->b);
      cnt += sprintf(line+cnt,"  ");
    }
    ta_asm.lines = put_line(ta_asm.lines,&ta_asm.n_lines,line);

    line_to_instr[idx] = (uintptr_t)n;
    n = n->next;
    idx++;
  }
}
int is_running = 0;
void btn_run(){
  clear_textarea(&ta_out);
  is_running = 1;
  frame_start();
  ins_node = instrs.head;
}

void btn_stop(){
  is_running = 0;
}


void handle_event(event_t* e){
  if (e->type == KEY_PRESSED){
    if (e->key == 13){
      int l = strlen(ta_edt.lines[cur_y])+1;
      char* nl = malloc(l-cur_x);
      strcpy(nl,ta_edt.lines[cur_y]+cur_x);
      ta_edt.lines[cur_y][cur_x] = 0;
      ta_edt.n_lines++;
      ta_edt.lines = realloc(ta_edt.lines, ta_edt.n_lines*sizeof(char*));
      memmove(ta_edt.lines+cur_y+2,ta_edt.lines+cur_y+1,(ta_edt.n_lines-cur_y-2)*sizeof(char*));
      ta_edt.lines[cur_y+1] = nl;
      cur_y++;
      cur_x=0;
    }else if (e->key >= ' ' && e->key <= '~'){
      int l = strlen(ta_edt.lines[cur_y])+1;
      ta_edt.lines[cur_y] = realloc(ta_edt.lines[cur_y], l+1);
      memmove(ta_edt.lines[cur_y]+cur_x+1, ta_edt.lines[cur_y]+cur_x, l-cur_x);
      ta_edt.lines[cur_y][cur_x] = e->key;
      cur_x++;
    }else if (e->key == 127){
      int l = strlen(ta_edt.lines[cur_y])+1;
      if (cur_x){
        memmove(ta_edt.lines[cur_y]+cur_x-1, ta_edt.lines[cur_y]+cur_x, l-cur_x);
        cur_x--;
      }else if (cur_y){
        int l0 = strlen(ta_edt.lines[cur_y-1]);
        int l1 = strlen(ta_edt.lines[cur_y]);
        ta_edt.lines[cur_y] = realloc(ta_edt.lines[cur_y], l0+l1+1);
        memcpy(ta_edt.lines[cur_y-1]+l0, ta_edt.lines[cur_y], l1+1);
        free(ta_edt.lines[cur_y]);
        memmove(ta_edt.lines+cur_y, ta_edt.lines+cur_y+1, (ta_edt.n_lines-cur_y-1)*sizeof(char*) );
        ta_edt.n_lines --;
        cur_y--;
        cur_x = l0;
      }
    }
    if (e->key == KEY_RARR){
      cur_x ++;
    }else if (e->key == KEY_LARR){
      cur_x --;
    }
    int len = strlen(ta_edt.lines[cur_y]);
    if (cur_x > len){
      cur_x = 0;
      cur_y ++;
    }
    if (cur_x < 0){
      if (cur_y){
        cur_y --;
        cur_x = strlen(ta_edt.lines[cur_y]);
      }else{
        cur_x = 0;
      }
    }
    if (e->key == KEY_UARR){
      if (cur_y > 0) cur_y --;
    }else if (e->key == KEY_DARR){
      if (cur_y < ta_edt.n_lines-1) cur_y ++;
    }
    len = strlen(ta_edt.lines[cur_y]);
    if (cur_x > len){
      cur_x = len;
    }
    while (cur_y - ta_edt.roll_y >= MID_ROWS){
      ta_edt.roll_y++;
    }
    while (cur_y - ta_edt.roll_y < 0){
      ta_edt.roll_y--;
    }
    
  }else if (e->type == WHEEL_SCROLLED){
    if (in_textarea(&ta_edt,mouse_x,mouse_y)){
      scroll_textarea(&ta_edt,e->x,e->y);
    }else if (in_textarea(&ta_out,mouse_x,mouse_y)){
      scroll_textarea(&ta_out,e->x,e->y);
    }else if (in_textarea(&ta_asm,mouse_x,mouse_y)){
      scroll_textarea(&ta_asm,e->x,e->y);
    }
  }else if (e->type == MOUSE_PRESSED){
    if (in_textarea(&ta_edt,mouse_x,mouse_y)){
      int col = e->x / FONT_W;
      int row = e->y / FONT_H;
      int tx = ta_edt.roll_x + col - LIN_COLS;
      int ty = ta_edt.roll_y + row - TOP_ROWS;
      cur_x = tx;
      cur_y = ty;
      while (cur_y >= ta_edt.n_lines) cur_y--;
      int len = strlen(ta_edt.lines[cur_y]);
      
      while (cur_x > len) cur_x--;
      
    }else{
      int col = e->x / FONT_W;
      int row = e->y / FONT_H;
      if (row == 0){
        if (col < 8){
          btn_asm();
        }else if (col < 16){
          btn_run();
        }else if (col < 24){
          btn_stop();
        }
      }
    }
  }else if (e->type == MOUSE_MOVED){
    mouse_x = e->x / FONT_W;
    mouse_y = e->y / FONT_H;
  }
}

int frame = 0;

void update(){
  memset(cons,0,sizeof(cons));
  memset(cons_rgb,0,sizeof(cons_rgb));
  for (int i = 0; i < WIN_COLS; i++){
    cons_rgb[0][i] = 0xffb6b6aa;
  }
  for (int i = 0; i < WIN_COLS; i++){
    cons_rgb[TOP_ROWS+MID_ROWS][i] = 0xff242455;
  }

  strcpy( (void*)(cons[0]   ),"[\x1a ASM ]");
  strcpy( (void*)(cons[0]+8 ),"[\x10 RUN ]");
  strcpy( (void*)(cons[0]+16),"[\xfeSTOP ]");
  strcpy( (void*)(cons[0]+24),"[\xafSTEP ]");

  if (mouse_y == 0){
    for (int i = 0; i < WIN_COLS/8; i++){
      if (mouse_x < i*8+8){
        for (int j = 0; j < 8; j++){
          cons_rgb[0][i*8+j] &= ~0xff000000;
        }
        break;
      }
    }
  }

  for (int i = 0; i < MID_ROWS; i++){
    int ty = ta_edt.roll_y+i;
    int cy = TOP_ROWS+i;
    
    char s[16]={0};
    if (ty < ta_edt.n_lines){
      sprintf(s,"%d    ",ty);
    }
    for (int j = 0; j < LIN_COLS; j++){
      cons[cy][j] = s[j];
      cons_rgb[cy][j] = 0xff494955;
    }
  }
  draw_textarea(&ta_edt,keywords,sizeof(keywords)/sizeof(char*),0);

  int cy = TOP_ROWS + cur_y - ta_edt.roll_y;
  int cx = LIN_COLS + cur_x - ta_edt.roll_x;
  if (in_textarea(&ta_edt,cx,cy)){
    if ((frame>>0)&1){
      cons_rgb[cy][cx] |= 0xff000000;
    }else{
      cons_rgb[cy][cx] &= ~0xff000000;
    }
  }


  int cur_line = -1;
  for (int i = 0; i < ta_asm.n_lines; i++){
    if ((uintptr_t)ins_node == line_to_instr[i]){
      cur_line = i;
    }
  }
  if (cur_line >= 0){
    while (cur_line < ta_asm.roll_y){
      ta_asm.roll_y--;
    }
    while (cur_line > ta_asm.roll_y + MID_ROWS){
      ta_asm.roll_y++;
    }
  }

  draw_textarea(&ta_asm,asm_keywords,sizeof(asm_keywords)/sizeof(char*),0);

  if (cur_line >= 0){
    for (int j = 0; j < ASM_COLS-1; j++){
      int cx = LIN_COLS+EDT_COLS+j;
      int cy = cur_line - ta_asm.roll_y + TOP_ROWS;
      cons_rgb[cy][cx] |= 0xff000000;
    }
  }

  draw_textarea(&ta_out,NULL,0,1);

  frame++;
}

GLuint usr_fbo=0, usr_tex, usr_w, usr_h;
int poll_flag = 0;

void usr_enter(){
  
  if (usr_fbo){
    glBindFramebuffer(GL_FRAMEBUFFER, usr_fbo);
    GLint tex = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &tex);
    glViewport(0, 0, usr_w, usr_h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, usr_w, usr_h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
  }


}

void usr_exit(){

  if (usr_fbo){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    reshape(WIN_W,WIN_H);

    glBindTexture(GL_TEXTURE_2D, usr_tex);
    glEnable(GL_TEXTURE_2D);
    // glClearColor(1.0,1.0,1.0,1.0);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(1,1,1,1);
    glBegin(GL_QUADS);
    // glTexCoord2f(0.0f, 1.0f); glVertex2f(0,0);
    // glTexCoord2f(1.0f, 1.0f); glVertex2f(0+usr_w,0);
    // glTexCoord2f(1.0f, 0.0f); glVertex2f(0+usr_w,0+usr_h);
    // glTexCoord2f(0.0f, 0.0f); glVertex2f(0,0+usr_h);
    float x = (WIN_COLS-USR_COLS)*FONT_W;
    float y = TOP_ROWS*FONT_H;
    float w = usr_w;
    float h = usr_h;
    if (w > h){
      if (w > USR_COLS*FONT_W){
        h *= (USR_COLS*FONT_W)/w;
        w = USR_COLS*FONT_W;
      }
    }else{
      if (h > USR_ROWS*FONT_H){
        w *= (USR_ROWS*FONT_H)/h;
        h = USR_ROWS*FONT_H;
      }
    }

    glTexCoord2f(0.0f, 1.0f); glVertex2f(x,y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x+w,y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x+w,y+h);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x,y+h);
    glEnd();
    glDisable(GL_TEXTURE_2D);

  }
}

void cb_init_(int w, int h){
  poll_flag = 1;
  glEnable(GL_TEXTURE_2D);
  glGenFramebuffers(1, &usr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, usr_fbo);
  glGenTextures(1, &usr_tex);
  glBindTexture(GL_TEXTURE_2D, usr_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, usr_tex, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  usr_w = w;
  usr_h = h;

}

void cb_poll_(void* data){
  poll_flag = 2;
}

void cb_print_(char* s){
  // printf("%s",s);
  if (!ta_out.lines){
    ta_out.lines = malloc(sizeof(char**));
    ta_out.lines[0] = calloc(1,1);
    ta_out.n_lines = 1;
  }
  char c;
  while ((c=*s)){
    if (c == '\n'){
      ta_out.lines = realloc(ta_out.lines,(ta_out.n_lines+1)*sizeof(char**));
      ta_out.lines[ta_out.n_lines] = calloc(1,1);
      ta_out.n_lines++;
      scroll_textarea(&ta_out, 0, -1);
    }else{
      int l = strlen(ta_out.lines[ta_out.n_lines-1])+1;
      ta_out.lines[ta_out.n_lines-1] = realloc(ta_out.lines[ta_out.n_lines-1],l+1);
      ta_out.lines[ta_out.n_lines-1][l-1] = c;
      ta_out.lines[ta_out.n_lines-1][l] = 0;
    }
    s++;
  }
}

__attribute__((visibility("default")))
void (*__win_intern_hook_init)(int, int) = cb_init_;
__attribute__((visibility("default")))
void (*__win_intern_hook_poll)(void*) = cb_poll_;

__attribute__((visibility("default")))
void (*__io_intern_hook_print)(char*) = cb_print_;



int main(int argc, char** argv) {

  void *lib = dlopen("std/win/platform/windowing.so", RTLD_NOW);
  windowing_init = dlsym(lib, "window_init");
  windowing_poll = dlsym(lib, "window_poll");
  windowing_exit = dlsym(lib, "window_exit");

  windowing_init(WIN_W, WIN_H);

  reshape(WIN_W,WIN_H);

  build_font_texture();
  build_text_buffer();

  FILE* fd;

  fd = fopen("examples/10print.dh","r");
  // fd = fopen("../ldcc/tests/test231.txt","r");

  ta_edt.lines = read_file_lines(fd, ta_edt.lines,&ta_edt.n_lines);

  // fd = fopen("build/ir.dsm","r");
  // ta_asm.lines = read_file_lines(fd,&ta_asm.n_lines);
  
  // cb_init_(100,100);

  while (1){
    // clock_t start_time = clock();

 
    int n_events = 0;
    event_t* events = windowing_poll(&n_events);
    render_text();

    for (int i = 0; i < n_events; i++){
      handle_event(events+i);
    }
    update();


    // for (int i = 0; i < 8; i++){
    //   for (int j = 0; j < 32; j++){
    //     cons[i][j] = i*32+j;
    //     cons_rgb[i][j] = 0xffffff;
    //   }
    // }

    usr_enter();
    // for (int i = 0; i < 1; i++){
    while (is_running){
      if (ins_node){
        // print_instr(ins_node->data);
        ins_node = execute_instr(ins_node);
      }else{
        is_running = 0;
      }
      if (poll_flag == 0){
        break;
      }else if (poll_flag == 2){
        poll_flag = 1;
        break;
      }
      break;
    }
    // }
    usr_exit();
    

    // clock_t end_time = clock();
    // double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    // double fps = 1.0/elapsed_time;
    // printf("FPS:%.2f\n", fps);
      
  }

  return 0;
}
