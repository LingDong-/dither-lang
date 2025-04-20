#include "interp.c"

int main(int arc, char** argv){
  FILE* fd;
  global_init();
  fd = fopen(argv[1],"r");
  list_t instrs = read_ir(fd);
  _G.layouts = read_layout(fd);
  fclose(fd);
  execute(&instrs);
}

