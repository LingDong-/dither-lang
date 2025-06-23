//

#include <math.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#endif


void (*cb_init_found)(int, int);
void (*cb_poll_found)(void*);

uint64_t win_impl_init(int w, int h, int flags){
  void **cb_init_ptr = dlsym(RTLD_DEFAULT, "__win_intern_hook_init");
  cb_init_found = *cb_init_ptr;
  void **cb_poll_ptr = dlsym(RTLD_DEFAULT, "__win_intern_hook_poll");
  cb_poll_found = *cb_poll_ptr;
  // printf("%p %p %p %p\n",cb_init_ptr,cb_init_found,cb_poll_ptr,cb_poll_found);
  cb_init_found(w,h);
  return 0;
}

void win_impl_poll(void* data){
  cb_poll_found(data);
}

void win_impl_exit(){

}
