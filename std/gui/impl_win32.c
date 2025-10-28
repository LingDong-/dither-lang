#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdi32.lib")


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
  HWND hSlider;
  HWND hEdit;
} slider1f_t;

typedef struct toggle1i_st {
  struct row_st* next;
  char type;
  char* name;
  int val;
  HWND hCheckbox;
} toggle1i_t;

int n_row = 0;
row_t* head = NULL;


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
      HDC hdc = (HDC)wParam;
      SetTextColor(hdc, RGB(0,0,0));
      static HBRUSH hBrush = NULL;
      if (!hBrush) hBrush = CreateSolidBrush(RGB(255, 255, 255));
      SetBkColor(hdc, RGB(255,255,255));
      return (INT_PTR)hBrush;
    }
    case WM_HSCROLL: {
      HWND src = (HWND)lParam;
      row_t* node = head;
      while (node){
        if (node->type == ROW_SLIDER1F){
          slider1f_t* u = (slider1f_t*)node;
          if (src == u->hSlider) {
            int tick = (int)SendMessage(src, TBM_GETPOS, 0, 0);
            float val = u->min + tick*(u->max-u->min)/100.0;
            u->val = val;
            char buf[32];
            sprintf(buf, "%f", val);
            SetWindowText(u->hEdit, buf);
            return 0;
          }
        }
        node = node->next;
      }
      return 0;
    }
    case WM_COMMAND: {
      HWND src = (HWND)lParam;
      WORD code = HIWORD(wParam);
      row_t* node = head;
      while (node){
        if (node->type == ROW_SLIDER1F){
          slider1f_t* u = (slider1f_t*)node;
          if (src == u->hEdit && code == EN_CHANGE) {
            char buf[32];
            GetWindowText(u->hEdit, buf, sizeof(buf));
            float val = atof(buf);
            if (isnan(val)){
              val = u->min;
            }
            if (val < u->min) val = u->min;
            if (val > u->max) val = u->max;
            u->val = val;
            int tick = ((val-u->min)/(u->max-u->min))*100;
            SendMessage(u->hSlider, TBM_SETPOS, TRUE, tick);
            return 0;
          }
        }else if (node->type == ROW_TOGGLE1I){
          toggle1i_t* u = (toggle1i_t*)node;
          if (src == u->hCheckbox && code == BN_CLICKED) {
            LRESULT state = SendMessage(u->hCheckbox, BM_GETCHECK, 0, 0);
            BOOL checked = (state == BST_CHECKED);
            u->val = checked;
            return 0;
          }
        }
        node = node->next;
      }
      return 0;
    }
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}


HWND hwnd;
HFONT hFont;
void gui_impl_init(){
  const char CLASS_NAME[] = "PANEL_CLASS_NAME";
  WNDCLASS wc = {0};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = CLASS_NAME;
  wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClass(&wc);
  RECT rect = { 0, 0, 250, 25};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  int windowWidth = rect.right - rect.left;
  int windowHeight = rect.bottom - rect.top;
  RECT workArea;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
  int x = workArea.left + 100;
  int y = workArea.top + 100;

  hwnd = CreateWindowEx(WS_EX_PALETTEWINDOW, wc.lpszClassName, "Parameters", WS_VISIBLE,
                      x,y, windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);
                      
  ShowWindow(hwnd, SW_SHOW);
  hFont = CreateFont(
    14, 0, 0, 0,
    FW_NORMAL,
    FALSE, FALSE, FALSE,
    ANSI_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_SWISS,
    "MS Sans Serif"
  );                  
}
void gui_impl__slider1f(char* name,float x,float l,float r){
  char buf[32];
  sprintf(buf, "%f", x);
  int tick = (x-l)/(r-l)*100.0;

  HWND hLabel = CreateWindowEx(0, "STATIC", name,
                                WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                0, n_row*25+0, 80, 20,
                                hwnd, NULL, GetModuleHandle(NULL), NULL);

  HWND hSlider = CreateWindowEx(0, TRACKBAR_CLASS, name,
    WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
    80, n_row*25+0, 110, 20,
    hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
  SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
  SendMessage(hSlider, TBM_SETPOS, TRUE, tick);

  HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", buf,
                                WS_CHILD | WS_VISIBLE,
                                190, n_row*25+0, 40, 20,
                                hwnd, NULL, GetModuleHandle(NULL), NULL);
  HWND hSpin = CreateWindowEx(0, UPDOWN_CLASS, "",
                                  WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                                  230, n_row*25+0, 25, 20,
                                  hwnd, NULL, GetModuleHandle(NULL), NULL);
  // SendMessage(hSpin, UDM_SETBUDDY, (WPARAM)hEdit, 0);
  SendMessage(hSpin, UDM_SETRANGE32, 0, 100);
  SendMessage(hSpin, UDM_SETPOS32, 0, tick);

  SendMessage(hSlider, WM_SETFONT, (WPARAM)hFont, TRUE);
  SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
  SendMessage(hSpin, WM_SETFONT, (WPARAM)hFont, TRUE);
  SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

  slider1f_t* row = malloc(sizeof(slider1f_t));
  row->type = ROW_SLIDER1F;
  row->name = strdup(name);
  row->next = NULL;
  row->min = l;
  row->max = r;
  row->val = x;
  row->hSlider = hSlider;
  row->hEdit = hEdit;
  if (head == NULL){
    head = (row_t*)row;
  }else{
    row_t* prev = head;
    while (prev->next) prev = prev->next;
    prev->next = (row_t*)row;
  }
  n_row++;

  RECT rect = { 0, 0, 250, n_row*25};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  int windowWidth = rect.right - rect.left;
  int windowHeight = rect.bottom - rect.top;
  SetWindowPos(hwnd, NULL, 0, 0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

void gui_impl__toggle1i(char* name, int x){
  
  HWND hLabel = CreateWindowEx(
    0, "STATIC", name,
    WS_CHILD | WS_VISIBLE | SS_RIGHT,
    0, n_row*25 + 0, 80, 20,
    hwnd, NULL, GetModuleHandle(NULL), NULL);

  HWND hCheckbox = CreateWindowEx(
    0, "BUTTON", "",
    WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
    232, n_row*25 + 0, 20, 20,
    hwnd, (HMENU)100 + n_row, GetModuleHandle(NULL), NULL);

  SendMessage(hCheckbox, BM_SETCHECK, x?BST_CHECKED:BST_UNCHECKED, 0);
  SendMessage(hCheckbox, WM_SETFONT, (WPARAM)hFont, TRUE);
  SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

  toggle1i_t* row = malloc(sizeof(toggle1i_t));
  row->type = ROW_TOGGLE1I;
  row->name = strdup(name);
  row->next = NULL;
  row->val = x;
  row->hCheckbox = hCheckbox;
  if (head == NULL){
    head = (row_t*)row;
  }else{
    row_t* prev = head;
    while (prev->next) prev = prev->next;
    prev->next = (row_t*)row;
  }
  n_row++;

  RECT rect = { 0, 0, 250, n_row*25};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  int windowWidth = rect.right - rect.left;
  int windowHeight = rect.bottom - rect.top;
  SetWindowPos(hwnd, NULL, 0, 0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE | SWP_NOZORDER);

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

int gui_impl__get1i(char* name){
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
  MSG msg;
  if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}