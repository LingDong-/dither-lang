//

#include <math.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>

#include "platform/windowing.h"


void win_impl_init(int w, int h){
  const char* dir = getenv("DITHER_ROOT");
  if (!dir) dir = ".";
  char full_path[512];
  snprintf(full_path, sizeof(full_path), "%s/%s", dir, "std/win/platform/windowing.so");

  void *lib = dlopen(full_path, RTLD_NOW);
  windowing_init = dlsym(lib, "window_init");
  windowing_poll = dlsym(lib, "window_poll");
  windowing_exit = dlsym(lib, "window_exit");

  windowing_init(w,h);

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
