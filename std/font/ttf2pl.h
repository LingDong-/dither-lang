#include <stdio.h>
#include <stdint.h>
#include <string.h>

void t2p_dbg_moveto(float x,float y) {
  printf("M %f %f ",(x),(y));
}
void t2p_dbg_lineto(float x,float y) {
  printf("L %f %f ",(x),(y));
}

uint16_t t2p_u16be(FILE* fp){
  return (fgetc(fp)<<8) | fgetc(fp);
}
uint32_t t2p_u32be(FILE* fp){
  return (fgetc(fp)<<24) | (fgetc(fp)<<16) | (fgetc(fp)<<8) | fgetc(fp);
}
int16_t t2p_i16be(FILE* fp){
  uint16_t u = t2p_u16be(fp);
  return *(int16_t*)(&u);;
}
float t2p_2d14(FILE* fp){
  int16_t i = t2p_i16be(fp);
  return (float)i/(float)(1<<14);
}

int t2p_table_lookup(FILE* fp, const char* name){
  fseek(fp,0,SEEK_SET);
  int offs = 0;
  if (t2p_u32be(fp) == 0x74746366){
    fseek(fp,12,SEEK_SET);
    offs = t2p_u32be(fp);
  }
  fseek(fp,offs+4,SEEK_SET);
  int ntbl = t2p_u16be(fp);

  for (int i = 0; i < ntbl; i++){
    fseek(fp,offs+12+16*i,SEEK_SET);
    if (name[0] != fgetc(fp)) continue;
    if (name[1] != fgetc(fp)) continue;
    if (name[2] != fgetc(fp)) continue;
    if (name[3] != fgetc(fp)) continue;
    fseek(fp,offs+20+16*i,SEEK_SET);
    return t2p_u32be(fp);
  }
  return 0;
}

int t2p_cmap_lookup(FILE* fp, int code){
  int offs = t2p_table_lookup(fp, "cmap");
  fseek(fp,offs,SEEK_SET);
  int vers = t2p_u16be(fp);
  int ntbl = t2p_u16be(fp);

  int fmt=-1;
  int sofs;
  int rank=-1;

  for (int i = 0; i < ntbl; i++){
    fseek(fp,offs+4+i*8,SEEK_SET);
    int platid = t2p_u16be(fp);
    int encid = t2p_u16be(fp);
    int sofsx = t2p_u32be(fp);
    fseek(fp,offs+sofsx,SEEK_SET);
    int fmtx = t2p_u16be(fp);
    int rankx =platid*10000+encid*100+fmtx; //heuristic
    if (fmtx == 6) rankx=1;
    if (rankx > rank){
      if ((fmtx == 0 || fmtx == 4 || fmtx == 6 || fmtx == 12)){
        fmt = fmtx;
        sofs = sofsx;
        rank = rankx;
      }
    }
  }
  if (fmt == 0){
    if (code > 255) return 0;
    fseek(fp,offs+sofs+6+code,SEEK_SET);
    return fgetc(fp);
  }else if (fmt == 4){
    fseek(fp,offs+sofs+6,SEEK_SET);
    int nseg = t2p_u16be(fp)>>1;
    for (int j = 0; j < nseg; j++){
      fseek(fp,offs+sofs+14+j*2,SEEK_SET);
      int cend = t2p_u16be(fp);
      if (code > cend) continue;
      fseek(fp,offs+sofs+16+nseg*2+j*2,SEEK_SET);
      int cstart = t2p_u16be(fp);
      if (code < cstart) continue;
      fseek(fp,offs+sofs+16+nseg*4+j*2,SEEK_SET);
      int idelta = t2p_i16be(fp);
      int pidoffs = offs+sofs+16+nseg*6+j*2;
      fseek(fp,pidoffs,SEEK_SET);
      int idoffs = t2p_u16be(fp);
      int gid;
      if (idoffs == 0){
        gid = code + idelta;
      }else{
        fseek(fp, pidoffs+idoffs+(code-cstart)*2,SEEK_SET);
        gid = t2p_u16be(fp);
        if (gid){
          gid += idelta;
        }
      }
      gid &= 0xffff;
      return gid;
    }
  }else if (fmt == 6){
    fseek(fp,offs+sofs+6,SEEK_SET);
    int cstart = t2p_u16be(fp);
    int nent = t2p_u16be(fp);
    if (code < cstart || code >= cstart+nent) return 0;
    fseek(fp,(code - cstart)*2,SEEK_CUR);
    return t2p_u16be(fp);
  }else if (fmt == 12){
    fseek(fp,offs+sofs+12,SEEK_SET);
    int nseg = t2p_u32be(fp);
    for (int j = 0; j < nseg; j++){
      fseek(fp,offs+sofs+16+j*12,SEEK_SET);
      int cstart = t2p_u32be(fp);
      if (code < cstart) continue;

      int cend = t2p_u32be(fp);
      if (code > cend) continue;

      int sid = t2p_u32be(fp);
      return code-cstart+sid;
    }
  }
  
  return 0;
}

