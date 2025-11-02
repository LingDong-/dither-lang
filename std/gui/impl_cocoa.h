void gui_impl_init();
void gui_impl__slider1f(char* name,float x,float l,float r);
void gui_impl__slider1i(char* name,int x,int l,int r);
void gui_impl__toggle1i(char* name,int x);
void gui_impl__field1s(char* name,char* x);
float gui_impl__get1f(char* name);
int gui_impl__get1i(char* name);
char* gui_impl__get1s(char* name);
void gui_impl_poll();