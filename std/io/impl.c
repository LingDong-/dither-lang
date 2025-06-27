#include <stdio.h>
#include <dlfcn.h>

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
  fseek(fd, 0, SEEK_END);
  long file_size = ftell(fd);
  rewind(fd);
  char *buffer = malloc(file_size);
  volatile size_t _ = fread(buffer, 1, file_size, fd);
  fclose(fd);
  *n = file_size;
  return buffer;
}