int t2p_loca_lookup(FILE* fp, int gid){
  int head = t2p_table_lookup(fp, "head");
  fseek(fp,head+50,SEEK_SET);
  int fmt = t2p_i16be(fp);
  int offs = t2p_table_lookup(fp,"loca");
  if (fmt){
    fseek(fp,offs+gid*4,SEEK_SET);
    int i0 = t2p_u32be(fp);
    int i1 = t2p_u32be(fp);
    if (i0 == i1) return -1;
    return i0;
  }else{
    fseek(fp,offs+gid*2,SEEK_SET);
    int i0 = t2p_u16be(fp);
    int i1 = t2p_u16be(fp);
    if (i0 == i1) return -1;
    return i0*2;
  }
  return 0;
}

void t2p_glyf_lookup(
  FILE* fp, int gid, float* trfm,
  int* no, float* xo, float* yo,
  void (*cb)(char,float,float)
){
  int loca = t2p_loca_lookup(fp,gid);
  if (loca < 0){
    if (no) *no = 0;
    if (xo) *xo = 0;
    if (yo) *yo = 0;
    return;
  }
  int offs = t2p_table_lookup(fp, "glyf") + loca;
  fseek(fp,offs,SEEK_SET);

  int nctr = t2p_i16be(fp);

  if (nctr < 0){
    fseek(fp,offs+10,SEEK_SET);
    int flags;
    float lx = 0;
    float ly = 0;
    do {
      flags = t2p_u16be(fp);
      int cid = t2p_u16be(fp);
      int a1,a2;
      if (flags & 0x2){
        if ( flags & 0x1) {
          a1 = t2p_i16be(fp);
          a2 = t2p_i16be(fp);
        } else {
          a1 = (int8_t)fgetc(fp);
          a2 = (int8_t)fgetc(fp);
        }
      }else{
        if ( flags & 0x1) {
          a1 = t2p_u16be(fp);
          a2 = t2p_u16be(fp);
        } else {
          a1 = fgetc(fp);
          a2 = fgetc(fp);
        }
      }
      float mat[6] = {
        1,0,0,
        0,1,0
      };
      if ( flags & 0x8 ) {
        mat[0] = mat[3] = t2p_2d14(fp);
      } else if ( flags & 0x40 ) {
        mat[0] = t2p_2d14(fp);
        mat[4] = t2p_2d14(fp);
      } else if ( flags & 0x80 ) {
        mat[0] = t2p_2d14(fp);
        mat[3] = t2p_2d14(fp);
        mat[1] = t2p_2d14(fp);
        mat[4] = t2p_2d14(fp);
      }
      int old = ftell(fp);
      if (flags & 0x2){
        if (flags & 0x0800){
          mat[2] = a1 * mat[0] + a2 * mat[1];
          mat[5] = a1 * mat[3] + a2 * mat[4];
        }else{
          mat[2] = a1;
          mat[5] = a2;
        }
        mat[2] -= lx;
        mat[5] -= ly;
      }else{
        int ne = a1;
        float x0,y0,x1,y1;
        t2p_glyf_lookup(fp,gid,trfm,&ne,&x0,&y0,NULL);
        ne = a2;
        t2p_glyf_lookup(fp,cid,trfm,&ne,&x1,&y1,NULL);
        mat[2] = x0-x1;
        mat[5] = y0-y1;
      }
      if (trfm){
        float nmat[6] = {
          trfm[0]*mat[0]+trfm[1]*mat[3],
          trfm[0]*mat[1]+trfm[1]*mat[4],
          trfm[0]*mat[2]+trfm[1]*mat[5]+trfm[2],
          trfm[3]*mat[0]+trfm[4]*mat[3],
          trfm[3]*mat[1]+trfm[4]*mat[4],
          trfm[3]*mat[2]+trfm[4]*mat[5]+trfm[5],
        };
        memcpy(mat,nmat,6*sizeof(float));
      }
      float xe,ye;
      if (no && ((*no) != -1)){
        int ne = *no;
        t2p_glyf_lookup(fp,cid,mat,&ne,&xe,&ye,cb);
        if (ne == *no){
          *xo = lx;
          *yo = ly;
          return;
        }else{
          *no = (*no) - ne;
        }
      }else{
        t2p_glyf_lookup(fp,cid,mat,NULL,&xe,&ye,cb);
      }
      lx += xe;
      ly += ye;
      fseek(fp,old,SEEK_SET);
    } while ( flags & 0x20 );
    return;
  }
  fseek(fp,offs+10+2*nctr,SEEK_SET);
  int nins = t2p_u16be(fp);

  fseek(fp,offs+10+2*(nctr-1),SEEK_SET);
  int npts = t2p_u16be(fp)+1;

  int xlen = 0;
  int xrem = npts;
  int fofs = offs+12+2*nctr+nins;
  fseek(fp,fofs,SEEK_SET);
  while (xrem){
    int flag = fgetc(fp);
    int mul = 1;
    if (flag & 0x08){ //REPEAT_FLAG
      mul += fgetc(fp);
    }
    xrem -= mul;
    if (flag & 0x02){ //X_SHORT_VECTOR
      xlen += mul;
      continue;
    }else{
      if (flag & 0x10){ //X_IS_SAME
        continue;
      }else{
        xlen += 2*mul;
      }
    }
  }
  
  int xofs = ftell(fp);
  int yofs = xofs+xlen;
  int pidx = 0;
  int pend = 0;
  float x = 0;
  float y = 0;
  while (pidx < npts){
    fseek(fp,fofs,SEEK_SET);
    int flag = fgetc(fp);
    int mul = 1;
    int oncurve = flag & 0x01;
    if (flag & 0x08){ //REPEAT_FLAG
      mul += fgetc(fp);
    }
    fofs = ftell(fp);
    for (int i = 0; i < mul; i++){
      float dx,dy;
      fseek(fp,xofs,SEEK_SET);
      if (flag & 0x02){ //X_SHORT_VECTOR
        if (flag & 0x10){ //POSITIVE_X
          dx = fgetc(fp);
        }else{
          dx = -fgetc(fp);
        }
      }else{
        if (flag & 0x10){ //X_IS_SAME
          dx = 0;
        }else{
          dx = t2p_i16be(fp);
        }
      }
      xofs = ftell(fp);
      fseek(fp,yofs,SEEK_SET);
      if (flag & 0x04){ //Y_SHORT_VECTOR
        if (flag & 0x20){ //POSITIVE_Y
          dy = fgetc(fp);
        }else{
          dy = -fgetc(fp);
        }
      }else{
        if (flag & 0x20){ //Y_IS_SAME
          dy = 0;
        }else{
          dy = t2p_i16be(fp);
        }
      }
      yofs = ftell(fp);
      int isend = 0;
      fseek(fp,offs+10+pend*2,SEEK_SET);
      if (pidx == t2p_u16be(fp)){
        isend = 0x80;
        pend++;
      }
      if (trfm){
        float dx1 = dx * trfm[0] + dy * trfm[1];
        float dy1 = dx * trfm[3] + dy * trfm[4];
        if (pidx == 0){
          dx1 += trfm[2];
          dy1 += trfm[5];
        }
        dx = dx1;
        dy = dy1;
      }
      x += dx;
      y += dy;
      if (no && ((*no) != -1) && ((*no)==pidx)){
        *xo = x;
        *yo = y;
        return;
      }
      if (cb) cb(isend|oncurve,dx,dy);
      pidx++;
    }
  }
  if (no) *no = npts;
  if (xo) *xo = x;
  if (yo) *yo = y;
}

