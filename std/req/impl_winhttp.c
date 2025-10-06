#include <windows.h>
#include <winhttp.h>
#include <stdio.h>

#pragma comment(lib, "winhttp.lib")

void req_impl__http(
  char *method, char *url,
  char *body, int n_body, char **headers, int n_headers,
  int* out_status, char** out_body, int* out_n_body,
  char*** out_headers, int* out_n_headers,
  char** out_url
) {
  int nurl = strlen(url);
  wchar_t* wurl = malloc((nurl+1)*sizeof(wchar_t));
  for (int i = 0; i < nurl+1; i++) wurl[i] = url[i];
  int nmethod = strlen(method);
  wchar_t* wmethod = malloc((nmethod+1)*sizeof(wchar_t));
  for (int i = 0; i < nmethod+1; i++) wmethod[i] = method[i];

  URL_COMPONENTS urlComp = {0};
  urlComp.dwStructSize = sizeof(urlComp);
  urlComp.dwHostNameLength = 1;
  urlComp.dwUrlPathLength = 1;
  WinHttpCrackUrl(wurl, 0, 0, &urlComp);

  wchar_t* host = malloc((urlComp.dwHostNameLength + 1) * sizeof(wchar_t));
  wchar_t* path = malloc((urlComp.dwUrlPathLength + 1) * sizeof(wchar_t));
  urlComp.lpszHostName = host;
  urlComp.dwHostNameLength += 1;
  urlComp.lpszUrlPath = path;
  urlComp.dwUrlPathLength += 1;
  WinHttpCrackUrl(wurl, 0, 0, &urlComp);

  free(wurl);

  HINTERNET hSession = WinHttpOpen(
    L"WinHTTP", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  HINTERNET hConnect = WinHttpConnect(hSession, host, urlComp.nPort, 0);
  HINTERNET hRequest = WinHttpOpenRequest(
    hConnect, wmethod, path,
    NULL, WINHTTP_NO_REFERER,
    WINHTTP_DEFAULT_ACCEPT_TYPES,
    urlComp.nScheme == INTERNET_SCHEME_HTTPS
        ? WINHTTP_FLAG_SECURE : 0);
        
  free(wmethod);
  free(host);
  free(path);

  for (int i = 0; i < n_headers; i++){
    int n = strlen(headers[i]);
    wchar_t* wheader = malloc((n+1)*sizeof(wchar_t));
    for (int j = 0; j < n; j++) wheader[i] = headers[i][j];
    WinHttpAddRequestHeaders(
      hRequest,wheader,(DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
    free(wheader);
  }

  WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                               body, n_body, n_body, 0);

  WinHttpReceiveResponse(hRequest, NULL);

  DWORD status = 0;
  DWORD size = sizeof(status);
  WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                      NULL, &status, &size, NULL);
  *out_status = status;

  DWORD headersSize = 0;
  wchar_t* headersBuffer = NULL;
  WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, 
    WINHTTP_HEADER_NAME_BY_INDEX, NULL, &headersSize, WINHTTP_NO_HEADER_INDEX);
  if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
    headersBuffer = (wchar_t*)malloc(headersSize);
    if (headersBuffer) {
      WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, 
        WINHTTP_HEADER_NAME_BY_INDEX, headersBuffer, &headersSize, WINHTTP_NO_HEADER_INDEX);
    }
  }
  int chunk_headers_size = headersSize/sizeof(wchar_t);
  char* chunk_headers = malloc(chunk_headers_size);
  for (int i = 0; i < chunk_headers_size; i++){
    chunk_headers[i] = headersBuffer[i];
  }
  free(headersBuffer);

  int n_lines = 0;
  for (int i = 1; i < chunk_headers_size; i++){
    if (chunk_headers[i] == '\n'){
      chunk_headers[i-1]=0;
      n_lines++;
    }
  }
  if (n_lines) n_lines--;
  char** lines = malloc(n_lines*sizeof(char*));
  int idx = 0;
  lines[idx++] = chunk_headers;
  for (int i = 1; i < chunk_headers_size; i++){
    if (chunk_headers[i] == '\n'){
      lines[idx++] = chunk_headers+i+1;
      if (idx == n_lines){
        break;
      }
    }
  }
  *out_headers = lines;
  *out_n_headers = n_lines;


  BYTE* bodyBuffer = NULL;
  size_t totalSize = 0;

  while (TRUE) {
    DWORD bytesAvailable = 0;
    WinHttpQueryDataAvailable(hRequest, &bytesAvailable);
    if (bytesAvailable == 0) {
      break;
    }
    bodyBuffer = (BYTE*)realloc(bodyBuffer, totalSize + bytesAvailable);
    DWORD bytesRead = 0;
    
    WinHttpReadData(hRequest, bodyBuffer + totalSize, bytesAvailable, &bytesRead);
    totalSize += bytesRead;
  }

  *out_body = bodyBuffer;
  *out_n_body = totalSize;
  *out_url = strdup(url); //not supported

  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);
}
