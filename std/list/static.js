globalThis.$list = new function(){
  var that = this;

  that.slice = function(){
    let [a,i,j] = $pop_args(3);
    return a.slice(i,j)
  }
  that.insert = function(){
    let [a,i,x] = $pop_args(3);
    a.splice(i,0,$value(x));
  }
  that.erase = function(){
    let [a,i,j] = $pop_args(3);
    a.splice(i,j-i);
  }
  that.make = function(){
    let [n,x] = $pop_args(3);
    return new Array(n).fill(0).map(_=>$value(x));
  }
  that.length = function(){
    let [x] = $pop_args(1);
    return x.length;
  }
}

