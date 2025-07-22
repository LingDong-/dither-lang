globalThis.$vec = new function(){
  let that = this;
  that.mag = function(){
    let [v] = $pop_args(1);
    let s = 0;
    for (let i = 0; i < v.length; i++){
      s += v[i]*v[i];
    }
    return Math.sqrt(s);
  }
  that.dir = function(){
    let [v] = $pop_args(1);
    let s = 0;
    for (let i = 0; i < v.length; i++){
      s += v[i]*v[i];
    }
    let u = v.slice();
    if (s){
      s = 1.0/Math.sqrt(s);
      for (let i = 0; i < u.length; i++){
        u[i] *= s;
      }
    }
    return u;
  }
  that.dot = function(){
    let [u,v] = $pop_args(2);
    let s = 0;
    for (let i = 0; i < v.length; i++){
      s += u[i]*v[i];
    }
    return s;
  }
}