void exch_impl__json_enter_dic();
void exch_impl__json_exit_dic();

void exch_impl__json_enter_lst();
void exch_impl__json_exit_lst();

void exch_impl__json_key_len(int n);
void exch_impl__json_key(char* s);

void exch_impl__json_num(float x);
void exch_impl__json_str_len(int n);
void exch_impl__json_str(char* s);


#define _STATE_IDLE  0
#define _STATE_DIC_K 1
#define _STATE_DIC_V 2
#define _STATE_ARR   3
#define _STATE_VAL   4

int exch_impl__decode_json(char* src){
  int idx = 0;
  int len = strlen(src);
  int state = _STATE_IDLE;

  while (idx < len){
    char c = src[idx];
    if (state == _STATE_IDLE){
      if (c == '{'){
        exch_impl__json_enter_dic();
        state = _STATE_DIC_K;
        idx++;
      }else if (c == '['){
        exch_impl__json_enter_lst();
        state = _STATE_ARR;
        idx++;
      }else{
        state = _STATE_VAL;
      }
    }else if (state == _STATE_DIC_K){
      if (c == '"'){
        idx++;
        int idx0 = idx;
        while (1){
          char b = src[idx];
          if (b == '"'){
            break;
          }else if (b == '\\'){
            idx++;
          }
          idx ++;
        }
        exch_impl__json_key_len(idx-idx0);
        exch_impl__json_key(src+idx0);
        idx++;
      }else if (c == ':'){
        state = _STATE_DIC_V;
        idx++;
      }
    }else if (state == _STATE_ARR || state == _STATE_DIC_V || state == _STATE_VAL){
      if (c == '"'){
        idx++;
        int idx0 = idx;
        while (1){
          char b = src[idx];
          if (b == '"'){
            break;
          }else if (b == '\\'){
            idx++;
          }
          idx ++;
        }
        exch_impl__json_str_len(idx-idx0);
        exch_impl__json_str(src+idx0);
        idx++;
      }else if (c == ','){
        if (state == _STATE_DIC_V){
          state = _STATE_DIC_K;
        }
        idx++;
      }else if (c == '{'){
        idx += exch_impl__decode_json(src+idx);
      }else if (c == '['){
        idx += exch_impl__decode_json(src+idx);
      }else if (c == '}'){
        exch_impl__json_exit_dic();
        return idx+1;
      }else if (c == ']'){
        exch_impl__json_exit_lst();
        return idx+1;
      }else{
        int idx0 = idx;
        while (1){
          char b = src[idx];
          if (b == ',' || b == '}' || b == ']' || b == 0){
            break;
          }
          idx ++;
        }
        int n = idx-idx0+1;
        char s[n];
        memcpy(s,src+idx0,n-1);
        s[n-1] = 0;
        if (strcmp(s,"true")==0){
          exch_impl__json_num(1);
        }else if (strcmp(s,"false")==0){
          exch_impl__json_num(0);
        }else{
          exch_impl__json_num(atof(s));
        }
      }
    } 
  }
  return idx;
}
