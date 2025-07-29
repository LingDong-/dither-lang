//

#include <stdio.h>
#include "../../src/interp.c"

#include "impl.c"

#ifndef EXPORTED
#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__ ((visibility ("default")))
#endif
#endif


EXPORTED void req__http(var_t* ret,  gstate_t* _g){

  obj_t* re = ARG_POP(_g,obj);
  obj_t* rq = ARG_POP(_g,obj);

  stn_t* method = *(stn_t**)(((char*)(rq->data))+8);
  stn_t* url = *(stn_t**)(((char*)(rq->data))+16);
  lst_t* headers = *(lst_t**)(((char*)(rq->data))+24);
  lst_t* body = *(lst_t**)(((char*)(rq->data))+32);

  char** iheaders = malloc(headers->n * 8);
  for (int i = 0; i < headers->n; i++){
    iheaders[i] = ((stn_t**)(headers->data))[i]->data;
  }

  int status;
  char* out_body;
  int out_n_body;
  char** out_headers;
  int out_n_headers;
  char* out_url;

  req_impl__http(method->data,url->data,body->data,body->n,iheaders,headers->n,
    &status,&out_body,&out_n_body,&out_headers,&out_n_headers,&out_url
  );

  free(iheaders);

  ((int32_t*)(re->data))[2] = status;

  stn_t* old_url = *((stn_t**)( ((char*)(re->data)) +12));
  int n_out_url = strlen(out_url);
  
  stn_t* s = gc_alloc_(_g,sizeof(stn_t)+n_out_url+1);
  s->n = n_out_url;
  s->w = 1;
  s->type = old_url->type;
  memcpy(s->data, out_url, n_out_url);
  ((stn_t**)( ((char*)(re->data)) +12))[0] = s;
  free(out_url);

  lst_t* oheaders = *((lst_t**)( ((char*)(re->data)) +20));
  oheaders->n = out_n_headers;
  oheaders->cap = out_n_headers+1;
  oheaders->data = realloc(oheaders->data, (oheaders->cap)*8);
  for (int i = 0; i < out_n_headers; i++){
    int n = strlen(out_headers[i]);
    stn_t* s = gc_alloc_(_g,sizeof(stn_t)+n+1);
    s->n = n;
    s->w = 1;
    s->type = old_url->type;
    memcpy(s->data, out_headers[i], n);
    ((stn_t**)(oheaders->data))[i] = s;
  }
  free(out_headers[0]);
  free(out_headers);

  lst_t* obody = *((lst_t**)( ((char*)(re->data)) +28));
  obody->n = out_n_body;
  obody->cap = out_n_body+1;
  obody->data = realloc(obody->data,(obody->cap));
  memcpy(obody->data, out_body, out_n_body);
  free(out_body);
}

EXPORTED void lib_init_req(gstate_t* _g){
  register_cfunc(&(_g->cfuncs), "req._http", req__http);
}


