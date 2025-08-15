#include "interp.c"

#ifdef _WIN32
  #pragma comment(lib, "wsock32.lib")
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netdb.h>
#endif

int main(int argc, char** argv){
  char* pth_inp = NULL;
  char* pth_map = NULL;
  char* pth_tcp = NULL;
  for (int i = 1; i < argc; i++){
    if (strcmp(argv[i],"--map")==0){
      pth_map = argv[++i];
    }else if (strcmp(argv[i],"--tcp")==0){
      pth_tcp = argv[++i];
    }else{
      pth_inp = argv[i];
    }
  }

  FILE* fd;
  global_init();

  if (pth_tcp == NULL){
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
  }else{
    int npth = strlen(pth_tcp);
    char* host = strdup(pth_tcp);
    char* port = NULL;
    for (int i = 0; i < npth; i++){
      if (host[i]==':'){
        host[i]=0;
        port = host+(i+1);
      }
    }
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(1, 1), &wsa);
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *he = gethostbyname(host);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    server.sin_addr = *(struct in_addr *)he->h_addr;
    memset(server.sin_zero, 0, sizeof(server.sin_zero));
    connect(sock, (struct sockaddr *)&server, sizeof(server));
#else
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(host, port, &hints, &res);
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sock, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
#endif
    map_t* frame = frame_start();
  
    char buffer[1024];
    char* line = NULL;
    while (1) {
      int bytes = recv(sock, buffer, sizeof(buffer), 0);
      if (bytes <= 0) {
        break;
      }
      int did = 0;
      for (int i = 0; i < bytes; i++){
        if (buffer[i] == '\0'){
          did = 1;
          int n = line == NULL ? 0 : strlen(line);
          line = realloc(line,n+i+1);
          memcpy(line+n,buffer,i+1);
          
          FILE* fd;
          fd = tmpfile();
          fwrite(line,1,n+i+1,fd);
          rewind(fd);
          list_t instrs = read_ir(fd);
          // free_layouts(&(_G.layouts));
          // _G.layouts = read_layout(fd);
          map_t layouts = read_layout(fd);
          fclose(fd);

          map_t* m0 = &(_G.layouts);
          map_t* m1 = &(layouts);
          for (int k = 0; k < NUM_MAP_SLOTS; k++){
            for (int i = 0; i < m1->slots[k].len;i++){
              pair_t p = m1->slots[k].data[i];
              str_t s;
              s.data = p.key;
              s.len = strlen(p.key);
              map_overwrite(m0,&s,p.val);
              free(p.key);
            }
            free(m1->slots[k].data);
          }
          // map_nuke(m1);

          // print_instrs(&instrs);
          // print_layouts(&(_G.layouts));

          list_node_t* node = instrs.head;
          while (node){
            node = execute_instr(node);
          }
          // print_vars();
          line = realloc(line,bytes-i);
          memcpy(line,buffer+i+1,bytes-i-1);
          line[bytes-i-1] = 0;

          // free_instrs(&instrs);
          break;
        }
      }
      if (!did){
        int n = line == NULL ? 0 : strlen(line);
        line = realloc(line, n+bytes+1);
        memcpy(line+n,buffer,bytes);
        line[n+bytes] = 0;
      }else{
        const char *reply = "OK\n";
        send(sock, reply, (int)strlen(reply), 0);
      }
    }
    frame = frame_end();
  }
  global_exit();
}

