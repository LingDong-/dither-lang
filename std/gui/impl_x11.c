#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define ROW_SLIDER1F 1
#define ROW_TOGGLE1I 10

typedef struct row_st {
  struct row_st* next;
  char type;
  char* name;
} row_t;

typedef struct slider1f_st {
  struct row_st* next;
  char type;
  char* name;
  float val;
  float min;
  float max;
} slider1f_t;

typedef struct toggle1i_st {
  struct row_st* next;
  char type;
  char* name;
  int val;
} toggle1i_t;

Display *dis;
int screen;
Window win;
XEvent event;
Pixmap pix;
GC gc;

int n_row = 0;
row_t* head = NULL;

row_t* modal = NULL;
int modal_x = 0;
int modal_y = 0;
int modal_n = 0;
int modal_i = 0;
char modal_str[64];

void gui_impl__resize(){
  
  XSizeHints hints;
  hints.flags = PMinSize | PMaxSize;
  hints.min_width  = 250;
  hints.max_width  = 250;
  hints.min_height = n_row*25;
  hints.max_height = n_row*25;
  XSetWMNormalHints(dis, win, &hints);
}
void gui_impl_init(){
  dis=XOpenDisplay(NULL);
  screen=DefaultScreen(dis);
  win=XCreateSimpleWindow(dis,DefaultRootWindow(dis),0,0,250,25, 5,0,0xffffff);
  XSelectInput(dis, win, ExposureMask|KeyPressMask|ButtonPressMask|Button1MotionMask);
  XStoreName(dis, win, "Parameters");
  gui_impl__resize();

  XMapRaised(dis, win);

  gc = XCreateGC(dis, win, 0, NULL);
  XSetForeground(dis, gc, BlackPixel(dis, screen));
  XSetBackground(dis, gc, WhitePixel(dis, screen));

  XFontStruct *font = XLoadQueryFont(dis, "fixed");
  XSetFont(dis, gc, font->fid);

}


void gui_impl__draw(){
  row_t* node = head;
  int y = 2;
  while (node){
    char lbl[13];
    memset(lbl,' ',12);
    lbl[12]=0;
    int n = strlen(node->name);
    if (n<12){
      memcpy(lbl+12-n,node->name,n);
    }else{
      memcpy(lbl,node->name,5);
      memcpy(lbl+8,node->name+n-4,4);
      lbl[5] = lbl[6] = lbl[7] = '.';
    }
    if (node->type == ROW_SLIDER1F){
      slider1f_t* u = (slider1f_t*)node;
      float pct = (u->val - u->min)/(u->max - u->min);
      XDrawRectangle(dis, win, gc, 80, y, 110, 20);
      XFillRectangle(dis, win, gc, 85+100*pct-5, y, 10, 20);
      XDrawString(dis, win, gc, 5, y+17, lbl, 12);
      char vs[10];
      snprintf(vs,6,"%.4g",u->val);
      XDrawRectangle(dis, win, gc, 194, y, 37, 20);
      XDrawString(dis, win, gc, 196, y+17, vs, strlen(vs));

      XDrawRectangle(dis, win, gc, 235, y, 10, 10);
      XDrawRectangle(dis, win, gc, 235, y+10, 10, 10);
      XDrawString(dis, win, gc, 238, y+10, "+", 1);
      XDrawString(dis, win, gc, 238, y+20, "-", 1);
    }else if (node->type == ROW_TOGGLE1I){
      toggle1i_t* u = (toggle1i_t*)node;
      XDrawString(dis, win, gc, 5, y+17, lbl, 12);
      XDrawRectangle(dis, win, gc, 225, y, 20, 20);
      if (u->val){
        XFillRectangle(dis, win, gc, 229, y+4, 13, 13);
      }
    }
    node = node->next;
    y += 25;
  }
  if (modal){
    XFillRectangle(dis, win, gc, modal_x, modal_y, modal_n*6, 20);
    XSetForeground(dis, gc, WhitePixel(dis, screen));
    XDrawString(dis, win, gc, modal_x+1, modal_y+17, modal_str, modal_i);
    XDrawLine(dis,win,gc,modal_x+modal_i*6+3,modal_y+3,modal_x+modal_i*6+3,modal_y+17);
    XSetForeground(dis, gc, BlackPixel(dis, screen));
  }
}

