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
  that.max = function(){
    let [x,y] = $pop_args(2);
    return Math.max(x,y);
  }
  that.min = function(){
    let [x,y] = $pop_args(2);
    return Math.min(x,y);
  }
}

