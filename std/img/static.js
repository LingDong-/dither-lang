
globalThis.$img = new function(){
  var that = this;

  function getImageMimeType(byteArray) {
    if (!byteArray || byteArray.length < 4) return null;
    const bytes = new Uint8Array(byteArray);
    if (bytes[0] === 0x89 && bytes[1] === 0x50 && bytes[2] === 0x4E && bytes[3] === 0x47) {
      return 'image/png';
    }
    if (bytes[0] === 0xFF && bytes[1] === 0xD8 && bytes[2] === 0xFF) {
      return 'image/jpeg';
    }
    if (bytes[0] === 0x47 && bytes[1] === 0x49 && bytes[2] === 0x46) {
      return 'image/gif';
    }
    if (bytes[0] === 0x42 && bytes[1] === 0x4D) {
      return 'image/bmp';
    }
    if (
      bytes[0] === 0x52 && bytes[1] === 0x49 && bytes[2] === 0x46 && bytes[3] === 0x46 && 
      bytes[8] === 0x57 && bytes[9] === 0x45 && bytes[10] === 0x42 && bytes[11] === 0x50  
    ) {
      return 'image/webp';
    }
  }
  that.decode = async function(){
    let [byteArray] = $pop_args(1);
    return await new Promise((resolve, reject) => {
      const blob = new Blob([new Uint8Array(byteArray)], { type: getImageMimeType(byteArray) });
      const url = URL.createObjectURL(blob);
      const img = new Image();
      img.onload = () => {
        const canvas = document.createElement('canvas');
        canvas.width = img.width;
        canvas.height = img.height;
        const ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0);
        let data = ctx.getImageData(0, 0, canvas.width, canvas.height).data;
        
        const out = Object.assign(
          data,
          {__dims:[img.height,img.width,4]}
        );
        URL.revokeObjectURL(url);
        resolve(out);
      };
      img.src = url;
    });
  }
  that.encode = async function(){
    let [type,pixels] = $pop_args(2);

    return await new Promise((resolve, reject) => {
      const canvas = document.createElement('canvas');
      canvas.width = pixels.__dims[1];
      canvas.height = pixels.__dims[0];
      const ctx = canvas.getContext('2d');
      const imageData = new ImageData(new Uint8ClampedArray(pixels), pixels.__dims[1], pixels.__dims[0]);
      ctx.putImageData(imageData, 0, 0);
      canvas.toBlob((blob) => {
        const reader = new FileReader();
        reader.onloadend = () => {
          const arrayBuffer = reader.result;
          resolve(Array.from(new Uint8Array(arrayBuffer)));
        };
        reader.onerror = reject;
        reader.readAsArrayBuffer(blob);
      }, 'image/'+type.replace('jpg','jpeg'), 0.92);
    });
  }



  const MASK_NORM = 3
  const NORM_L1   = 1
  const NORM_L2   = 0
  const NORM_LINF = 2
  const MASK_COLOR    = 7
  const MASK_ALPHA    = (7<<4)
  const MASK_SCALE    = (7<<8)
  const COLOR_COPY       =0
  const COLOR_RGB_GRAY   =1
  const COLOR_RGB_BGR    =2
  const COLOR_RGB_HSV    =3
  const COLOR_HSV_RGB    =4
  const COLOR_LIN_SRGB   =5
  const COLOR_SRGB_LIN   =6
  const COLOR_INVERT     =7
  const ALPHA_COPY       =0
  const ALPHA_DROP       =16
  const ALPHA_PREMUL     =32
  const ALPHA_STRAIGHTEN =48
  const THRESH_BINARY   =256
  const THRESH_AUTO     =512
  const THRESH_ADAPTIVE =768
  const MORPH_ERODE       =16
  const MORPH_DILATE      =32
  const MORPH_OPEN        =48
  const MORPH_CLOSE       =64
  const MORPH_SKELETONIZE =80
  const BORDER_ZERO =0
  const BORDER_COPY =16
  const INT16_MAX =32767;
  const INT16_MIN =-32768;
  let tmp_buf = new ArrayBuffer(0);
  function EDT_f(x, i, g_i) {
    return (x - i) * (x - i) + g_i * g_i;
  }
  function EDT_Sep(i, u, g_i, g_u) {
    return (u * u - i * i + g_u * g_u - g_i * g_i) / (2 * (u - i));
  }
  function MDT_f(x, i, g_i) {	
    return Math.abs(x-i) + g_i;	
  }
  function MDT_Sep(i, u, g_i, g_u) {
    if (g_u >= (g_i + u - i))
      return INT16_MAX;
    if (g_i > (g_u + u - i))
      return INT16_MIN;
    return (g_u - g_i + u + i)/2;
  }
  function CDT_f(x, i, g_i) {	
    return MAX(abs(x-i), g_i);
  }
  function CDT_Sep(i, u, g_i, g_u) {
    if (g_i <= g_u)
      return Math.max(i+g_u, ((i+u)/2));
    else
      return Math.min(u-g_i, ((i+u)/2));
  }
  that.dist_transform = function(){
    let [b,flags,dt] = $pop_args(3);
    let m = b.__dims[1];
    let n = b.__dims[0];
    dt.__dims[0] = b.__dims[0];
    dt.__dims[1] = b.__dims[1];
    let f;
    let Sep;
    if ((flags&MASK_NORM) == NORM_L1){
      f = MDT_f;
      Sep = MDT_Sep;
    }else if ((flags&MASK_NORM) == NORM_L2){
      f = EDT_f;
      Sep = EDT_Sep;
    }else if ((flags&MASK_NORM) == NORM_LINF){
      f = CDT_f;
      Sep = CDT_Sep;
    }
    let do_voro = !!(flags&12);
    let tsz = m*n*2+m*2+m*2+m*n*4;
    if (tmp_buf.byteLength < tsz){
      tmp_buf = new ArrayBuffer(tsz);
    }
    let g = new Int16Array(tmp_buf,0,m*n);
    let s = new Int16Array(tmp_buf,m*n*2,m);
    let t = new Int16Array(tmp_buf,m*n*2+m*2,m);
    let v = new Int32Array(tmp_buf,m*n*2+m*2+m*2,m*n);
    for (let x = 0; x < m; x++) {
      if (b[x + 0 * m]){
        g[x + 0 * m] = 0;
        v[x + 0 * m] = b[x];
      }else{
        g[x + 0 * m] = m+n;
        v[x + 0 * m] = 0;
      }
      for (let y = 1; y < n; y++) {
        if (b[x + y * m]){
          g[x + y * m] = 0;
          v[x + y * m] = b[x + y * m];
        }else{
          g[x + y * m] = 1 + g[x + (y - 1) * m];
          v[x + y * m] = v[x + (y - 1) * m];
        }
      }
      for (let y = n - 2; y >= 0; y--) {
        if (g[x + (y + 1) * m] < g[x + y * m]){
          g[x + y * m] = 1 + g[x + (y + 1) * m];
          v[x + y * m] = v[x + (y + 1) * m];
        }
      }
    }
    let q = 0;
    let w;
    for (let y = 0; y < n; y++) {
      q = 0;
      s[0] = 0;
      t[0] = 0;
      for (let u = 1; u < m; u++) {
        while (q >= 0 && f(t[q], s[q], g[s[q] + y * m]) > f(t[q], u, g[u + y * m]))
          q--;
        if (q < 0) {
          q = 0;
          s[0] = u;
        } else {
          w = 1 + Sep(s[q], u, g[s[q] + y * m], g[u + y * m]);
          if (w < m) {
            q++;
            s[q] = u;
            t[q] = w;
          }
        }
      }
      for (let u = m - 1; u >= 0; u--) {
        let d = f(u, s[q], g[s[q] + y * m]);
        if (f == EDT_f) d = Math.sqrt(d);
        dt[u + y * m] = d;
        if (do_voro) b[u + y * m] = v[s[q] + y * m];
        if (u == t[q]) q--;
      }
    }
  }
  function rgb2hsv(r,g,b){
    let h,s,v;
    let rgbMin, rgbMax;
    rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
    rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
    v = rgbMax;
    if (v == 0){
      h = 0;
      s = 0;
      return [h,s,v];
    }
    s = (rgbMax - rgbMin) / rgbMax;
    if (s == 0){
      h = 0;
      return [h,s,v];
    }
    if (rgbMax == r)
      h = 1.0/6.0 * (g - b) / (rgbMax - rgbMin);
    else if (rgbMax == g)
      h = 2.0/6.0 + 1.0/6.0 * (b - r) / (rgbMax - rgbMin);
    else
      h = 4.0/6.0 + 1.0/6.0 * (r - g) / (rgbMax - rgbMin);
    return [h,s,v];
  }
  function hsv2rgb(h,s,v){
    let r,g,b;
    if (s == 0){
      r = v;
      g = v;
      b = v;
      return [r,g,b];
    }
    let hh = h+1.0;
    hh -= ~~hh;
    hh *= 6.0;
    let i = ~~hh;
    let ff = hh-i;
    let p = v * (1.0 - s);
    let q = v * (1.0 - (s*ff));
    let t = v * (1.0 - (s*(1.0-ff)));
    if (i==0){
      r = v; g = t; b = p;
    }else if (i==1){
      r = q; g = v; b = p;
    }else if (i==2){
      r = p; g = v; b = t;
    }else if (i==3){
      r = p; g = q; b = v;
    }else if (i==4){
      r = t; g = p; b = v;
    }else{
      r = v; g = p; b = q;
    }
    return [r,g,b];
  }
  that.convert = function(){
    let [pix,flags,out] = $pop_args(3);
    if (out.__dims.length == 3 && (out.__dims[2]==0||(out.__dims[0]==0&&out.__dims[1]==0))){
      out.__dims[2] = 1;
      if (pix.__dims.length == 3){
        out.__dims[2] = pix.__dims[2];
      }
      if ((out.__dims[2] == 4 || out.__dims[2] == 2) && (flags&MASK_ALPHA) == ALPHA_DROP){
        out.__dims[2] --;
      }
    }
    let ic = pix.__dims[2]??1;
    let oc = out.__dims[2]??1;
    let h = out.__dims[0] = pix.__dims[0];
    let w = out.__dims[1] = pix.__dims[1];
    let sizeof_dtype0 = 1;
    let cons0 = Uint8Array;
    let div0 = 255;
    let div1 = 255;
    if (pix.__type.elt[0]=='f32'){
      sizeof_dtype0 = 4;
      cons0 = Float32Array;
      div0 = 1;
    }
    if (out.__type.elt[0]=='f32'){
      div1 = 1;
    }
    let icc = (ic <= 2) ? 1 : 3;
    let occ = (oc <= 2) ? 1 : 3;
    let inp = pix;
    if ((flags & MASK_ALPHA) == ALPHA_STRAIGHTEN && (ic==2||ic==4)){
      let n = w*h*ic*sizeof_dtype0;
      if (n > tmp_buf.byteLength){
        tmp_buf = new ArrayBuffer(n);
      }
      let tmp = new cons0(tmp_buf,0,w*h*ic);
      let icc = (ic <= 2) ? 1 : 3;
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          tmp[(i*w+j)*ic+icc] = inp[(i*w+j)*ic+icc];
          for (let k = 0; k < ic; k++){
            if (inp[(i*w+j)*ic+icc] == 0){
              tmp[(i*w+j)*ic+k] = 0;
            }else{
              tmp[(i*w+j)*ic+k] = (inp[(i*w+j)*ic+k]*div0/inp[(i*w+j)*ic+icc]);
            }
          }
        }
      }
      inp = tmp;
    }
    if ((flags & MASK_COLOR) == COLOR_COPY || 
        ((flags & MASK_COLOR) == COLOR_RGB_GRAY) && ic <= 2){
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          for (let k = 0; k < occ; k++){
            out[(i*w+j)*oc+k] = inp[(i*w+j)*ic+k%icc]*div1/div0;
          }
        }
      }
    }else if ((flags & MASK_COLOR) == COLOR_INVERT){
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          for (let k = 0; k < occ; k++){
            out[(i*w+j)*oc+k] = (div0-inp[(i*w+j)*ic+k%icc])*div1/div0;
          }
        }
      }
    }else if ((flags & MASK_COLOR) == COLOR_RGB_GRAY){
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          let ir = inp[(i*w+j)*ic+0];
          let ig = inp[(i*w+j)*ic+1];
          let ib = inp[(i*w+j)*ic+2];
          out[(i*w+j)*oc] = (ir*0.2126 + ig*0.7152 + ib*0.0722)/div0*div1;
        }
      }
    }else if ((flags & MASK_COLOR) == COLOR_RGB_BGR){
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          for (let k = 0; k < occ; k++){
            out[(i*w+j)*oc+k] = inp[(i*w+j)*ic+icc-1-(k%icc)]*div1/div0;
          }
        }
      }
    }else if ((flags & MASK_COLOR) == COLOR_RGB_HSV || (flags & MASK_COLOR) == COLOR_HSV_RGB){
      let F;
      if ((flags & MASK_COLOR) == COLOR_RGB_HSV) F = rgb2hsv;
      if ((flags & MASK_COLOR) == COLOR_HSV_RGB) F = hsv2rgb;
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          let ir = (inp[(i*w+j)*ic+0%icc])/div0;
          let ig = (inp[(i*w+j)*ic+1%icc])/div0;
          let ib = (inp[(i*w+j)*ic+2%icc])/div0;
          let [oh,os,ov] = F(ir,ig,ib);
          out[(i*w+j)*oc+0%occ] = oh*div1;
          out[(i*w+j)*oc+1%occ] = os*div1;
          out[(i*w+j)*oc+2%occ] = ov*div1;
        }
      }
    }else if ((flags & MASK_COLOR) == COLOR_SRGB_LIN || (flags & MASK_COLOR) == COLOR_LIN_SRGB){
      let gamma = ((flags & MASK_COLOR) == COLOR_SRGB_LIN) ? 2.2 : 0.45;
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          for (let k = 0; k < occ; k++){
            out[(i*w+j)*oc+k] = Math.pow((float)(inp[(i*w+j)*ic+k%icc])/div0,gamma)*div1;
          }
        }
      }
    }
    if ((flags & MASK_ALPHA) != ALPHA_DROP && (oc == 2 || oc == 4) && (ic == 2 || ic == 4)){
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          out[(i*w+j)*oc+occ] = inp[(i*w+j)*ic+icc];
        }
      }
    }
    if ((flags & MASK_ALPHA) == ALPHA_PREMUL && (oc == 2 || oc == 4)){
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          for (let k = 0; k < occ; k++){
            out[(i*w+j)*oc+k] = (out[(i*w+j)*ic+k]*inp[(i*w+j)*ic+icc])*div1/div0/div0;
          }
        }
      }
    }
    if (out.__type.elt[0]=='u8'){
      for (let i = 0; i < out.length; i++) out[i]&=0xff;
    }
  }

  that.threshold = function(){
    let [pix, thresh, flags] = $pop_args(5);
    let w = pix.__dims[1];
    let h = pix.__dims[0];
    if ((flags & 0xff00) == THRESH_BINARY){
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          let b = pix[i*w+j] > thresh;
          pix[i*w+j] = b ? 255 : 0;
        }
      }
    }else if ((flags & 0xff00) == THRESH_AUTO){
      let hsz = 256*4;
      if (tmp_buf.byteLength < hsz){
        tmp_buf = new ArrayBuffer(hsz);
      }
      let hist = new Int32Array(tmp_buf);
      for (let i = 0; i < hist.length; i++){
        hist[i] = 0;
      }
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          let v = pix[i*w+j];
          hist[v]++;
        }
      }
      let total = w*h;
      let sum = 0;
      for (let i = 0; i < 256; i++) sum += i*hist[i];
      let sumB = 0;
      let wB = 0;
      let maxVar = 0;
      let threshold = 0;
      for (let t = 0; t < 256; t++){
        wB += hist[t];
        if (wB == 0) continue;
        let wF = total - wB;
        if (wF == 0) break;
        sumB += t * hist[t];
        let mB = sumB / wB;
        let mF = (sum - sumB) / wF;
        let betweenVar = wB * wF * (mB-mF) * (mB-mF);
        if (betweenVar > maxVar){
          maxVar = betweenVar;
          threshold = t;
        }
      }
      thresh = threshold;
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          let b = pix[i*w+j] > thresh;
          pix[i*w+j] = b ? 255 : 0;
        }
      }
    }else if ((flags & 0xff00) == THRESH_ADAPTIVE){
      let sz = w*h*2;
      if (tmp_buf.byteLength < sz){
        tmp_buf = new ArrayBuffer(sz);
      }
      let blurx = new Uint8Array(tmp_buf,0,w*h);
      let blury = new Uint8Array(tmp_buf,w*h,w*h);
      let sig = flags & 0xff;
      let rad = sig*3;
      let ksz = rad*2+1;
      let kern = new Array(ksz);
      for (let i = 0; i < ksz; i++){
        kern[i] = Math.exp(-(i-rad)*(i-rad)/(2.0*sig*sig));
      }
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          let n = 0;
          let s = 0;
          for (let k = j-rad; k <= j+rad; k++){
            if (k < 0) continue;
            if (k >= w) continue;
            let ki = kern[k-j+rad];
            s += pix[i*w+k]*ki;
            n+=ki;
          }
          blurx[i*w+j] = s/n;
        }
      }
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          let n = 0;
          let s = 0;       
          for (let k = i-rad; k <= i+rad; k++){
            if (k < 0) continue;
            if (k >= h) continue;
            let ki = kern[k-i+rad];
            s += blurx[k*w+j]*ki;
            n+=ki;
          }
          blury[i*w+j] = s/n;
        }
      }
      for (let i = 0; i < h; i++){
        for (let j = 0; j < w; j++){
          let b = pix[i*w+j] > blury[i*w+j]+thresh;
          pix[i*w+j] = b ? 255 : 0;
          // pix[i*w+j] = blury[i*w+j];
        }
      }
    }
  }
  function erode_or_dilate(pix,w,h,kern,rad,flags,out){
    let ksz = rad*2+1;
    let v0 = ((flags & 0xf0) == MORPH_DILATE) ? 0 : 255;
    for (let i = 0; i < h; i++){
      for (let j = 0; j < w; j++){
        let v = v0;
        for (let k = i-rad; k <= i+rad; k++){
          if (k < 0) continue;
          if (k >= h) continue;
          for (let l = j-rad; l <= j+rad; l++){
            if (l < 0) continue;
            if (l >= w) continue;
            let e = kern[ (k-i+rad) * ksz + (l-j+rad) ];
            if (!e) continue;
            if (v0){
              v = Math.min(v,pix[k*w+l]);
            }else{
              v = Math.max(v,pix[k*w+l]);
            }
          }
        }
        out[i*w+j] = v;
      }
    }
  }
  function thinning_zs_iteration(im, W, H, iter) {
    let diff = 0;
    for (let i = 1; i < H-1; i++){
      for (let j = 1; j < W-1; j++){
        let p2 = im[(i-1)*W+j]   & 1;
        let p3 = im[(i-1)*W+j+1] & 1;
        let p4 = im[(i)*W+j+1]   & 1;
        let p5 = im[(i+1)*W+j+1] & 1;
        let p6 = im[(i+1)*W+j]   & 1;
        let p7 = im[(i+1)*W+j-1] & 1;
        let p8 = im[(i)*W+j-1]   & 1;
        let p9 = im[(i-1)*W+j-1] & 1;
        let A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
        let B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
        let m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
        let m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);
        if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
          im[i*W+j] |= 2;
      }
    }
    for (let i = 0; i < H*W; i++){
      let marker = im[i]>>1;
      let old = im[i]&1;
      im[i] = old & (!marker);
      if ((!diff) && (im[i] != old)){
        diff = 1;
      }
    }
    return diff;
  }
  function thinning_zs(im, W, H){
    let diff = 1;
    do {
      diff &= thinning_zs_iteration(im,W,H,0);
      diff &= thinning_zs_iteration(im,W,H,1);
    }while (diff);
  }
  that.morphology = function(){
    let [pix,rad,flags,out] = $pop_args(4);
    let w = pix.__dims[1];
    let h = pix.__dims[0];
    out.__dims[0] = h;
    out.__dims[1] = w;

    if (((flags & 0xf0) == MORPH_SKELETONIZE)){
      let sz = w*h;
      if (tmp_buf.byteLength < sz){
        tmp_buf = new ArrayBuffer(sz);
      }
      buf = new Uint8Array(tmp_buf,0,w*h);
      for (let i = 0; i < w*h; i++){
        buf[i] = pix[i] > 128 ? 1 : 0;
      }
      thinning_zs(buf,w,h);
      for (let i = 0; i < w*h; i++){
        out[i] = buf[i]?255:0;
      }
      return;
    }

    let ksz = rad*2+1;
    let sz = ksz*ksz;
    let proc = out;
    if (out == pix){
      sz += w*h;
    }
    let inter = null;
    let twostep = ((flags & 0xf0) == MORPH_OPEN) || ((flags & 0xf0) == MORPH_CLOSE);
    if (twostep){
      sz += w*h;
    }
    if (tmp_buf.byteLength < sz){
      tmp_buf = new ArrayBuffer(sz);
    }
    sz = ksz*ksz;
    if (out == pix){
      proc = new Uint8Array(tmp_buf,sz);
      sz += w*h;
    }
    if (twostep){
      inter = new Uint8Array(tmp_buf,sz);
    }

    let kern = new Uint8Array(tmp_buf,0,ksz*ksz);

    if ((flags & 0xf) == NORM_LINF){
      for (let i = 0; i < ksz*ksz; i++) kern[i] = 1;
    }else if ((flags & 0xf) == NORM_L1){
      for (let i = 0; i < ksz; i++){
        for (let j = 0; j < ksz; j++){
          let d = Math.abs(i-rad)+Math.abs(j-rad);
          kern[i*ksz+j] = (d <= rad);
        }
      }
    }else if ((flags & 0xf) == NORM_L2){
      for (let i = 0; i < ksz; i++){
        for (let j = 0; j < ksz; j++){
          let d = Math.hypot(i-rad,j-rad);
          kern[i*ksz+j] = (d <= rad);
        }
      }
    }
    if ((flags & 0xf0) == MORPH_ERODE || (flags & 0xf0) == MORPH_DILATE){
      erode_or_dilate(pix,w,h,kern,rad,flags,proc);
    }else if ((flags & 0xf0) == MORPH_OPEN){
      erode_or_dilate(pix,w,h,kern,rad,MORPH_ERODE,inter);
      erode_or_dilate(inter,w,h,kern,rad,MORPH_DILATE,proc);
    }else if ((flags & 0xf0) == MORPH_CLOSE){
      erode_or_dilate(pix,w,h,kern,rad,MORPH_DILATE,inter);
      erode_or_dilate(inter,w,h,kern,rad,MORPH_ERODE,proc);
    }
    if (out == proc) return;
    for (let i = 0; i < w*h; i++) out[i] = proc[i];
  }
  that.convolve = function(){
    let [pix,kern,flags,out] = $pop_args(4);
    let dtype = pix.__type.elt[0];
    let dsize = (dtype == 'u8') ? 1 : 4;
    let w = pix.__dims[1];
    let h = pix.__dims[0];
    out.__dims[0] = h;
    out.__dims[1] = w;

    let sz = w*h*dsize;
    
    if (tmp_buf.byteLength < sz){
      tmp_buf = new ArrayBuffer(sz);
    }
    if (dtype == 'u8'){
      proc = new Uint8Array(tmp_buf,0,w*h);
    }else{
      proc = new Float32Array(tmp_buf,0,w*h);
    }
    
    let kw = kern.__dims[1];
    let kh = kern.__dims[0];

    let kx = ~~((kw-1)/2);
    let ky = ~~((kh-1)/2);
    let border = (flags&0xf0)==BORDER_COPY;
    for (let i = 0; i < h; i++){
      for (let j = 0; j < w; j++){
        let s = 0.0;
        for (let k = 0; k < kh; k++){
          for (let l = 0; l < kw; l++){
            let ii = i+k-kx;
            let jj = j+l-ky;
            let iii = Math.min(Math.max(ii,0),h-1);
            let jjj = Math.min(Math.max(jj,0),w-1);
            if (border || (iii==ii&&jjj==jj)){
              s += pix[iii*w+jjj]*kern[k*kw+l];
            }
          }
        }
        proc[i*w+j] = s;
      }
    }
    for (let i = 0; i < w*h; i++) out[i] = proc[i];
  }

  // https://gist.github.com/LingDong-/b99cdbe814e600d8152c0eefeef01ab3  
  let N_PIXEL_NEIGHBOR = 8;
  // give pixel neighborhood counter-clockwise ID's for
  // easier access with findContour algorithm
  function neighborIDToIndex(i, j, id){
    if (id == 0){return [i,j+1];}
    if (id == 1){return [i-1,j+1];}
    if (id == 2){return [i-1,j];}
    if (id == 3){return [i-1,j-1];}
    if (id == 4){return [i,j-1];}
    if (id == 5){return [i+1,j-1];}
    if (id == 6){return [i+1,j];}
    if (id == 7){return [i+1,j+1];}
    return null;
  }
  function neighborIndexToID(i0, j0, i, j){
    let di = i - i0;
    let dj = j - j0;
    if (di == 0 && dj == 1){return 0;}
    if (di ==-1 && dj == 1){return 1;}
    if (di ==-1 && dj == 0){return 2;}
    if (di ==-1 && dj ==-1){return 3;}
    if (di == 0 && dj ==-1){return 4;}
    if (di == 1 && dj ==-1){return 5;}
    if (di == 1 && dj == 0){return 6;}
    if (di == 1 && dj == 1){return 7;}
    return -1;
  }

  // first counter clockwise non-zero element in neighborhood
  function ccwNon0(F, w, h, i0, j0, i, j, offset){
    let id = neighborIndexToID(i0,j0,i,j);
    for (let k = 0; k < N_PIXEL_NEIGHBOR; k++){
      let kk = (k+id+offset + N_PIXEL_NEIGHBOR*2) % N_PIXEL_NEIGHBOR;
      let ij = neighborIDToIndex(i0,j0,kk);
      if (F[ij[0]*w+ij[1]]!=0){
        return ij;
      }
    }
    return null;
  }

  // first clockwise non-zero element in neighborhood
  function cwNon0(F, w, h, i0, j0, i, j, offset){
    let id = neighborIndexToID(i0,j0,i,j);
    for (let k = 0; k < N_PIXEL_NEIGHBOR; k++){
      let kk = (-k+id-offset + N_PIXEL_NEIGHBOR*2) % N_PIXEL_NEIGHBOR;
      let ij = neighborIDToIndex(i0,j0,kk);
      if (F[ij[0]*w+ij[1]]!=0){
        return ij;
      }
    }
    return null;
  }
  /**
   * Find contours in a binary image
   * <p>
   * Implements Suzuki, S. and Abe, K.
   * Topological Structural Analysis of Digitized Binary Images by Border Following.
   * <p>
   * See source code for step-by-step correspondence to the paper's algorithm
   * description.
   * @param  F    The bitmap, stored in 1-dimensional row-major form. 
   *              0=background, 1=foreground, will be modified by the function
   *              to hold semantic information
   * @param  w    Width of the bitmap
   * @param  h    Height of the bitmap
   * @return      An array of contours found in the image.
   * @see         Contour
   */
   function findContours(F, w, h) {
    // Topological Structural Analysis of Digitized Binary Images by Border Following.
    // Suzuki, S. and Abe, K., CVGIP 30 1, pp 32-46 (1985)
    let nbd = 1;
    let lnbd = 1;

    let contours = [];
    
    // Without loss of generality, we assume that 0-pixels fill the frame 
    // of a binary picture
    for (let i = 1; i < h-1; i++){
      F[i*w] = 0; F[i*w+w-1]=0;
    }
    for (let i = 0; i < w; i++){
      F[i] = 0; F[w*h-1-i]=0;
    }

    //Scan the picture with a TV raster and perform the following steps 
    //for each pixel such that fij # 0. Every time we begin to scan a 
    //new row of the picture, reset LNBD to 1.
    for (let i = 1; i < h-1; i++) {
      lnbd = 1;

      for (let j = 1; j < w-1; j++) {
        
        let i2 = 0, j2 = 0;
        if (F[i*w+j] == 0) {
          continue;
        }
        //(a) If fij = 1 and fi, j-1 = 0, then decide that the pixel 
        //(i, j) is the border following starting point of an outer 
        //border, increment NBD, and (i2, j2) <- (i, j - 1).
        if (F[i*w+j] == 1 && F[i*w+(j-1)] == 0) {
          nbd ++;
          i2 = i;
          j2 = j-1;
          
          
        //(b) Else if fij >= 1 and fi,j+1 = 0, then decide that the 
        //pixel (i, j) is the border following starting point of a 
        //hole border, increment NBD, (i2, j2) <- (i, j + 1), and 
        //LNBD + fij in case fij > 1.  
        } else if (F[i*w+j]>=1 && F[i*w+j+1] == 0) {
          nbd ++;
          i2 = i;
          j2 = j+1;
          if (F[i*w+j]>1) {
            lnbd = F[i*w+j];
          }
          
          
        } else {
          //(c) Otherwise, go to (4).
          //(4) If fij != 1, then LNBD <- |fij| and resume the raster
          //scan from pixel (i,j+1). The algorithm terminates when the
          //scan reaches the lower right corner of the picture
          if (F[i*w+j]!=1){lnbd = Math.abs(F[i*w+j]);}
          continue;
          
        }
        //(2) Depending on the types of the newly found border 
        //and the border with the sequential number LNBD 
        //(i.e., the last border met on the current row), 
        //decide the parent of the current border as shown in Table 1.
        // TABLE 1
        // Decision Rule for the Parent Border of the Newly Found Border B
        // ----------------------------------------------------------------
        // Type of border B'
        // \    with the sequential
        //     \     number LNBD
        // Type of B \                Outer border         Hole border
        // ---------------------------------------------------------------     
        // Outer border               The parent border    The border B'
        //                            of the border B'
        //
        // Hole border                The border B'      The parent border
        //                                               of the border B'
        // ----------------------------------------------------------------
        
        let B = {};
        B.points = []
        B.points.push([j,i]);
        B.isHole = (j2 == j+1);
        B.id = nbd;
        contours.push(B);

        let B0 = {}
        for (let c = 0; c < contours.length; c++){
          if (contours[c].id == lnbd){
            B0 = contours[c];
            break;
          }
        }
        if (B0.isHole){
          if (B.isHole){
            B.parent = B0.parent;
          }else{
            B.parent = lnbd;
          }
        }else{
          if (B.isHole){
            B.parent = lnbd;
          }else{
            B.parent = B0.parent;
          }
        }
        
        //(3) From the starting point (i, j), follow the detected border: 
        //this is done by the following substeps (3.1) through (3.5).
        
        //(3.1) Starting from (i2, j2), look around clockwise the pixels 
        //in the neigh- borhood of (i, j) and tind a nonzero pixel. 
        //Let (i1, j1) be the first found nonzero pixel. If no nonzero 
        //pixel is found, assign -NBD to fij and go to (4).
        let i1 = -1, j1 = -1;
        let i1j1 = cwNon0(F,w,h,i,j,i2,j2,0);
        if (i1j1 == null){
          F[i*w+j] = -nbd;
          //go to (4)
          if (F[i*w+j]!=1){lnbd = Math.abs(F[i*w+j]);}
          continue;
        }
        i1 = i1j1[0]; j1 = i1j1[1];
        
        // (3.2) (i2, j2) <- (i1, j1) ad (i3,j3) <- (i, j).
        i2 = i1;
        j2 = j1;
        let i3 = i;
        let j3 = j;
        
        while (true){

          //(3.3) Starting from the next elementof the pixel (i2, j2) 
          //in the counterclock- wise order, examine counterclockwise 
          //the pixels in the neighborhood of the current pixel (i3, j3) 
          //to find a nonzero pixel and let the first one be (i4, j4).
          
          let i4j4 = ccwNon0(F,w,h,i3,j3,i2,j2,1);
   
          var i4 = i4j4[0];
          var j4 = i4j4[1];

          contours[contours.length-1].points.push([j4,i4]);
          
          //(a) If the pixel (i3, j3 + 1) is a O-pixel examined in the
          //substep (3.3) then fi3, j3 <-  -NBD.
          if (F[i3*w+j3+1] == 0){
            F[i3*w+j3] = -nbd;
            
          //(b) If the pixel (i3, j3 + 1) is not a O-pixel examined 
          //in the substep (3.3) and fi3,j3 = 1, then fi3,j3 <- NBD.
          }else if (F[i3*w+j3] == 1){
            F[i3*w+j3] = nbd;
          }else{
            //(c) Otherwise, do not change fi3, j3.
          }
          
          //(3.5) If (i4, j4) = (i, j) and (i3, j3) = (i1, j1) 
          //(coming back to the starting point), then go to (4);
          if (i4 == i && j4 == j && i3 == i1 && j3 == j1){
            if (F[i*w+j]!=1){lnbd = Math.abs(F[i*w+j]);}
            break;
            
          //otherwise, (i2, j2) + (i3, j3),(i3, j3) + (i4, j4), 
          //and go back to (3.3).
          }else{
            i2 = i3;
            j2 = j3;
            i3 = i4;
            j3 = j4;
          }
        }
      }
    }
    return contours;
  }

  that.find_contours = function(){
    let [pix] = $pop_args(1);
    let h = pix.__dims[0];
    let w = pix.__dims[1];
    if (tmp_buf.byteLength < w*h*4){
      tmp_buf = new ArrayBuffer(w*h*4);
    }
    let F = new Int32Array(tmp_buf,0,w*h);
    for (let i = 0; i < w*h; i++){
      F[i] = (pix[i])?1:0;
    }
    let contours = findContours(F, w, h);
    let out = [];
    const eps = 0.001;
    for (let i = 0; i < contours.length; i++){
      out.push([])
      for (let j = 0; j < contours[i].points.length; j++){
        let x1 = contours[i].points[j][0];
        let y1 = contours[i].points[j][1];
        if (j == 0 || j == contours[i].points.length-1){
          out.at(-1).push([x1,y1])
        }else{
          let x0 = out.at(-1).at(-1)[0];
          let y0 = out.at(-1).at(-1)[1];
          let x2 = contours[i].points[j+1][0];
          let y2 = contours[i].points[j+1][1];
          let cw = (((x1)-(x0))*((y2)-(y0)) - ((x2)-(x0))*((y1)-(y0)));
          if (cw > eps || cw < -eps){
            out.at(-1).push([x1,y1]);
          }
        }
      }
      if (out.at(-1).length < 3){
        out.pop();
      }else{
        out.at(-1).forEach(v=>v.__type = {con:'vec',elt:['f32',2]});
        out.at(-1).__type = {con:'list',elt:[{con:'vec',elt:['f32',2]}]}
      }
    }
    // console.log(out,contours);
    return out;
  }

  function lbl_assoc(lbl_eq, pw, pn){
    if (pw == pn) return pw;
    if (!pw) return pn;
    if (!pn) return pw;
    let pa = pw;
    let pb = pn;
    if (pa > pb){
      pa = pn;
      pb = pw;
    }
    if (!lbl_eq[pb]){
      lbl_eq[pb] = pa;
      return pa;
    }
    let pc = lbl_eq[pb];
    return (lbl_eq[pb] = lbl_assoc(lbl_eq,pa,pc));
  }

  that.label_blobs = function(){
    let [pix,out] = $pop_args(2);
    let h = out.__dims[0] = pix.__dims[0];
    let w = out.__dims[1] = pix.__dims[1];
    let lbl_eq = [0];
    let lbl = 1;
    for (let i = 0; i < h; i++){
      for (let j = 0; j < w; j++){
        if (pix[i*w+j]){
          let pw = j ? out[i*w+j-1] : 0;
          let pn = i ? out[i*w+j-w] : 0;
          if (pw == 0 && pn == 0){
            out[i*w+j] = lbl;
            lbl_eq[lbl] = 0;
            lbl++;
          }else{
            out[i*w+j] = lbl_assoc(lbl_eq,pw,pn);
          }
        }else{
          out[i*w+j] = 0;
        }
      }
    }
    let uniques = new Array(lbl).fill(0);
    let nid = 0;
    for (let i = 1; i < lbl; i++){
      if (!lbl_eq[i]){
        uniques[i] = nid++;
      }
    }
    for (let i = 0; i < h; i++){
      for (let j = 0; j < w; j++){
        if (out[i*w+j]){
          let ini = out[i*w+j];
          let bot = ini;
          while (lbl_eq[bot]){
            bot = lbl_eq[bot];
          }
          if (bot != ini){
            lbl_eq[ini] = bot;
          }
          out[i*w+j] = uniques[bot];
        }
      }
    }
  }
}

