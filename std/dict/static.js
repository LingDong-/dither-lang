globalThis.$dict = new function(){
  var that = this;

  that.keys = function(){
    let [dic] = $pop_args(1);
    let lst = Object.entries(dic).filter(a=>!a[0].startsWith('__')).map(a=>a[1]).flat().map(a=>a[0])
    return lst;
  }
  that.values = function(){
    let [dic] = $pop_args(1);
    let lst = Object.entries(dic).filter(a=>!a[0].startsWith('__')).map(a=>a[1]).flat().map(a=>a[1])
    return lst;
  }
}

