globalThis.$exch = new function(){
  var that = this;

  let jt_obj;
  let jt_num;
  let jt_str;
  let jt_lst;
  let jt_dic;
  let jt_uon;
  function jbuild(jn){
    let o = {
      __type:jt_obj,
      data : {__val: null, __sel: -1, __type: jt_uon}
    }
    o.this = o;
    if (Array.isArray(jn)){
      o.data.__sel = 0;
      o.data.__val = Object.assign(jn.map(x=>jbuild(x)),{__type:jt_lst});
    }else if (typeof jn == 'object'){
      o.data.__sel = 1;
      o.data.__val = Object.assign(Object.fromEntries(Object.entries(jn).map(([x,y])=>[x,[[x,jbuild(y)]]])),{__type:jt_dic});
    }else if (typeof jn == 'string'){
      o.data.__sel = 2;
      o.data.__val = Object.assign([jn],{__type:jt_str});
    }else{
      o.data.__sel = 3;
      o.data.__val = Object.assign(new Float32Array([Number(jn)]),{__type:jt_num});
    }
    return o;
  }
  function jencode(o){
    let s = [];
    if (o.data.__sel == 0){
      s.push("[");
      for (let i = 0; i < o.data.__val.length; i++){
        if (i) s.push(",");
        s.push(jencode(o.data.__val[i]));
      }
      s.push("]");
    }else if (o.data.__sel == 1){
      s.push("{");
      let first = 1;
      for (let k in o.data.__val){
        if (k.startsWith("__")) continue;
        if (!first) s.push(",");
        first = 0;
        s.push(JSON.stringify(k)+":");
        s.push(jencode(o.data.__val[k][0][1]));
      }
      s.push("}");
    }else if (o.data.__sel == 2){
      s.push(JSON.stringify($unwrap(o.data.__val)));
    }else if (o.data.__sel == 3){
      s.push($unwrap(o.data.__val))
    }
    return s.join("");
  }

  that._decode_json = function(){
    let [o,s] = $pop_args(2);

    jt_obj = o.__type;
    jt_uon = o.data.__type;
    jt_lst = jt_uon.elt[0];
    jt_dic = jt_uon.elt[1];
    jt_str = jt_uon.elt[2];
    jt_num = jt_uon.elt[3];

    let jn = JSON.parse(s);

    o.data = jbuild(jn).data;
    // console.log(o);
  }
  that._encode_json = function(){
    let [o] = $pop_args(1);
    return jencode(o);
  }

}

