//

#include <math.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>

#define CONTEXT_2D  1
#define CONTEXT_3D  2

#include "platform/windowing.h"

uint64_t win_impl_init(int w, int h, int flag){

  const char* dir = getenv("DITHER_ROOT");
  if (!dir) dir = ".";
  char full_path[512];
  if (flag & CONTEXT_2D){
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, 
      #if DITHER_WIN_USE_COCOA
        #if DITHER_2D_USE_OPENGL
          "std/win/platform/glcocoa.so"
        #elif DITHER_2D_USE_COREGRAPHICS
          "std/win/platform/coregraphics.so"
        #else
          ""
        #endif
      #elif DITHER_WIN_USE_X11
        #if DITHER_2D_USE_OPENGL
          "std/win/platform/glx.so"
        #else
          ""
        #endif
      #else
        ""
      #endif
    );
  }else if (flag & CONTEXT_3D){
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, 
      #if DITHER_WIN_USE_COCOA
        "std/win/platform/glcocoa.so"
      #elif DITHER_WIN_USE_X11
        "std/win/platform/glx.so"
      #else
        ""
      #endif
    );
  }

  void *lib = dlopen(full_path, RTLD_NOW);
  windowing_init = dlsym(lib, "window_init");
  windowing_poll = dlsym(lib, "window_poll");
  windowing_exit = dlsym(lib, "window_exit");

  return (uintptr_t)windowing_init(w,h);
  
}

void win_impl_poll(void* data){
  int n_events = 1;
  event_t* event = windowing_poll(&n_events);
  if (n_events){
    memcpy(data+8, event, sizeof(event_t));
  }else{
    memset(data+8, 0, sizeof(event_t));
  }
}

void win_impl_exit(){
  windowing_exit();
}
