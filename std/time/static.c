
#ifdef _WIN32
#include "impl_winapi.c"
#else
#include "impl_posix.c"
#endif

void time__fps(){
  float __ARG(x); 
  float y = time_impl_fps(x);
  __put_ret(&y);
}

void time__millis(){
  double y = time_impl_millis();
  __put_ret(&y);
}

void time__delay(){
  float __ARG(x); 
  time_impl_delay(x);
}


void time__stamp(){
  double y = time_impl_stamp();
  __put_ret(&y);
}


void time__local(){
  
  double __ARG(x);

  char* tup = __gc_alloc(VART_TUP,59);
  ((char*)tup)[0]  = VART_I32;
  ((char*)tup)[5]  = VART_I32;
  ((char*)tup)[10] = VART_I32;
  ((char*)tup)[15] = VART_I32;
  ((char*)tup)[20] = VART_I32;
  ((char*)tup)[25] = VART_I32;
  ((char*)tup)[30] = 0;

  *(int32_t*)(tup+1)  = 35;
  *(int32_t*)(tup+6)  = 39;
  *(int32_t*)(tup+11) = 43;
  *(int32_t*)(tup+16) = 47;
  *(int32_t*)(tup+21) = 51;
  *(int32_t*)(tup+26) = 55;
  *(int32_t*)(tup+31) = 59;

  time_impl_local(x,
    ((int32_t*)(tup+35))+0, 
    ((int32_t*)(tup+35))+1, 
    ((int32_t*)(tup+35))+2,
    ((int32_t*)(tup+35))+3,
    ((int32_t*)(tup+35))+4,
    ((int32_t*)(tup+35))+5
  );

  __put_ret(&tup);
}