struct {
  float x;
  float y;
  float x0;
  float y0;
  float qx;
  float qy;
  float qx0;
  float qy0;
  int q;
  int q0;
  int first;
  float unit;
  int reso;
  void (*lineto)(float,float);
  void (*moveto)(float,float);
} t2p_ds;

void t2p_quadratic_bezier(
  float x0,float y0,float x1,float y1,
  float x2,float y2,
  float t, float* xo, float* yo){
  float s = 1-t;
  float s2 = s*s;
  float t2 = t*t;
  (*xo) = s2*x0+2*s*t*x1+t2*x2;
  (*yo) = s2*y0+2*s*t*y1+t2*y2;
}

void t2p_discretizer(char flag, float dx, float dy){
  float dxf = dx *t2p_ds.unit;
  float dyf =-dy *t2p_ds.unit;
  float x = t2p_ds.x + t2p_ds.qx + dxf;
  float y = t2p_ds.y + t2p_ds.qy + dyf;
  if (t2p_ds.first){
    if (flag & 0x01){
      t2p_ds.moveto(t2p_ds.x0 = t2p_ds.x = x, t2p_ds.y0 = t2p_ds.y = y);
      t2p_ds.q = t2p_ds.q0 = 0;
      t2p_ds.qx = t2p_ds.qy = 0;
    }else{
      t2p_ds.q0 = t2p_ds.q = 1;
      t2p_ds.qx = dxf;
      t2p_ds.qy = dyf;
    }
    t2p_ds.first = 0;
  }else if (!(flag & 0x01)){
    if (t2p_ds.q){
      float cx = t2p_ds.x + t2p_ds.qx;
      float cy = t2p_ds.y + t2p_ds.qy;
      float mx = (cx + x)*0.5;
      float my = (cy + y)*0.5;
      if (t2p_ds.q0 == 1){
        t2p_ds.x0 = mx;
        t2p_ds.y0 = my;
        t2p_ds.qx0 = cx;
        t2p_ds.qy0 = cy;
        t2p_ds.q0 = 2;
        t2p_ds.moveto(mx,my);
      }else{
        for (int i = 0; i < t2p_ds.reso; i++){
          float t = (i+1)/(float)t2p_ds.reso;
          float xt,yt;
          t2p_quadratic_bezier(t2p_ds.x,t2p_ds.y,cx,cy,mx,my,t,&xt,&yt);
          t2p_ds.lineto(xt,yt);
        }
      }
      t2p_ds.x = mx;
      t2p_ds.y = my;
      t2p_ds.qx = x-mx;
      t2p_ds.qy = y-my;
    }else{
      t2p_ds.qx = dxf;
      t2p_ds.qy = dyf;
      t2p_ds.q = 1;
    }
  }else if (t2p_ds.q){
    float cx = t2p_ds.x + t2p_ds.qx;
    float cy = t2p_ds.y + t2p_ds.qy;
    if (t2p_ds.q0 == 1){
      t2p_ds.qx0 = cx;
      t2p_ds.qy0 = cy;
      t2p_ds.q0 = 2;
      t2p_ds.moveto(t2p_ds.x0 = x, t2p_ds.y0 = y);
    }else{
      for (int i = 0; i < t2p_ds.reso; i++){
        float t = (i+1)/(float)t2p_ds.reso;
        float xt,yt;
        t2p_quadratic_bezier(t2p_ds.x,t2p_ds.y,cx,cy,x,y,t,&xt,&yt);
        t2p_ds.lineto(xt,yt);
      }
    }
    t2p_ds.q = 0;
    t2p_ds.qx = t2p_ds.qy = 0;
    t2p_ds.x = x;
    t2p_ds.y = y;
  }else{
    t2p_ds.lineto(t2p_ds.x = x, t2p_ds.y = y);
  }
  if (flag & 0x80){
    if (t2p_ds.q){
      float cx = t2p_ds.x + t2p_ds.qx;
      float cy = t2p_ds.y + t2p_ds.qy;
      if (t2p_ds.q0){
        float mx = (cx + t2p_ds.qx0)*0.5;
        float my = (cy + t2p_ds.qy0)*0.5;
        for (int i = 0; i < t2p_ds.reso; i++){
          float t = (i+1)/(float)t2p_ds.reso;
          float xt,yt;
          t2p_quadratic_bezier(t2p_ds.x,t2p_ds.y,cx,cy,mx,my,t,&xt,&yt);
          t2p_ds.lineto(xt,yt);
        }
        for (int i = 0; i < t2p_ds.reso; i++){
          float t = (i+1)/(float)t2p_ds.reso;
          float xt,yt;
          t2p_quadratic_bezier(mx,my,t2p_ds.qx0,t2p_ds.qy0,t2p_ds.x0,t2p_ds.y0,t,&xt,&yt);
          t2p_ds.lineto(xt,yt);
        }
      }else{
        for (int i = 0; i < t2p_ds.reso; i++){
          float t = (i+1)/(float)t2p_ds.reso;
          float xt,yt;
          t2p_quadratic_bezier(t2p_ds.x,t2p_ds.y,cx,cy,t2p_ds.x0,t2p_ds.y0,t,&xt,&yt);
          t2p_ds.lineto(xt,yt);
        }
      }
      t2p_ds.q = t2p_ds.q0 = 0;
      t2p_ds.qx = t2p_ds.qy = 0;
      t2p_ds.x = x;
      t2p_ds.y = y;
    }else if (t2p_ds.q0){
      for (int i = 0; i < t2p_ds.reso; i++){
        float t = (i+1)/(float)t2p_ds.reso;
        float xt,yt;
        t2p_quadratic_bezier(t2p_ds.x,t2p_ds.y,t2p_ds.qx0,t2p_ds.qy0,t2p_ds.x0,t2p_ds.y0,t,&xt,&yt);
        t2p_ds.lineto(xt,yt);
      }
    }else if (x != t2p_ds.x0 || y != t2p_ds.y0){
      t2p_ds.lineto(t2p_ds.x0,t2p_ds.y0);
    }
    t2p_ds.first = 1;
  }
}

