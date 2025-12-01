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
  that.has = function(){
    let [dic,x] = $pop_args(2);
    let keys = Object.entries(dic).filter(a=>!a[0].startsWith('__')).map(a=>a[1]).flat().map(a=>a[0]);
    for (let i = 0; i < keys.length; i++){
      if ($eq(keys[i],x)){
        return 1;
      }
    }
    return 0;
  }
}

