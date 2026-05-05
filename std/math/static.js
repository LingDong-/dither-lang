globalThis.$math = new function(){
  var that = this;
  that.pi = Math.PI;
  that.random = function(){
    return Math.random();
  }
  that.sin = function(){
    let [x] = $pop_args(1);
    return Math.sin(x);
  }
  that.cos = function(){
    let [x] = $pop_args(1);
    return Math.cos(x);
  }
  that.acos = function(){
    let [x] = $pop_args(1);
    return Math.acos(x);
  }
  that.abs = function(){
    let [x] = $pop_args(1);
    return Math.abs(x);
  }
  that.max = function(){
    let [x,y] = $pop_args(2);
    return Math.max(x,y);
  }
  that.min = function(){
    let [x,y] = $pop_args(2);
    return Math.min(x,y);
  }
  that.atan2 = function(){
    let [y,x] = $pop_args(2);
    return Math.atan2(y,x);
  }
  that.hypot = function(){
    let [x,y] = $pop_args(2);
    return Math.hypot(x,y);
  }
  that.round = function(){
    let [x] = $pop_args(1);
    return Math.round(x);
  }
  that.floor = function(){
    let [x] = $pop_args(1);
    return Math.floor(x);
  }
  that.ceil = function(){
    let [x] = $pop_args(1);
    return Math.ceil(x);
  }
  that.exp = function(){
    let [x] = $pop_args(1);
    return Math.exp(x);
  }
  that.sqrt = function(){
    let [x] = $pop_args(1);
    return Math.sqrt(x);
  }
  that.tan = function(){
    let [x] = $pop_args(1);
    return Math.tan(x);
  }
  that.bitcast = function(){
    let t = $args.at(-1).__type;
    let [x] = $pop_args(1);
    let buf = new ArrayBuffer(4);
    let f = new Float32Array(buf);
    let u = new Uint32Array(buf);
    if (t == 'f32'){
      f[0] = x; return u[0];
    }else{
      u[0] = x; return f[0];
    }
  }
}