void t2p_glyph(FILE* fp, int gid, int reso,
  void (*moveto)(float,float),
  void (*lineto)(float,float)
){
  int head = t2p_table_lookup(fp, "head");
  fseek(fp,head+18,SEEK_SET);
  t2p_ds.unit = 1.0/t2p_u16be(fp);
  t2p_ds.x = 0;
  t2p_ds.y = 0;
  t2p_ds.q = 0;
  t2p_ds.q0 = 0;
  t2p_ds.qx = 0;
  t2p_ds.qy = 0;
  t2p_ds.first = 1;
  t2p_ds.reso = reso;
  t2p_ds.lineto = lineto?lineto:t2p_dbg_lineto;
  t2p_ds.moveto = moveto?moveto:t2p_dbg_moveto;
  t2p_glyf_lookup(fp,gid,NULL,NULL,NULL,NULL,t2p_discretizer);
}

void t2p_hmtx_lookup(FILE* fp, int gid, int* advw, int* lsb){
  int hhea = t2p_table_lookup(fp, "hhea");
  fseek(fp,hhea+34,SEEK_SET);
  int nhm = t2p_u16be(fp);
  int hmtx = t2p_table_lookup(fp, "hmtx");
  if (gid < nhm){
    fseek(fp,hmtx+gid*4,SEEK_SET);
    *advw = t2p_u16be(fp);
    *lsb = t2p_i16be(fp);
  }else{
    fseek(fp,hmtx+(nhm-1)*4,SEEK_SET);
    *advw = t2p_u16be(fp);
    fseek(fp,hmtx+nhm*4+(gid-nhm)*2,SEEK_SET);
    *lsb = t2p_i16be(fp);
  }
}

