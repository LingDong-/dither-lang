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
  FILE* fd = fopen(s, "w");
  fwrite(data,1,n,fd);
  fclose(fd);
}
