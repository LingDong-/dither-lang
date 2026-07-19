globalThis.$regx = new function(){
  var that = this;

  that.test = function(){
    let [s,re] = $pop_args(2);
    return Number(new RegExp(`^(?:${re})$`).test(s));
  }

  that.find = function(retype){
    let [str,re] = $pop_args(2);
    let rx = new RegExp(re, "g");
    let o = [];
    o.__type = retype;
    for (const m of str.matchAll(rx)) {
      let groups = [...m];
      groups.__type = retype.elt[0].elt[1];
      let tup = [m.index, groups];
      tup.__type = retype.elt[0];
      o.push(tup);
    }
    return o;
  }

  that.replace = function(){
    let [s,re,rep] = $pop_args(3);
    let rx = new RegExp(re, "g");
    return s.replace(rx,rep);
  }
}