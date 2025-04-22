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

#include "../../src/interp.c"
#include "../../std/win/platform/windowing.h"

char* keywords[] = {
  "namespace","continue","typedef","include","return","break","while","const",
  "else","func","func","dict","list",
  "for","i16","u16","i32","u32","i64","u64","f32","f64","tup","vec","arr","str",
  "if","do","as","i8","u8"
};

#define FONT_W 8
#define FONT_H 16
#define FONT_COLS 16
#define FONT_ROWS 8
#define FONT_N 128
#define FONT_TEX_W (FONT_COLS * FONT_W)
#define FONT_TEX_H (FONT_ROWS * FONT_H)
#define FONT_TEX_HH (FONT_TEX_H*2)

#define WIN_COLS 160
#define WIN_ROWS 45

#define WIN_W (FONT_W*WIN_COLS)
#define WIN_H (FONT_H*WIN_ROWS)

#define LIN_COLS 4
#define EDT_COLS 60
#define ASM_COLS 50
#define VAR_COLS 46

#define TOP_ROWS 1
#define MID_ROWS 43
#define BOT_ROWS 1

#define HL_COMMENT 0x6d6d6d
#define HL_KEYWORD 0xff00ff
#define HL_SIGIL   0xffff00
#define HL_NUMBER  0x00ff00
#define HL_STRING  0xff6d00
#define HL_BASE    0xffffff
#define HL_FUNC    0x00ffff

#include "font_bitmap.c"

char cons[WIN_H][WIN_W] = {0};
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
      char ch = cons[i][j];
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

