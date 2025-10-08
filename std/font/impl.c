#include "ttf2pl.h"
#include "hershey.h"

#define FEAT_SMCP 1
#define FEAT_ONUM 2
#define FEAT_HIST 4
#define FEAT_LIGA 256
#define FEAT_KERN 65536

#undef ARR_DEF
#define ARR_DEF(dtype) \
  typedef struct { int len; int cap; dtype* data; } dtype ## _arr_t;

#undef ARR_INIT
#define ARR_INIT(dtype,name) \
  name.len = 0;  \
  name.cap = 8; \
  name.data = (dtype*) malloc((name.cap)*sizeof(dtype));

#undef ARR_PUSH
#ifdef _WIN32
#define ARR_ITEM_FORCE_CAST(dtype,item) item
#else
#define ARR_ITEM_FORCE_CAST(dtype,item) (dtype)item
#endif
#define ARR_PUSH(dtype,name,item) \
  if (name.cap < name.len+1){ \
    int hs = name.cap/2; \
    name.cap = name.len+MAX(1,hs); \
    name.data = (dtype*)realloc(name.data, (name.cap)*sizeof(dtype) ); \
  }\
  name.data[name.len] = ARR_ITEM_FORCE_CAST(dtype,item);\
  name.len += 1;

#undef ARR_POP
#define ARR_POP(dtype,name) (name.data[--name.len])

#undef ARR_CLEAR
#define ARR_CLEAR(dtype,name) {name.len = 0;}


typedef struct font_st {
  FILE* fp;
  int fmt;
} font_t;

ARR_DEF(font_t);
font_t_arr_t fonts;

int font_impl__lookup(int id, int code, uint32_t flag){
  if (fonts.data[id].fmt){
    return hf_cmap_lookup(fonts.data[id].fp, fonts.data[id].fmt-1, code);
  }

  int gid = t2p_cmap_lookup(fonts.data[id].fp, code);
  if (flag & FEAT_SMCP){
    gid = t2p_gsub_lookup(fonts.data[id].fp,"smcp",gid,0,NULL);
  }
  if (flag & FEAT_ONUM){
    gid = t2p_gsub_lookup(fonts.data[id].fp,"onum",gid,0,NULL);
  }
  if (flag & FEAT_HIST){
    gid = t2p_gsub_lookup(fonts.data[id].fp,"hist",gid,0,NULL);
  }
  return gid;
}

int font_impl__ligature(int id, int* n_code, int* codes, uint32_t flag){
  if (!(*n_code)){
    return 0;
  }
  if (fonts.data[id].fmt){
    *n_code = 1;
    return hf_cmap_lookup(fonts.data[id].fp, fonts.data[id].fmt-1, codes[0]);
  }
  #if _WIN32
  int* gids = (int*)_alloca(*n_code*sizeof(int));
  #else
  int gids[*n_code];
  #endif

  for (int i = 0; i < *n_code; i++){
    gids[i] = t2p_cmap_lookup(fonts.data[id].fp, codes[i]);
  }
  int gid = -1;
  int n = *n_code - 1;
  if (flag & FEAT_LIGA){
    gid = t2p_gsub_lookup(fonts.data[id].fp,"liga",gids[0],&n,gids+1);
    if (gid == gids[0]){
      n = *n_code - 1;
      gid = t2p_gsub_lookup(fonts.data[id].fp,"dlig",gids[0],&n,gids+1);
      if (gid == gids[0]){
        n = *n_code - 1;
        gid = t2p_gsub_lookup(fonts.data[id].fp,"hlig",gids[0],&n,gids+1);
      }
    }
  }
  if (gid == -1){
    gid = gids[0];
    *n_code = 1;
  }
  *n_code = n+1;
  if (flag & FEAT_SMCP){
    gid = t2p_gsub_lookup(fonts.data[id].fp,"smcp",gid,0,NULL);
  }
  if (flag & FEAT_ONUM){
    gid = t2p_gsub_lookup(fonts.data[id].fp,"onum",gid,0,NULL);
  }
  if (flag & FEAT_HIST){
    gid = t2p_gsub_lookup(fonts.data[id].fp,"hist",gid,0,NULL);
  }
  
  return gid;
}


int n_polys;
int* m_polys;
float* polys;
int n_pts = 0;

void font_impl__moveto(float x, float y){
  m_polys = (int*)realloc(m_polys,sizeof(int)*(n_polys+1));
  m_polys[n_polys]=1;
  
  polys = realloc(polys,sizeof(float)*2*(n_pts+1));
  polys[n_pts*2] = x;
  polys[n_pts*2+1] = y;
  n_pts ++;
  n_polys++;
}
void font_impl__lineto(float x, float y){
  m_polys[n_polys-1]++;

  polys = realloc(polys,sizeof(float)*2*(n_pts+1));
  polys[n_pts*2] = x;
  polys[n_pts*2+1] = y;

  n_pts++;
}

float* font_impl__glyph(int id, int gid, int reso, int* n, int** m){
  n_polys = 0;
  n_pts = 0;
  if (fonts.data[id].fmt == 0){
    t2p_glyph(fonts.data[id].fp, gid, reso, font_impl__moveto, font_impl__lineto);
  }else{
    hf_glyph(fonts.data[id].fp, gid, font_impl__moveto, font_impl__lineto);
  }
  *n = n_polys;
  *m = m_polys;
  return polys;
}

float font_impl__advance(int id, int gid, int hid, int flag){
  if (fonts.data[id].fmt){
    return hf_advance(fonts.data[id].fp, gid);
  }
  return t2p_advance(fonts.data[id].fp, gid, hid, !!(flag & FEAT_KERN));
}

int font_impl_decode(int n_bytes, char* bytes){
  #ifdef _WIN32
    FILE *fp = tmpfile();
    fwrite(bytes,1,n_bytes,fp);
    rewind(fp);
  #else
    FILE* fp = fmemopen(bytes,n_bytes,"rb");
  #endif

  font_t f;
  f.fp = fp;
  f.fmt = 0;
  ARR_PUSH(font_t, fonts, f);
  return fonts.len-1;
}

int font_impl_hershey(int cset){
  #ifdef _WIN32
    FILE *fp = tmpfile();
    fwrite(hf_data,1,sizeof(hf_data),fp);
    rewind(fp);
  #else
    FILE* fp = fmemopen((char*)hf_data,sizeof(hf_data),"rb");
  #endif

  font_t f;
  f.fp = fp;
  f.fmt = cset;
  ARR_PUSH(font_t, fonts, f);
  return fonts.len-1;
}
