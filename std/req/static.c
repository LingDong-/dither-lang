//
#include <stdio.h>

#include "impl.c"

void req___http(){
  char* re;
  __pop_arg(&re, sizeof(re));

  char* rq;
  __pop_arg(&rq, sizeof(rq));

  char* method = *(char**)(rq+28);
  char* url = *(char**)(rq+36);
  __list_t* headers = *(__list_t**)(rq+44);
  __list_t* body = *(__list_t**)(rq+52);

  int status;
  char* out_body;
  int out_n_body;
  char** out_headers;
  int out_n_headers;
  char* out_url;

  req_impl__http(method,url,body->data,body->n,(char**)(headers->data),headers->n,
    &status,&out_body,&out_n_body,&out_headers,&out_n_headers,&out_url
  );

  *(int32_t*)(re+24) = status;

  int n_out_url = strlen(out_url);
  char* s = __gc_alloc(VART_STR,n_out_url+1);
  memcpy(s,out_url,n_out_url);
  ((char**)(re+28))[0] = s;
  free(out_url);

  __list_t* oheaders = *(__list_t**)(re+36);
  oheaders->n = out_n_headers;
  oheaders->cap = out_n_headers+1;
  oheaders->data = realloc(oheaders->data,oheaders->cap*8);
  for (int i = 0; i < out_n_headers; i++){
    int n = strlen(out_headers[i]);
    char* s = __gc_alloc(VART_STR,n+1);
    memcpy(s, out_headers[i], n);
    ((char**)(oheaders->data))[i] = s;
  }
  free(out_headers[0]);
  free(out_headers);

  __list_t* obody = *(__list_t**)(re+44);
  obody->n = out_n_body;
  obody->cap = out_n_body+1;
  obody->data = realloc(obody->data, obody->cap);
  memcpy(obody->data, out_body, out_n_body);
  free(out_body);
}