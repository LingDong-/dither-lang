var TTF2PL = new function(){
  let SEEK_SET = 0;
  let SEEK_CUR = 1;
  function fgetc(fp){
    return fp.buf[fp.pos++]
  }
  function ftell(fp){
    return fp.pos;
  }
  function fseek(fp,pos,mode){
    if (mode == SEEK_SET){
      fp.pos = pos;
    }else if (mode == SEEK_POS){
      fp.pos += pos;
    }
  }
  function u16be(fp){
    return (fgetc(fp)<<8) | fgetc(fp);
  }
  function u32be(fp){
    return ((fgetc(fp)<<24)>>>0) | (fgetc(fp)<<16) | (fgetc(fp)<<8) | fgetc(fp);
  }
  function i16be(fp){
    let u = u16be(fp);
    const buffer = new ArrayBuffer(2);
    new Uint16Array(buffer)[0] = u;
    return new Int16Array(buffer)[0];
  }
  function i8xe(fp){
    let u = fgetc(fp);
    const buffer = new ArrayBuffer(1);
    new Uint8Array(buffer)[0] = u;
    return new Int8Array(buffer)[0];
  }
  function f2d14(fp){
    let i = i16be(fp);
    return i/(1<<14);
  }
  function table_lookup(fp,name){
    fseek(fp,0,SEEK_SET);
    let offs = 0;
    if (u32be(fp) == 0x74746366){
      fseek(fp,12,SEEK_SET);
      offs = u32be(fp);
    }
    fseek(fp,offs+4,SEEK_SET);
    let ntbl = u16be(fp);
    for (let i = 0; i < ntbl; i++){
      fseek(fp,offs+12+16*i,SEEK_SET);
      if (name.charCodeAt(0) != fgetc(fp)) continue;
      if (name.charCodeAt(1) != fgetc(fp)) continue;
      if (name.charCodeAt(2) != fgetc(fp)) continue;
      if (name.charCodeAt(3) != fgetc(fp)) continue;
      fseek(fp,offs+20+16*i,SEEK_SET);
      return u32be(fp);
    }
    return 0;
  }
  function cmap_lookup(fp,code){
    let offs = table_lookup(fp, "cmap");
    fseek(fp,offs,SEEK_SET);
    let vers = u16be(fp);
    let ntbl = u16be(fp);
    let fmt=-1;
    let sofs;
    let rank=-1;
    for (let i = 0; i < ntbl; i++){
      fseek(fp,offs+4+i*8,SEEK_SET);
      let platid = u16be(fp);
      let encid = u16be(fp);
      let sofsx = u32be(fp);
      fseek(fp,offs+sofsx,SEEK_SET);
      let fmtx = u16be(fp);
      let rankx =platid*10000+encid*100+fmtx; //heuristic
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
      let nseg = u16be(fp)>>1;
      for (let j = 0; j < nseg; j++){
        fseek(fp,offs+sofs+14+j*2,SEEK_SET);
        let cend = u16be(fp);
        if (code > cend) continue;
        fseek(fp,offs+sofs+16+nseg*2+j*2,SEEK_SET);
        let cstart = u16be(fp);
        if (code < cstart) continue;
        fseek(fp,offs+sofs+16+nseg*4+j*2,SEEK_SET);
        let idelta = i16be(fp);
        let pidoffs = offs+sofs+16+nseg*6+j*2;
        fseek(fp,pidoffs,SEEK_SET);
        let idoffs = u16be(fp);
        let gid;
        if (idoffs == 0){
          gid = code + idelta;
        }else{
          fseek(fp, pidoffs+idoffs+(code-cstart)*2,SEEK_SET);
          gid = u16be(fp);
          if (gid){
            gid += idelta;
          }
        }
        gid &= 0xffff;
        return gid;
      }
    }else if (fmt == 6){
      fseek(fp,offs+sofs+6,SEEK_SET);
      let cstart = u16be(fp);
      let nent = u16be(fp);
      if (code < cstart || code >= cstart+nent) return 0;
      fseek(fp,(code - cstart)*2,SEEK_CUR);
      return u16be(fp);
    }else if (fmt == 12){
      fseek(fp,offs+sofs+12,SEEK_SET);
      let nseg = u32be(fp);
      for (let j = 0; j < nseg; j++){
        fseek(fp,offs+sofs+16+j*12,SEEK_SET);
        let cstart = u32be(fp);
        if (code < cstart) continue;

        let cend = u32be(fp);
        if (code > cend) continue;

        let sid = u32be(fp);
        return code-cstart+sid;
      }
    }
    return 0;
  }

  function loca_lookup(fp, gid){
    let head = table_lookup(fp, "head");
    fseek(fp,head+50,SEEK_SET);
    let fmt = i16be(fp);
    let offs = table_lookup(fp,"loca");
    if (fmt){
      fseek(fp,offs+gid*4,SEEK_SET);
      let i0 = u32be(fp);
      let i1 = u32be(fp);
      if (i0 == i1) return -1;
      return i0;
    }else{
      fseek(fp,offs+gid*2,SEEK_SET);
      let i0 = u16be(fp);
      let i1 = u16be(fp);
      if (i0 == i1) return -1;
      return i0*2;
    }
    return 0;
  }

  function glyf_lookup(fp, gid, trfm, O, cb){
    let loca = loca_lookup(fp,gid);
    if (loca < 0){
      O.n = 0;
      O.x = 0;
      O.y = 0;
      return;
    }
    let offs = table_lookup(fp, "glyf") + loca;
    fseek(fp,offs,SEEK_SET);

    let nctr = i16be(fp);

    if (nctr < 0){
      fseek(fp,offs+10,SEEK_SET);
      let flags;
      let lx = 0;
      let ly = 0;
      do {
        flags = u16be(fp);
        let cid = u16be(fp);
        let a1,a2;
        if (flags & 0x2){
          if ( flags & 0x1) {
            a1 = i16be(fp);
            a2 = i16be(fp);
          } else {
            a1 = i8xe(fp);
            a2 = i8xe(fp);
          }
        }else{
          if ( flags & 0x1) {
            a1 = u16be(fp);
            a2 = u16be(fp);
          } else {
            a1 = fgetc(fp);
            a2 = fgetc(fp);
          }
        }
        let mat = [
          1,0,0,
          0,1,0
        ];
        if ( flags & 0x8 ) {
          mat[0] = mat[3] = f2d14(fp);
        } else if ( flags & 0x40 ) {
          mat[0] = f2d14(fp);
          mat[4] = f2d14(fp);
        } else if ( flags & 0x80 ) {
          mat[0] = f2d14(fp);
          mat[3] = f2d14(fp);
          mat[1] = f2d14(fp);
          mat[4] = f2d14(fp);
        }
        let old = ftell(fp);
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
          let o = {n:a1};
          glyf_lookup(fp,gid,trfm,o,null);
          let x0 = o.x;
          let y0 = o.y;
          o = {n:a2};
          glyf_lookup(fp,cid,trfm,o,null);
          let x1 = o.x;
          let y1 = o.y;
          mat[2] = x0-x1;
          mat[5] = y0-y1;
        }
        if (trfm){
          let nmat = [
            trfm[0]*mat[0]+trfm[1]*mat[3],
            trfm[0]*mat[1]+trfm[1]*mat[4],
            trfm[0]*mat[2]+trfm[1]*mat[5]+trfm[2],
            trfm[3]*mat[0]+trfm[4]*mat[3],
            trfm[3]*mat[1]+trfm[4]*mat[4],
            trfm[3]*mat[2]+trfm[4]*mat[5]+trfm[5],
          ];
          nmat.forEach((v,i)=>mat[i]=v);
        }
        let o = {};
        if (O.n != undefined && O.n != -1){
          o = {n:O.n};
          glyf_lookup(fp,cid,mat,o,cb);
          if (o.n == O.n){
            O.x = lx;
            O.y = ly;
            return;
          }else{
            O.n -= o.n;
          }
        }else{
          glyf_lookup(fp,cid,mat,o,cb);
        }
        lx += o.x;
        ly += o.y;
        fseek(fp,old,SEEK_SET);
      } while ( flags & 0x20 );
      return;
    }
    fseek(fp,offs+10+2*nctr,SEEK_SET);
    let nins = u16be(fp);

    fseek(fp,offs+10+2*(nctr-1),SEEK_SET);
    let npts = u16be(fp)+1;

    let xlen = 0;
    let xrem = npts;
    let fofs = offs+12+2*nctr+nins;
    fseek(fp,fofs,SEEK_SET);
    while (xrem){
      let flag = fgetc(fp);
      let mul = 1;
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
    
    let xofs = ftell(fp);
    let yofs = xofs+xlen;
    let pidx = 0;
    let pend = 0;
    let x = 0;
    let y = 0;
    while (pidx < npts){
      fseek(fp,fofs,SEEK_SET);
      let flag = fgetc(fp);
      let mul = 1;
      let oncurve = flag & 0x01;
      if (flag & 0x08){ //REPEAT_FLAG
        mul += fgetc(fp);
      }
      fofs = ftell(fp);
      for (let i = 0; i < mul; i++){
        let dx,dy;
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
            dx = i16be(fp);
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
            dy = i16be(fp);
          }
        }
        yofs = ftell(fp);
        let isend = 0;
        fseek(fp,offs+10+pend*2,SEEK_SET);
        if (pidx == u16be(fp)){
          isend = 0x80;
          pend++;
        }
        if (trfm){
          let dx1 = dx * trfm[0] + dy * trfm[1];
          let dy1 = dx * trfm[3] + dy * trfm[4];
          if (pidx == 0){
            dx1 += trfm[2];
            dy1 += trfm[5];
          }
          dx = dx1;
          dy = dy1;
        }
        x += dx;
        y += dy;
        if (O.n==pidx){
          O.x = x;
          O.y = y;
          return;
        }
        if (cb) cb(isend|oncurve,dx,dy);
        pidx++;
      }
    }
    O.n = npts;
    O.x = x;
    O.y = y;
  }

  let ds = {
    x:0,
    y:0,
    x0:0,
    y0:0,
    qx:0,
    qy:0,
    qx0:0,
    qy0:0,
    q:0,
    q0:0,
    first:0,
    unit:0,
    reso:0,
    lineto:(x,y)=>0,
    moveto:(x,y)=>0,
  };

  function quadratic_bezier(x0,y0,x1,y1,x2,y2,t){
    let s = 1-t;
    let s2 = s*s;
    let t2 = t*t;
    let xo = s2*x0+2*s*t*x1+t2*x2;
    let yo = s2*y0+2*s*t*y1+t2*y2;
    return [xo,yo];
  }

  function discretizer(flag, dx, dy){
    let dxf = dx *ds.unit;
    let dyf =-dy *ds.unit;
    let x = ds.x + ds.qx + dxf;
    let y = ds.y + ds.qy + dyf;
    if (ds.first){
      if (flag & 0x01){
        ds.moveto(ds.x0 = ds.x = x, ds.y0 = ds.y = y);
        ds.q = ds.q0 = 0;
        ds.qx = ds.qy = 0;
      }else{
        ds.q0 = ds.q = 1;
        ds.qx = dxf;
        ds.qy = dyf;
      }
      ds.first = 0;
    }else if (!(flag & 0x01)){
      if (ds.q){
        let cx = ds.x + ds.qx;
        let cy = ds.y + ds.qy;
        let mx = (cx + x)*0.5;
        let my = (cy + y)*0.5;
        if (ds.q0 == 1){
          ds.x0 = mx;
          ds.y0 = my;
          ds.qx0 = cx;
          ds.qy0 = cy;
          ds.q0 = 2;
          ds.moveto(mx,my);
        }else{
          for (let i = 0; i < ds.reso; i++){
            let t = (i+1)/ds.reso;
            let [xt,yt] = quadratic_bezier(ds.x,ds.y,cx,cy,mx,my,t);
            ds.lineto(xt,yt);
          }
        }
        ds.x = mx;
        ds.y = my;
        ds.qx = x-mx;
        ds.qy = y-my;
      }else{
        ds.qx = dxf;
        ds.qy = dyf;
        ds.q = 1;
      }
    }else if (ds.q){
      let cx = ds.x + ds.qx;
      let cy = ds.y + ds.qy;
      if (ds.q0 == 1){
        ds.qx0 = cx;
        ds.qy0 = cy;
        ds.q0 = 2;
        ds.moveto(ds.x0 = x, ds.y0 = y);
      }else{
        for (let i = 0; i < ds.reso; i++){
          let t = (i+1)/ds.reso;
          let [xt,yt]= quadratic_bezier(ds.x,ds.y,cx,cy,x,y,t);
          ds.lineto(xt,yt);
        }
      }
      ds.q = 0;
      ds.qx = ds.qy = 0;
      ds.x = x;
      ds.y = y;
    }else{
      ds.lineto(ds.x = x, ds.y = y);
    }
    if (flag & 0x80){
      if (ds.q){
        let cx = ds.x + ds.qx;
        let cy = ds.y + ds.qy;
        if (ds.q0){
          let mx = (cx + ds.qx0)*0.5;
          let my = (cy + ds.qy0)*0.5;
          for (let i = 0; i < ds.reso; i++){
            let t = (i+1)/ds.reso;
            let [xt,yt]=quadratic_bezier(ds.x,ds.y,cx,cy,mx,my,t);
            ds.lineto(xt,yt);
          }
          for (let i = 0; i < ds.reso; i++){
            let t = (i+1)/ds.reso;
            let [xt,yt]=quadratic_bezier(mx,my,ds.qx0,ds.qy0,ds.x0,ds.y0,t);
            ds.lineto(xt,yt);
          }
        }else{
          for (let i = 0; i < ds.reso; i++){
            let t = (i+1)/ds.reso;
            let [xt,yt]=quadratic_bezier(ds.x,ds.y,cx,cy,ds.x0,ds.y0,t);
            ds.lineto(xt,yt);
          }
        }
        ds.q = ds.q0 = 0;
        ds.qx = ds.qy = 0;
        ds.x = x;
        ds.y = y;
      }else if (ds.q0){
        for (let i = 0; i < ds.reso; i++){
          let t = (i+1)/ds.reso;
          let [xt,yt]= quadratic_bezier(ds.x,ds.y,ds.qx0,ds.qy0,ds.x0,ds.y0,t);
          ds.lineto(xt,yt);
        }
      }else if (x != ds.x0 || y != ds.y0){
        ds.lineto(ds.x0,ds.y0);
      }
      ds.first = 1;
    }
  }

  function glyph(fp,gid,reso,moveto,lineto){
    let head = table_lookup(fp, "head");
    fseek(fp,head+18,SEEK_SET);
    ds.unit = 1.0/u16be(fp);
    ds.x = 0;
    ds.y = 0;
    ds.q = 0;
    ds.q0 = 0;
    ds.qx = 0;
    ds.qy = 0;
    ds.first = 1;
    ds.reso = reso;
    ds.lineto = lineto?lineto:(x,y)=>console.log(`L ${x} ${y}`);
    ds.moveto = moveto?moveto:(x,y)=>console.log(`M ${x} ${y}`);
    glyf_lookup(fp,gid,null,{},discretizer);
  }

  function hmtx_lookup(fp, gid){
    let advw,lsb;
    let hhea = table_lookup(fp, "hhea");
    fseek(fp,hhea+34,SEEK_SET);
    let nhm = u16be(fp);
    let hmtx = table_lookup(fp, "hmtx");
    if (gid < nhm){
      fseek(fp,hmtx+gid*4,SEEK_SET);
      advw = u16be(fp);
      lsb = i16be(fp);
    }else{
      fseek(fp,hmtx+(nhm-1)*4,SEEK_SET);
      advw = u16be(fp);
      fseek(fp,hmtx+nhm*4+(gid-nhm)*2,SEEK_SET);
      lsb = i16be(fp);
    }
    return [advw,lsb];
  }

  function kern_lookup(fp, aid, bid){
    let key = ((aid<<16)>>>0) | bid;
    let kern = table_lookup(fp, "kern");
    fseek(fp,kern+2,SEEK_SET);
    let ntbl = u16be(fp);

    fseek(fp,kern+4,SEEK_SET);
    for (let i = 0; i < ntbl; i++){
      let ver = u16be(fp);
      let len = u16be(fp);
      let cov = u16be(fp);
      let fmt = (cov>>8) & 0xff;
      if (fmt != 0){
        fseek(fp,len-6,SEEK_CUR);
        continue;
      }
      let npair = u16be(fp);
      let sran = u16be(fp);
      let esel = u16be(fp);
      let rshf = u16be(fp);
      let offs = ftell(fp);
      let probe = sran;
      let base = 0;
      while (probe){
        fseek(fp,offs+base+probe,SEEK_SET);
        let pair = u32be(fp);
        if (key == pair){
          return i16be(fp);
        }else if (key > pair){
          base += probe;
        }
        probe >>= 1;
      }
    }
    return 0;
  }


  function gsub_lookup(fp, tag, gid, num_next, next_gids){
    let gsub = table_lookup(fp, "GSUB");
    if (gsub == 0) return gid;
    fseek(fp,gsub+6,SEEK_SET);
    let featl = u16be(fp);
    let lul = u16be(fp);
    fseek(fp,gsub+featl,SEEK_SET);
    let nfeat = i16be(fp);
    for (let i = 0; i < nfeat; i++){
      fseek(fp,gsub+featl+2+i*6,SEEK_SET);
      if (tag.charCodeAt(0) != fgetc(fp)) continue;
      if (tag.charCodeAt(1) != fgetc(fp)) continue;
      if (tag.charCodeAt(2) != fgetc(fp)) continue;
      if (tag.charCodeAt(3) != fgetc(fp)) continue;
      let offs = u16be(fp);
      fseek(fp,gsub+featl+offs,SEEK_SET);
      let param = i16be(fp);
      let nlui = u16be(fp);
      for (let j = 0; j < nlui; j++){
        fseek(fp,gsub+featl+offs+4+j*2,SEEK_SET);
        let idx = u16be(fp);
        fseek(fp, gsub+lul+2+idx*2, SEEK_SET);
        let lofs = u16be(fp);
        fseek(fp, gsub+lul+lofs, SEEK_SET);
        let typ = u16be(fp);
        if (typ != 1 && typ != 4) continue;

        let flg = u16be(fp);
        let nst = u16be(fp);
        for (let k = 0; k < nst; k++){
          fseek(fp, gsub+lul+lofs+6+k*2, SEEK_SET);
          let sofs = u16be(fp);
          fseek(fp,gsub+lul+lofs+sofs,SEEK_SET);
          let sfmt = u16be(fp);
          let cofs = u16be(fp);
          fseek(fp,gsub+lul+lofs+sofs+cofs,SEEK_SET);
          let cfmt = u16be(fp);
          let cidx = -1;
          if (cfmt == 1){
            let n = u16be(fp);
            for (let l = 0; l < n; l++){
              if (gid == u16be(fp)){
                cidx = l;
                break;
              }
            }
          }else if (cfmt == 2){
            let n = u16be(fp);
            for (let l = 0; l < n; l++){
              let gstart = u16be(fp);
              let gend = u16be(fp);
              let sci = u16be(fp);
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
              let delta = i16be(fp);
              return (gid + delta) & 0xffff;
            }else if (sfmt == 2){
              let nglyf = u16be(fp);
              fseek(fp,cidx*2,SEEK_CUR);
              return u16be(fp);
            }
          }else if (typ == 4){
            if (sfmt == 1){
              let nlset = u16be(fp);
              fseek(fp,gsub+lul+lofs+sofs+6+cidx*2,SEEK_SET);
              let lsofs = u16be(fp);
              fseek(fp,gsub+lul+lofs+sofs+lsofs,SEEK_SET);
              let nlig = u16be(fp);

              for (let n = num_next[0]; n > 0; n--){
                for (let l = 0; l < nlig; l++){
                  fseek(fp,gsub+lul+lofs+sofs+lsofs+2+l*2,SEEK_SET);
                  let lgofs = u16be(fp);
                  fseek(fp,gsub+lul+lofs+sofs+lsofs+lgofs,SEEK_SET);
                  let lid = u16be(fp);
                  let ncom = u16be(fp)-1;
                  if (ncom != n) continue;
                  let ok = 1;
                  for (let m = 0; m < ncom; m++){
                    if (next_gids[m] != u16be(fp)){
                      ok = 0;
                      break;
                    }
                  }
                  if (ok){
                    num_next[0] = n;
                    return lid;
                  }
                }
              
              }
            }
          }
        }
      }
    }
    if (num_next)num_next[0] = 0;
    return gid;
  }

  function class_def_lookup(fp, gid){
    let cdofs = ftell(fp);
    let fmt = u16be(fp);
    if (fmt == 1){
      let gstart = u16be(fp);
      let nglyf = u16be(fp);
      if (gstart <= gid && gid < gstart+nglyf){
        let ci = gid-gstart;
        fseek(fp,cdofs+6+ci*2,SEEK_SET);
        return u16be(fp);
      }
    }else if (fmt == 2){
      let ncran = u16be(fp);
      for (let i = 0; i < ncran; i++){
        let gstart = u16be(fp);
        let gend = u16be(fp);
        let cls = u16be(fp);
        if (gstart <= gid && gid <= gend){
          return cls;
        }
      }
    }
    return 0;
  }


  function gpos_kern_lookup(fp, aid, bid){
    let gsub = table_lookup(fp, "GPOS");
    if (gsub == 0) return 0;
    fseek(fp,gsub+6,SEEK_SET);
    let featl = u16be(fp);
    let lul = u16be(fp);
    fseek(fp,gsub+featl,SEEK_SET);
    let nfeat = i16be(fp);
    for (let i = 0; i < nfeat; i++){
      fseek(fp,gsub+featl+2+i*6,SEEK_SET);
      if ('k'.charCodeAt(0) != fgetc(fp)) continue;
      if ('e'.charCodeAt(0) != fgetc(fp)) continue;
      if ('r'.charCodeAt(0) != fgetc(fp)) continue;
      if ('n'.charCodeAt(0) != fgetc(fp)) continue;
      let offs = u16be(fp);
      fseek(fp,gsub+featl+offs,SEEK_SET);
      let param = i16be(fp);
      let nlui = u16be(fp);
      for (let j = 0; j < nlui; j++){
        fseek(fp,gsub+featl+offs+4+j*2,SEEK_SET);
        let idx = u16be(fp);
        fseek(fp, gsub+lul+2+idx*2, SEEK_SET);
        let lofs = u16be(fp);
        fseek(fp, gsub+lul+lofs, SEEK_SET);
        let typ = u16be(fp);

        if (typ != 2) continue;
        let flg = u16be(fp);
        let nst = u16be(fp);
        for (let k = 0; k < nst; k++){
          fseek(fp, gsub+lul+lofs+6+k*2, SEEK_SET);
          let sofs = u16be(fp);
          fseek(fp,gsub+lul+lofs+sofs,SEEK_SET);
          let sfmt = u16be(fp);
          let cofs = u16be(fp);
          fseek(fp,gsub+lul+lofs+sofs+cofs,SEEK_SET);
          let cfmt = u16be(fp);
          let cidx = -1;
          if (cfmt == 1){
            let n = u16be(fp);
            for (let l = 0; l < n; l++){
              if (aid == u16be(fp)){
                cidx = l;
                break;
              }
            }
          }else if (cfmt == 2){
            let n = u16be(fp);
            for (let l = 0; l < n; l++){
              let gstart = u16be(fp);
              let gend = u16be(fp);
              let sci = u16be(fp);
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
            let vfmt1 = u16be(fp);
            let vfmt2 = u16be(fp);
            if (vfmt1 != 4 || vfmt2 != 0){
              continue;
            }
            let v1sz = ((vfmt1&1)+!!(vfmt1&2)+!!(vfmt1&4)+!!(vfmt1&8)+!!(vfmt1&16)+!!(vfmt1&32)+!!(vfmt1&64)+!!(vfmt1&128))*2;
            let v2sz = ((vfmt2&1)+!!(vfmt2&2)+!!(vfmt2&4)+!!(vfmt2&8)+!!(vfmt2&16)+!!(vfmt2&32)+!!(vfmt2&64)+!!(vfmt2&128))*2;

            if (sfmt == 1){
              let npset = u16be(fp);
              fseek(fp,gsub+lul+lofs+sofs+10+cidx*2,SEEK_SET);
              let psofs = u16be(fp);

              fseek(fp,gsub+lul+lofs+sofs+psofs,SEEK_SET);
              let npval = u16be(fp);
              for (let l = 0; l < npval; l++){
                fseek(fp,gsub+lul+lofs+sofs+psofs+2+(2+v1sz+v2sz)*l,SEEK_SET);
                let gid = u16be(fp);
                if (gid == bid){
                  fseek(fp,gsub+lul+lofs+sofs+psofs+2+(2+v1sz+v2sz)*l+2,SEEK_SET);
                  return i16be(fp);
                }
              }

            }else if (sfmt == 2){
              let cd1ofs = u16be(fp);
              let cd2ofs = u16be(fp);
              let nc1 = u16be(fp);
              let nc2 = u16be(fp);
              
              fseek(fp,gsub+lul+lofs+sofs+cd1ofs,SEEK_SET);
              let cls1 = class_def_lookup(fp,aid);

              fseek(fp,gsub+lul+lofs+sofs+cd2ofs,SEEK_SET);
              let cls2 = class_def_lookup(fp,bid);

              fseek(fp,gsub+lul+lofs+sofs+16 + cls1 * (v1sz+v2sz)*nc2 + cls2*(v1sz+v2sz),SEEK_SET);
              return i16be(fp);
            }
          }
        }
      }
    }
    return 0;
  }



  function advance(fp, aid, bid, kern){
    let head = table_lookup(fp, "head");
    fseek(fp,head+18,SEEK_SET);
    let unit = 1.0/u16be(fp);

    let awa, awb, lsa, lsb;
    if (aid != -1){
      ;[awa,lsa] = hmtx_lookup(fp,aid);
    }
    if (bid != -1){
      ;[awb,lsb] = hmtx_lookup(fp,bid);
    }
    if (aid == -1){
      return 0;
    }
    if (bid == -1){
      return awa*unit;
    } 
    let k = 0;
    if (kern){
      k += kern_lookup(fp,aid,bid);
      if (!k){
        k += gpos_kern_lookup(fp,aid,bid);
      }
    }
    return (awa+k)*unit;
  }

  function lineheight(fp){
    let head = table_lookup(fp, "head");
    fseek(fp,head+18,SEEK_SET);
    let unit = 1.0/u16be(fp);

    let hhea = table_lookup(fp, "hhea");
    fseek(fp,hhea+4,SEEK_SET);
    let asc = i16be(fp);
    let des = i16be(fp);
    let gap = i16be(fp);
    return (asc-des+gap)*unit;
  }
  this.glyph = glyph;
  this.cmap_lookup = cmap_lookup;
  this.gsub_lookup = gsub_lookup;
  this.advance = advance;
  this.lineheight = lineheight;
}

