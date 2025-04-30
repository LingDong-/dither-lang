globalThis.$str = new function(){
  var that = this;

  that.length = function(){
    let [s] = $pop_args(1);
    return s.length;
  }
  that.chr = function(){
    let [i] = $pop_args(1);
    return String.fromCharCode(i);
  }
  that.ord = function(){
    let [s] = $pop_args(1);
    return s.charCodeAt(0);
  }
}

