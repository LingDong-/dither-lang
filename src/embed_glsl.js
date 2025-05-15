function embed_glsl_frag(ast,scopes){
  let out = [];

  let didfun = {};
  let didvar = {};

  function docompile(ast){
    let o = "";
    if (ast.key == 'vlit'){
      o += printtype(ast.typ)+"(";
      let vs = ast.val.flat();
      for (let i = 0; i < vs.length; i++){
        if (i) o += ',';
        o += docompile(vs[i]);
      }
      o += ")";
    }else if (ast.key == 'term'){
      if (ast.tag == 'numbr'){
        if (ast.typ == 'f32'){
          o += `float(${ast.val})`
        }else if (ast.typ == 'i32'){
          o += `int(${ast.val})`
        }
      }else if (ast.tag == 'ident'){
        o += ast.val;
      }
    }else if (ast.key == 'bloc'){
      for (let i = 0; i < ast.val.length; i++){
        o+=docompile(ast.val[i])
      }
    }else if (ast.key == 'retn'){
      out.push(`return ${docompile(ast.val)};`);
    }else if (['+','-','*','/'].includes(ast.key)){
      o += `${docompile(ast.lhs)}${ast.key}${docompile(ast.rhs)}`
    }else if (ast.key == 'swiz'){
      o += `${docompile(ast.lhs)}.${ast.rhs.val}`;
    }else if (ast.key == 'call'){
 
      if (ast.fun.key == 'a.b'){

        if (ast.fun.rhs.val == 'sample' && ast.fun.lhs.typ.endsWith(".Texture")){
          o += `texture2D(${ast.fun.lhs.val},${ast.arg.map(docompile).join(',')})`;
        }else if (ast.fun.lhs.val == 'math'){
          o += `${ast.fun.rhs.val[0].ipl.nom.val}(${ast.arg.map(docompile).join(',')})`
        }else if (ast.fun.lhs.val == 'vec'){
          let nom = ast.fun.rhs.val[0].ipl.nom.val;
          if (nom == 'mag'){
            o += `length(${ast.arg.map(docompile).join(',')})`
          }else if (nom == 'dir'){
            o += `normalize(${ast.arg.map(docompile).join(',')})`
          }
        }
        
      }else{
        o += `${ast.fun.val[0].ipl.nom.val}(${ast.arg.map(docompile).join(',')})`;
      }
    }else if (ast.key == 'cast'){
      o += `${printtype(ast.rhs.typ)}(${docompile(ast.lhs)})`
    }else if (ast.key == 'noop'){
      o += ``;
    }else if (ast.key == 'decl'){
      let typ;
      if (ast.ano){
        typ = ast.ano.typ;
      }else{
        typ = ast.val.typ;
      }
      if (ast.val){
        out.push(`${printtype(typ)} ${ast.nom.val} = ${docompile(ast.val)};`);
      }else{
        out.push(`${printtype(typ)} ${ast.nom.val};`);
      }
      
    }else{
      console.log(ast)
    }
    return o;
  }
  function printtype(typ){
    if (typ.con == 'vec'){
      if (typ.elt[0] == 'f32'){
        return typ.con+typ.elt[1];
      }else if (typ.elt[0] == 'i32'){
        return 'i'+typ.con+typ.elt[1];
      }
    }else if (typ == 'i32'){
      return `int`
    }else if (typ == 'f32'){
      return `float`
    }else if (typeof typ == 'string' && typ.includes('.')){
      let [nmsp,name] = typ.split(".");
      if (scopes[Number(nmsp)].__names.endsWith(".frag")){
        if (name == 'Texture'){
          return `sampler2D`;
        }
      }
    }else{
      return typ;
    }
  }
  function compilefunc(ast){
    if (didfun[ast.agt]) return;
    didfun[ast.agt] = 1;
    let caps = scopes[ast.agt].__captr??[];
    for (let i = 0; i < caps.length; i++){
      if (typeof caps[i].typ == 'string' && caps[i].typ.startsWith('__func_ovld_')){
        searchfunc(caps[i]);
      }else{
        searchvar(caps[i]);
      }
    }
   
    let oa = [];
    for (let k in scopes[ast.agt]){
      if (k.startsWith('__')) continue;
      oa.push(`${printtype(scopes[ast.agt][k].typ)} ${k}`);
    }
    out.push(`${printtype(ast.typ.elt[1])} ${ast.ipl.nom.val}(`+oa.join(',')+'){');
    out.push(docompile(ast.ipl.bdy));
    out.push(`}`);
  }
  function searchvar(info){
    for (let i = 0; i < scopes.length; i++){
      for (let k in scopes[i]){
        if (k == info.nom && info.ori == '__'+i){
          let id = info.ori+'.'+info.nom;
          if (didvar[id]) return;
          didvar[id] = 1;
          let a = scopes[i][k];
          if (a.val !== null){
            out.push(`${printtype(a.typ)} ${k} = ${docompile(a.val)};`);
          }else{
            out.push(`uniform ${printtype(a.typ)} ${k};`);
          }
          
          
        }
      }
    }
  }
  function searchfunc(info){
    for (let i = 0; i < scopes.length; i++){
      for (let k in scopes[i]){
        if (typeof scopes[i][k].typ == 'string' && scopes[i][k].typ.startsWith('__func_ovld')){
          for (let j = 0; j < scopes[i][k].val.length; j++){
            if (scopes[i][k].val[j].ipl.mty == info.typ){
              return compilefunc(scopes[i][k].val[j]);
            }
          }
        }
      }
    }
  }

  // console.dir(ast,{depth:10000});
  // console.dir(scopes,{depth:10000});

  compilefunc(ast);

  out.push(`void main(){`);
  out.push(`gl_FragColor = ${ast.ipl.nom.val}(gl_FragCoord.xy);`);
  out.push(`}`);
  let ret = "#version 120\n"+out.join('\n');
  console.log(ret);
  return ret;
}

if (typeof module !== 'undefined'){
  module.exports = {fragment:embed_glsl_frag}
}