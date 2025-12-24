#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "windowing.h"

#define MAX_EVENTS 64 

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

event_t event_buffer[MAX_EVENTS];
event_t out_buffer[MAX_EVENTS];
int event_count = 0;

void add_event(int type, int key, float x, float y) {
  if (event_count && type == MOUSE_M0VED && event_buffer[event_count-1].type == type){
    event_buffer[event_count-1].x = x;
    event_buffer[event_count-1].y = y;
    return;
  }
  if (event_count >= MAX_EVENTS) {
    memmove(event_buffer, event_buffer + 1, sizeof(event_t) * (MAX_EVENTS - 1));
    event_count = MAX_EVENTS - 1;
  }
  event_buffer[event_count].type = type;
  event_buffer[event_count].key = key;
  event_buffer[event_count].x = x;
  event_buffer[event_count].y = y;
  event_count++;
}

int width;
int height;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_DESTROY) {
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND hwnd;

EXPORTED void** window_init(int w, int h, int flags){

  const char CLASS_NAME[] = "WINDOW_CLASS_NAME";
  WNDCLASS wc = {0};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClass(&wc);
  RECT rect = { 0, 0, w,h };
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  int windowWidth = rect.right - rect.left;
  int windowHeight = rect.bottom - rect.top;
  RECT workArea;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
  int x = workArea.left + (workArea.right - workArea.left - windowWidth) / 2;
  int y = workArea.top + (workArea.bottom - workArea.top - windowHeight) / 2;

  hwnd = CreateWindow(wc.lpszClassName, "", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                      x,y, windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);
  
  width = w;
  height = h;
  // printf("%p\n",hwnd);
  return (void**)hwnd;
}

int get_key_code(MSG* msg){
  BYTE keyboardState[256];
  WCHAR unicodeChar[4];
  GetKeyboardState(keyboardState);
  UINT scanCode = (msg->lParam >> 16) & 0xFF;
  int len = ToUnicode((UINT)msg->wParam, scanCode, keyboardState, unicodeChar, 4, 0);
  if (len == 1 && unicodeChar[0] >= 0 && unicodeChar[0] < 127) {
    return (int)unicodeChar[0];
  } else {
    UINT vk = msg->wParam;
    if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU) {
        vk = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
    }
    switch (vk){
      case VK_F1      : return KEY_F1    ;
      case VK_F2      : return KEY_F2    ;
      case VK_F3      : return KEY_F3    ;
      case VK_F4      : return KEY_F4    ;
      case VK_F5      : return KEY_F5    ;
      case VK_F6      : return KEY_F6    ;
      case VK_F7      : return KEY_F7    ;
      case VK_F8      : return KEY_F8    ;
      case VK_F9      : return KEY_F9    ;
      case VK_F10     : return KEY_F10   ;
      case VK_F11     : return KEY_F11   ;
      case VK_F12     : return KEY_F12   ;
      case VK_LEFT    : return KEY_LARR  ;
      case VK_UP      : return KEY_UARR  ;
      case VK_RIGHT   : return KEY_RARR  ;
      case VK_DOWN    : return KEY_DARR  ;
      case VK_LSHIFT  : return KEY_LSHIFT;
      case VK_RSHIFT  : return KEY_RSHIFT;
      case VK_LCONTROL: return KEY_LCTRL ;
      case VK_RCONTROL: return KEY_RCTRL ;
      case VK_LMENU   : return KEY_LALT  ;
      case VK_RMENU   : return KEY_RALT  ;
      case VK_LWIN    : return KEY_LCMD  ;
      case VK_RWIN    : return KEY_RCMD  ;
    }
  }
  return 0;
}


EXPORTED event_t* window_poll(int* out_count){
  MSG msg;

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_MOUSEMOVE) {
      add_event(
        MOUSE_M0VED,
        0,
        GET_X_LPARAM(msg.lParam),
        GET_Y_LPARAM(msg.lParam)
      );
    }else if (msg.message == WM_LBUTTONDOWN){
      add_event(
        MOUSE_PRESSED,
        MOUSE_LEFT,
        GET_X_LPARAM(msg.lParam),
        GET_Y_LPARAM(msg.lParam)
      );
    }else if (msg.message == WM_LBUTTONUP){
      add_event(
        MOUSE_RELEASED,
        MOUSE_LEFT,
        GET_X_LPARAM(msg.lParam),
        GET_Y_LPARAM(msg.lParam)
      );
    }else if (msg.message == WM_RBUTTONDOWN){
      add_event(
        MOUSE_PRESSED,
        MOUSE_RIGHT,
        GET_X_LPARAM(msg.lParam),
        GET_Y_LPARAM(msg.lParam)
      );
    }else if (msg.message == WM_RBUTTONUP){
      add_event(
        MOUSE_RELEASED,
        MOUSE_RIGHT,
        GET_X_LPARAM(msg.lParam),
        GET_Y_LPARAM(msg.lParam)
      );
    }else if (msg.message == WM_KEYDOWN){
      add_event(
        KEY_PRESSED,
        get_key_code(&msg),
        0,
        0
      );
    }else if (msg.message == WM_KEYUP){
      add_event(
        KEY_RELEASED,
        get_key_code(&msg),
        0,
        0
      );
    }else if (msg.message == WM_MOUSEWHEEL){
      add_event(
        WHEEL_SCROLLED,
        0,
        0,
        GET_WHEEL_DELTA_WPARAM(msg.wParam)
      );
    }else if (msg.message == WM_MOUSEHWHEEL){
      add_event(
        WHEEL_SCROLLED,
        0,
        GET_WHEEL_DELTA_WPARAM(msg.wParam),
        0
      );
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  if (*out_count == 0){
    *out_count = event_count;
  }
  if (event_count){
    memcpy(out_buffer, event_buffer, (*out_count)*sizeof(event_t));
    memmove(event_buffer, event_buffer + (*out_count), (MAX_EVENTS-*out_count)*sizeof(event_t));
    event_count -= *out_count;
  }else{
    *out_count = 0;
  }
  return out_buffer;
}


EXPORTED void window_exit(){

}