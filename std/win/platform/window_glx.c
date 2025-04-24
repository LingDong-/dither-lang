#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/glx.h>

#include <string.h>
#include <stdio.h>

#include "windowing.h"

#define MAX_EVENTS 64 

#ifndef EXPORTED
#define EXPORTED __attribute__((visibility("default")))
#endif

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

Display *display;
Window root;
XVisualInfo *vi;
Colormap cmap;
XSetWindowAttributes swa;
Window win;
GLXContext glxContext;
XEvent event;

EXPORTED void window_init(int w, int h){
  display = XOpenDisplay(NULL);
  root = DefaultRootWindow(display);
  static int visual_attribs[] = {
    GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_ALPHA_SIZE, 8,
    // GLX_SAMPLE_BUFFERS, 1,
    // GLX_SAMPLES, 4,
    None
  };
  vi = glXChooseVisual(display, 0, visual_attribs);
  // vi = glXChooseVisual(display, 0, (int[]) {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_ALPHA_SIZE, 8, None});
  cmap = XCreateColormap(display, root, vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;


  int screen = DefaultScreen(display);
  int screen_width = XDisplayWidth(display, screen);
  int screen_height = XDisplayHeight(display, screen);
  int x = (screen_width - w) / 2;
  int y = (screen_height - h) / 2;
  win = XCreateWindow(display, root, x, y, w, h, 0, vi->depth, InputOutput, vi->visual,
                      CWColormap | CWEventMask, &swa);
  XSizeHints *size_hints = XAllocSizeHints();
  if (size_hints) {
      size_hints->flags = PPosition;
      size_hints->x = x;
      size_hints->y = y;
      XSetNormalHints(display, win, size_hints);
      XFree(size_hints);
  }

  // win = XCreateWindow(display, root, 0, 0, w, h, 0, vi->depth, InputOutput, vi->visual,
  //                     CWColormap | CWEventMask, &swa);

  glxContext = glXCreateContext(display, vi, NULL, GL_TRUE);
  glXMakeCurrent(display, win, glxContext);

  XMapWindow(display, win);
  width = w;
  height = h;

  //XSetStandardProperties(dis,win,window_name,"",None,NULL,0,NULL);
}

int get_key_code(){
  char keybuf[32];
  KeySym keysym;
  int len = XLookupString(&event.xkey, keybuf, sizeof(keybuf), &keysym, NULL);
  if (len == 1){
    return keybuf[0];
  }
  return (int)keysym;
}

int kig = 0; //defeat autorepeat

EXPORTED event_t* window_poll(int* out_count){
  char keybuf[16];
  int n;
  while ((n = XPending(display)) > 0) {
    XNextEvent(display, &event);
    if (event.type == ConfigureNotify || event.type == MotionNotify){
      while (XEventsQueued(display, QueuedAfterReading)){
        XEvent nev;
        XPeekEvent(display, &nev);
        if (nev.type == event.type){
          XNextEvent(display, &event);
        }else{
          break;
        }
      }
    }
    if (event.type == ButtonPress) {
      add_event(
        MOUSE_PRESSED,
        event.xbutton.button,
        event.xbutton.x,
        event.xbutton.y
      );
    }else if (event.type == ButtonRelease) {
      add_event(
        MOUSE_RELEASED,
        event.xbutton.button,
        event.xbutton.x,
        event.xbutton.y
      );
    }else if (event.type == MotionNotify) {
      add_event(
        MOUSE_MOVED,
        0,
        event.xmotion.x,
        event.xmotion.y
      );
    }else if (event.type == KeyPress) {
      if (kig){
        kig = 0;
      }//else
      {
        add_event(
          KEY_PRESSED,
          get_key_code(),
          event.xkey.x,
          event.xkey.y
        );
      }
    }else if (event.type == KeyRelease) {
      if (XEventsQueued(display, QueuedAfterReading)){
        XEvent nev;
        XPeekEvent(display, &nev);
        if (nev.type == KeyPress && nev.xkey.time == event.xkey.time &&
            nev.xkey.keycode == event.xkey.keycode){
          kig = 1;
        }
      }
      if (!kig){
        add_event(
          KEY_RELEASED,
          get_key_code(),
          event.xkey.x,
          event.xkey.y
        );
      }
    }else if (event.type == ConfigureNotify){
      width = event.xconfigure.width;
      height = event.xconfigure.height;
      add_event(
        WINDOW_RESIZED,
        0,
        width,
        height
      );
    }
  }
  glXSwapBuffers(display, win);

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
  glXMakeCurrent(display, None, NULL);
  glXDestroyContext(display, glxContext);
  XDestroyWindow(display, win);
  XCloseDisplay(display);
}