void gui_impl__slider1f(char* name,float x,float l,float r){
  slider1f_t* row = malloc(sizeof(slider1f_t));
  row->type = ROW_SLIDER1F;
  row->name = strdup(name);
  row->next = NULL;
  row->min = l;
  row->max = r;
  row->val = x;
  if (head == NULL){
    head = (row_t*)row;
  }else{
    row_t* prev = head;
    while (prev->next) prev = prev->next;
    prev->next = (row_t*)row;
  }
  n_row++;

  XResizeWindow(dis,win,250,n_row*25);
  gui_impl__resize();
}

void gui_impl__toggle1i(char* name,int x){
  toggle1i_t* row = malloc(sizeof(toggle1i_t));
  row->type = ROW_TOGGLE1I;
  row->name = strdup(name);
  row->next = NULL;
  row->val = x;
  if (head == NULL){
    head = (row_t*)row;
  }else{
    row_t* prev = head;
    while (prev->next) prev = prev->next;
    prev->next = (row_t*)row;
  }
  n_row++;
  XResizeWindow(dis,win,250,n_row*25);
  gui_impl__resize();
}



float gui_impl__get1f(char* name){
  row_t* node = head;
  while (node){
    if (!strcmp(node->name,name)){
      if (node->type == ROW_SLIDER1F){
        slider1f_t* u = (slider1f_t*)node;
        return u->val;
      }
    }
    node = node->next;
  }
  return 0;
}

float gui_impl__get1i(char* name){

  row_t* node = head;
  while (node){
    if (!strcmp(node->name,name)){
      if (node->type == ROW_TOGGLE1I){
        toggle1i_t* u = (toggle1i_t*)node;
        
        return u->val;
      }
    }
    node = node->next;
  }
  return 0;
}

void gui_impl_poll(){
  int n;
  if ((n = XPending(dis)) > 0) {
    XNextEvent(dis, &event);
    if (event.type==Expose){
      gui_impl__draw();
    }else if (!modal && (event.type == ButtonPress || event.type == MotionNotify)) {
      float mx = event.xbutton.x;
      float my = event.xbutton.y;
      row_t* node = head;
      int y = 2;
      while (node){
        if (y <= my && my <= y+20){
          if (node->type == ROW_SLIDER1F){
            if (mx < 200){
              float v = (mx-85)/100.0;
              if (v < 0) v=0;
              if (v > 1.0) v=1.0;
              slider1f_t* u = (slider1f_t*)node;
              u->val = u->min + v * (u->max-u->min);
            }else if (mx < 230){
              modal_x = 195;
              modal_y = y;
              modal_n = 6;
              modal_i = 0;
              modal = node;
            }else{
              slider1f_t* u = (slider1f_t*)node;
              float v = u->val;
              float stp = (u->max-u->min)*0.005;
              if (my < y+10){
                v += stp;
              }else{
                v -= stp;
              }
              u->val = v;
            }
          }else if (node->type == ROW_TOGGLE1I){
            if (event.type == ButtonPress){
              toggle1i_t* u = (toggle1i_t*)node;
              u->val = !u->val;
            }
          }
        }
        node = node->next;
        y += 25;
      }
      XClearWindow(dis, win);
      gui_impl__draw();
    }else if (event.type == KeyPress){
      if (modal){
        char keybuf[32];
        KeySym keysym;
        int key;
        int len = XLookupString(&event.xkey, keybuf, sizeof(keybuf), &keysym, NULL);
        if (len == 1){
          key = keybuf[0];
        }else{
          key = keysym;
        }
        if (key == 8){
          if (modal_i > 0){
            modal_i--;
          }
        }else if (key == 13){
          
          if (modal->type == ROW_SLIDER1F){
            slider1f_t* u = (slider1f_t*)modal;
            modal_str[modal_i] = 0;
            float v = atof(modal_str);
            if (!isnan(v)){
              if (v < u->min){
                u->val = u->min;
              }else if (v > u->max){
                u->val = u->max;
              }else{
                u->val = v;
              }
            }
          }
          modal = NULL;
        }else if (key == 27){
          modal = NULL;
        }else if (modal_i < modal_n){
          modal_str[modal_i++] = key;
        }
        XClearWindow(dis, win);
        gui_impl__draw();
      }

    }
  }
}