int t2p_kern_lookup(FILE* fp, int aid, int bid){
  uint32_t key = ((uint32_t)aid<<16) | bid;
  int kern = t2p_table_lookup(fp, "kern");
  fseek(fp,kern+2,SEEK_SET);
  int ntbl = t2p_u16be(fp);

  fseek(fp,kern+4,SEEK_SET);
  for (int i = 0; i < ntbl; i++){
    int ver = t2p_u16be(fp);
    int len = t2p_u16be(fp);
    int cov = t2p_u16be(fp);
    int fmt = (cov>>8) & 0xff;
    if (fmt != 0){
      fseek(fp,len-6,SEEK_CUR);
      continue;
    }
    int npair = t2p_u16be(fp);
    int sran = t2p_u16be(fp);
    int esel = t2p_u16be(fp);
    int rshf = t2p_u16be(fp);
    int offs = ftell(fp);
    int probe = sran;
    int base = 0;
    while (probe){
      fseek(fp,offs+base+probe,SEEK_SET);
      uint32_t pair = t2p_u32be(fp);
      if (key == pair){
        return t2p_i16be(fp);
      }else if (key > pair){
        base += probe;
      }
      probe >>= 1;
    }
  }
  return 0;
}


int t2p_gsub_lookup(FILE* fp, char* tag, int gid, int* num_next, int* next_gids){
  int gsub = t2p_table_lookup(fp, "GSUB");
  if (gsub == 0) return gid;
  fseek(fp,gsub+6,SEEK_SET);
  int featl = t2p_u16be(fp);
  int lul = t2p_u16be(fp);
  fseek(fp,gsub+featl,SEEK_SET);
  int nfeat = t2p_i16be(fp);
  for (int i = 0; i < nfeat; i++){
    fseek(fp,gsub+featl+2+i*6,SEEK_SET);
    if (tag[0] != fgetc(fp)) continue;
    if (tag[1] != fgetc(fp)) continue;
    if (tag[2] != fgetc(fp)) continue;
    if (tag[3] != fgetc(fp)) continue;
    int offs = t2p_u16be(fp);
    fseek(fp,gsub+featl+offs,SEEK_SET);
    int param = t2p_i16be(fp);
    int nlui = t2p_u16be(fp);
    for (int j = 0; j < nlui; j++){
      fseek(fp,gsub+featl+offs+4+j*2,SEEK_SET);
      int idx = t2p_u16be(fp);
      fseek(fp, gsub+lul+2+idx*2, SEEK_SET);
      int lofs = t2p_u16be(fp);
      fseek(fp, gsub+lul+lofs, SEEK_SET);
      int typ = t2p_u16be(fp);
      if (typ != 1 && typ != 4) continue;

      int flg = t2p_u16be(fp);
      int nst = t2p_u16be(fp);
      for (int k = 0; k < nst; k++){
        fseek(fp, gsub+lul+lofs+6+k*2, SEEK_SET);
        int sofs = t2p_u16be(fp);
        fseek(fp,gsub+lul+lofs+sofs,SEEK_SET);
        int sfmt = t2p_u16be(fp);
        int cofs = t2p_u16be(fp);
        fseek(fp,gsub+lul+lofs+sofs+cofs,SEEK_SET);
        int cfmt = t2p_u16be(fp);
        int cidx = -1;
        if (cfmt == 1){
          int n = t2p_u16be(fp);
          for (int l = 0; l < n; l++){
            if (gid == t2p_u16be(fp)){
              cidx = l;
              break;
            }
          }
        }else if (cfmt == 2){
          int n = t2p_u16be(fp);
          for (int l = 0; l < n; l++){
            int gstart = t2p_u16be(fp);
            int gend = t2p_u16be(fp);
            int sci = t2p_u16be(fp);
            if (gstart <= gid && gid <= gend){
              cidx = gid-gstart+sci;
              break;
            }
          }
        }else{
          continue;
        }
        if (cidx < 0) continue;
        fseek(fp,gsub+lul+lofs+sofs+4,SEEK_SET);
        if (typ == 1){
          if (sfmt == 1){
            int delta = t2p_i16be(fp);
            return (gid + delta) & 0xffff;
          }else if (sfmt == 2){
            int nglyf = t2p_u16be(fp);
            fseek(fp,cidx*2,SEEK_CUR);
            return t2p_u16be(fp);
          }
        }else if (typ == 4){
          if (sfmt == 1){
            int nlset = t2p_u16be(fp);
            fseek(fp,gsub+lul+lofs+sofs+6+cidx*2,SEEK_SET);
            int lsofs = t2p_u16be(fp);
            fseek(fp,gsub+lul+lofs+sofs+lsofs,SEEK_SET);
            int nlig = t2p_u16be(fp);

            for (int n = *num_next; n > 0; n--){
              for (int l = 0; l < nlig; l++){
                fseek(fp,gsub+lul+lofs+sofs+lsofs+2+l*2,SEEK_SET);
                int lgofs = t2p_u16be(fp);
                fseek(fp,gsub+lul+lofs+sofs+lsofs+lgofs,SEEK_SET);
                int lid = t2p_u16be(fp);
                int ncom = t2p_u16be(fp)-1;
                if (ncom != n) continue;
                int ok = 1;
                for (int m = 0; m < ncom; m++){
                  if (next_gids[m] != t2p_u16be(fp)){
                    ok = 0;
                    break;
                  }
                }
                if (ok){
                  *num_next = n;
                  return lid;
                }
              }
            
            }
          }
        }
      }
    }
  }
  if (num_next)*num_next = 0;
  return gid;
}

