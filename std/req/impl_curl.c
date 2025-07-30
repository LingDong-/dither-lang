#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

typedef struct chunk_st {
  size_t size;
  char* data;
} chunk_t;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t real_size = size * nmemb;
  chunk_t* mem = (chunk_t*)userp;
  char* ptr = realloc(mem->data, mem->size + real_size);
  mem->data = ptr;
  memcpy(&(mem->data[mem->size]), contents, real_size);
  mem->size += real_size;
  return real_size;
}

void req_impl__http(
  char *method, char *url,
  char *body, int n_body, char **headers, int n_headers,
  int* out_status, char** out_body, int* out_n_body,
  char*** out_headers, int* out_n_headers,
  char** out_url
){
  CURL *curl = curl_easy_init();

  struct curl_slist *header_list = NULL;
  if (n_headers){
    for (int i = 0; i < n_headers; i++) {
      header_list = curl_slist_append(header_list, headers[i]);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
  }
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
  if (n_body) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, n_body);
  }
 
  chunk_t chunk_body = {0};
  chunk_t chunk_headers = {0};

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk_body);

  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &chunk_headers);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.88.1");

  CURLcode res = curl_easy_perform(curl);
  long status = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
  *out_status = status;

  *out_body = chunk_body.data;
  *out_n_body = chunk_body.size;
  
  char* final_url;
  curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &final_url);
  *out_url = strdup(final_url);

  int n_lines = 0;
  for (int i = 0; i < chunk_headers.size; i++){
    if (chunk_headers.data[i] == '\n'){
      chunk_headers.data[i-1]=0;
      n_lines++;
    }
  }
  if (n_lines) n_lines--;
  char** lines = malloc(n_lines*sizeof(char*));
  int idx = 0;
  lines[idx++] = chunk_headers.data;
  for (int i = 1; i < chunk_headers.size; i++){
    if (chunk_headers.data[i] == '\n'){
      lines[idx++] = chunk_headers.data+i+1;
      if (idx == n_lines){
        break;
      }
    }
  }
  *out_headers = lines;
  *out_n_headers = n_lines;
  curl_slist_free_all(header_list);
  curl_easy_cleanup(curl);
}