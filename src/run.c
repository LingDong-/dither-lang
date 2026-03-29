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
  int do_step = 0;
  for (int i = 1; i < argc; i++){
    if (strcmp(argv[i],"--map")==0){
      pth_map = argv[++i];
    }else if (strcmp(argv[i],"--tcp")==0){
      pth_tcp = argv[++i];
    }else if (strcmp(argv[i],"--step")==0){
      do_step = 1;
    }else{
      pth_inp = argv[i];
    }
  }

  FILE* fd;
  global_init();

  if (pth_tcp == NULL){
    fd = fopen(pth_inp,"rb");
    list_t instrs = read_ir(fd);
    _G->layouts = read_layout(fd);

    if (pth_map){
      fclose(fd);
      fd = fopen(pth_map,"rb");
      read_srcmap(&instrs, fd);
    }

    fclose(fd);

    if (do_step){
      map_t* frame = frame_start();
      list_node_t* n = instrs.head;
      list_node_t* bp = NULL;
      while (n){
        char cmd[64];
        printf("\x1b[94mdbg>\x1b[0m");
        fflush(stdout);
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strcspn(cmd, "\r\n")] = 0;
        if (cmd[0] == 0 || !strncmp(cmd,"step",4)){
          int cnt = 1;
          if (cmd[0] && cmd[4]==' '){
            cnt = atoi(cmd+5);
          }
          for (int i = 0; i < cnt; i++){
            // print_instr((instr_t*)(n->data));
            n = execute_instr(n);
            if (!n || n == bp){
              break;
            }
          }
        }else if (!strcmp(cmd,"go")){
          while (n){
            n = execute_instr(n);
            if (n == bp){
              break;
            }
          }
        }else if (!strcmp(cmd,"dump")){
          print_vars();
        }else if (!strcmp(cmd,"list")){
          list_node_t* nn = instrs.head;
          while (nn){
            instr_t* ins = (instr_t*)nn->data;
            printf("%lx ",((uintptr_t)nn) );
            print_instr(ins);
            nn = nn->next;
          }
        }else if (!strncmp(cmd,"break ",6)){
          bp = (list_node_t *)(uintptr_t)strtoull(cmd+6,NULL,16);
        }else if (!strcmp(cmd,"peek")){
          printf("%lx ",((uintptr_t)n) );
          print_instr((instr_t*)(n->data));
        }
      }
      frame = frame_end();
      if (!(_G->flags & GFLG_NOGC)){
        gc_run();
      }

    }else{
      execute(&instrs);
    }

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
    map_t lblmap = {0};

    char buffer[1024];
    char* line = NULL;
    int dbg = 0;
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
          if (line[0] == '$'){
            if (!strcmp(line,"$dump")){
              print_vars();
            }else if (!strcmp(line,"$nuke")){
              global_exit();
              global_init();
              frame = frame_start();
            }else if (!strcmp(line,"$dbug")){
              dbg ^= 1;
            }
          }else{ 
            FILE* fd;
            fd = tmpfile();
            fwrite(line,1,n+i+1,fd);
            rewind(fd);
            list_t instrs = read_ir_update_labels(fd,&lblmap);
            // free_layouts(&(_G->layouts));
            // _G->layouts = read_layout(fd);
            map_t layouts = read_layout(fd);
            fclose(fd);

            if (dbg) print_layouts(&(layouts));

            map_t* m0 = &(_G->layouts);
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
            // if (dbg) print_layouts(&(_G->layouts));

            if (dbg) print_instrs(&instrs);
            
            list_node_t* node = instrs.head;
            term_t* insa = NULL;
            map_t* cur_frame = (map_t*)(_G->vars.tail->data);
            while (node){
              // if (dbg) print_instr(((instr_t*)(node->data)));

              if ((map_t*)(_G->vars.tail->data) == cur_frame){
                opran_t* a = ((instr_t*)(node->data))->a;
                if (a && a->tag == OPRD_TERM){
                  term_t* term = (term_t*)a;
                  if (term->mode == TERM_IDEN){
                    insa = term;
                  }
                }
              }
              node = execute_instr(node);
            }
            if (insa){
              var_t* v = find_var(&(insa->u.str));
              str_t s = str_new();
              printf("\x1b[2m%s = \x1b[0m",insa->u.str.data);
              to_str(v->type->vart, &(v->u), &s);
              printf("\x1b[2m%s\x1b[0m\n",s.data);
              free(s.data);
            }
            // print_vars();
            // free_instrs(&instrs);
          }
          line = realloc(line,bytes-i);
          memcpy(line,buffer+i+1,bytes-i-1);
          line[bytes-i-1] = 0;

          break;
        }
      }
      if (!did){
        int n = line == NULL ? 0 : strlen(line);
        line = realloc(line, n+bytes+1);
        memcpy(line+n,buffer,bytes);
        line[n+bytes] = 0;
      }else{
        fflush(stdout);
        fflush(stderr);
        const char *reply = "OK\n";
        send(sock, reply, (int)strlen(reply), 0);
      }
    }
    frame = frame_end();
  }
  global_exit();
}