int t2p_class_def_lookup(FILE* fp, int gid){
  int cdofs = ftell(fp);
  int fmt = t2p_u16be(fp);
  if (fmt == 1){
    int gstart = t2p_u16be(fp);
    int nglyf = t2p_u16be(fp);
    if (gstart <= gid && gid < gstart+nglyf){
      int ci = gid-gstart;
      fseek(fp,cdofs+6+ci*2,SEEK_SET);
      return t2p_u16be(fp);
    }
  }else if (fmt == 2){
    int ncran = t2p_u16be(fp);
    for (int i = 0; i < ncran; i++){
      int gstart = t2p_u16be(fp);
      int gend = t2p_u16be(fp);
      int cls = t2p_u16be(fp);
      if (gstart <= gid && gid <= gend){
        return cls;
      }
    }
  }
  return 0;
}


int t2p_gpos_kern_lookup(FILE* fp, int aid, int bid){
  int gsub = t2p_table_lookup(fp, "GPOS");
  if (gsub == 0) return 0;
  fseek(fp,gsub+6,SEEK_SET);
  int featl = t2p_u16be(fp);
  int lul = t2p_u16be(fp);
  fseek(fp,gsub+featl,SEEK_SET);
  int nfeat = t2p_i16be(fp);
  for (int i = 0; i < nfeat; i++){
    fseek(fp,gsub+featl+2+i*6,SEEK_SET);
    if ('k' != fgetc(fp)) continue;
    if ('e' != fgetc(fp)) continue;
    if ('r' != fgetc(fp)) continue;
    if ('n' != fgetc(fp)) continue;
    int offs = t2p_u16be(fp);
    fseek(fp,gsub+featl+offs,SEEK_SET);
    int param = t2p_i16be(fp);
    int nlui = t2p_u16be(fp);
    for (int j = 0; j < nlui; j++){
      fseek(fp,gsub+featl+offs+4+j*2,SEEK_SET);
      int idx = t2p_u16be(fp);
      fseek(fp, gsub+lul+2+idx*2, SEEK_SET);
      int lofs = t2p_u16be(fp);
      fseek(fp, gsub+lul+lofs, SEEK_SET);
      int typ = t2p_u16be(fp);

      if (typ != 2) continue;
      int flg = t2p_u16be(fp);
      int nst = t2p_u16be(fp);
      for (int k = 0; k < nst; k++){
        fseek(fp, gsub+lul+lofs+6+k*2, SEEK_SET);
        int sofs = t2p_u16be(fp);
        fseek(fp,gsub+lul+lofs+sofs,SEEK_SET);
        int sfmt = t2p_u16be(fp);
        int cofs = t2p_u16be(fp);
        fseek(fp,gsub+lul+lofs+sofs+cofs,SEEK_SET);
        int cfmt = t2p_u16be(fp);
        int cidx = -1;
        if (cfmt == 1){
          int n = t2p_u16be(fp);
          for (int l = 0; l < n; l++){
            if (aid == t2p_u16be(fp)){
              cidx = l;
              break;
            }
          }
        }else if (cfmt == 2){
          int n = t2p_u16be(fp);
          for (int l = 0; l < n; l++){
            int gstart = t2p_u16be(fp);
            int gend = t2p_u16be(fp);
            int sci = t2p_u16be(fp);
            if (gstart <= aid && aid <= gend){
              cidx = aid-gstart+sci;
              break;
            }
          }
        }else{
          continue;
        }
        if (cidx < 0) continue;
        fseek(fp,gsub+lul+lofs+sofs+4,SEEK_SET);

        if (typ == 2){
          int vfmt1 = t2p_u16be(fp);
          int vfmt2 = t2p_u16be(fp);
          if (vfmt1 != 4 || vfmt2 != 0){
            continue;
          }
          int v1sz = ((vfmt1&1)+!!(vfmt1&2)+!!(vfmt1&4)+!!(vfmt1&8)+!!(vfmt1&16)+!!(vfmt1&32)+!!(vfmt1&64)+!!(vfmt1&128))*2;
          int v2sz = ((vfmt2&1)+!!(vfmt2&2)+!!(vfmt2&4)+!!(vfmt2&8)+!!(vfmt2&16)+!!(vfmt2&32)+!!(vfmt2&64)+!!(vfmt2&128))*2;

          if (sfmt == 1){
            int npset = t2p_u16be(fp);
            fseek(fp,gsub+lul+lofs+sofs+10+cidx*2,SEEK_SET);
            int psofs = t2p_u16be(fp);

            fseek(fp,gsub+lul+lofs+sofs+psofs,SEEK_SET);
            int npval = t2p_u16be(fp);
            for (int l = 0; l < npval; l++){
              fseek(fp,gsub+lul+lofs+sofs+psofs+2+(2+v1sz+v2sz)*l,SEEK_SET);
              int gid = t2p_u16be(fp);
              if (gid == bid){
                fseek(fp,gsub+lul+lofs+sofs+psofs+2+(2+v1sz+v2sz)*l+2,SEEK_SET);
                return t2p_i16be(fp);
              }
            }

          }else if (sfmt == 2){
            int cd1ofs = t2p_u16be(fp);
            int cd2ofs = t2p_u16be(fp);
            int nc1 = t2p_u16be(fp);
            int nc2 = t2p_u16be(fp);
            
            fseek(fp,gsub+lul+lofs+sofs+cd1ofs,SEEK_SET);
            int cls1 = t2p_class_def_lookup(fp,aid);

            fseek(fp,gsub+lul+lofs+sofs+cd2ofs,SEEK_SET);
            int cls2 = t2p_class_def_lookup(fp,bid);

            fseek(fp,gsub+lul+lofs+sofs+16 + cls1 * (v1sz+v2sz)*nc2 + cls2*(v1sz+v2sz),SEEK_SET);
            return t2p_i16be(fp);
          }
        }
      }
    }
  }
  return 0;
}



