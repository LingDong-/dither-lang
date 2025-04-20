//
void str__length(){
  char* __ARG(s); 
  int l = strlen(s);
  __put_ret(&(s));
}

void str__chr(){
  int32_t __ARG(a); 
  char* o = __gc_alloc(VART_STR,2);

  o[0] = a;
    
  __put_ret(&(o));
}

