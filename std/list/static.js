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
    let [n,x] = $pop_args(2);
    return new Array(n).fill(0).map(_=>$value(x));
  }
  that.length = function(){
    let [x] = $pop_args(1);
    return x.length;
  }
  that._sort = function(){
    let [a,b] = $pop_args(2);
    let ab = a.map((x,i)=>[b[i],x]);
    ab.sort((x,y)=>x[0]-y[0]);
    for (let i = 0; i < a.length; i++){
      a[i] = ab[i][1];
    }
  }
}

