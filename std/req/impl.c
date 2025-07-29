#ifdef _WIN32
  #include "impl_winhttp.c"
#else
  #include "impl_curl.c"
#endif
