#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define dlsym(handle,symbol) GetProcAddress(handle,symbol)
#define RTLD_DEFAULT GetModuleHandle(NULL)
#else
#include <dlfcn.h>
#endif

void (*__io_intern_hook_print_found)(char*);
int __io_intern_hooked_print = 0;

void io_impl_print(char* s){
  if (__io_intern_hooked_print == -1){
    printf("%s",s);
  }else if (__io_intern_hooked_print == 0){
    void **ptr = dlsym(RTLD_DEFAULT, "__io_intern_hook_print");
    if (ptr == NULL){
      __io_intern_hooked_print = -1;
      printf("%s",s);
    }else{
      __io_intern_hooked_print = 1;
      __io_intern_hook_print_found = *ptr;
      __io_intern_hook_print_found(s);
    }
  }else{
    __io_intern_hook_print_found(s);
  }
}

void io_impl_write_file(char* s, void* data, int n){
  FILE* fd = fopen(s, "wb");
  fwrite(data,1,n,fd);
  fclose(fd);
}


char* io_impl_read_file(char* s, int* n){
  FILE* fd = fopen(s, "rb");
  if (fd == NULL){
    *n = 0;
    return NULL;
  }
  size_t capacity = 4096;
  size_t size = 0;
  char *buffer = malloc(capacity);
  size_t bytes_read;
  while ((bytes_read = fread(buffer + size, 1, capacity - size, fd)) > 0) {
    size += bytes_read;
    if (size == capacity) {
      capacity *= 2;
      char *tmp = realloc(buffer, capacity);
      buffer = tmp;
    }
  }
  fclose(fd);
  *n = size;
  return buffer;
}
