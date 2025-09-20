
var TO_JS = function(cfg){
  let lib = `
  var $numtyps = ['i8','u8','i16','u16','i32','u32','i64','u64','f32','f64'];
  var $args = [];
  var $caps = [];
  function $include(x){
    if (globalThis.__dh_intern_hooked_include){
      return __dh_intern_hooked_include(x);
    }else if (typeof module !== 'undefined'){
      return require('fs').readFileSync(x).toString();
    }else{
      var xh = new XMLHttpRequest();
      xh.open("GET",x,false);
      xh.send(null);
      return xh.responseText;
    }
  }
  function $is_ref(x){
    if (!x || !x.__type) return false;
    return $numtyps.includes(x.__type) || x.__type == 'str' || x.__type.con == 'vec' || x.__type.con == 'tup';
  }
  function $value(x){
    if ($is_ref(x)){
      return Object.assign(x.slice(),{__type:x.__type});
    }
    return x;
  }
  function $typed_value(x,__type){
    if ($is_ref(x)){
      return Object.assign(x.slice(),{__type:x.__type});
    }else if (typeof x == 'number'){
      return Object.assign(new $typed_cons[__type]([x]),{__type});
    }else if (typeof x == 'string'){
      return Object.assign([x],{__type});
    }
    return x;
  }
  function $unwrap(x){
    if ( $numtyps.includes(x?.__type) || x?.__type == 'str'){
      return x[0];
    }
    return x;
  }
  function $pop_args(n){
    return $args.splice(-n).map($unwrap);
  }
  var $typed_cons = {
    "u8":Uint8Array,
    "i8":Int8Array,
    "u16":Uint16Array,
    "i16":Int16Array,
    "u32":Uint32Array,
    "i32":Int32Array,
    "u64":BigUint64Array,
    "i64":BigInt64Array,
    "f32":Float32Array,
    "f64":Float64Array,
  }
  function $assign(dst,src){
    if (typeof dst !== 'object' || dst === null){
      return src;
    }else if (typeof src !== 'object'){
      dst[0] = src;
      return dst;
    }else{
      // return Object.assign(dst,src);
      src.__type = dst.__type;
      return src;
    }
  }
  function $to_str(x){
    if (typeof x == 'undefined' || x == null){
      return 'null';
    }else if (typeof x != 'object'){
      return x.toString();
    }else if ($numtyps.includes(x.__type)){
      return x[0].toString();
    }else if (x.__type.con == 'vec'){
      return '{'+x.toString()+'}'
    }else if (x.__type.con == 'list'){
      return '{'+x.map($to_str).join(',')+'}';
    }else if (x.__type.con == 'arr'){
      return '['+x.__dim.join(',')+']{'+x.map($to_str).join(',')+'}';
    }else if (x.__type.con == 'dict'){
      return '{'+Object.entries(x).filter(a=>!a[0].startsWith('__')).map(a=>a[1]).flat().map(a=>$to_str(a[0])+':'+$to_str(a[1])).join(',')+'}'
    }else if (x.__type.con == 'union'){
      return $to_str(x.__val);
    }else if (x.__type == 'str'){
      return x[0];
    }else if (x.__type.con == 'tup'){
      return '['+x.map($to_str).join(',')+']';
    }else{
      return '[object:'+JSON.stringify(x.__type)+']';
    }
  }
  function $eq(x,y){
    if (x === y) return true;
    x = $unwrap(x);
    y = $unwrap(y);
    if (x === y) return true;
    if (x?.__type?.con == 'vec' && y?.__type?.con == 'vec'){
      return x.toString() == y.toString();
    }
    return false;
  }
  function $hash_slot(dict,key){
    let l = dict[key];
    if (!l){
      dict[key] = l = [[key,$value(dict.__zero)]];
      return l[0];
    }
    for (let i = 0; i < l.length; i++){
      if ($eq(l[i][0],key)){
        return l[i];
      }
    }
    l.push([key,$value(dict.__zero)]);
    return l.at(-1);
  }
  `
  eval(lib);



  function UNIMPL(){
    console.error("UNIMPLEMENTED");
    console.trace();
    process.exit();
  }

  function parse_layout(ls){
    let lo = {};
    let ca = "";
    for (let i = 0; i < ls.length; i++){
      if (ls[i][0] == '\t'){
        let vs = ls[i].trim().split('\t');
        vs[0] = Number(vs[0]);
        vs[1] = clean(vs[1]);
        vs[2] = read_type(vs[2]);
        lo[ca].fields.push(vs);
      }else{
        let [a,b] = ls[i].trim().split("\t");
        lo[ca = read_type(a)] = {
          size: Number(b),
          fields:[],
        }
      }
    }
    return lo;
  }

  function parse_ir(txt){
    
    let ls = txt.split("\n").filter(x=>x.trim().length);
    let o = [];
    let lo = {};
    for (let i = 0; i < ls.length; i++){
      if (ls[i].trim() == "eoir"){
        lo = parse_layout(ls.slice(i+1));
        break;
      }
      ls[i] = ls[i].trim();
      let lbl = "";
      let ln = ls[i];
      if (ls[i].includes(':')){
        let q = ls[i].split(':')
        if (!q[0].includes('"')){
          lbl = q[0];
          ln = q.splice(1).join(':');
        }
      }
      let lx = [];
      let ac = "";
      let st = 0;
      for (let j = 0; j < ln.length; j++){
        if (st == 0 && ln[j] == " "){
          lx.push(ac);
          ac = "";
        }else if (ln[j] == '"'){
          if (st == 0){
            st = 1;
          }else if (st == 1){
            st = 0;
          }else{
            ac += '\\'
            st = 1;
          }
          ac += ln[j];
        }else if (st && ln[j] == '\\'){
          if (st == 1){
            st = 2;
          }else{
            ac += '\\\\'
            st = 1;
          }
        }else{
          if (st == 2){
            ac += '\\'
            st = 1;
          }
          ac += ln[j];
        }
      }
      lx.push(ac);
      lx = lx.map(x=>x.trim()).filter(x=>x.length);
      o.push([lbl,lx])
    }
    return [o,lo];
  }

  function read_type(s){
    let acc = "";
    let cstk = [];
    let cptr = [];
    cstk.push(cptr);
    function proc(x){
      return maybenum(x);
    }
    for (let i = 0; i < s.length; i++){
      if (s[i] == '['){
        cptr.push({con:proc(acc),elt:[]});
        acc = "";
        cptr = cptr.at(-1).elt;
        cstk.push(cptr);
      }else if (s[i] == ']'){
        if (acc.length){
          cptr.push(proc(acc));
          acc = "";
        }
        cstk.pop();
        cptr = cstk.at(-1);
      }else if (s[i] == ','){
        if (acc.length){
          cptr.push(proc(acc));
          acc = "";
        }
      }else{
        acc += s[i];
      }
    }
    if (acc.length){
      o = proc(acc);
    }else{
      o = cstk[0][0];
    }

    function flat(o){
      if (o.con){
        if (o.con.includes('.')){
          return `${o.con}[${o.elt.map(flat).join(',')}]`;
        }else{
          return {con:o.con,elt:o.elt.map(flat)}
        }
      }
      return o;
    }
    o = flat(o);
    return o;
  }


  function maybenum(x){
    let re = /^(?:0[xX][0-9A-Fa-f]+|[+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)$/;
    if (re.test(x)){
      let y = parseFloat(x);
      if (y==0){
        return parseInt(x);
      }
      return y;
    }
    return x;
  }

  function clean(x){
    if (x[0] == '"') return x;
    x = maybenum(x);
    if (typeof x == 'string'){
      return x
        .replace(/\[/g,'_\$L_')
        .replace(/\]/g,'_\$7_')
        .replace(/\,/g,'_\$9_')
        .replace(/\./g,'_\$o_')
        .replace(/^(\d)/, '_\$N_$1')
    }
    return x;
  }
  function shortid(){
    var id = "";
    for (var i = 0; i < 6; i++){
      id+=String.fromCharCode(~~(Math.random()*26)+0x41);
    }
    return id;
  }

  function transpile_js(instrs,layout){

    let o = [];
    let lookup = {};
    function vec_type_flat_n(tb){
      return eval(tb.elt.slice(1).join('*'))
    }
    function cast(a,b,ins){
      let ta = lookup[a];
      let tb = lookup[b];
      if (ta == 'void'){
        o.push(`${get_ptr(a)} = null`);
      }else if (ta == 'str'){
        o.push(`${get_ptr(a)} = $to_str(${get_ptr(b)})`);
      }else if ($numtyps.includes(ta) && $numtyps.includes(tb)){
        o.push(`${get_ptr(a)} = new $typed_cons.${ta}([${get_ptr(b)}])[0]`);
      }else if ($numtyps.includes(ta) && typeof b == 'number'){
        o.push(`${get_ptr(a)} = new $typed_cons.${ta}([${get_ptr(b)}])[0]`);
      }else if (ta.con == 'vec' && ($numtyps.includes(tb) || typeof b == 'number')){
        for (let i = 0; i < vec_type_flat_n(ta); i++){
          o.push(`${a}[${i}] = ${get_ptr(b)}`);
        }
      }else if (ta.con == 'union'){
        if (typeof b == 'number'){
          if (ins[2].includes(".")){
            o.push(`${a}.__sel = ${ta.elt.indexOf('f32')};`);
          }else{
            o.push(`${a}.__sel = ${ta.elt.indexOf('i32')};`);
          }
        }else if (b[0] == '"'){
          o.push(`${a}.__sel = ${ta.elt.indexOf('str')};`);
        }else{
          o.push(`${a}.__sel = ${ta.elt.map(x=>JSON.stringify(x)).indexOf(JSON.stringify(tb))};`);
        }
        o.push(`${a}.__val = $value(${b});`);
      }else if (tb.con == 'union'){
        o.push(`${a} = $value(${b}.__val);`);
      }else if (ta.con == 'vec' && tb.con == 'vec'){
        for (let i = 0; i < vec_type_flat_n(ta); i++){
          o.push(`${a}[${i}] = ${b}[${i}]`);
        }
      }else{
        console.log(a,b,ta,tb)
        UNIMPL();
      }
    }

    function math(op,a,b,c){
      let os = {
        add:'+',sub:'-',mul:'*',div:'/',pow:'**',mod:'%',band:'&',bor:'|',xor:'^'
      }[op];
      let typ = lookup[a];
      if (typ.con == 'vec'){
        for (let i = 0; i < vec_type_flat_n(typ); i++){
          o.push(`${a}[${i}]=${b}[${i}]${os}${c}[${i}];`);
        }
      }else if (os == '/' && typ.startsWith && !typ.startsWith('f')){
        o.push(`${get_ptr(a)}=new $typed_cons.${typ}([$unwrap(${get_ptr(b)})${os}$unwrap(${get_ptr(c)})])[0];`);
      }else{
        o.push(`${get_ptr(a)}=$unwrap(${get_ptr(b)})${os}$unwrap(${get_ptr(c)});`);
      }
    }

    function compare(op,a,b,c){
      let os = {
        leq:'<=',geq:'>=',lt:'<',gt:'>',eq:'==',neq:'!='
      }[op];
      let tb = lookup[b];
      let tc = lookup[c];
      if ($numtyps.includes(tb) || $numtyps.includes(tb) || typeof b == 'number' || typeof c == 'number'){
        o.push(`${get_ptr(a)}=Number(${get_ptr(b)}${os}${get_ptr(c)});`);
      }else if (op == 'eq' && tb == 'str'){
        o.push(`${get_ptr(a)}=Number(${get_ptr(b)}==$to_str(${get_ptr(c)}));`);
      }else if (op == 'eq' && tc == 'str'){
        o.push(`${get_ptr(a)}=Number(${get_ptr(c)}==$to_str(${get_ptr(b)}));`);
      }else{
        UNIMPL();
      }
    }

    function get_ptr(x){
      if (typeof x == 'number' || x[0] == '"'){
        return x;
      }else if (x.includes('+')){
        let [v,idx] = x.split('+');
        let t = lookup[v];
        if (t.con == 'vec'){
          return `${v}[${idx}]`;
        }else if (t.con == 'list'){
          return `${v}[${idx}]`;
        }else if (t.con == 'arr'){
          if (lookup[idx] && lookup[idx].con == 'vec'){
            let ii = "0";
            let stride = "1";
            for (let i = t.elt[1]-1; i >= 0; i--){
              ii += `+(${idx}[${i}] * ${stride})`;
              stride += `*(${v}.__dims[${i}])`;
            }
            idx = `(${ii})`;
          }
          return `${v}[${idx}]`;
        }else if (t.con == 'dict'){
          return `$hash_slot(${v},${idx})[1]`;
        }else if (t.con == 'tup'){
          return `${v}[${idx}]`;
        }else if (typeof t == 'string'){
          return `${v}["${idx}"]`;
        }else{
          UNIMPL();
        }
      }else{
        let t = lookup[x];
        if ($numtyps.includes(t) || t == 'str'){
          if (allcaps[x]){
            return `${x}[0]`
          }else{
            return `${x}`
          }
        }else{
          return x;
        }
      }
    }

    function type_zero(typ,nowrap){
      if (typ.con == 'vec'){
        return `Object.assign(new $typed_cons.${typ.elt[0]}(${typ.elt.slice(1).join('*')}),{__type:${JSON.stringify(typ)}})`
      }else if (typ.con == 'tup'){
        return `Object.assign([${typ.elt.map(x=>type_zero(x)).join(',')}],{__type:${JSON.stringify(typ)}})`
      }else if (typ == 'str'){
        if (nowrap){
          return `""`
        }else{
          return `Object.assign([""],{__type:${JSON.stringify(typ)}})${nowrap?'[0]':''}`
        }
      }else if (typ == 'i64' || typ == 'u64'){
        return `Object.assign(new $typed_cons.${typ}(1),{__type:${JSON.stringify(typ)}})${nowrap?'[0]':''}`
      }else if ($numtyps.includes(typ)){
        if (nowrap){
          return `0`;
        }else{
          return `Object.assign(new $typed_cons.${typ}(1),{__type:${JSON.stringify(typ)}})`
        }
      }else if (typ.con == 'list'){
        return `Object.assign([],{__type:${JSON.stringify(typ)}})`;
      }else if (typ.con == 'arr'){
        return `Object.assign([],{__dims:[${'0,'.repeat(typ.elt[1])}],__type:${JSON.stringify(typ)}})`;
      }else if (typ.con == 'union'){
        return `{__val:null,__sel:-1,__type:${JSON.stringify(typ)}}`;
      }else{
        if (nowrap){
          return 'null';
        }
        return `{__type:${JSON.stringify(typ)}}`;
      }
    }

    let funcs = {};
    let typds = {};
    let curfun;
    let curtypd;
    let inmain = 0;
    let lbl2int = {};
    let lblidx = 1;
    let allcaps = [];
    let putback = [];

    for (let i = 1; i < instrs.length; i++){
      let [lbl, ins] = instrs[i];
      if (lbl.length){
        lbl2int[lbl] = lblidx++;
        if (lbl == '__main__'){
          inmain = 1;
        }else if (lbl.startsWith("__func_ovld_")){
          let funname = clean(lbl);
          funcs[funname] = curfun = {dcap:[]};
        }else if (lbl.startsWith("__typd_")){
          typds[clean(lbl)] = curtypd = {instrs:[]};
          inmain = -inmain;
        }else if (lbl.startsWith("end__typd_")){
          curtypd = null;
          inmain = -inmain;
        }else{
          
        }
      }else{
      }
      if (curtypd && ins.length){
        curtypd.instrs.push(ins);
        instrs[i][1] = ["nop"];
      }else if (ins[0] == 'dcap'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        curfun.dcap.push({nom,typ});
        allcaps[nom] = 1;
      }else if (ins[0] == 'argr'){
        let nom = clean(ins[1]);
        allcaps[nom] = 1;
      }
    }
    instrs.shift();
    // console.log(allcaps);
    let infun = 0;
    inmain = 0;

    for (let i = 0; i < instrs.length; i++){
      let [lbl, ins] = instrs[i];
      
      // console.log(lbl,ins)
      if (lbl.length){
        if (lbl == '__main__'){

          if (infun){
            putback.splice(0,Infinity);
            o.push(`default:$goto=0;break;}}}`);
          }
          // o.push(`(async function main(){`);
          o.push(`let $goto = -1`);
          o.push(`$$: while ($goto){switch($goto){case -1:`);
          infun = 0;
          inmain = 1;
        }else if (lbl.startsWith("__func_ovld_")){

          if (infun){
            putback.splice(0,Infinity);
            o.push(`default:$goto=null;break;}}}`);
          }
          let funname = clean(lbl);
          o.push(`async function ${funname}(){`);
          o.push(`let $goto = -1`);
          o.push(`$$: while ($goto){switch($goto){case -1:`);
          infun = 1;

        }else{
          o.push(`case ${lbl2int[lbl]}:/*${lbl}*/`);
        }
      }else{
        if (infun || inmain){
        }
      }
      trans_instr(ins);
    }
    function trans_instr(ins){
      if (!ins.length || ins[0] == 'nop'){

      }else if (ins[0] == 'decl'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        lookup[nom] = typ;
        if (allcaps[nom]){
          o.push(`var ${nom} = ${type_zero(typ)};`);
        }else{
          o.push(`var ${nom} = ${type_zero(typ,1)};`);
        }
      }else if (ins[0] == 'alloc'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        lookup[nom] = typ;
        if (typ.con == 'list'){
          o.push(`var ${nom} = Object.assign(new Array(${ins[3]}).fill(0).map(_=>$value(${type_zero(typ.elt[0],1)})), {__type:${JSON.stringify(typ)}})`);
        }else if (typ.con == 'arr'){
          let ndim = typ.elt[1];
          let cnt = ins[3];
          let is2d = cnt & (1<<30);
          let n = cnt;
          let d0 = cnt;
          let d1 = 1;
          if (is2d){
            d0 = ((cnt >> 15) & 0x7fff);
            d1 = (cnt & 0x7fff);
            n = d0 * d1;
          }
          let ds = [d0,d1];
          while (ds.length < ndim) ds.push(1);
          o.push(`var ${nom} = Object.assign(new Array(${n}).fill(0).map(_=>$value(${type_zero(typ.elt[0],1)})), {__dims:${JSON.stringify(ds)},__type:${JSON.stringify(typ)}})`);
        }else if (typ.con == 'dict'){
          o.push(`var ${nom} = {__zero:${type_zero(typ.elt[1],1)},__type:${JSON.stringify(typ)}}`);
        }else if (typeof typ == 'string'){
          let lo = layout[typ];
          o.push(`var ${nom} = {__type:${JSON.stringify(typ)}};`);
          for (let i = 0; i < lo.fields.length; i++){
            o.push(`${nom}["${lo.fields[i][1]}"] = ${type_zero(lo.fields[i][2],1)};`);
          }
        }else{
          UNIMPL();
        }
      }else if (ins[0] == 'dcap'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        lookup[nom] = typ;
        o.push(`var ${nom} = $args.pop();`);
        putback.push(nom);
      }else if (ins[0] == 'jmp'){
        let l = ins[1];
        o.push(`$goto=${lbl2int[l]};/*${l}*/ continue $$;`);
      }else if (ins[0] == 'jeqz'){
        let l = ins[2];
        o.push(`if (!${get_ptr(clean(ins[1]))}){$goto=${lbl2int[l]};/*${l}*/ continue $$;}`);
      }else if (ins[0] == 'mov'){
        let a = clean(ins[1])
        let b = clean(ins[2]);
        o.push(`${get_ptr(a)} = $value(${get_ptr(b)});`);
      }else if (['add','sub','mul','div','mod','pow'].includes(ins[0])){
        math(ins[0], clean(ins[1]), clean(ins[2]), clean(ins[3]));
      }else if (['band','bor','xor'].includes(ins[0])){
        math(ins[0], clean(ins[1]), clean(ins[2]), clean(ins[3]));
      }else if (ins[0] == 'bnot'){
        o.push(`${get_ptr(clean(ins[1]))} = ~${get_ptr(clean(ins[2]))};`);
      }else if (ins[0] == 'lnot'){
        o.push(`${get_ptr(clean(ins[1]))} = !${get_ptr(clean(ins[2]))};`);
      }else if (ins[0] == 'shl'){
        o.push(`${get_ptr(clean(ins[1]))} = ${get_ptr(clean(ins[2]))} << ${get_ptr(clean(ins[3]))};`);
      }else if (ins[0] == 'shr'){
        o.push(`${get_ptr(clean(ins[1]))} = ${get_ptr(clean(ins[2]))} >> ${get_ptr(clean(ins[3]))};`);
      }else if (['leq','geq','lt','gt','neq','eq'].includes(ins[0])){
        compare(ins[0], clean(ins[1]), clean(ins[2]), clean(ins[3]));
      }else if (ins[0] == 'matmul'){
        let c = clean(ins[1]);
        let a = clean(ins[2]);
        let b = clean(ins[3]);
        let ta = lookup[a];
        let tb = lookup[b];
        let nr0 = ta.elt[1];
        let nc0 = 1;
        let nr1 = tb.elt[1];
        let nc1 = 1;
        if (ta.elt.length == 3){
          nc0 = ta.elt[2];
        }
        if (tb.elt.length == 3){
          nc1 = tb.elt[2];
        }
        let nr = nr0;
        let nc = nc1;
        for (let i = 0; i < nr; i++){
          for (let j = 0; j < nc; j++){
            let s = "0";
            for (let k = 0; k < nc0; k++){
              s += `+${a}[${i*nc0+k}]*${b}[${k*nc1+j}]`;
            }
            o.push(`${c}[${i*nc1+j}]=${s};`);
          }
        }
      }else if (ins[0] == 'cast'){
        cast(clean(ins[1]),clean(ins[2]),ins);
      }else if (ins[0] == 'ccall'){
        let tmp = shortid();
        o.push(`${tmp} = $${ins[2]}();`);
        o.push(`if (${tmp} instanceof Promise) {${tmp} = await ${tmp};}`);
        let a = clean(ins[1]);
        o.push(`${a}=$assign(${a},${tmp});`);
      }else if (ins[0] == 'argw'){
        // console.log(ins)
        o.push(`$args.push($typed_value(${clean(ins[1])},${JSON.stringify(read_type(ins[2]))}));`);
      }else if (ins[0] == 'argr'){
        let nom = clean(ins[1]);
        let typ = read_type(ins[2]);
        lookup[nom] = typ;
        o.push(`var ${nom} = $args.pop();`);
      }else if (ins[0] == 'fpak'){
        let nom = clean(ins[1]);
        let ptr = clean(ins[3]);
        
        let {dcap} = funcs[ptr];
        o.push(`var ${nom} = {__funptr:${ptr},__captr:[],__type:'func'}`);
        for (let i = dcap.length-1; i>=0; i--){
          o.push(`${nom}.__captr.push($value(${dcap[i].nom}))`);
        }
      }else if (ins[0] == 'cap'){
        // pass
      }else if (ins[0] == 'call'){
        let v = clean(ins[1])
        let funname = clean(ins[2]);

        if (funcs[funname]){
          for (let i = funcs[funname].dcap.length-1; i>=0; i--){
            let {typ,nom} = funcs[funname].dcap[i];
            o.push(`$args.push(${nom})`);
          }
          o.push(`${v}=$assign(${v},await ${funname}());`);
          for (let i = funcs[funname].dcap.length-1; i>=0; i--){
            let {typ,nom} = funcs[funname].dcap[i];
            o.push(`${nom} = $caps.pop();`);
          }
        }else if (typds[funname]){
          let oldlookup = Object.assign({},lookup);
          o.push("{")
          for (let i = 0; i < typds[funname].instrs.length; i++){
            if (typds[funname].instrs[i][0] == 'ret'){
              o.push(`${get_ptr(v)} = ${clean(typds[funname].instrs[i][1])};`);
            }else{
              trans_instr(typds[funname].instrs[i]);
            }
          }
          Object.assign(lookup,oldlookup);
          o.push("}")
        }
        

      }else if (ins[0] == 'rcall'){
        let v = clean(ins[1])
        let fun = clean(ins[2]);
        o.push(`$args.push(...${fun}.__captr.map($value))`);
        o.push(`${v}=$assign(${v},await ${fun}.__funptr());`);

      }else if (ins[0] == 'ret'){
        for (let i = 0; i < putback.length; i++){
          o.push(`$caps.push(${putback[i]});`);
        }
        if (ins[1]){
          let a = clean(ins[1]);
          o.push(`return ${a};`);
        }
        o.push(`return;`);
      }else if (ins[0] == 'incl'){
        if (cfg.preclude){
          o.push($include(ins[1].slice(1,-1)+"/static.js"));
        }else{
          o.push(`eval($include(${ins[1]}+"/static.js"))`); 
        }
      }else if (ins[0] == 'utag'){
        let a = clean(ins[1]);
        let b = clean(ins[2]);
        let typ = read_type(ins[3]);
        let tb = lookup[b];
        let idx = tb.elt.map(x=>JSON.stringify(x)).indexOf(JSON.stringify(typ));
        o.push(`${a} = Number(${idx} == (${b}).__sel);`);
      }else{
        UNIMPL();
        console.log(ins)
      }
    }
    o.push(`default:$goto=0;break;}}})()`);
    o.unshift(`(async function (){`)
    o.unshift(lib);
    return o.join('\n')
  }
  this.parse_ir = parse_ir;
  this.transpile = transpile_js;
}


if (typeof module !== 'undefined'){
  if (require.main !== module){
    module.exports = TO_JS;
  }else{
    let to_js = new TO_JS({preclude:1});
    let inp_pth;
    let out_pth;
    for (let i = 2; i < process.argv.length; i++){
      if (process.argv[i] == '-o' || process.argv[i] == '--output'){
        out_pth = process.argv[++i];
      }else{
        inp_pth = process.argv[i];
      }
    }
    const fs = require('fs');
    let txt = fs.readFileSync(inp_pth).toString();
    let [ir,layout] = to_js.parse_ir(txt);
    // console.dir(layout,{depth:Infinity})
    fs.writeFileSync(out_pth,to_js.transpile(ir,layout));
  }
}