char** read_file_lines(FILE* fd, int* out_num_lines) {
  int lines_cap = 64;
  char** lines = malloc(lines_cap * sizeof(char*));
  int num_lines = 0;
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

int roll_y = 0;
int roll_x = 0;

int cur_x = 0;
int cur_y = 0;

int n_lines;
char** lines;


void handle_event(event_t* e){
  if (e->type == KEY_PRESSED){
    if (e->key == 13){
      int l = strlen(lines[cur_y])+1;
      char* nl = malloc(l-cur_x);
      strcpy(nl,lines[cur_y]+cur_x);
      lines[cur_y][cur_x] = 0;
      n_lines++;
      lines = realloc(lines, n_lines*sizeof(char*));
      memmove(lines+cur_y+2,lines+cur_y+1,(n_lines-cur_y-2)*sizeof(char*));
      lines[cur_y+1] = nl;
      cur_y++;
      cur_x=0;
    }else if (e->key >= ' ' && e->key <= '~'){
      int l = strlen(lines[cur_y])+1;
      lines[cur_y] = realloc(lines[cur_y], l+1);
      memmove(lines[cur_y]+cur_x+1, lines[cur_y]+cur_x, l-cur_x);
      lines[cur_y][cur_x] = e->key;
      cur_x++;
    }else if (e->key == 127){
      int l = strlen(lines[cur_y])+1;
      if (cur_x){
        memmove(lines[cur_y]+cur_x-1, lines[cur_y]+cur_x, l-cur_x);
        cur_x--;
      }else if (cur_y){
        int l0 = strlen(lines[cur_y-1]);
        int l1 = strlen(lines[cur_y]);
        lines[cur_y] = realloc(lines[cur_y], l0+l1+1);
        memcpy(lines[cur_y-1]+l0, lines[cur_y], l1+1);
        free(lines[cur_y]);
        memmove(lines+cur_y, lines+cur_y+1, (n_lines-cur_y-1)*sizeof(char*) );
        n_lines --;
        cur_y--;
        cur_x = l0;
      }
    }


    if (e->key == KEY_RARR){
      cur_x ++;
    }else if (e->key == KEY_LARR){
      cur_x --;
    }
    int len = strlen(lines[cur_y]);
    if (cur_x > len){
      cur_x = 0;
      cur_y ++;
    }
    if (cur_x < 0){
      if (cur_y){
        cur_y --;
        cur_x = strlen(lines[cur_y]);
      }else{
        cur_x = 0;
      }
    }
    if (e->key == KEY_UARR){
      if (cur_y > 0) cur_y --;
    }else if (e->key == KEY_DARR){
      if (cur_y < n_lines-1) cur_y ++;
    }
    len = strlen(lines[cur_y]);
    if (cur_x > len){
      cur_x = len;
    }
    while (cur_y - roll_y >= MID_ROWS){
      roll_y++;
    }
    while (cur_y - roll_y < 0){
      roll_y--;
    }


    
  }else if (e->type == WHEEL_SCROLLED){
    roll_y -= e->y;
    if (roll_y < 0) roll_y = 0;
    if (roll_y > n_lines - MID_ROWS) roll_y = n_lines-MID_ROWS;
    roll_x -= e->x;
    if (roll_x < 0) roll_x = 0;
    if (e->x < 0){
      int maxlen = 0;
      for (int i = 0; i < n_lines; i++){
        maxlen = MAX(maxlen,strlen(lines[i]));
      }
      if (roll_x > maxlen-EDT_COLS) roll_x = maxlen-EDT_COLS;
    }
  }else if (e->type == MOUSE_PRESSED){
    int col = e->x / FONT_W;
    int row = e->y / FONT_H;
    int tx = roll_x + col - LIN_COLS;
    int ty = roll_y + row - TOP_ROWS;
    cur_x = tx;
    cur_y = ty;
    int len = strlen(lines[cur_y]);
    while (cur_x > len) cur_x--;
  }
}
#define IS_SIGIL(c) ((c)<'0' || ((c)>'9'&&(c)<'A') || ((c)>'Z'&&(c)<'_') || (c)>'z')

int frame = 0;

void update(){
  memset(cons,0,sizeof(cons));
  memset(cons_rgb,0,sizeof(cons_rgb));

  int blcm = 0;

  for (int i = 0; i < MID_ROWS; i++){
    int ty = roll_y+i;
    int cy = TOP_ROWS+i;
    if (ty >= n_lines) continue;
    int len = strlen(lines[ty]);

    char s[16];
    sprintf(s,"%d    ",ty);

    for (int j = 0; j < LIN_COLS; j++){
      cons[cy][j] = s[j];
      cons_rgb[cy][j] = 0xff6d6d6d;
    }

    for (int j = 0; j < EDT_COLS; j++){
      int tx = roll_x+j;
      int cx = LIN_COLS + j;
      if (tx > len) continue;
      char c = lines[ty][tx];
      cons[cy][cx] = c;
      if (cons_rgb[cy][cx] == 0){
        if (blcm){
          cons_rgb[cy][cx] = HL_COMMENT;
          if (c == '*' && lines[ty][tx+1] == '/'){
            cons_rgb[cy][cx+1] = HL_COMMENT;
            blcm = 0;
          }
        }else if (c == '/' && lines[ty][tx+1] == '*'){
          blcm = 1;
          cons_rgb[cy][cx] = HL_COMMENT;
        }else if (c == '/' && lines[ty][tx+1] == '/'){
          int q = 0;
          do{
            cons_rgb[cy][cx+q] = HL_COMMENT;
          }while (lines[ty][tx+(q++)]);
        }else if(c == '"'){
          int q = 0;
          int esc = 0;
          do{
            cons_rgb[cy][cx+q] = HL_STRING;
            if (lines[ty][tx+q] == '\\'){
              esc = 1;
            }else{
              if (q && lines[ty][tx+q] == '"' && !esc){
                break;
              }
              esc = 0;
            }
          }while (lines[ty][tx+(q++)]);
        }else if ('0'<=c && c<='9'){
          if (!tx || IS_SIGIL(lines[ty][tx-1])){
            int q = 0;
            do{
              
              int b = lines[ty][tx+q];
              if (b == 'x' || b == 'X' || b == 'b' || b == 'B' || b == 'e' || b == 'E' ||
                  b == '.' || b == '+' || b == '-' ||
                  ('0'<=b && b<='9')){
              }else{
                cons_rgb[cy][cx+q] = HL_BASE;
                break;
              }
              cons_rgb[cy][cx+q] = HL_NUMBER;
            }while (lines[ty][tx+(q++)]);
          }else{
            cons_rgb[cy][cx] = HL_BASE;
          }
        }else if (c == '(' && tx > 0){
          cons_rgb[cy][cx] = HL_SIGIL;
          int q = 0;
          int sp = 0;
          while (tx+(--q) >= 0){
            int b = lines[ty][tx+q];
            if (b == ' '){
              if (q == -1 || sp){
                sp = 1;
              }else{
                break;
              }
            }else{
              sp = 0;
              if ((cons_rgb[cy][cx+q]&0xffffff) != 0xffffff){
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
          
          for (int k = 0; k < sizeof(keywords)/sizeof(char*); k++){
            int ok = 1;
            int l = strlen(keywords[k]);
            for (int q = 0; q < l; q++){
              int a = keywords[k][q];
              int b = lines[ty][tx+q];
              if (a != b){
                ok = 0;
                break;
              }
            }
            if (ok){
              int b = lines[ty][tx+l];
              if (! IS_SIGIL(b)){
                ok = 0;
              }
              if (tx > 0){
                b = lines[ty][tx-1];
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
      if (ty == cur_y && tx == cur_x){
        if ((frame>>9)&1){
          cons_rgb[cy][cx] |= 0xff000000;
        }
      }
    }
  }
  frame++;
}



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
  fd = fopen("examples/boids.dh","r");

  lines = read_file_lines(fd,&n_lines);

  while (1){
    int n_events = 0;
    event_t* events = windowing_poll(&n_events);

    for (int i = 0; i < n_events; i++){
      handle_event(events+i);
    }
    
    update();
    render_text();
    
  }

  return 0;
}