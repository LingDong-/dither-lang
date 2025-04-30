globalThis.$gx = new function(){
  let that = this;
  let ctx;
  let cnv;
  let fbos = [];
  let no_stroke = 0;
  let no_fill = 0;
  that._size = function(){
    let [w,h] = $pop_args(2);
    cnv = document.getElementById("canvas");
    ctx = cnv.getContext('2d');
    ctx.strokeStyle="black";
    ctx.fillStyle="white";
  }
  that._init_graphics = function(){
    let [pg,w,h] = $pop_args(3);
    let c = document.createElement("canvas");
    c.width = w;
    c.height = h;
    fbos.push(c);
    pg.w = w;
    pg.h = h;
    pg.fbo = fbos.length-1;
    pg.tex = fbos.length-1;
  }
  that._begin_fbo = function(){
    let [fbo] = $pop_args(1);
    ctx = fbos[fbo].getContext('2d');
  }
  that._end_fbo = function(){
    ctx = cnv.getContext('2d');
  }
  that._draw_texture = function(){
    let [tex,x,y,w,h] = $pop_args(5);
    ctx.drawImage(fbos[tex], x,y,w,h);
  }
  that.background = function(){
    let [r,g,b,a] = $pop_args(4);
    ctx.save();
    ctx.resetTransform();
    ctx.fillStyle = `rgba(${~~(r*255)}, ${~~(g*255)}, ${~~(b*255)}, ${(~~(a*1000))/1000})`;
    ctx.fillRect(0,0,ctx.canvas.width,ctx.canvas.height);
    ctx.restore();
  }
  that.stroke = function(){
    let [r,g,b,a] = $pop_args(4);
    ctx.strokeStyle = `rgba(${~~(r*255)}, ${~~(g*255)}, ${~~(b*255)}, ${(~~(a*1000))/1000})`;
    no_stroke = 0;
  }
  that.fill = function(){
    let [r,g,b,a] = $pop_args(4);
    ctx.fillStyle = `rgba(${~~(r*255)}, ${~~(g*255)}, ${~~(b*255)}, ${(~~(a*1000))/1000})`;
    no_fill = 0;
  }
  that.no_fill = function(){
    no_fill = 1;
  }
  that.no_stroke = function(){
    no_stroke = 1;
  }
  that.stroke_weight = function(){
    let [x] = $pop_args(1);
    ctx.lineWidth = x;
  }
  that.point = function(){
    let [x,y] = $pop_args(2);
    if (no_stroke) return;
    let w = ctx.lineWidth;
    ctx.save();
    ctx.fillStyle = ctx.strokeStyle;
    ctx.fillRect(x-w/2,y-w/2,w,w);
    ctx.restore();
  }
  that.line = function(){
    let [x0,y0,x1,y1] = $pop_args(4);
    if (no_stroke) return;
    ctx.beginPath();
    ctx.moveTo(x0,y0);
    ctx.lineTo(x1,y1);
    ctx.stroke();
  }
  that.rect = function(){
    let [x,y,w,h] = $pop_args(4);
    if (!no_fill)
    ctx.fillRect(x,y,w,h);
    if (!no_stroke)
    ctx.strokeRect(x,y,w,h);
  }
  that.ellipse = function(){
    let [x,y,w,h] = $pop_args(4);
    if (no_fill && no_stroke) return;
    ctx.beginPath();
    ctx.ellipse(x, y, w/2, h/2, 0, 0, 2 * Math.PI);
  
    if (!no_fill)
    ctx.fill();
    if (!no_stroke)
    ctx.stroke();
  }

}