float t2p_advance(FILE* fp, int aid, int bid, int kern){
  int head = t2p_table_lookup(fp, "head");
  fseek(fp,head+18,SEEK_SET);
  float unit = 1.0/t2p_u16be(fp);

  int awa, awb, lsa, lsb;
  if (aid != -1){
    t2p_hmtx_lookup(fp,aid,&awa,&lsa);
  }
  if (bid != -1){
    t2p_hmtx_lookup(fp,bid,&awb,&lsb);
  }
  if (aid == -1){
    return 0;
  }
  if (bid == -1){
    return awa*unit;
  } 
  int k = 0;
  if (kern){
    k += t2p_kern_lookup(fp,aid,bid);
    if (!k){
      k += t2p_gpos_kern_lookup(fp,aid,bid);
    }
  }
  
  return (awa+k)*unit;
}

float t2p_lineheight(FILE* fp){
  int head = t2p_table_lookup(fp, "head");
  fseek(fp,head+18,SEEK_SET);
  float unit = 1.0/t2p_u16be(fp);

  int hhea = t2p_table_lookup(fp, "hhea");
  fseek(fp,hhea+4,SEEK_SET);
  int asc = t2p_i16be(fp);
  int des = t2p_i16be(fp);
  int gap = t2p_i16be(fp);
  return (asc-des+gap)*unit;
}