globalThis.$font = new function(){
  let that = this;

  let FEAT_SMCP=1
  let FEAT_ONUM=2
  let FEAT_HIST=4
  let FEAT_LIGA=256
  let FEAT_KERN=65536

  let fonts = [];
  that.decode = function(){
    let [bytes] = $pop_args(1);
    let fp = {
      pos:0,
      buf:bytes
    };
    fonts.push({fp});
    return {id:fonts.length-1};
  }
  that._lookup = function(){
    let [id,x,flag] = $pop_args(3);
    if (typeof x == 'number'){
      let gid = TTF2PL.cmap_lookup(fonts[id].fp,x);
      if (flag & FEAT_SMCP){
        gid = TTF2PL.gsub_lookup(fonts[id].fp,"smcp",gid,[0],[]);
      }
      if (flag & FEAT_ONUM){
        gid = TTF2PL.gsub_lookup(fonts[id].fp,"onum",gid,[0],[]);
      }
      if (flag & FEAT_HIST){
        gid = TTF2PL.gsub_lookup(fonts[id].fp,"hist",gid,[0],[]);
      }
      return gid;
    }else{
      let gids = new Array(x.length);
      for (let i = 0; i < x.length; i++){
        gids[i] = TTF2PL.cmap_lookup(fonts[id].fp, x[i]);
      }
      let gid = -1;
      let n = [x.length - 1];
      if (flag & FEAT_LIGA){
        gid = TTF2PL.gsub_lookup(fonts[id].fp,"liga",gids[0],n,gids.slice(1));
        if (gid == gids[0]){
          n = [x.length - 1];
          gid = TTF2PL.gsub_lookup(fonts[id].fp,"dlig",gids[0],n,gids.slice(1));
          if (gid == gids[0]){
            n = [x.length - 1];
            gid = TTF2PL.gsub_lookup(fonts[id].fp,"hlig",gids[0],n,gids.slice(1));
          }
        }
      }
      let n_code;
      if (gid == -1){
        gid = gids[0];
        n_code = 1;
      }
      n_code = n[0]+1;
      if (flag & FEAT_SMCP){
        gid = TTF2PL.gsub_lookup(fonts[id].fp,"smcp",gid,[0],[]);
      }
      if (flag & FEAT_ONUM){
        gid = TTF2PL.gsub_lookup(fonts[id].fp,"onum",gid,[0],[]);
      }
      if (flag & FEAT_HIST){
        gid = TTF2PL.gsub_lookup(fonts[id].fp,"hist",gid,[0],[]);
      }
      x.splice(0,n_code);
      return gid;
    }
  }
  that._advance = function(){
    let [id,gidx,hidx,flag] = $pop_args(4);
    return TTF2PL.advance(fonts[id].fp,gidx,hidx,Number(!!(flag & FEAT_KERN)));
  }
  that._glyph = function(){
    let [id,gidx,reso] = $pop_args(3);
    let polys = [];
    polys.__type = {con:'list',elt:[{con:'list',elt:[{con:'vec',elt:['f32',2]}]}]}
    function moveto(x,y){
      let ps = [];
      ps.__type = polys.__type.elt[0];
      polys.push(ps);
      lineto(x,y);
    }
    function lineto(x,y){
      let p = [x,y];
      p.__type = polys.__type.elt[0].elt[0];
      polys.at(-1).push(p);
    }
    TTF2PL.glyph(fonts[id].fp,gidx,reso,moveto,lineto);
    return polys;
  }

};