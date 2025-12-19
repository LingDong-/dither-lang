globalThis.$geom = new function(){
  let that = this;

  const LHS_CAPL   = 1
  const RHS_CAPL   = 4
  const LHS_CAPR   = 2
  const RHS_CAPR   = 8
  const RET_POINTS = 0
  const RET_PARAMS = 16
  const BY_COUNT   = 0
  const BY_SPACING = 1
  const MODE_POLYLINE = 0
  const MODE_POLYGON  =128
  const MODE_ALIGNED  = 0
  const MODE_ORIENTED =16
  const OP_INCLUDE =1
  const OP_EXCLUDE =2
  const TYPE_BEZIER   =16
  const TYPE_CATROM   =32
  const TYPE_BSPLINE  =48
  const ORD_QUADRATIC = 2
  const ORD_CUBIC     = 3

  function point_list_typed(x,nd){
    for (let i = 0; i < x.length; i++){
      x[i].__type = {con:'vec',elt:['f32',nd]}
    }
    return x;
  }
  function point_list_list_typed(x,nd){
    for (let i = 0; i < x.length; i++){
      x[i] = point_list_typed(x[i],nd);
      x[i].__type = {con:'list',elt:[{con:'vec',elt:['f32',nd]}]}
    }
    return x;
  }

  let acc_len = [];
  let n_acc_len = 0;
  that.poly_resample = function(){
    let [points, n, flags] = $pop_args(3);
    let n_points = points.length;
    let n_poly = n_points;
    if (flags & MODE_POLYGON){
      n_poly++;
    }
    if (n_poly+1 > n_acc_len){
      acc_len = new Array(n_acc_len = n_poly+1);
    }
    let tot_len = 0;
    for (let i = 0; i < n_poly-1; i++){
      let dx = points[i][0] - points[(i+1)%n_points][0];
      let dy = points[i][1] - points[(i+1)%n_points][1];
      tot_len += Math.sqrt(dx*dx+dy*dy);
      acc_len[i+1] = tot_len;
    }
    acc_len[0] = 0;
    acc_len[n_poly] = tot_len;
    let spacing = 0;
    let count = 0;
    if (flags & BY_COUNT){
      spacing = tot_len/n;
      count = Math.ceil(n);
    }else{ // BY_SPACING
      spacing = n;
      count = Math.ceil(tot_len/spacing);
    }
    let out = new Array(count);
    let idx = 0;
    let lidx = 0;
    for (let l = 0; l < tot_len; l+= spacing){
      for (let i = lidx; i < n_poly; i++){
        if (acc_len[i] <= l && l < acc_len[i+1]){
          let t = (l-acc_len[i])/(acc_len[i+1]-acc_len[i]);
          let x = points[i][0]*(1-t)+points[(i+1)%n_points][0]*t;
          let y = points[i][1]*(1-t)+points[(i+1)%n_points][1]*t;
          out[idx] = [x,y];
          idx ++;
          lidx = i;
          if (idx == count){
            return out;
          }
          break;
        }
      }
    }
    return point_list_typed(out,2);
  }

  let CWISE=(x0,y0,x1,y1,x2,y2)=>(((x1)-(x0))*((y2)-(y0)) - ((x2)-(x0))*((y1)-(y0)));

  that.pt_in_poly = function(){
    let [pt,points] = $pop_args(3);
    let [x,y] = pt;
    let n_points = points.length;
    let wn = 0;
    for (let i = 0, j = n_points-1; i < n_points; j = i++){
      let xi = points[i][0];
      let yi = points[i][1];
      let xj = points[j][0];
      let yj = points[j][1];
      if (yj <= y){
        if (yi > y){
          if (CWISE(xj,yj,xi,yi,x,y)>0){
            wn++;
          }
        }
      }else{
        if (yi <= y){
          if (CWISE(xj,yj,xi,yi,x,y)<0){
            wn--;
          }
        }
      }
    }
    return Number(wn != 0);
  }

  function dist_pt_seg(x0,y0,x1,y1,x2,y2){
    let A = x0-x1;
    let B = y0-y1;
    let C = x2-x1;
    let D = y2-y1;
    let dot = A*C+B*D;
    let len_sq = C*C+D*D;
    let param = -1.0;
    if (len_sq != 0){
      param = dot/len_sq;
    }
    let xx,yy;
    if (param < 0){
      xx = x1; yy = y1;
    }else if (param > 1){
      xx = x2; yy = y2;
    }else{
      xx = x1 + param*C;
      yy = y1 + param*D;
    }
    let dx = x0-xx;
    let dy = y0-yy;
    return Math.sqrt(dx*dx+dy*dy);
  }

  that.poly_simplify = function(){
    let [points,eps] = $pop_args(2);
    function impl(points,start,end,eps){
      let n_points = end - start;
      if (n_points <= 2){
        return point_list_typed([
          points[0].slice(),
          points[1].slice()
        ]);
      }
      let dmax = 0;
      let argmax = -1;
      for (let i = start+1; i < end-1; i++){
        let d = dist_pt_seg(
          ...points[i],
          ...points[start],
          ...points[end-1]
        );
        if (d > dmax){
          dmax = d;
          argmax = i;
        }
      }
      if (dmax > eps){
        let L = impl(points, start, argmax+1, eps);
        let R = impl(points, argmax, end, eps);
        out = L.slice(0,-1).concat(R);
      }else{
        out = [
          points[start].slice(),
          points[end-1].slice()
        ]
      }
      return out;
    }
    return point_list_typed(impl(points,0,points.length,eps));
  }

  let cur_px;
  let cur_py;
  function cmp_angle(a,b){
    let x0 = a[0] - cur_px;
    let y0 = a[1] - cur_py;
    let x1 = b[0] - cur_px;
    let y1 = b[1] - cur_py;
    let vc = x0*y1 - y0*x1;
    if (vc == 0){
      vc = x0+y0-x1-y1;
      return Math.sign(vc);
    }
    return -Math.sign(vc);
  }

  function convex_hull(points){
    let n_points = points.length;
    if (n_points<=1){
      return point_list_typed(points.map(x=>x.slice()));
    }
    let mi = 0;
    let my = Infinity;
    let mx = Infinity;
    for (let i = 0; i < n_points; i++){
      if (points[i][1]<my || (points[i][1]==my && points[i][0]<mx)){
        mx = points[i][0];
        my = points[i][1];
        mi = i;
      }
    }
    let [px,py] = points[mi];
    let sorted = points.slice(0,mi).concat(points.slice(mi+1));
    cur_px = px;
    cur_py = py;
    sorted.sort(cmp_angle);
    sorted = sorted.map(x=>x.slice());
    let stack = [[px,py],sorted[0]];
    for (let i = 0; i < n_points-1; i++){
      while (stack.length >= 2 && CWISE(
        ...stack[stack.length-2],
        ...stack[stack.length-1],
        ...sorted[i],
      )<=0){
        stack.pop()
      }
      stack.push(sorted[i])
    }
    return point_list_typed(stack);
  }

  that.convex_hull = function(){
    let [points] = $pop_args(1);
    return point_list_typed(convex_hull(points));
  }

  let PT_IN_TRI = (p0x,p0y,p1x,p1y,p2x,p2y,p3x,p3y) => (
    CWISE(p0x,p0y, p1x,p1y, p2x,p2y)>=0 &&
    CWISE(p0x,p0y, p2x,p2y, p3x,p3y)>=0 &&
    CWISE(p0x,p0y, p3x,p3y, p1x,p1y)>=0);
  
  function triangulate_simple(points){
    let n_points = points.length;
    let out = [];
    let skips = new Array(n_points).fill(0);
    let i = -1;
    let skipped = -1;
    while (out.length < (n_points-2)*3){
      i++;
      skipped++;
      if (skipped > n_points){
        return out;
      }
      let i0 = i % n_points;
      i0 = (i0+skips[i0])%n_points;
      let i1 = (i0+1) % n_points;
      i1 = (i1+skips[i1])%n_points;
      let i2 = (i1+1) % n_points;
      i2 = (i2+skips[i2])%n_points;
      if (CWISE(
        ...points[i0],
        ...points[i1],
        ...points[i2] 
      )<=0) continue;
      let ok = 1;
      for (let j = 0; j < n_points; j++){
        let j0 = j % n_points;
        j0 = (j0+skips[j0])%n_points;
        let j1 = (j0+1) % n_points;
        j1 = (j1+skips[j1])%n_points;
        let j2 = (j1+1) % n_points;
        j2 = (j2+skips[j2])%n_points;
        if (j1 == i0 || j1 == i1 || j1 == i2) continue;
        if (CWISE(
          ...points[j0],
          ...points[j1],
          ...points[j2] 
        )>=0) continue;
        if (PT_IN_TRI(
          ...points[j1],
          ...points[i0],
          ...points[i1],
          ...points[i2] 
        )){
          ok = 0;
          break;
        }
      }
      if (!ok) continue;
      out.push(i0,i1,i2)
      skips[i1] = skips[(i1+1)%n_points]+1;
      let k = i1;
      while (k = (k-1+n_points)%n_points, skips[k]){
        skips[k] += skips[i1];
      }
      i--;
      skipped = 0;
    }
    return out;
  }
  that.triangulate = function(){
    let [points] = $pop_args(1);
    return triangulate_simple(points.slice());
  }

  function circum_circle(xp,yp,x1,y1,x2,y2,x3,y3){
    let xc=0,yc=0,rsqr=0;
    const EPSILON = 0.000001;
    let m1,m2,mx1,mx2,my1,my2;
    let dx,dy,drsqr;
    let fabsy1y2 = Math.abs(y1-y2);
    let fabsy2y3 = Math.abs(y2-y3);
    if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON) return [0,xc,yc,rsqr];
    if (fabsy1y2 < EPSILON) {
      m2 = - (x3-x2) / (y3-y2);
      mx2 = (x2 + x3) / 2.0;
      my2 = (y2 + y3) / 2.0;
      xc = (x2 + x1) / 2.0;
      yc = m2 * (xc - mx2) + my2;
    } else if (fabsy2y3 < EPSILON) {
      m1 = - (x2-x1) / (y2-y1);
      mx1 = (x1 + x2) / 2.0;
      my1 = (y1 + y2) / 2.0;
      xc = (x3 + x2) / 2.0;
      yc = m1 * (xc - mx1) + my1;
    } else {
      m1 = - (x2-x1) / (y2-y1);
      m2 = - (x3-x2) / (y3-y2);
      mx1 = (x1 + x2) / 2.0;
      mx2 = (x2 + x3) / 2.0;
      my1 = (y1 + y2) / 2.0;
      my2 = (y2 + y3) / 2.0;
      xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
      if (fabsy1y2 > fabsy2y3) {
        yc = m1 * (xc - mx1) + my1;
      } else {
        yc = m2 * (xc - mx2) + my2;
      }
    }
    dx = x2 - xc;
    dy = y2 - yc;
    rsqr = dx*dx + dy*dy;
    dx = xp - xc;
    dy = yp - yc;
    drsqr = dx*dx + dy*dy;
    return [((drsqr - rsqr) <= EPSILON), xc,yc,rsqr];
  }
  function cmp_x(a,b){
    let x0 = a[0];
    let x1 = b[0];
    return Math.sign(x0-x1);
  }
  function delaunay_bowyer_watson_bourke(nv,pxyz,V){
    // based on https://paulbourke.net/papers/triangulate/
    let complete;
    let edges = [];
    let nedge = 0;
    let trimax;
    let inside;
    let ntri;
    let xp,yp,x1,y1,x2,y2,x3,y3,xc,yc,r;
    let xmin,xmax,ymin,ymax,xmid,ymid;
    let dx,dy,dmax;
    trimax = 4 * nv;
    complete = new Array(trimax);
    xmin = pxyz[0][0];
    ymin = pxyz[0][1];
    xmax = xmin;
    ymax = ymin;
    for (let i=1;i<nv;i++) {
      if (pxyz[i][0] < xmin) xmin = pxyz[i][0];
      if (pxyz[i][0] > xmax) xmax = pxyz[i][0];
      if (pxyz[i][1] < ymin) ymin = pxyz[i][1];
      if (pxyz[i][1] > ymax) ymax = pxyz[i][1];
    }
    dx = xmax - xmin;
    dy = ymax - ymin;
    dmax = (dx > dy) ? dx : dy;
    xmid = (xmax + xmin) / 2.0;
    ymid = (ymax + ymin) / 2.0;
    pxyz[nv+0][0] = xmid - 20 * dmax;
    pxyz[nv+0][1] = ymid - dmax;
    pxyz[nv+1][0] = xmid;
    pxyz[nv+1][1] = ymid + 20 * dmax;
    pxyz[nv+2][0] = xmid + 20 * dmax;
    pxyz[nv+2][1] = ymid - dmax;
    V[0] = nv;
    V[1] = nv+1;
    V[2] = nv+2;
    complete[0] = 0;
    ntri = 1;
    
    for (let i=0;i<nv;i++) {
      xp = pxyz[i][0];
      yp = pxyz[i][1];
      nedge = 0;
      for (let j=0;j<ntri;j++) {
        if (complete[j]) continue;
        x1 = pxyz[V[j*3+0]][0];
        y1 = pxyz[V[j*3+0]][1];
        x2 = pxyz[V[j*3+1]][0];
        y2 = pxyz[V[j*3+1]][1];
        x3 = pxyz[V[j*3+2]][0];
        y3 = pxyz[V[j*3+2]][1];
        ;[inside,xc,yc,r] = circum_circle(xp,yp,x1,y1,x2,y2,x3,y3);
        if (xc < xp && ((xp-xc)*(xp-xc)) > r) complete[j] = 1;
        if (inside) {
          edges[(nedge+0)*2+0] = V[j*3+0];
          edges[(nedge+0)*2+1] = V[j*3+1];
          edges[(nedge+1)*2+0] = V[j*3+1];
          edges[(nedge+1)*2+1] = V[j*3+2];
          edges[(nedge+2)*2+0] = V[j*3+2];
          edges[(nedge+2)*2+1] = V[j*3+0];
          nedge += 3;
          V[j*3+0] = V[(ntri-1)*3+0];
          V[j*3+1] = V[(ntri-1)*3+1];
          V[j*3+2] = V[(ntri-1)*3+2];
          complete[j] = complete[ntri-1];
          ntri--;
          j--;
        }
      }
      for (let j=0;j<nedge-1;j++) {
        for (let k=j+1;k<nedge;k++) {
          if ((edges[j*2] == edges[k*2+1]) && (edges[j*2+1] == edges[k*2])) {
            edges[j*2+0] = -1;
            edges[j*2+1] = -1;
            edges[k*2+0] = -1;
            edges[k*2+1] = -1;
          }
          if ((edges[j*2] == edges[k*2]) && (edges[j*2+1] == edges[k*2+1])) {
            edges[j*2+0] = -1;
            edges[j*2+1] = -1;
            edges[k*2+0] = -1;
            edges[k*2+1] = -1;
          }
        }
      }
      for (let j=0;j<nedge;j++) {
        if (edges[j*2] < 0 || edges[j*2+1] < 0) continue;
        V[ntri*3+0] = edges[j*2];
        V[ntri*3+1] = edges[j*2+1];
        V[ntri*3+2] = i;
        complete[ntri] = 0;
        ntri++;
      }
    }
    return ntri;
  }
  that.delaunay = function(){
    let [points] = $pop_args(1);
    let n_points = points.length;
    let pxyz = new Array(n_points);
    for (let i = 0; i < n_points; i++){
      pxyz[i] = [...points[i], i];
    }
    pxyz.sort(cmp_x);
    pxyz.push([0,0,-1],[0,0,-1],[0,0,-1]);

    let V = new Array(n_points*9);
    let ntri = delaunay_bowyer_watson_bourke(n_points, pxyz, V);

    for (let i=0;i<ntri;i++) {
      if (V[i*3+0] >= n_points || V[i*3+1] >= n_points || V[i*3+2] >= n_points) {
        V[i*3+0] = V[(ntri-1)*3+0];
        V[i*3+1] = V[(ntri-1)*3+1];
        V[i*3+2] = V[(ntri-1)*3+2];
        ntri--;
        i--;
      }
    }
    for (let i = 0; i < ntri*3; i++){
      V[i] = pxyz[V[i]][2];
    }
    return V.slice(0,ntri*3);
  }

  function site_add_vertex(points, sites, idx, x, y){
    let ang = Math.atan2(y-points[idx][1], x-points[idx][0]);
    let ii;
    for (ii = 0; ii < sites[idx].vs.length; ii++){
      if (ang < sites[idx].angs[ii]){
        break;
      }
    }
    sites[idx].angs.splice(ii,0,ang);
    sites[idx].vs.splice(ii,0,[x,y]);
  }

  that.voronoi = function(){
    let [points] = $pop_args(1);
    let n_points = points.length;
    let pxyz = new Array(n_points);
    for (let i = 0; i < n_points; i++){
      pxyz[i] = [...points[i], i];
    }
    pxyz.sort(cmp_x);
    pxyz.push([0,0,-1],[0,0,-1],[0,0,-1]);

    let V = new Array(n_points*9);
    let ntri = delaunay_bowyer_watson_bourke(n_points, pxyz, V);
    let sites = new Array(n_points).fill(0).map(_=>({
      angs:[],vs:[]
    }));

    for (let i = 0; i < ntri; i++){
      let i0 = V[i*3];
      let i1 = V[i*3+1];
      let i2 = V[i*3+2];
      let [inside,xc,yc,rsqr] = circum_circle(0,0, 
        pxyz[i0][0], pxyz[i0][1],
        pxyz[i1][0], pxyz[i1][1],
        pxyz[i2][0], pxyz[i2][1],
      );
      if (i0 < n_points) site_add_vertex(points,sites,pxyz[i0][2], xc,yc);
      if (i1 < n_points) site_add_vertex(points,sites,pxyz[i1][2], xc,yc);
      if (i2 < n_points) site_add_vertex(points,sites,pxyz[i2][2], xc,yc);
    }
    return point_list_list_typed(sites.map(x=>x.vs));
  }

  function aabb_2d(points){
    let minX = Infinity, maxX = -Infinity;
    let minY = Infinity, maxY = -Infinity;
    for (let i = 0; i < points.length; i++){
      let x = points[i][0];
      let y = points[i][1];
      minX = Math.min(minX, x);
      maxX = Math.max(maxX, x);
      minY = Math.min(minY, y);
      maxY = Math.max(maxY, y);
    }
    let w = maxX - minX;
    let h = maxY - minY;
    return [(minX+maxX)*0.5,(minY+maxY)*0.5,w/2,h/2,1,0,0,1];
  }

  function obb_2d_pca(points){
    let n_points = points.length;
    let meanX = 0;
    let meanY = 0;
    for (let i = 0; i < n_points; i++){
      meanX += points[i][0];
      meanY += points[i][1];
    }
    meanX /= n_points;
    meanY /= n_points;
    let covXX = 0, covXY = 0, covYY = 0;
    for (let i = 0; i < n_points; i++){
      let dx = points[i][0] - meanX;
      let dy = points[i][1] - meanY;
      covXX += dx*dx;
      covXY += dx*dy;
      covYY += dy*dy;
    }
    covXX /= n_points;
    covXY /= n_points;
    covYY /= n_points;
    let trace = covXX + covYY;
    let det = covXX*covYY - covXY*covXY;
    let temp = Math.sqrt((trace*trace)/4 - det);
    let eig1X = trace/2 + temp;
    let eig1Y = trace/2 - temp;
    let eigvec1X = 1;
    let eigvec1Y = 0;
    if (Math.abs(covXY) > 1e-6){
      eigvec1X = eig1X - covYY;
      eigvec1Y = covXY;
    }
    let mag = Math.hypot(eigvec1X,eigvec1Y);
    eigvec1X /= mag;
    eigvec1Y /= mag;
    let eigvec2X = -eigvec1Y;
    let eigvec2Y = eigvec1X;
    let minX = INFINITY, maxX = -INFINITY;
    let minY = INFINITY, maxY = -INFINITY;
    for (let i = 0; i < n_points; i++){
      let dx = points[i*2] - meanX;
      let dy = points[i*2+1] - meanY;
      let projX = dx*eigvec1X + dy*eigvec1Y;
      let projY = dx*eigvec2X + dy*eigvec2Y;
      minX = Math.min(minX, projX);
      maxX = Math.max(maxX, projX);
      minY = Math.min(minY, projY);
      maxY = Math.max(maxY, projY);
    }
    let w = maxX - minX;
    let h = maxY - minY;
    let cx = (maxX+minX)/2;
    let cy = (maxY+minY)/2;
    let ccx = cx*eigvec1X + cy*eigvec2X;
    let ccy = cx*eigvec1Y + cy*eigvec2Y;
    return [
      meanX+ccx,
      meanY+ccy,
      w/2,
      h/2,
      eigvec1X,
      eigvec2X,
      eigvec1Y,
      eigvec2Y,
    ]
  }

  function obb_2d_rotcal(points){
    let out = new Array(8).fill(0);
    let hull,nh;
    let RCHULL_PROJ = (ux,uy,idx)=>((ux)*hull[((idx)%nh)][0] + (uy)*hull[((idx)%nh)][1])
    let RCHULL_EDGE = (idx)=>{
      let ux = (hull[(((idx)+1)%nh)][0]-hull[((idx)%nh)][0]);
      let uy = (hull[(((idx)+1)%nh)][1]-hull[((idx)%nh)][1]); 
      let l = Math.hypot(ux,uy); 
      if (l){ux/=l; uy/=l;}
      return [ux,uy];
    }
    hull = convex_hull(points);
    nh = hull.length;
    let kMinX,kMinY,kMaxX,kMaxY;
    let u1x,u1y,u2x,u2y;
    ;[u1x,u1y] = RCHULL_EDGE(0);
    u2x = -u1y, u2y = u1x;
    let maxX=-Infinity, minX=Infinity, maxY=-Infinity, minY=Infinity;
    for(let t=0;t<nh;t++){
      let px = RCHULL_PROJ(u1x,u1y,t);
      let py = RCHULL_PROJ(u2x,u2y,t);
      if(px>maxX){maxX=px; kMaxX=t;}
      if(px<minX){minX=px; kMinX=t;}
      if(py>maxY){maxY=py; kMaxY=t;}
      if(py<minY){minY=py; kMinY=t;}
    }
    let bestArea = Infinity;

    for (let i = 0; i < nh; i++){
      let maxX=-Infinity, minX=Infinity, maxY=-Infinity, minY=Infinity;
      let u1x,u1y,u2x,u2y;
      ;[u1x,u1y] = RCHULL_EDGE(i);
      u2x = -u1y, u2y = u1x;
      while(RCHULL_PROJ(u1x,u1y,kMaxX+1) > (maxX=RCHULL_PROJ(u1x,u1y,kMaxX))) kMaxX++; 
      while(RCHULL_PROJ(u1x,u1y,kMinX+1) < (minX=RCHULL_PROJ(u1x,u1y,kMinX))) kMinX++; 
      while(RCHULL_PROJ(u2x,u2y,kMaxY+1) > (maxY=RCHULL_PROJ(u2x,u2y,kMaxY))) kMaxY++; 
      while(RCHULL_PROJ(u2x,u2y,kMinY+1) < (minY=RCHULL_PROJ(u2x,u2y,kMinY))) kMinY++; 
      let width = (maxX-minX);
      let height = (maxY-minY);
      let area = width*height;
      if (area < bestArea){
        bestArea = area;
        let cx = (minX+maxX)*0.5;
        let cy = (minY+maxY)*0.5;
        out[0] = u1x*cx + u2x*cy;
        out[1] = u1y*cx + u2y*cy;
        out[2] = width*0.5;
        out[3] = height*0.5;
        out[4] = u1x;
        out[5] = u2x;
        out[6] = u1y;
        out[7] = u2y;
      }
    }
    return out;
  }
  function power_iter(
    a00, a01, a02,
    a10, a11, a12,
    a20, a21, a22,
  ){
    let v0 = Math.PI;
    let v1 = Math.E;
    let v2 = Math.SQRT2;
    for (let k = 0; k < 12; k++){
      let x0 = a00*v0+a01*v1+a02*v2;
      let x1 = a10*v0+a11*v1+a12*v2;
      let x2 = a20*v0+a21*v1+a22*v2;
      let n = Math.sqrt(x0*x0+x1*x1+x2*x2);
      if (n < 1e-12) break;
      v0 = x0/n;
      v1 = x1/n;
      v2 = x2/n;
    }
    return [v0,v1,v2];
  }
  function obb_3d_pca(points){
    let n_points = points.length;
    let meanX = 0, meanY = 0, meanZ = 0;
    for (let i = 0; i < n_points; i++){
      meanX += points[i][0];
      meanY += points[i][1];
      meanZ += points[i][2];
    }
    meanX /= n_points;
    meanY /= n_points;
    meanZ /= n_points;
    let cxx=0, cxy=0, cxz=0, cyy=0, cyz=0, czz=0;
    for (let i = 0; i < n_points; i++){
      let dx = points[i][0] - meanX;
      let dy = points[i][1] - meanY;
      let dz = points[i][2] - meanZ;
      cxx += dx*dx; cxy += dx*dy; cxz += dx*dz; 
      cyy += dy*dy; cyz += dy*dz; czz += dz*dz;
    }
    cxx/=n_points; cxy/=n_points; cxz/=n_points;
    cyy/=n_points; cyz/=n_points; czz/=n_points;

    let [e1x, e1y, e1z] = power_iter(
      cxx,cxy,cxz, 
      cxy,cyy,cyz, 
      cxz,cyz,czz
    );

    let lambda =
      e1x*(cxx*e1x+cxy*e1y+cxz*e1z)+
      e1y*(cxy*e1x+cyy*e1y+cyz*e1z)+
      e1z*(cxz*e1x+cyz*e1y+czz*e1z);
    
    let [e2x, e2y, e2z] = power_iter(
      cxx-lambda*e1x*e1x, cxy-lambda*e1x*e1y, cxz-lambda*e1x*e1z,
      cxy-lambda*e1y*e1x, cyy-lambda*e1y*e1y, cyz-lambda*e1y*e1z,
      cxz-lambda*e1z*e1x, cyz-lambda*e1z*e1y, czz-lambda*e1z*e1z,
    );

    let e3x = e1y*e2z-e1z*e2y;
    let e3y = e1z*e2x-e1x*e2z;
    let e3z = e1x*e2y-e1y*e2x;

    let minX = Infinity, minY = Infinity, minZ = Infinity;
    let maxX =-Infinity, maxY =-Infinity, maxZ =-Infinity;

    for (let i = 0; i < n_points; i++){
      let dx = points[i][0] - meanX;
      let dy = points[i][1] - meanY;
      let dz = points[i][2] - meanZ;
      let projX = dx*e1x+dy*e1y+dz*e1z;
      let projY = dx*e2x+dy*e2y+dz*e2z;
      let projZ = dx*e3x+dy*e3y+dz*e3z;
      minX = Math.min(minX, projX);
      maxX = Math.max(maxX, projX);
      minY = Math.min(minY, projY);
      maxY = Math.max(maxY, projY);
      minZ = Math.min(minZ, projZ);
      maxZ = Math.max(maxZ, projZ);
    }
    let cx = (minX+maxX)*0.5;
    let cy = (minY+maxY)*0.5;
    let cz = (minZ+maxZ)*0.5;

    return [
      meanX + cx*e1x + cy*e2x + cz*e3x,
      meanY + cx*e1y + cy*e2y + cz*e3y,
      meanZ + cx*e1z + cy*e2z + cz*e3z,
      (maxX-minX)*0.5,
      (maxY-minY)*0.5,
      (maxZ-minZ)*0.5,
      e1x, e2x, e3x,
      e1y, e2y, e3y,
      e1z, e2z, e3z,
    ];
  }

  function rotate_axes(a,b,th){
    let c=Math.cos(th), s=Math.sin(th);
    return [
      c*a[0]+s*b[0], c*a[1]+s*b[1], c*a[2]+s*b[2],
      -s*a[0]+c*b[0],-s*a[1]+c*b[1],-s*a[2]+c*b[2]
    ];
  }

  function obb_3d_refine(points,out){
    let best_vol = out[3]*out[4]*out[5]*8;
    let best_axes = [
      out[6], out[9], out[12],
      out[7], out[10],out[13],
      out[8], out[11],out[14]
    ];
    let angle = 0.15;
    let cx,cy,cz;
    for (let iter = 0; iter < 2; iter++){
      for (let i = 0; i < 3; i++){
        let j = (i+1)%3;
        let k = (i+2)%3;
        for (let s = -1; s <=1; s+=2){
          let try_axes = new Array(9);
          let m = rotate_axes(
            best_axes.slice(i*3,i*3+3), 
            best_axes.slice(j*3,j*3+3), s*angle);
          try_axes[i*3]=m[0];
          try_axes[i*3+1]=m[1];
          try_axes[i*3+2]=m[2];
          try_axes[j*3]=m[3];
          try_axes[j*3+1]=m[4];
          try_axes[j*3+2]=m[5];
          try_axes[k*3]=best_axes[k*3];
          try_axes[k*3+1]=best_axes[k*3+1];
          try_axes[k*3+2]=best_axes[k*3+2];

          let minX = Infinity, minY = Infinity, minZ = Infinity;
          let maxX =-Infinity, maxY =-Infinity, maxZ =-Infinity;
          for (let p = 0; p < points.length; p++){
            let dx = points[p][0] -  out[0];
            let dy = points[p][1] -out[1];
            let dz = points[p][2] -out[2];
            let projX = dx*try_axes[0]+dy*try_axes[1]+dz*try_axes[2];
            let projY = dx*try_axes[3]+dy*try_axes[4]+dz*try_axes[5];
            let projZ = dx*try_axes[6]+dy*try_axes[7]+dz*try_axes[8];
            minX = Math.min(minX, projX);
            maxX = Math.max(maxX, projX);
            minY = Math.min(minY, projY);
            maxY = Math.max(maxY, projY);
            minZ = Math.min(minZ, projZ);
            maxZ = Math.max(maxZ, projZ);
          }
          let vol = (maxX-minX)*(maxY-minY)*(maxZ-minZ);
          if (vol < best_vol){
            best_vol = vol;
            best_axes = try_axes;
            cx = (minX+maxX)*0.5;
            cy = (minY+maxY)*0.5;
            cz = (minZ+maxZ)*0.5;
            out[3] = (maxX-minX)*0.5;
            out[4] = (maxY-minY)*0.5;
            out[5] = (maxZ-minZ)*0.5;
          }
        }
      }
      angle *= 0.5;
    }
    out[0] += cx*best_axes[0] + cy*best_axes[3] + cz*best_axes[6];
    out[1] += cx*best_axes[1] + cy*best_axes[4] + cz*best_axes[7];
    out[2] += cx*best_axes[2] + cy*best_axes[5] + cz*best_axes[8];
    out[6] = best_axes[0]; out[7] = best_axes[3]; out[8] = best_axes[6];
    out[9] = best_axes[1]; out[10]= best_axes[4]; out[11]= best_axes[7];
    out[12]= best_axes[2]; out[13]= best_axes[5]; out[14]= best_axes[8];
  }


  function aabb_3d(points){
    let minX = Infinity, maxX = -Infinity;
    let minY = Infinity, maxY = -Infinity;
    let minZ = Infinity, maxZ = -Infinity;
    for (let i = 0; i < points.length; i++){
      let x = points[i][0];
      let y = points[i][1];
      let z = points[i][2];
      minX = Math.min(minX, x);
      maxX = Math.max(maxX, x);
      minY = Math.min(minY, y);
      maxY = Math.max(maxY, y);
      minZ = Math.min(minZ, z);
      maxZ = Math.max(maxZ, z);
    }
    return [
      (minX+maxX)*0.5,
      (minY+maxY)*0.5,
      (minZ+maxZ)*0.5,
      (maxX-minX)*0.5,
      (maxY-minY)*0.5,
      (maxZ-minZ)*0.5,
      1,0,0,
      0,1,0,
      0,0,1,
    ];
  }

  that.bbox = function(){
    let [points,flags] = $pop_args(2);
    let nd = Number(points.__type.elt[0].elt[1]);
    let out = [];
    if (nd == 2){
      if ((flags&0xf0)==MODE_ORIENTED){
        if (flags&0xf){
          out = obb_2d_rotcal(points);
        }else{
          out = obb_2d_pca(points);
        }
      }else if ((flags&0xf0)==MODE_ALIGNED){
        out = aabb_2d(points);
      }
      let a = out.slice(0,2);
      let b = out.slice(2,4);
      let c = out.slice(4);
      a.__type = {con:'vec',elt:['f32',2]}
      b.__type = {con:'vec',elt:['f32',2]}
      c.__type = {con:'vec',elt:['f32',2,2]}
      return [a,b,c];
    }else if (nd == 3){
      if ((flags&0xf0)==MODE_ORIENTED){
        out = obb_3d_pca(points);
        if (flags&0xf){
          obb_3d_refine(points,out);
        }
      }else if ((flags&0xf0)==MODE_ALIGNED){
        out = aabb_3d(points);
      }
      let a = out.slice(0,3);
      let b = out.slice(3,6);
      let c = out.slice(6);
      a.__type = {con:'vec',elt:['f32',3]}
      b.__type = {con:'vec',elt:['f32',3]}
      c.__type = {con:'vec',elt:['f32',3,3]}
      return [a,b,c];
    }
  }
};