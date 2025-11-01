function embed_gui_extract(ast,scopes){
  let nmsp = ast.val;
  if (nmsp == 'global') nmsp = '__0';
  let sco;
  for (let i = 0; i < scopes.length; i++){
    if (scopes[i].__names == nmsp || scopes[i].__names.endsWith('.'+nmsp)){
      sco = scopes[i];
      break;
    }
  }
  function somepos(cst){
    for (let k in cst){
      if (k == 'pos'){
        return cst[k];
      }else if (typeof cst[k] == 'object'){
        let v = somepos(cst[k]);
        if (v) return v;
      }
    }
    return null;
  }

  let vars = [];
  for (let k in sco){
    let pos = somepos(sco[k]);
    if (k.startsWith('__')) continue;
    let v;
    if (nmsp != '__0'){
      v = {
        key: 'a.b',
        lhs: { tag: 'ident', val: nmsp, pos },
        rhs: { tag: 'ident', val: k, pos}
      }
    }else{
      v = { tag: 'ident', val:k,pos}
    }
    if (sco[k].nom && sco[k].nom.hnt){
      if (sco[k].nom.hnt.key == 'subs' && sco[k].nom.hnt.con.val == 'param'){
        vars.push({nom:k, var:v, typ:sco[k].typ, val:sco[k].val, arg:sco[k].nom.hnt.idx, pos});
      }else if (sco[k].nom.hnt.tag == 'ident' && sco[k].nom.hnt.val == 'param'){
        vars.push({nom:k, var:v, typ:sco[k].typ, val:sco[k].val, arg:[], pos});
      }
    }
  }
  return vars;
}


var embed_gui_layout = {
  type:"void",
  exec:function (ast,scopes){
    let vars = embed_gui_extract(ast,scopes);
    let out = [];
    for (let i = 0; i < vars.length; i++){
      let f = 'slider';
      let parg = vars[i].arg;
      if (vars[i].typ == 'i32' && vars[i].arg.length == 2
        && vars[i].arg[0].val == '0' && vars[i].arg[1].val == '1'
      ){
        f = 'toggle';
        parg = [];
      }else if (vars[i].typ == 'str'){
        f = 'field';
      }
      out.push({
        key: 'call',
        fun: {
          key: 'a.b',
          lhs: { tag: 'ident', val: 'gui', pos:vars[i].pos},
          rhs : {tag : 'ident', val: f, pos:vars[i].pos},
        },
        arg:[
          { tag: 'strlt', val: `"${vars[i].nom}"`, pos:vars[i].pos},
          vars[i].val,
          ...parg,
        ]
      })
      // console.dir(out,{depth:Infinity});
    }
    out.unshift({
      key: 'call',
      fun: {
        key: 'a.b',
        lhs: { tag: 'ident', val: 'gui', pos: [ 0,0 ] },
        rhs: { tag: 'ident', val: 'init', pos: [ 0,0 ] }
      },
      arg: []
    });
    let wrap = {
      key: 'bloc',
      val:out,
      pos:[0,0]
    }
    return wrap;
  }
}

var embed_gui_sync = {
  type:"void",
  exec:function (ast,scopes){
    let vars = embed_gui_extract(ast,scopes);
    let out = [];
    for (let i = 0; i < vars.length; i++){
      let {pos} = vars[i];
      out.push({
        key: '=',
        lhs: vars[i].var,
        rhs: {
          key: 'call',
          fun: {
            key: 'subs',
            con: {
              key: 'a.b',
              lhs: { tag: 'ident', val: 'gui', pos},
              rhs: { tag: 'ident', val: 'get', pos}
            },
            idx: [ { tag: 'ident', val: vars[i].typ, pos} ]
          },
          arg: [ { tag: 'strlt', val: `"${vars[i].nom}"`, pos} ]
        }
      });
    }

    out.unshift({
      key: 'call',
      fun: {
        key: 'a.b',
        lhs: { tag: 'ident', val: 'gui', pos: [ 0,0 ] },
        rhs: { tag: 'ident', val: 'poll', pos: [ 0,0 ] }
      },
      arg: []
    });
    let wrap = {
      key: 'bloc',
      val:out,
      pos:[0,0]
    }
    return wrap;
  }
}

if (typeof module !== 'undefined'){
  module.exports = {'gui/layout':embed_gui_layout,'gui/sync':embed_gui_sync}
}