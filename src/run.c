#include "interp.c"

int main(int argc, char** argv){
  char* pth_inp = NULL;
  char* pth_map = NULL;
  for (int i = 1; i < argc; i++){
    if (strcmp(argv[i],"--map")==0){
      pth_map = argv[++i];
    }else{
      pth_inp = argv[i];
    }
  }

  FILE* fd;
  global_init();
  fd = fopen(pth_inp,"rb");
  list_t instrs = read_ir(fd);
  _G.layouts = read_layout(fd);

  if (pth_map){
    fclose(fd);
    fd = fopen(pth_map,"rb");
    read_srcmap(&instrs, fd);
  }

  fclose(fd);
  execute(&instrs);
}

