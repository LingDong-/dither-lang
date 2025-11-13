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
  that.decode = function(){
    let [a,e] = $pop_args(2);
    if (Array.isArray(a)){
      a = new Uint8Array(a).buffer;
    }
    return new TextDecoder(e).decode(a);
  }
  that.slice = function(){
    let [a,i,j] = $pop_args(3);
    return a.slice(i,j);
  }
  that.split = function(){
    let [a,b] = $pop_args(2);
    return a.split(b);
  }
  that.trim = function(){
    let [a,b,e] = $pop_args(3);
    let start = 0;
    let end = a.length;
    if (e & 1){
      while (b.includes(a[start])) start++;
    }
    if (e & 2){
      while (b.includes(a[end-1])) end--;
    }
    return a.slice(start,end);
  }
  that.join = function(){
    let [s,a] = $pop_args(2);
    return a.join(s);
  }
}

