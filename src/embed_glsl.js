function embed_glsl_frag(ast,scopes){
  let out = [];

  let didfun = {};
  let didvar = {};
  let need_noise_impl = 0;
  let noise_impl = `
  float GLSLIMPL_perlin_array(int idx){
    return fract(sin(float(idx)*129.898)*43758.5453);
  }
  float GLSLIMPL_noise(float x, float y, float z) {
    int xi, yi, zi, of;
    float xf, yf, zf, rxf, ryf, r, ampl, n1, n2, n3;
    if (x<0.0) { x=-x; } if (y<0.0) { y=-y; } if (z<0.0) { z=-z; }
    xi=int(x); yi=int(y); zi=int(z);
    xf = x - float(xi); yf = y - float(yi); zf = z - float(zi);
    r=0.0; ampl=0.5;
    for (int o=0; o<4; o++) {
      of=xi+(yi*16)+(zi*256);
      rxf = 0.5*(1.0-cos(xf*3.14159265)); 
      ryf = 0.5*(1.0-cos(yf*3.14159265));
      n1  = GLSLIMPL_perlin_array(of);
      n1 += rxf*(GLSLIMPL_perlin_array((of+1))-n1);
      n2  = GLSLIMPL_perlin_array(of+16);
      n2 += rxf*(GLSLIMPL_perlin_array(of+17)-n2);
      n1 += ryf*(n2-n1);
      of += 256;
      n2  = GLSLIMPL_perlin_array(of);
      n2 += rxf*(GLSLIMPL_perlin_array(of+1)-n2);
      n3  = GLSLIMPL_perlin_array(of+16);
      n3 += rxf*(GLSLIMPL_perlin_array(of+17)-n3);
      n2 += ryf*(n3-n2);
      n1 += 0.5*(1.0-cos(zf*3.14159265))*(n2-n1);
      r += n1*ampl;
      ampl *= 0.5;
      xi*=2; xf*=2.0; yi*=2; yf*=2.0; zi*=2; zf*=2.0;
      if (xf>=1.0) { xi++; xf-=1.0; }
      if (yf>=1.0) { yi++; yf-=1.0; }
      if (zf>=1.0) { zi++; zf-=1.0; }
    }
    return r;
  }
  `

  function shortid(){
    var id = "";
    for (var i = 0; i < 6; i++){
      id+=String.fromCharCode(~~(Math.random()*26)+0x41);
    }
    return id;
  }
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
    }else if (ast.key == 'alit'){
      o += `${printtype(ast.typ.elt[0])}[${ast.val.length*ast.val[0].length}](`;
      for (let i = 0; i < ast.val.length; i++){
        for (let j = 0; j < ast.val[i].length; j++){
          if (i||j) o += ',';
          o += docompile(ast.val[i][j]);
        }
      }
      o += ")"
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
        let r = docompile(ast.val[i]);
        if (r.length) o += r+';';
      }
    }else if (ast.key == 'retn'){
      out.push(`return ${docompile(ast.val)};`);
    }else if (['+','-','*','/','==','||','&&','>','<','>=','<='].includes(ast.key)){
      o += `(${docompile(ast.lhs)}${ast.key}${docompile(ast.rhs)})`
    }else if (['+=','-=','*=','/='].includes(ast.key)){
      out.push(`${docompile(ast.lhs)}${ast.key}${docompile(ast.rhs)};`);
    }else if (['u++','u--'].includes(ast.key)){
      o += `(${docompile(ast.val)}${ast.key.slice(1)})`
    }else if (['+u','-u','++u','--u'].includes(ast.key)){
      o += `${ast.key.slice(0,-1)}${docompile(ast.val)}`
    }else if (ast.key == '%'){
      o += `${printtype(ast.typ)}(mod(float(${docompile(ast.lhs)}),float(${docompile(ast.rhs)})))`;
    }else if (ast.key == '**'){
      o += `pow(${docompile(ast.lhs)},${docompile(ast.rhs)})`;
    }else if (ast.key == '@*'){
      o += `(${docompile(ast.lhs)}*${docompile(ast.rhs)})`;
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
          }else if (nom == 'dot'){
            o += `dot(${ast.arg.map(docompile).join(',')})`
          }else if (nom == 'cross'){
            o += `cross(${ast.arg.map(docompile).join(',')})`
          }
        }else if (ast.fun.lhs.val == 'rand'){
          let nom = ast.fun.rhs.val[0].ipl.nom.val;
          if (nom == 'noise'){
            let args = ast.arg.map(docompile);
            if (args.length < 3) args.push('0');
            need_noise_impl = 1;
            o += `GLSLIMPL_noise(${args.join(',')})`;
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
    }else if (ast.key == '='){
      // o += `(${docompile(ast.lhs)} = ${docompile(ast.rhs)})`;
      out.push(`${docompile(ast.lhs)} = ${docompile(ast.rhs)};`)
    }else if (ast.key == 'cond'){
      out.push(`if (${docompile(ast.chk)}){`);
      out.push(docompile(ast.lhs));
      if (ast.rhs){
        out.push('}else{')
        out.push(docompile(ast.rhs));
      }
      out.push('}')
    }else if (ast.key == 'loop' && ast.chk && ast.ini && ast.stp && !ast.ck2){

      out.push(`for (`);
      docompile(ast.ini)
      out.push(`${docompile(ast.chk)}; ${docompile(ast.stp)}){`)
      out.push(docompile(ast.bdy));
      out.push(`}`);
    }else if (ast.key == 'subs'){
      
      if (ast.con.typ.con == 'vec' && ast.con.typ.elt.length > 2){
        let idx = shortid();
        out.push(`/*CONST*/int ${idx} = int(${docompile(ast.idx)});`);
        o += `(${docompile(ast.con)}[${idx}/${ast.con.typ.elt[1]}][int(mod(float(${idx}),float(${ast.con.typ.elt[1]})))])`;
      }else if (ast.con.typ.con == 'arr'){
        if (ast.idx.typ.con == 'vec'){
          let idx = shortid();
          out.push(`ivec2 ${idx} = ${docompile(ast.idx)}]\]\
          ;`);
          let con = docompile(ast.con);
          o += `(${con}/*ARRAY_SUBSCRIPT_BEGIN*/[int(${idx}.y*${con}__stride+${idx}.x)/*ARRAY_SUBSCRIPT_END*/])`;
        }else{
          o += `(${docompile(ast.con)}/*ARRAY_SUBSCRIPT_BEGIN*/[int(${docompile(ast.idx)})/*ARRAY_SUBSCRIPT_END*/])`;
        }
      }else{
        o += `(${docompile(ast.con)}[int(${docompile(ast.idx)})])`;
      }
    }else if (ast.key == 'a.b'){
      if (ast.lhs.tag == 'ident' && ast.lhs.val == 'math'){
        if (ast.rhs.tag == 'ident' && ast.rhs.val == 'PI'){
          o += `3.14159265357`;
        }
      }
    }else{
      console.log(ast)
    }
    return o;
  }
  function printtype(typ){
    if (typ.con == 'vec'){
      if (typ.elt[0] == 'f32'){
        if (typ.elt.length == 3){
          return 'mat'+typ.elt[1];
        }else{
          return typ.con+typ.elt[1];
        }
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
            if (a.typ.con == 'arr'){
              if (a.val.key == 'alit'){
                out.push(`/*ARRAY_LITERAL_BEGIN*/${printtype(a.typ.elt[0])} ${k}[${a.val.val.length*a.val.val[0].length}]=${docompile(a.val)};/*ARRAY_LITERAL_END*/`);
                out.push(`int ${k}__stride = ${a.val.val[0].length};`);
              }
            }else{
              out.push(`${printtype(a.typ)} ${k} = ${docompile(a.val)};`);
            }
          }else{
            // out.push(`uniform ${printtype(a.typ)} ${k};`);
            out.push(`${printtype(a.typ)} ${k};`);
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
  let vargs = ast.ipl.arg.map(x=>({val:x.lhs.val,typ:x.rhs.typ,hnt:x.lhs.hnt.val}));
  for (let i = 0; i < vargs.length; i++){
    let need = '';
    if (vargs[i].hnt == 'varying' || vargs[i].hnt == 'v'){
      need = `varying ${printtype(vargs[i].typ)} v_${vargs[i].val};`;
    }else if (vargs[i].hnt == 'uniform' || vargs[i].hnt == 'u'){
      need = `uniform ${printtype(vargs[i].typ)} ${vargs[i].val};`;
    }else if (vargs[i].hnt == 'compute'){
      need = {
        'd_position_dx':'varying vec3 v_position;',
        'd_position_dy':'varying vec3 v_position;',
        'd_uv_dx':'varying vec2 v_uv;',
        'd_uv_dy':'varying vec2 v_uv;',
      }[vargs[i].val];
    }
    if (!out.includes(need)){
      out.push(need);
    }
  }
  function maybe_builtin(x){
    if (x.hnt=='builtin'){
      return {
        'frag_coord':'gl_FragCoord',
        'front_facing':'gl_FrontFacing',
        'point_coord':'gl_PointCoord',
      }[x.val];
    }else if (x.hnt=='varying' || x.hnt=='v'){
      return 'v_'+x.val;
    }else if (x.hnt=='compute'){
      return {
        'd_position_dx':'dFdx(v_position)',
        'd_position_dy':'dFdy(v_position)',
        'd_uv_dx':'dFdx(v_uv)',
        'd_uv_dy':'dFdy(v_uv)',
      }[x.val];
    }
    return x.val;
  }
  compilefunc(ast);
  if (need_noise_impl){
    out.unshift(noise_impl);
  }

  out.push(`void main(){`);
  out.push(`gl_FragColor = ${ast.ipl.nom.val}(${[vargs.map(x=>maybe_builtin(x))].join(',')});`);
  out.push(`}`);
  let ret = "#version 120\n"+out.join('\n');
  console.log(ret);
  return ret;
}

if (typeof module !== 'undefined'){
  module.exports = {fragment:embed_glsl_frag}
}