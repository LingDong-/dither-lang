//
void str__length(){
  char* __ARG(s); 
  int32_t l = strlen(s);
  __put_ret(&(l));
}

void str__chr(){
  int32_t __ARG(a); 
  char* o = __gc_alloc(VART_STR,2);

  o[0] = a;
    
  __put_ret(&(o));
}

