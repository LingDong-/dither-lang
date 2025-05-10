globalThis.$arr = new function(){
  var that = this;

  that.reshape = function(){
    let [a,n] = $pop_args(2);
    let cnt = n.reduce((acc, num) => acc * num, 1);
    a.__dims = n;
    a.length = cnt;
  }
  that.make = function(){
    let [n,x] = $pop_args(2);
    let cnt = n.reduce((acc, num) => acc * num, 1);
    return Object.assign(new Array(cnt).fill(0).map(_=>$value(x)),{__dims:n.slice()});
  }
  that.shape = function(){
    let [x] = $pop_args(1);
    return x.__dims;
  }
}

