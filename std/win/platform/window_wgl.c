#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <stdio.h>

#include "windowing.h"

#define MAX_EVENTS 64 

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif

#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
typedef HGLRC (APIENTRYP PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

event_t event_buffer[MAX_EVENTS];
event_t out_buffer[MAX_EVENTS];
int event_count = 0;

void add_event(int type, int key, float x, float y) {
  if (event_count && type == MOUSE_MOVED && event_buffer[event_count-1].type == type){
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

HDC hdc;

EXPORTED void** window_init(int w, int h, int flags){

  const char CLASS_NAME[] = "WINDOW_CLASS_NAME";
  WNDCLASS wc = {0};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = CLASS_NAME;
  RegisterClass(&wc);
  RECT rect = { 0, 0, w,h };
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  int windowWidth = rect.right - rect.left;
  int windowHeight = rect.bottom - rect.top;
  HWND hwnd = CreateWindow(wc.lpszClassName, "", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);
  hdc = GetDC(hwnd);

  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(pfd), 1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA, 32, 0,0,0,0,0,0,0,0,0,0,0,24,8,0,
    PFD_MAIN_PLANE, 0,0,0,0
  };
  int pf = ChoosePixelFormat(hdc, &pfd);
  SetPixelFormat(hdc, pf, &pfd);
  HGLRC tempRC = wglCreateContext(hdc);
  wglMakeCurrent(hdc, tempRC);
  wglCreateContextAttribsARB = (void*)wglGetProcAddress("wglCreateContextAttribsARB");
  int attribs[] = {
    0x2091, 3, // WGL_CONTEXT_MAJOR_VERSION_ARB
    0x2092, 3, // WGL_CONTEXT_MINOR_VERSION_ARB
    0x9126, 0x00000002,    // WGL_CONTEXT_PROFILE_MASK_ARB = COMPATIBILITY
    0
  };
  HGLRC glrc = wglCreateContextAttribsARB(hdc, 0, attribs);
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(tempRC);
  wglMakeCurrent(hdc, glrc);
  width = w;
  height = h;
  return NULL;
}


EXPORTED event_t* window_poll(int* out_count){
  SwapBuffers(hdc);
  MSG msg;
  BYTE keyboardState[256];
  WCHAR unicodeChar[4];
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
      GetKeyboardState(keyboardState);
      UINT scanCode = (msg.lParam >> 16) & 0xFF;
      int len = ToUnicode((UINT)msg.wParam, scanCode, keyboardState, unicodeChar, 4, 0);
      if (len == 1 && unicodeChar[0] >= 32 && unicodeChar[0] < 127) {
        printf("Key down: ASCII %d ('%c')\n", unicodeChar[0], unicodeChar[0]);
      } else {
        UINT virtualCode = (UINT)msg.wParam;
        printf("Key down: Special Key (VK 0x%02X)\n", virtualCode);
      }
      add_event(
        KEY_PRESSED,
        msg.wParam,
        0,
        0
      );
    }else if (msg.message == WM_KEYUP){
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