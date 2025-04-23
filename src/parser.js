var PARSER = function(sys){
  let alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";
  let numer = "0123456789";
  let alphanumer = alpha+numer;
  let keywr = [
    "namespace",
    "continue",
    "typedef","include",
    "return",
    "break","while","const",
    "else","func",
    "for",
    "if","do","as"
  ];
  let sigil = [
    ">>=","<<=","@*=","...",
    "+=","-=","*=",'/=',"!=","==",">=","<=","&=","^=","|=","%=","++","--",
    "<?=",">?=",
    "&&","||","**","@*",">>","<<",
    "+","-","*","/","?",":","<",">",",",".","~","=","&","|","%","!","^",
    "{","}","[","]","(",")",";","\n"
  ];
  let biord = [
    ['**'],
    ['*','/','%','@*'],
    ['+','-'],
    ['<<','>>'],
    ['<','<=','>','>='],
    ['==','!='],
    ['&'],
    ['^'],
    ['|'],
    ['&&'],
    ['||'],
  ];
  let assgns = ['=','+=','-=','*=','/=','%=','&=','|=','^=','<<=','>>=','<?=','>?=','@*=','**='];
  let binops = biord.flat().concat(assgns).concat([","]);
  let intyps = ['i8','u8','i16','u16','i32','u32','i64','u64'];
  let nmtyps = [...intyps,'f32','f64'];
  let matyps = [...nmtyps,'vec'];
  let cntyps = ['vec','arr','list','tup'];
  let smtyps = [...nmtyps,'str'];
  let hctyps = [...cntyps,'str'];

  let state = {
    src:[],
  };

  function mkerr(tag,msg,pos){

    let [pthidx,idx] = pos??[0,0];
    let txt = state.src[pthidx].txt;

    let lns = txt.slice(0,idx).split("\n");
    let lino = lns.length;
    let chno = lns.at(-1).length+1;
    let rest = txt.slice(idx);
    rest = (rest+" ").slice(0,rest.indexOf('\n'));

    console.error(`[${tag} error] ${msg} at file "${state.src[pthidx].pth}" line ${lino} column ${chno}`);
    console.error((lns.at(-1)+rest).replace(/\t/g,' '));
    console.error(' '.repeat(chno-1)+'^');
    // console.trace();
    sys.process.exit(1);
  }

  function reader(pth){
    return new TextDecoder().decode(sys.fs.readFileSync(pth));
  }

  function tokenize(pth){
    let txt = reader(pth);

    for (let i = 0; i < state.src.length; i++){
      if (state.src[i].pth == pth){
        return [];
      }
    }
    let pthidx = state.src.length;
    state.src.push({
      pth,txt
    })

    function _tokenize(txt,posofs){
      let i = 0;
      let toks = [];
      function pushtok(tag,val,i){
        toks.push({tag,val,pos:[pthidx,posofs+i]});
      }
      while (i < txt.length){
        let a = txt[i];
        let ok = false;

        if (txt.slice(i,i+2)=='//'){
          let s = "";
          i++;
          while ((++i) < txt.length){
            if (txt[i] == '\n'){
              break;
            }else{
              s += txt[i];
            }
          }
          // pushtok('cmmnt',s,i);
          pushtok('sigil','\n',i)
          i++;
          continue;
        }else if (txt.slice(i,i+2)=='\\\n'){
          i+=2;
          continue;
        }else if (txt.slice(i,i+2)=='/*'){
          let s = "";
          i++;
          while ((++i) < txt.length){
            if (txt.slice(i,i+2) == '*/'){
              break;
            }else{
              s += txt[i];
            }
          }
          // pushtok('cmmnt',s,i);
          i+=2;
          continue;
        }
        for (let j = 0; j < keywr.length; j++){
          let end = i+keywr[j].length;
          if (keywr[j]==txt.slice(i,end)){
            if (!alphanumer.includes(txt[end])){
              pushtok('keywr',keywr[j],i);
              ok = true;
              i += keywr[j].length;
              break;
            }
          }
        }
        if (ok) continue;

        if (alpha.includes(a)){
          let s = a;
          while(alphanumer.includes(txt[++i])){
            s += txt[i];
          }
          pushtok('ident',s,i-s.length);
          continue;
        }
        
        for (let j = 0; j < sigil.length; j++){
          if (sigil[j]==txt.slice(i,i+sigil[j].length)){
            if (sigil[j] != '.' || !(numer.includes(txt[i+1]))){
              pushtok('sigil',sigil[j],i);
              ok = true;
              i += sigil[j].length;
              break;
            }
          }
        }

        if (ok) continue;
        let gotdot, gote, isb, isx;
        if (numer.includes(txt[i]) || (gotdot=(txt[i]=="."))){
          let s = a;
          while((++i) < txt.length){
            if (numer.includes(txt[i]) && !isb && !isx){
              s += txt[i];
            }else if (isb && "01_".includes(txt[i])){
              if (txt[i] != '_') s += txt[i];
            }else if (isx && (numer+"abcdef_").includes(txt[i].toLowerCase())){
              if (txt[i] != '_') s += txt[i];
            }else if (txt[i]=="."){
              if (gotdot) mkerr('tokenize','extra floating point in number',[pthidx,posofs+i]);
              s += txt[i];
              gotdot = true;
            }else if ( (isb=(txt[i].toLowerCase() == 'b')) || (isx=(txt[i].toLowerCase() == 'x'))){
              if (s.length != 1) mkerr('tokenize','misplaced b/x in number',[pthidx,posofs+i]);
              s += txt[i];
              gotdot = true;
            }else if ("Ee".includes(txt[i])){
              if (gote) mkerr('tokenize','extra E/e in number',[pthidx,posofs+i]);
              s += txt[i];
              gote = true;
            }else if ((txt[i]=='-' || txt[i]=='+') && (s.at(-1).toLowerCase() == 'e')){
              s += txt[i];
            }else if (!isx && !isb && txt[i].toLowerCase()=='f'){
              if (!gotdot && !gote) s+=".";
              i++;
              break;
            }else{
              break;
            }
          }
          pushtok('numbr',s.toLowerCase(),i);
          continue;
        }else if (a == '"' || a == "'"){
          let s = "";
          let esc;
          let sit = 0;
          let ss = [];
          let i0 = i;
          while((++i) < txt.length){
            // console.log(i,txt[i],sit)
            if (esc) {
              if ('nrt'.includes(txt[i])){
                s += JSON.parse('"\\'+txt[i]+'"');
              }else if (txt[i] == '\\'){
                s += '\\';
              }else if (txt[i] == '%'){
                s += '\\'+txt[i];
              }else{
                s += txt[i];
              }
              esc = false;
            }else if (txt[i] == '%'){
              s += txt[i];
              if (!(sit & 1)){
                sit ++;
              }
            }else if (sit){
              if (txt[i] == '{'){
                sit ++;
                if (sit == 2){
                  ss.push(s.slice(0,-1));
                  s = "";
                }else{
                  s += txt[i];
                }
              }else if (txt[i] == '}'){
                sit -= 2;
                if (sit == 0){
                  ss.push(_tokenize(s,i));
                  s = "";
                }else{
                  s += txt[i];
                }
              }else if (sit & 1){
                sit --;
                s += txt[i];
              }else{
                s += txt[i];
              }
              
            }else if (txt[i] == '\\'){
              esc = txt[i];
            }else if (sit == 0 && txt[i] == a){
              if (s.length) ss.push(s);
              i++;
              break;
            }else{
              s += txt[i];
            }
          }
          if (sit != 0){
            mkerr('tokenize',`malformed string interpolation`,[pthidx,posofs+i0]);
          }
          if (ss.length == 1 && a == "'"){
            pushtok('numbr',ss[0].charCodeAt(0).toString(),i);
          }else{
            pushtok('strlt',ss,i);
          }
          continue;
        }
        i++;
      }
      return toks;
    }
    return _tokenize(txt,0);
  }




  function parse(toks){

    let i = 0;
    let cst = {key:'bloc',val:[]};
    function gtok(x){
      return x??{};
    }
    function ntok(x,i){
      if (i < 0) i = 0;
      if (i >= x.length) i = x.length-1;
      return gtok(x[i])
    }
    function tokis(x,y){
      if (x == undefined){
        return false;
      }
      return (x.tag+':'+x.val) == y;
    }
    function tokamong(x,yy){
      if (x == undefined){
        return false;
      }
      return yy[yy.indexOf((x.tag+':'+x.val))];
    }

    function getexpr(i,untilclose=null,untilforce=null){
      let tmp = [];
      let lvl = {'()':0,'[]':0,'{}':0};
      let j;
      for (j = i; j < toks.length; j++){
        for (let k in lvl){
          if (tokis(toks[j],'sigil:'+k[0])){
            lvl[k]++;
          }else if (tokis(toks[j],'sigil:'+k[1])){
            lvl[k]--;

            if (lvl[k] < 0){            
              if (k[1] == untilclose){
                if (!untilforce || k[1] == untilforce){
                  return [++j,parseexpr(tmp)];
                }
              }else if (k[1] == '}'){
                if (!untilforce || k[1] == untilforce){
                  return [j,parseexpr(tmp)];
                }
              }else{
                mkerr('parse',`unmatched '${k[1]}'`,ntok(toks,j).pos);
              }
            }
          }
        }
        let sum = Object.values(lvl).reduce((a,c)=>(a+c),0);
        let wasbin = false;
        let conops = binops.concat(['!','~','...']);
        for (let k = 0; k < conops.length; k++){
          if (tokis(toks[j-1],'sigil:'+conops[k])){
            wasbin = true;
          }
        }
        if (!wasbin && sum == 0 && tokis(toks[j],'sigil:\n') && (!untilforce || untilforce == '\n')){
          j++;
          break;
        }
        if (sum == 0 && tokis(toks[j],'sigil:;') && (!untilforce || untilforce==';')){
          j++;
          break;
        }
        if (lvl['{}'] != 0 || !tokis(toks[j],'sigil:\n')){
          tmp.push(toks[j]);
        }
      }
      
      return [j,parseexpr(tmp)];
    }
    
    function parseexpr(tmp){
      // console.log(tmp)
      // for (let i = 0; i < tmp.length; i++){
      //   if (tokis(tmp[i],'sigil:;')){
      //     mkerr('parse',"unexpected ';' in expression",ntok(tmp,i).pos);
      //   }
      // }
      
      if (!tmp.length){
        return {key:'noop'};
      }
      let roughpos = tmp[0].pos;
      let i = 0;
      
      let out0 = [];

      function ggroup(k){
        // console.log(k,tmp)
        let lvl = 0;
        let val = [];
        while (i < tmp.length){
          val.push(tmp[i]);

          if (tokis(tmp[i],'sigil:'+k[0])){
            lvl++;
          }else if (tokis(tmp[i],'sigil:'+k[1])){
            lvl--;
            
            if (lvl == 0){            
              if (k == '{}'){
                // console.log('~',val);
                let v = parse(val.slice(1,-1));
                // console.dir(v,{depth:10000});
                return {key:k,val:v.val};
              }else{
                return {key:k,val:parseexpr(val.slice(1,-1))};
              }
            }
          }
          i++;
        }
        mkerr('parse',`unmatched '${k[0]}'`,ntok(tmp,0).pos);
      }

      // console.trace();
      // console.dir(tmp,{depth:100000})

      while (i < tmp.length){
        if (tokis(tmp[i],"sigil:(")){
          
          let g = ggroup('()');
          let prv = -1; while (tokis(out0.at(prv),'sigil:\n')){i--}
          if (out0.at(prv) && gtok(out0.at(prv)).tag != 'sigil' && gtok(out0.at(prv)).tag != 'keywr'){
            
            out0.push({key:'call',lhs:out0.pop(),rhs:g.val});
          }else{
            
            out0.push(g);
          }
        }else if (tokis(tmp[i],"sigil:[")){
          let g = ggroup('[]');
          let prv = -1; while (tokis(out0.at(prv),'sigil:\n')){prv--}
          if (out0.at(prv) && gtok(out0.at(prv)).tag != 'sigil'){
            out0.push({key:'subs',lhs:out0.pop(),rhs:g.val});
          }else{
            out0.push(g);
          }
        }else if (tokis(tmp[i],"sigil:{")){

          let g = ggroup('{}');
                  
          let prv = -1; while (tokis(out0.at(prv),'sigil:\n')){prv--}
          let pprv = prv-1; while (tokis(out0.at(pprv),'sigil:\n')){pprv--};

          // console.log(',',out0.at(prv), g.val);

          if (g.val.length <= 1 && 
            out0.at(prv) && (gtok(out0.at(prv)).tag == 'ident' || gtok(out0.at(prv)).key == 'subs' || gtok(out0.at(prv)).key == 'a.b') &&
            
            (!out0.at(pprv) || (!tokis(out0.at(pprv),'sigil::') && !tokis(out0.at(pprv),'sigil:]')  ))
          ){
            
            out0.push({key:'olit',lhs:out0.pop(),rhs:g.val[0]});
          }else{
            
            out0.push(g);
            // console.dir(out0,{depth:10000});
          }
        }else if (tokis(tmp[i],"sigil:.")){
          if (gtok(tmp[i+1]).tag != 'ident'){
            mkerr('parse','member access expects identifier',ntok(tmp,i).pos);
          }
          if (!tmp[i-1] || (tmp[i-1].tag == 'sigil' && !tokis(tmp[i-1],'sigil:)') && !tokis(tmp[i-1],'sigil:}') && !tokis(tmp[i-1],'sigil:]'))){
            // out0.push({key:'.u',val:tmp[i+1]});
            
            out0.push({key:'a.b',lhs:{tag:'ident',val:'this',pos:tmp[i+1].pos},rhs:tmp[i+1]});
          }else{
            out0.push({key:'a.b',lhs:out0.pop(),rhs:tmp[i+1]});
          }
          i++;
        }else if (tmp[i].tag == "strlt"){
          let val = [];
          for (let j = 0; j < tmp[i].val.length; j+=2){
            if (tmp[i].val[j] !== undefined){
              val.push({tag:"strlt",val:'"'+tmp[i].val[j]+'"',pos:tmp[i].pos});
            }
            if (tmp[i].val[j+1] !== undefined){
              val.push(parseexpr(tmp[i].val[j+1]));
            }
          }
          if (val.length == 1){
            out0.push(val[0])
          }else if (val.length == 0){
            out0.push({
              tag: 'strlt',
              val:'""',
              pos:tmp[i].pos,
            })
          }else{
            out0.push({
              key: 'strit',
              val,
              pos:tmp[i].pos,
            })
          }
        }else{
          out0.push(tmp[i]);
        }
        i++;
      }
      
      function dounary(out0,ks,ispost){
        let out1 = [];
        let i = 0;
        while (i < out0.length){
          if (tokamong(out0[i],ks)){
            if (!ispost && (out0[i-1] == undefined || out0[i-1].tag == 'sigil')){
              out1.push({key:out0[i].val+'u',val:out0[++i]});
            }else if (ispost && (out0[i+1] == undefined || out0[i+1].tag == 'sigil')){
              out1.push({key:'u'+out0[i].val,val:out1.pop(),is_unop:true});
            }else{
              out1.push(out0[i])
            }
          }else{
            out1.push(out0[i])
          }
          i++;
        }
        return out1;
      }

      let out1 = dounary(out0,['sigil:++','sigil:--'],true);

      let out2 = dounary(out1,['sigil:++','sigil:--','sigil:+','sigil:-','sigil:~','sigil:!','sigil:...'],false);

    

      function dobinary(out0,ks,dir=1){
        let out1 = [];
        let i = (out0.length-1)*(-dir+1)/2;
        while ((dir>0)?(i < out0.length):(i>=0)){
          if (tokamong(out0[i],ks)){
            let key = out0[i];
            let lhs = out1.pop();
            i += dir;
            let rhs = out0[i];
            if (dir < 0){
              ;[lhs,rhs] = [rhs,lhs]
            }
            if (!lhs)mkerr('parse',`binary operator '${key.val}' missing left operand`,gtok(key).pos);
            if (!rhs){
              if (key.val == ','){
                // rhs = null;
                out1.push(lhs);
              }else{
                mkerr('parse',`binary operator '${key.val}' missing right operand`,gtok(key).pos);
              }
            }else if (rhs.tag == 'sigil'){
              if (key.val == ':'){
                out1.push({key:key.val,lhs,rhs:null});
              }
              i -= dir;
            }else{
              out1.push({key:key.val,lhs,rhs,is_binop:true});
            }
          }else{
            out1.push(out0[i])
          }
          i+=dir;
        }
        if (dir<0){
          out1.reverse();
        }
        return out1;
      }

      out2 = dobinary(out2,["keywr:as"]);

      for (let k of biord){
        out2 = dobinary(out2, k.map(x=>"sigil:"+x));
      }

      function doternary(out0,ks){
        let out1 = [];
        let i = out0.length;
        while ((--i) >= 0){
          let found = false;
          for (let k of ks){
            if (tokis(out0[i],k[0]) && tokis(out1[1],k[1])){
              let key = out0[i].val+out1[1].val;
              if (!out0[i-1]) mkerr('parse',`ternary operator '${key}' missing left operand`,ntok(out0,i).pos);
              if (!out1[0])   mkerr('parse',`ternary operator '${key}' missing middle operand`,ntok(out0,i).pos);
              if (!out1[2])   mkerr('parse',`ternary operator '${key}' missing right operand`,ntok(out0,i).pos);
              out1.unshift({
                key,
                lhs:out0[i-1],mhs:out1.shift(),rhs:(out1.shift(),out1.shift()),
                is_ternop:true,
              })
              found = true;
              i--;
              break;
            }
          }
          if (!found){
            out1.unshift(out0[i]);
          }
        }

        return out1;
      }
      let out3 = doternary(out2,[['sigil:?','sigil::']]);

      
      let out4 = dobinary(out3,['sigil::']);
      
      function dofunc(out0){
        let out1 = [];
        let i = 0;
        while (i < out0.length){
          if (tokis(out0[i],"keywr:func")){
            let bdy = null;
            if (out0[i+2] && out0[i+2].key=='{}'){
              bdy = {key:'bloc',val:out0[i+2].val};
            }
            let arg = out0[i+1];
            let typ = null;
            let nom = null;
            let tem = null;
            
            if (arg.key == ':'){
              
              typ = arg.rhs;
              arg = arg.lhs;
            }
            if (arg.key == 'call'){
              nom = arg.lhs;
              arg = arg.rhs;
            }
            if (nom.key == 'subs'){
              tem = nom.rhs;
              nom = nom.lhs;
            }
            if (arg.key == '()'){
              arg = arg.val;
            }
            out1.push({key:'func',nom,tem,arg,ano:typ,bdy});
            i+=2;
          }else{
            out1.push(out0[i])
          }
          i++;
        }
        return out1;
      }
      
      let out5 = dofunc(out4);
      
      let out6 = dobinary(out5,assgns.map(x=>"sigil:"+x),-1);

      let out7 = dobinary(out6,['sigil:,']);

      if (out7.length != 1){
        // console.dir(out7,{depth:100000})
        mkerr('parse',`extra elements in expression`,roughpos);
      }
      return out7[0];
    }

    function getblock(i){
      let tmp = [];
      let lvl = 0;
      let j;
      for (j = i; j < toks.length; j++){
        if (tokis(toks[j],'sigil:{')){
          lvl++;
        }else if (tokis(toks[j],'sigil:}')){
          lvl--;
          if (lvl == -1){
            return [j+1,parse(tmp)];
          }
        }
        tmp.push(toks[j]);
      }
      mkerr('parse','unclosed block',ntok(toks,i).pos);
    }

    function getstatement(i,untilforce=null){
      function consume(i){
        while (tokis(toks[i],"sigil:\n")){
          i++;
        }
        return i;
      }
      let ret0,ret1,ret2,ret3;
      i = consume(i);
      if (tokis(toks[i],'sigil:;')){
        return [i+1,{key:'noop'}];
      }

      if (i >= toks.length){
        return [toks.length,null];
      }
      if (tokis(toks[i],'sigil:{')){
        ;[i,ret0] = getblock(++i);
        return [i,ret0];
      }else if (tokis(toks[i],'keywr:if')){
        i++;
        i = consume(i);
        if (!tokis(toks[i],'sigil:(')){
          mkerr('parse',`expected '(' after 'if'`,ntok(toks,i).pos);
        }
        ;[i,ret0] = getexpr(++i,')');
        i = consume(i);
        ;[i,ret1] = getstatement(i);
        ret2 = null;
        i = consume(i);
        if (tokis(toks[i],'keywr:else')){
          ;[i,ret2] = getstatement(++i);
        }
        return [i,{key:'cond',chk:ret0,lhs:ret1,rhs:ret2}];
      }else if (tokis(toks[i],'keywr:for')){
        i++;
        i = consume(i);
        if (!tokis(toks[i],'sigil:(')){
          mkerr('parse',`expected '(' after 'for'`,ntok( toks,i).pos);
        }
        i++;
        i = consume(i);
        ;[i,ret0] = getstatement(i,';');
        i = consume(i);
        ;[i,ret1] = getexpr(i,null,';');
        i = consume(i);
        ;[i,ret2] = getexpr(i,')',')');
        i = consume(i);
        ;[i,ret3] = getstatement(i);
        return [i,{key:'loop',ini:ret0,chk:ret1,stp:ret2,bdy:ret3}];

      }else if (tokis(toks[i],'keywr:while')){
        i++;
        i = consume(i);
        if (!tokis(toks[i],'sigil:(')){
          mkerr('parse',`expected '(' after 'while'`,ntok(toks,i).pos);
        }
        ;[i,ret0] = getexpr(++i,')');
        
        i = consume(i);
        
        ;[i,ret1] = getstatement(i);
        
        return [i,{key:'loop',chk:ret0,bdy:ret1}];

      }else if (tokis(toks[i],'keywr:do')){
        i++;
        i = consume(i);
        ;[i,ret0] = getstatement(i);
        i = consume(i);
        if (!tokis(toks[i],'keywr:while')){
          mkerr('parse',"expected 'while' in do-while loop",ntok(toks,i).pos);
        }
        i++;
        i = consume(i);
        if (!tokis(toks[i],'sigil:(')){
          mkerr('parse',`expected '(' after 'while'`,ntok(toks,i).pos);
        }
        ;[i,ret1] = getexpr(++i,')');
        i = consume(i);
        return [i,{key:'loop',ck2:ret1,bdy:ret0}]

      }else if (tokis(toks[i],'keywr:typedef')){
        let k = toks[i].val;
        i++;
        i = consume(i);
        // let nom = toks[i++];
        // if (gtok(nom).tag != 'ident'){
        //   mkerr('parse',`'${k}' expected identifier`,ntok(toks,i).pos)
        // }
        // i = consume(i);
        // ;[i,ret0] = getexpr(i);
        // return [i,{key:((k == 'typedef') ? 'typd' : 'sgnt'),nom,val:ret0}]
        ;[i,ret0] = getexpr(i);
        if (ret0.key != '='){
          mkerr(`parse`,`${k} expected '='`,ntok(toks,i).pos);
        }
        return [i,{key:'typd',lhs:ret0.lhs,rhs:ret0.rhs}];
      }else if (tokis(toks[i],'keywr:namespace')){
        i++;
        i = consume(i);
        let nom = toks[i++];
        if (gtok(nom).tag != 'ident'){
          mkerr('parse',`'namespace' expected identifier`,ntok(toks,i).pos)
        }
        i = consume(i);
        ;[i,ret0] = getexpr(i);
        if (!ret0 || ret0.key != '{}'){
          mkerr('parse',`'namespace' expected block`,ntok(toks,i).pos)
        }
        return [i,{key:'nmsp',nom,val:ret0.val}]
      }else if (tokis(toks[i],'keywr:return')){
        
        ;[i,ret0] = getexpr(++i);
        let pos = (toks[i]??toks[i-1]).pos;
        return [i,{key:'retn',val:ret0,pos}]

      }else if (tokis(toks[i],'keywr:break')){
        return [++i,{key:'break',pos:toks[i].pos}]
      }else if (tokis(toks[i],'keywr:continue')){
        return [++i,{key:'continue',pos:toks[i].pos}]

      }else if (tokis(toks[i],'keywr:include')){

        ;[i,ret0] = getexpr(++i);
        return [i,{key:'incl',val:ret0,pos:toks[i-1].pos}]

      }else if (gtok(toks[i]).tag == 'cmmnt'){
        return [++i,{key:'noop',txt:toks[i-1].val}]
        
      }else{
        ;[i,ret0] = getexpr(i,null,untilforce);
        return [i,ret0];
      }
    }
    

    while (i < toks.length){

      ;[i,stmt] = getstatement(i);
      // console.log(".....",stmt)

      if (stmt && stmt.key != 'noop'){
        
        if (stmt.key == 'incl'){
          let mypth = state.src[stmt.pos[0]].pth;
          let mydir = sys.path.dirname(mypth);
          let diropts = [mydir, sys.process.cwd(), ...(sys.search_paths??[])];
          
          let ok = 0;
          let urfil = stmt.val.val.slice(1,-1);
          for (let mydir of diropts){
            
            try{
              
              let urpth = sys.path.resolve(sys.path.join(mydir,urfil));
              // console.log(urpth);
              
              if (urpth.endsWith(".dh")){
                let tks = tokenize(urpth);
                let xst = parse(tks);
                xst.val.forEach(x=>cst.val.push(x));
                ok = 1;
              }else if (urpth.endsWith(".so")){
                let s = reader(urpth).split("/**BEGINHEADER**/")[1].split("/**ENDHEADER**/")[0];
                
                let rpath = sys.path.relative(sys.process.cwd(), urpth);

                stmt.val.val = '"'+rpath+'"'
                cst.val.push(stmt);
                let hdpth = "/tmp/lib_header_"+sys.path.basename(urfil,sys.path.extname(urfil))+(~~(Math.random()*10000)).toString().padStart(4,'0')+".dh";
                sys.fs.writeFileSync(hdpth,s);
                let tks = tokenize(sys.path.resolve(hdpth));
                let xst = parse(tks);
                xst.val.forEach(x=>cst.val.push(x));
                ok = 1;
              }else{
                let pth = sys.path.join(urpth,"header.dh");
                // let pth2 = sys.path.join(urpth,"dynamic.so");
                let _ = reader(pth);
                let rpath = sys.path.relative(sys.process.cwd(), urpth);
                stmt.val.val = '"'+rpath+'"'
                cst.val.push(stmt);
                let hdpth = pth;
                let tks = tokenize(sys.path.resolve(hdpth));
                let xst = parse(tks);
                xst.val.forEach(x=>cst.val.push(x));
                ok = 1;
              }
              
              break;
            }catch(e){
              // console.log(e)
            }
          }
          if (!ok){
            mkerr('parse', `cannot locate inlcude ${stmt.val.val}`,stmt.pos);
          }
        }else{
          cst.val.push(stmt);
        }
      }
    }
    // console.log('cst');
    // console.dir(cst,{depth:100000})
    return cst;
  }
  function clone(ff){
    return JSON.parse(JSON.stringify(ff));
  }

  function typeeq(t0,t1){
    return JSON.stringify(t0)==JSON.stringify(t1);
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

  function abstract(cst){
    function untree(cst){
      if (!cst){
        return [];
      }
      if (cst.key != ','){
        return [cst];
      }
      return untree(cst.lhs).concat(untree(cst.rhs));
    }
    function abstype(cst){
      if (cst == null) return null;
      if (cst.key == 'subs'){
        if (cst.lhs.tag == 'keywr') cst.lhs.tag = 'ident';
        if (cst.lhs.tag != 'ident') mkerr('syntax','expected identifier for type annotation',cst.lhs.pos);
        let rhs = untree(cst.rhs);
        let rrhs = [];
        for (let i = 0; i < rhs.length; i++){
          if (rhs[i].key != 'noop'){
            rrhs.push(abstype(rhs[i]));
          }
        }
        return {key:'type',ttp:'cntp',con:cst.lhs,elt:rrhs};
      }else if (cst.tag == 'ident'){
        return {key:'type',ttp:'smtp',val:cst};
      }else if (cst.tag == 'numbr'){
        return {key:'type',ttp:'nmtp',val:cst};
      }else if (cst.key == 'a.b'){
        return {key:'type',ttp:'smtp',val:{tag:'ident',val:abstype(cst.lhs).val.val + "." + cst.rhs.val,pos:cst.lhs.pos}};
      }else{
        // mkerr('syntax','unexpected token in type annotation',somepos(cst));
        return {key:'type',ttp:'cmtp',val:abstract(cst)};
      }
    }
    if (cst == null) return null;
    if (!cst.key) return cst;
    let ast = {};
    if (cst.key == 'bloc'){
      ast.key = cst.key;
      ast.val = [];
      for (let i = 0; i < cst.val.length; i++){
        ast.val.push(abstract(cst.val[i]));
      }
    }else if (cst.key == '='){
      if (cst.lhs.key == ':'){
        ast.key = 'decl';
        ast.nom = abstract(cst.lhs.lhs);
        ast.ano = abstype(cst.lhs.rhs);
        ast.val = abstract(cst.rhs);
      }else{
        ast.key = cst.key;
        ast.lhs = abstract(cst.lhs);
        ast.rhs = abstract(cst.rhs);
      }
    }else if (cst.key == ':'){
      ast.key = 'decl';
      ast.nom = abstract(cst.lhs);
      ast.ano = abstype(cst.rhs);
      ast.val = null;
    }else if (cst.key == 'loop'){
      ast.key = cst.key;
      ast.chk = abstract(cst.chk);
      ast.ck2 = abstract(cst.ck2);
      ast.ini = abstract(cst.ini);
      ast.stp = abstract(cst.stp);
      ast.bdy = abstract(cst.bdy);
    }else if (cst.key == 'cond'){
      ast.key = cst.key;
      ast.chk = abstract(cst.chk);
      ast.lhs = abstract(cst.lhs);
      ast.rhs = abstract(cst.rhs);
    }else if (cst.key == 'as'){
      ast.key = 'cast';
      ast.lhs = abstract(cst.lhs);
      ast.rhs = abstype(cst.rhs);
    }else if (cst.key == '[]'){
      ast.key = 'tlit';
      ast.val = untree(cst.val).map(abstract);
    }else if (cst.key == '{}'){
      let val = [];
      for (let i = 0; i < cst.val.length; i++){
        let vv = untree(cst.val[i]);
        for (let j = 0; j < vv.length; j++){
          vv[j]=abstract(vv[j]);
        }
        val.push(vv);
      }
      ast.key = 'vlit';
      ast.val = val;

    }else if (cst.key == 'olit'){
      let val = untree(cst.rhs);
      ast.key = cst.key;
      ast.lhs = abstype(cst.lhs);
      ast.rhs = [];
      for (let i = 0; i < val.length; i++){
        if (ast.lhs.con && ['list','arr','vec'].includes(ast.lhs.con.val)){
          val[i] = abstract(val[i]);
          ast.rhs.push(val[i]);
        }else{
          if (val[i].key != ':'){
            mkerr('syntax',`struct initializer requires field:value pairs`,somepos(val[i]));
          }
          val[i].lhs = abstract(val[i].lhs);
          val[i].rhs = abstract(val[i].rhs);
          val[i].key = 'kvpr';
          ast.rhs.push(val[i]);
        }
      }

    }else if (cst.key == '()'){
      ast = abstract(cst.val); 
    }else if (cst.key == 'func'){
      
      ast.key = cst.key;
      ast.nom = cst.nom;
      if (cst.tem){
        ast.tem = untree(cst.tem).map(abstype);
      }
      if (cst.arg.key == 'noop'){
        ast.arg = [];
      }else{
        ast.arg = untree(cst.arg);
      }
      let val = ast.arg;
      for (let i = 0; i < val.length; i++){
        if (val[i].key == ':'){
          val[i].key = 'atpr';
          if (val[i].lhs.tag != 'ident'){
            
            mkerr('syntax','expected identifier in identifier:type pair for function argument declaration',somepos(val[i]));
          }
          val[i].rhs = abstype(val[i].rhs);
        }else if (val[i].key == '='){
          if (val[i].lhs.key == ':'){
            val[i].lhs.key = 'atpr';
            if (val[i].lhs.lhs.tag != 'ident'){
              
              mkerr('syntax','expected identifier in identifier:type pair for function argument declaration',somepos(val[i]));
            }
            val[i].lhs.rhs = abstype(val[i].lhs.rhs);
          }
        }else if (val[i].tag != 'ident'){
          mkerr('syntax','expected identifier or identifier:type pair for function argument declaration',somepos(val[i]));
        }
      }
      ast.bdy = abstract(cst.bdy);
      // ast.bdy.is_func = true;
      ast.ano = abstype(cst.ano);

    }else if (cst.key == 'call'){
      ast.key = cst.key;
      ast.fun = abstract(cst.lhs);
      if (cst.rhs.key == 'noop'){
        ast.arg = [];
      }else{
        ast.arg = untree(cst.rhs);
      }
      for (let i = 0; i < ast.arg.length; i++){
        ast.arg[i] = abstract(ast.arg[i]);
      }
      
      
    }else if (cst.key == 'typd' ){
      ast.key = cst.key;
      ast.lhs = abstype(cst.lhs);
      
      if (cst.rhs.key == '{}'){
        cst.rhs.key = 'bloc';
        ast.rhs = abstract(cst.rhs);
      }else{
        ast.rhs = abstype(cst.rhs);
      }
    }else if (cst.key == 'nmsp'){
      ast.key = cst.key;
      ast.nom = cst.nom;
      ast.val = [];
      for (let i = 0; i < cst.val.length; i++){
        ast.val.push(abstract(cst.val[i]));
      }
    }else if (cst.key == 'subs'){
      ast.key = cst.key;
      ast.con = abstract(cst.lhs);
      // console.log(cst.rhs,untree(cst.rhs))
      ast.idx = untree(cst.rhs).map(abstract);
    }else if (cst.key == 'strit'){
      ast.key = cst.key;
      ast.nom = cst.nom;
      ast.val = [];
      for (let i = 0; i < cst.val.length; i++){
        ast.val.push(abstract(cst.val[i]));
      }
    }else{
      ast.key = cst.key;
      for (let k in cst){
        if (k == 'lhs'){
          ast.lhs = abstract(cst.lhs);
        }else if (k == 'mhs'){
          ast.mhs = abstract(cst.mhs);
        }else if (k == 'rhs'){
          ast.rhs = abstract(cst.rhs);
        }else if (k == 'val'){
          ast.val = abstract(cst.val);
        }else{
          ast[k] = cst[k];
        }
      }
    }
    return ast;
  }

  function eval_scoped(js,context) {
    return function() { with(this) { return eval(js); }; }.call(context);
  }

  function findvar(vars,ast,err='variable'){
    let x = ast.val;
    let recvs = [];
    for (let i = vars.length-1; i >= 0; i--){
      if (vars[i+1] && vars[i+1].__isfun){
        if (!vars[i+1].__captr) vars[i+1].__captr = [];
        recvs.push(vars[i+1].__captr);
      }
      
      if (vars[i][x]){
        let ori = vars[i].__names;//.replace(/\?\.?/g,'');
        for (let j = 0; j < recvs.length; j++){
          if (!recvs[j].map(x=>x.nom).includes(x)){
            recvs[j].push({nom:x,typ:vars[i][x].typ,ori});
            // console.log(recvs[j])
          }
        }
        return Object.assign({ori},vars[i][x]);
      }
    }
    if (err){
      mkerr('reference',`undefined ${err} '${x}'`,somepos(ast));
    }
    return null;
  }

  function findfuncbytype(scopes,typ){
    for (let i = 0; i < scopes.length; i++){
      for (let k in scopes[i]){
        if (scopes[i][k].typ == typ){
          return scopes[i][k];
        }
      }
    }
  }

  function findnmsp(scozoo,cur,nom){
    for (let i = cur.length; i >= 0; i--){
      let ser = cur.slice(0,i).concat([nom]).join('.');
      for (let j = 0; j < scozoo.length; j++){
        if (scozoo[j].__names==ser){
          return {typ:{con:'nmsp',elt:[j]}};
        }
      }
    }
  }

  function compcomp(ast,vars){

    function tojs(ast){
      if (ast.key == '<?='){
        return "Math.min("+tojs(ast.lhs) + "," + tojs(ast.rhs)+")";
      }else if (ast.key == '>?='){
        return "Math.max("+tojs(ast.lhs) + "," + tojs(ast.rhs)+")";
      }else if (ast.is_binop){
        return "("+tojs(ast.lhs) + ast.key + tojs(ast.rhs)+")";
      }else if (ast.is_binop){
        if (ast.key[0] == 'u'){
          return "("+ ast.key.slice(1) + tojs(ast.val)+")";
        }else{
          return "("+tojs(ast.val) + ast.key.slice(0,-1) +")";
        }
      }else if (ast.is_ternop){
        return `(${tojs(ast.lhs)})?(${tojs(ast.mhs)}):(${tojs(ast.rhs)})`
      }else if (ast.tag == 'ident'){
        return compcomp(findvar(vars,ast).val,vars);
      }else if (ast.tag == 'numbr'){
        return ast.val;
      }else{
        mkerr('const','expression not allowed in (presumed) constant expression',somepos(ast));
      }
    }
    try{
      return eval_scoped(tojs(ast));
    }catch(e){
      mkerr('const','error evaluating (presumed) constant expression',somepos(ast));
    }
  }

  function printtype(x){
    if (!x){
      return '???'
    }
    if (typeof x == 'string' || typeof x == 'number'){
      return `${x}`;
    }else{
      return `${x.con}[${x.elt.map(printtype).join(',')}]`;
    }
  }

  function shortid(){
    var id = "";
    for (var i = 0; i < 6; i++){
      id+=String.fromCharCode(~~(Math.random()*26)+0x41);
    }
    return id;
  }

  function maxtype(a,b,die=1){
    if (printtype(a)==printtype(b)){
      return a;
    }
    // console.log(a,b)
    // console.trace();
    
    let ordn = [...nmtyps,'str'];
    let ordv = [...cntyps];

    if (a == 'auto'){
      return b;
    }
    if (b == 'auto'){
      return a;
    }

    if (typeof a == 'string' && typeof b == 'string'){
      if (a == b){
        return a;
      }
      let ia = ordn.indexOf(a);
      let ib = ordn.indexOf(b);
      if (ia == -1 || ib == -1){
        if (die) mkerr('typecheck',`no cast between types '${a}' and '${b}'`,somepos({}));
        throw 'up'
      }
      return ordn[Math.max(ia,ib)];
    }else if (typeof a == 'number' && typeof b == 'number'){
      return Math.max(a,b);

    }else if (a.con && b.con){
      let o = {con:a.con,elt:[]};

      if (a.con != b.con){
        let ia = ordv.indexOf(a.con);
        let ib = ordv.indexOf(b.con);
        if (ia == -1 || ib == -1  ){
          if (die) mkerr('typecheck',`no cast between types '${printtype(a)}' and '${printtype(b)}'`,[0,0]);
          throw 'up'
        }
        if (ia > ib){
          return a;
        }else{
          return b;
        }
      }
      if (a.elt.length != b.elt.length){
        if (a.con != 'vec' || b.con != 'vec' || a.elt.slice(1).reduce((x,y)=>x*y,1) != b.elt.slice(1).reduce((x,y)=>x*y,1)){
          if (die) mkerr('typecheck',`no cast between types '${printtype(a)}' and '${printtype(b)}'`,[0,0]);
          throw 'up'
        } else{
          for (let i = 0; i < a.elt.length; i++){
            o.elt.push(a.elt[i]);
          }
        }
      }else if (a.con != 'vec' || b.con != 'vec'){
        if (die) mkerr('typecheck',`no cast between types '${printtype(a)}' and '${printtype(b)}'`,[0,0]);
        throw 'up'
      }else{
        for (let i = 0; i < a.elt.length; i++){
          o.elt.push(maxtype(a.elt[i],b.elt[i],die));
        }
      }
      return o;
    }else if (a.con == 'vec' && nmtyps.includes(b)){
      return a;
    }else if (b.con == 'vec' && nmtyps.includes(a)){
      return b;
    }else if (a.con == 'func' && typeof b == 'string' && b.startsWith('__func_ovld_')){
      return a;
    }else{
      if (die) mkerr('typecheck',`no cast between types '${printtype(a)}' and '${printtype(b)}'`,[0,0]);
      throw 'up'
    }
    
  }

  function infertypes(ast){
    let scozoo = [];
    let scostk = [];
    let namesp = [];
    let retyps = [];

    function new_scope(){
      return {__alias:{},__types:{},__names:''}
    }
    function get_scope(){
      return scostk.map(i=>scozoo[i]);
    }
    function cur_scope(){
      return scozoo[scostk.at(-1)];
    }
    // function add_scope(x,nom='*'){
    //   // if (nom===undefined) nom = scozoo.length;
    //   namesp.push(nom);
    //   x.__names = namesp.join('.')
    //   scostk.push(scozoo.length);
    //   scozoo.push(x);
    // }
    function add_scope(x,nom){
      if (nom===undefined) nom = '__'+scozoo.length+'';
      namesp.push(nom);
      x.__names = namesp.join('.')
      scostk.push(scozoo.length);
      scozoo.push(x);
    }
    function pop_scope(){
      namesp.pop();
      return scostk.pop();
    }

    function matchatmpl(fa,at,arg,tms,map){
      // console.log(fa,at,arg,tms,map)
      function unmap(fa){
        if (fa.con){
          return {con:fa.con, elt:fa.elt.map(unmap)}
        }else{
          for (let k in map){
            if (fa == k){
              return map[k];
            }
          }
        }
        return fa;
      }
      fa = unmap(fa);

      let fasp = printtype(fa);
      if (printtype(map[fasp]) == printtype(at)){
        return 50;
      }
      let score = 50;

      if (arg){

        console.assert(at==arg.typ);

        if (arg.tag == 'ident' && fa.con == 'func' && typeof arg.typ == 'string' && arg.typ.startsWith('__func_ovld_')){
          if (typeof arg.val == 'string'){
            let ar = fa.elt[0].elt.map(x=>({typ:x}));

            let fd =findvar(get_scope(),arg);
            try{
              matchftmpl(ar,fd.val,map);
              return score;
            }catch(e){

            }
          }else{
            let ok = 0;
            for (let k = 0; k < arg.val.length; k++){
              try{
                maxtypf(fa,arg.val[k].typ,arg,0);
                ok = 1;
                break;
              }catch(e){
              }
            }
            return ok*score;
          }
        }else{
          try{
            if (printtype(fa)==printtype(arg.typ)){
              return score-1;
            }
            maxtypf(fa,arg.typ,arg,0);
            return score-2;
          }catch(e){}
        }
      }
      
      for (let i = 0; i < tms.length; i++){

        if (printtype(at) == printtype(tms[i])){
          return score;
        }
        
        if (fasp == printtype(tms[i])){
          if (map[fasp]){
            try{
              map[fasp] = maxtype(map[fasp],at,1);
              score --;
            }catch(e){
              return 0;
            }
          }else{
            
            map[fasp] = at;
            if (fa.con){
              // TODO: recursive
              for (let k = 0; k < fa.elt.length; k++){
                map[fa.elt[k]] = at.elt[k];
              }
            }
          }
        }
      }

      if (printtype(map[fasp]) == printtype(at)){
        return score;
      }
      

      if (fa.con && (fa.con == at.con) && (fa.elt.length == at.elt.length)){

        let strict = (fa.con == 'map' || fa.con == 'list' || at.con == 'map' || at.con == 'list')
        for (let i = 0; i < fa.elt.length; i++){
          // console.log(fa.elt[i], at.elt[i], null, tms, map)
          let ok = matchatmpl(fa.elt[i], at.elt[i], null, tms, map);
          if (strict && ok < 50){
            ok = 0;
          }
          score = Math.min(ok,score);
          if (!ok) return ok;
        }
        return score;
      }
      // console.log(fa,at,tms)
      return 0;
    }


    function matchftmpl(args,funs,map0={}){
      // console.log(args);
      // console.log("-----")
      let scores = [];
      let fts = [];

      funs.sort((a,b)=>(Number(b.typ.con=='func')-Number(a.typ.con=='func')));
      // console.log(funs.length)
      let nnn = funs.length
      for (let i = 0; i < nnn; i++){

        let fas;
        let map = {};
        let tms = [];
        Object.assign(map,map0);
        let s = 0;
        if (funs[i].typ.con == 'func'){
          fas = funs[i].typ.elt[0].elt;
          s = 100;
        }else{
          fas = funs[i].tty.elt[0].elt;
          tms = funs[i].ipl.tem.map(x=>shrinktype(x,0));
          // tms = funs[i].ipl.tem.map(x=>x.val.val);
          s = 50;
        }
        
        if (fas.length < args.length || funs[i].mac > args.length){
          scores.push([i,0]);
          continue;
        }
        for (let j = 0; j < args.length; j++){
          // console.log(printtype(args[j].typ) , printtype(fas[j]));
          if (printtype(args[j].typ) != printtype(fas[j])){
            // console.log(map)
            // console.log(printtype(args[j].typ) , printtype(fas[j]), tms, map)
            s = Math.min(s,matchatmpl(fas[j], args[j].typ, args[j], tms, map ));
            s = Math.max(s,1);
            
            if (!s){
              break;
            }
          }
        }
        
        if (!s){
          continue;
        }

        let n = funs[i].ctx.length;

        if (funs[i].typ.con == 'func'){

          fts[i] =funs[i].ipl.ano.typ;
          scores.push([i,s,funs[i].typ]);
          
          if (funs[i].did == false){
            
            funs[i].did = true;
            
            scostk.push(...funs[i].ctx);

            scostk.push(funs[i].agt);
            retyps.push(fts[i]);
            if (funs[i].ipl.bdy){
              doinfer(funs[i].ipl.bdy);
            }
            for (let j = 0; j < n+1; j++){
              scostk.pop();
            }
            retyps.pop();
            
          }
        }else{
          scostk.push(...funs[i].ctx);
          
          add_scope(new_scope());

          for (let k in map){ 
            cur_scope().__alias[k] = map[k];
          }
          for (let j = 0; j < funs[i].agn.length; j++){
            cur_scope()[funs[i].agn[j]] = {typ:unalias_recursive(fas[j]),val:null};
            
          }
          cur_scope().__isfun = true;
          // console.dir(funs[i].tty,{depth:100000})
          // console.dir(unalias_recursive(funs[i].tty),{depth:100000})
          let nf = {
            typ: unalias_recursive(funs[i].tty),
            ipl: clone(funs[i].ipl),
            ctx: funs[i].ctx,
            agt: scostk.at(-1),
            did: true,
          }
          funs.push(nf);

          nf.ipl.ano.typ = unalias_recursive(nf.ipl.ano.typ);
          fts[i] = shrinktype(funs[i].ipl.ano);
          
          retyps.push(fts[i]);
          if (nf.ipl.bdy)
            doinfer(nf.ipl.bdy);
          retyps.pop();

          pop_scope();
          
          for (let j = 0; j < n; j++){
            scostk.pop();
          }
          scores.push([i,s-1,nf.typ,()=>funs.splice(funs.indexOf(nf),1)]);
        }

        
        if (s == 100){
          break;
        }
    
      }
      function killlosers(win){
        for (let i = 0; i < scores.length; i++){
          if (i != win && scores[i][3]){
            scores[i][3]();
          }
        }
      }
      // console.log(funs.length)
      scores.sort((a,b)=>(b[1]-a[1]));

      // console.dir(scores,{depth:1000000})

      
      if (scores[0][1] <= 0){
        mkerr('typecheck',`no matching overload found, try ${printtype(funs[scores[0][0]].typ)} etc...`,somepos(args)||somepos(funs));
      }
      // console.log(funs);
      // console.log(funs.map(x=>Number(x.typ.con=='func')))
      // funs.sort((a,b)=>(Number(b.typ.con=='func')-Number(a.typ.con=='func')));
      // console.log(funs);
      // process.exit();
      // console.dir(scores,{depth:5000000});

      if (scores.length < 2){
        killlosers(0);
        return [fts[scores[0][0]],scores[0][2]];
      }
      
      if (scores[0][1] == scores[1][1]){
        // console.dir(funs,{depth:7});
        // console.log(funs)
        // console.dir(scores,{depth:10});
        let t0 = scores[0][2]
        let t1 = scores[1][2]
        mkerr('typecheck',`found tie in funciton matching: ${printtype(t0)} and ${printtype(t1)}`,somepos(args));
      }

      killlosers(0);
      return [fts[scores[0][0]],scores[0][2]];
    }

    
    

    function unalias(x){
      let vars = get_scope();
      for (let i = vars.length-1; i >= 0; i--){
        if (vars[i].__alias[x]){
          return vars[i].__alias[x];
        }
      }
      return x;
    }
    function unalias_recursive(x){
      if (x.con){
        return {con:unalias(x.con),elt:x.elt.map(unalias_recursive)};
      }else{
        return unalias(x);
      }
    }

    function fixtype(x){

      let m;
      if (x.con){
        if (cntyps.includes(x.con) || x.con == 'func' || x.con == 'dict'){
          return x;
        }
        for (let i = scostk.length-1; i>= 0; i--){
          let vars = scozoo[scostk[i]];
          if (m = vars.__types[scostk[i]+'.'+x.con]){
            if (!m.typ.con){
              mkerr('typecheck',`overcompleted type '${printtype(x)}', expected '${printtype(m.typ)}'`,somepos(ast));
            }
            
            let xelts = x.elt;
            let jelts = JSON.stringify(xelts);
            let ok = false;
            
            for (let j = 0; j < m.cpy.length; j++){
              if (JSON.stringify(m.cpy[j].elt) == jelts){
                ok = true;
                break;
              }
            }
            x.con = scostk[i]+'.'+x.con;
            
            if (ok){
              return x;
            }
            let mm = clone(m.tem);
            m.cpy.push({elt:xelts,val:mm,sig:scozoo.length});

            let melts = m.typ.elt;

            let map = {};
            for (let j = 0; j < melts.length; j++){
              map[melts[j]] = xelts[j];
            }

            add_scope(new_scope());

            for (let k in map){
              cur_scope().__alias[k] = map[k];
            }
            cur_scope().this = {typ:unalias_recursive(m.typ),val:'__this'}
          
            // console.log(cur_scope(),map)
            for (let j = 0; j < mm.val.length; j++){
              // console.log(',',mm.val[j])
              doinfer(mm.val[j]);
            }
            mm.sco = scostk.slice();

            pop_scope();

            
            return x;
          }
        }
      }else{
        if (smtyps.includes(x)){
          return x;
        }
        if (typeof x == 'number'){
          return x;
        }
        let xa = x.split(".").slice(0,-1).join('.');
        let xb = x.split(".").at(-1);

        for (let i = scostk.length-1; i>= 0; i--){
          let idx = scostk[i];

          if (xa.length){
            for (let j = 0; j < scozoo.length; j++){
              if (scozoo[j].__names == scozoo[idx].__names+"."+xa){
                idx = j;
                break;
              }
            }
          }
          if (m = scozoo[idx].__alias[idx+'.'+xb]){
            return fixtype(scozoo[idx].__alias[idx+'.'+xb])
          }
          if (m = scozoo[idx].__types[idx+'.'+xb]){
            xb = idx+'.'+xb;
            if (m.typ.con){
              mkerr('typecheck',`incomplete type '${printtype(x)}', expected '${printtype(m.typ)}'`,somepos(ast));
            }
            return xb;
          }
        }
      }
      mkerr('typecheck',`unrecognized type '${printtype(x)}'`,somepos(ast));
    }


    function tryfixtype(x){
      let m;
      if (x.con){
     
        x = clone(x);
        for (let i = 0; i < x.elt.length; i++){
          if (typeof x.elt[i] != 'number'){
            x.elt[i] = tryfixtype(x.elt[i]);
          }
        }
        return x;
      }else{
        if (smtyps.includes(x)){
          return x;
        }
        let xa = x.split(".").slice(0,-1).join('.');
        let xb = x.split(".").at(-1);
        for (let i = scostk.length-1; i>= 0; i--){
          let idx = scostk[i];
          if (xa.length){
            for (let j = 0; j < scozoo.length; j++){
              if (scozoo[j].__names == scozoo[idx].__names+"."+xa){
                idx = j;
                break;
              }
            }
          }
          if (m = scozoo[idx].__types[idx+'.'+xb]){
            xb = idx+'.'+xb;
            if (m.typ.con){
              return x;
            }
            return xb;
          }
        }
      }
      return x;
    }

    function shrinktype(ast,strict=1){
      
      if (ast.ttp == 'smtp'){
        let o = unalias(ast.val.val);
        if (strict) return fixtype(o);
        return o;
      }else if (ast.ttp == 'cntp'){
        let o = {con:unalias(ast.con.val),elt:ast.elt.map(x=>shrinktype(x,strict))};
        // if (o.con == 'vec'){
        //   o.elt = [o.elt[0],o.elt.slice(1).reduce((a,b)=>a*b,1)];
        // }
        if (strict) return fixtype(o);
        return o;
      }else if (ast.ttp == 'nmtp'){
        return Number(ast.val.val);
      }else if (ast.ttp == 'cmtp'){
        return compcomp(ast.val,get_scope());
        
      }
      mkerr('typecheck','unrecognized type',somepos(ast));
    }

    function fieldtype(typ,nom,abort=1){
      // let vars = get_scope();
      let vars;

      if (typ.con){
        vars = get_scope();
        let key = typ.con;
        let idx = -1;
        if (key.includes(".")){
          idx = Number(key.split(".")[0]);
        }
        for (let i = vars.length-1; i>=0; i--){
          if (vars[i].__types[key] && (idx==-1 || idx == i)){
            for (let j = 0; j < vars[i].__types[key].cpy.length; j++){
              
              if (typeeq(vars[i].__types[key].cpy[j].elt, typ.elt)){
                return findvar([scozoo[vars[i].__types[key].cpy[j].sig]],nom,'field').typ;
              }
            }
          }
        }
      }else{
        vars = scozoo;
        let key = typ;
        for (let i = vars.length-1; i>=0; i--){
          if (vars[i].__types[key]){
            return findvar([scozoo[vars[i].__types[key].sig]],nom,'field').typ;
          }
        }
      }
      if (!abort){
        return null
      }
      mkerr('typecheck',`field not found: '${nom.val}' of ${printtype(typ)}`,somepos(ast));
    }

    
    function maxtypd(a,b,c){
      let vars = get_scope();
      let key = typ;
      for (let i = vars.length-1; i>=0; i--){
        if (vars[i].__types[key]){
          return findvar([scozoo[vars[i].__types[key].sig]],nom,'field').typ;
        }
      }
    }

    function maxtypf(a,b,c,die=1){
      if (c.tag == 'ident' && typeof c.val == 'string' && a.con == 'func' && typeof b == 'string' && b.startsWith('__func_ovld_')){
        
        let arg = a.elt[0].elt.map(x=>({typ:x}));

        let fd =findvar(get_scope(),c);
        matchftmpl(arg,fd.val);
        return a;
      }else{
        return maxtype(a,b,die);
      }
    }

    function realizefunc(ast){
      function doit(fi){
        if (fi.did) return;
        let ft =fi.ipl.ano.typ;
        fi.did = true;
        let n = fi.ctx.length;
        scostk.push(...fi.ctx);
        
        scostk.push(fi.agt);
        retyps.push(ft)
        if (fi.ipl.bdy)
          doinfer(fi.ipl.bdy);
        for (let j = 0; j < n+1; j++){
          scostk.pop();
        }
        retyps.pop()
      }
      if (ast.is_fun_alias){
                
        let fi = ast.val[0]
        
        doit(fi);
      }else if (ast.key == 'func'){
        for (let i = scostk.length-1; i>= 0; i--){
          let vars = scozoo[scostk[i]];
          for (let k in vars){
            if (vars[k].typ == ast.mty){
              for (let j = 0; j < vars[k].val.length; j++){
                doit(vars[k].val[j]);
              }
            }
          }
        }
      }
    }

    function doinfer(ast){
      
      if (ast.tag){
        if (ast.tag == 'numbr'){
          ast.key = 'term';
          if (ast.val.toLowerCase().includes('e') || ast.val.toLowerCase().includes('.')){
            ast.typ = 'f32';
          }else{
            ast.typ = 'i32';
          }
        }else if (ast.tag == 'strlt'){
          ast.key = 'term';
          ast.typ = 'str';
          // ast.val = ast.val;
          
        }else if (ast.tag == 'ident'){
          let fv = findvar(get_scope(),ast,null);
          if (fv){
            // if (fv.typ.startsWith && fv.typ.startsWith('__func_ovld_')){
            //   ast.rty = matchftmpl(ast.arg,fv.val);
            // }
            ast.typ = fv.typ;
            ast.ori = fv.ori;

            if (typeof fv.typ == 'string' && fv.typ.startsWith('__func_ovld_')){
              ast.val = fv.val;
              
              // ast.val = fv.val.map(x=>({typ:x.typ,ipl:{ano:{typ:x.ipl.ano.typ}},ctx:x.ctx}))
              ast.is_fun_alias = 1;
            }
          }else{
            fv = findnmsp(scozoo,namesp,ast.val);
            if (!fv){
              mkerr('typecheck',`undefined identifier '${ast.val}'`,somepos(ast));
            }
            ast.typ = fv.typ
          }
          ast.key = 'term';
        }else{
          
          mkerr('typecheck','unexpected token',somepos(ast));
        }
      }else{
        if (ast.key == 'type'){
          ast.typ = shrinktype(ast);

        }else if (ast.key == 'bloc'){
          add_scope(new_scope());
          
          for (let i = 0; i < ast.val.length; i++){
            doinfer(ast.val[i]);
          }
          ast.typ = 'void';
          ast.sco = scostk.slice();
          pop_scope();
        }else if (ast.key == 'nmsp'){
          add_scope(new_scope(),ast.nom.val);
          for (let i = 0; i < ast.val.length; i++){
            doinfer(ast.val[i]);
          }
          ast.typ = 'void';
          ast.sco = scostk.slice();
          pop_scope();

        }else if (ast.key == 'decl'){
          // console.log("_______________________________________",namesp)
          if (ast.val){
            doinfer(ast.val);
            // realizefunc(ast.val);
          }

          if (ast.nom.key == 'tlit'){
            if (ast.ano){
              doinfer(ast.ano);
              if (ast.ano.typ.con != 'tup'){
                mkerr('typecheck',`expected annotation to be tuple, got ${printtype(ast.ano.typ)}`,somepos(ast.ano));
              }
            }
            if (ast.val){
              if (ast.nom.val.length != ast.val.typ.elt.length){
                mkerr('typecheck',`unpack tuple size mismatch, ${ast.nom.val.length} != ${ast.val.typ.elt.length}`,somepos(ast));
              }
            }
            if (ast.ano && ast.val){
              if (ast.ano.typ.elt.length != ast.val.typ.elt.length){
                mkerr('typecheck',`unpack tuple size mismatch, ${ast.ano.typ.elt.length} != ${ast.val.typ.elt.length}`,somepos(ast));
              }
              for (let i = 0; i < ast.val.typ.length; i++){
                if (ast.val.key == 'tlit'){
                  maxtypf(ast.ano.typ.elt[i],ast.val.val[i].typ,ast.val.val[i]);
                }else{
                  maxtype(ast.ano.typ,ast.val.typ.elt[i]);
                }
              }
            }
            for (let i = 0; i < ast.nom.val.length; i++){
              
              if (ast.ano){
                cur_scope()[ast.nom.val[i].val] = {typ:ast.ano.typ.elt[i],val:null}
              }else{
                cur_scope()[ast.nom.val[i].val] = {typ:ast.val.typ.elt[i],val:null}
              }
              if (ast.val && ast.val.key == 'tlit'){
                cur_scope()[ast.nom.val[i].val].val = ast.val.val[i];
              }
            }
            // console.log(cur_scope())

          }else if (ast.nom.key == 'vlit'){
            
            if (ast.ano){
              doinfer(ast.ano);
              if (ast.ano.typ.con != 'vec'){
                mkerr('typecheck',`expected annotation to be vector, got ${printtype(ast.ano.typ)}`,somepos(ast.ano));
              }
            }
            if (ast.val){
              let l0 = ast.nom.val.flat().length;
              let l1 = ast.val.typ.elt.slice(1).reduce((a,b)=>a*b,1);
              if (l0 != l1  ){
                mkerr('typecheck',`unpack vector dimension mismatch, ${l0} != ${l1}`,somepos(ast));
              }
            }
            if (ast.ano && ast.val){
              let l0 = ast.ano.typ.elt.slice(1).join(',');
              let l1 = ast.val.typ.elt.slice(1).join(',');
              if (l0 != l1){
                mkerr('typecheck',`unpack vector dimension mismatch, ${l0} != ${l1}`,somepos(ast));
              }
              for (let i = 0; i < ast.val.typ.elt.length; i++){
                maxtype(ast.ano.typ.elt[i], ast.val.typ.elt[i]);
              }
            }

            for (let i = 0; i < ast.nom.val.length; i++){
              for (let j = 0; j < ast.nom.val[i].length; j++){
                if (ast.ano){
                  cur_scope()[ast.nom.val[i][j].val] = {typ:ast.ano.typ.elt[0],val:null}
                }else{
                  cur_scope()[ast.nom.val[i][j].val] = {typ:ast.val.typ.elt[0],val:null}
                }
                if (ast.val && ast.val.key == 'vlit'){
                  cur_scope()[ast.nom.val[i][j].val].val = ast.val.val[i][j];
                }
              }
            }

            
          }else{
            ast.nom.key = 'term';
            if (ast.ano){
              doinfer(ast.ano);
              cur_scope()[ast.nom.val] = {typ:ast.ano.typ,val:ast.val};
            }else{
              cur_scope()[ast.nom.val] = {typ:ast.val.typ,val:ast.val};
            }
            if (ast.ano && ast.val){
              maxtypf(ast.ano.typ,ast.val.typ,ast.val);
            }
          }
          
          ast.typ = 'void';
          // console.log("x_______________________________________",namesp)
          ast.ori = namesp.slice();

        }else if (ast.key == '='){
          
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          // realizefunc(ast.rhs);
          let typ = maxtypf(ast.lhs.typ,ast.rhs.typ,ast.rhs);
          ast.typ = ast.lhs.typ;

        }else if (ast.key == 'cast'){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          ast.typ = ast.rhs.typ;
          
        }else if (ast.key == 'tlit'){
          let typs = [];
          for (let i = 0; i < ast.val.length; i++){
            doinfer(ast.val[i]);
            typs.push(ast.val[i].typ);
          }
          ast.typ = {con:'tup',elt:typs};
        }else if (ast.key == 'vlit'){
          let w=1,h=1,t='auto';
          h = ast.val.length;
          for (let i = 0; i < ast.val.length; i++){
            w = Math.max(w,ast.val[i].length);
            for (let j = 0; j < ast.val[i].length; j++){
              doinfer(ast.val[i][j]);
              t = maxtype(t,ast.val[i][j].typ);
            }
          }
          if (h > 1){
            ast.typ = {con:'vec',elt:[t,h,w]};
          }else{
            ast.typ = {con:'vec',elt:[t,w]};
          }
          
        }else if (ast.key == 'olit'){

          let lht = shrinktype(ast.lhs);

          if (lht.con && lht.con == 'arr'){
            ast.key = 'alit';
            for (let i = 0; i < ast.rhs.length; i++){
              doinfer(ast.rhs[i]);
              maxtype(ast.rhs[i].typ, lht.elt[0]);
            }
          }else if (lht.con && lht.con == 'list'){
            ast.key = 'llit';
            for (let i = 0; i < ast.rhs.length; i++){
              doinfer(ast.rhs[i]);
              maxtype(ast.rhs[i].typ, lht.elt[0]);
            }
          }else if (lht.con && lht.con == 'dict'){
            ast.key = 'mlit';
            for (let i = 0; i < ast.rhs.length; i++){
              doinfer(ast.rhs[i].lhs);
              doinfer(ast.rhs[i].rhs);
              maxtype(ast.rhs[i].lhs.typ, lht.elt[0]);
              maxtype(ast.rhs[i].rhs.typ, lht.elt[1]);
            }
          }else if (lht.con && lht.con == 'vec'){
            
            for (let i = 0; i < ast.rhs.length; i++){
              doinfer(ast.rhs[i]);
              maxtype(ast.rhs[i].typ, lht.elt[0]);
            }
            ast.key = 'vlit';
            ast.typ = ast.lhs;
            ast.val = ast.rhs;
          }else{
            for (let i = 0; i < ast.rhs.length; i++){
              let fdnom = ast.rhs[i].lhs;
              doinfer(ast.rhs[i].rhs);
              let typ = fieldtype(lht,fdnom);
              // ast.rhs[i].rhs.rty = maxtype(typ,ast.rhs[i].rhs.typ)
              ast.rhs[i].rhs.rty = typ;
            }
          }
          ast.typ = lht;

        }else if (ast.key == 'func'){
        
          function add_func(f){
            if (!cur_scope()[ast.nom.val]){
              cur_scope()[ast.nom.val] = {typ:"__func_ovld_"+ast.nom.val+'_'+shortid(),val:[]};
            }
            f.ipl.mty = cur_scope()[ast.nom.val].typ;
            cur_scope()[ast.nom.val].val.push(f);
          }
          let o = {con:'func',elt:[{con:'tup',elt:[]}]};
          let avar = new_scope();
          let argnames = [];
          let minargc = -1;
          
          for (let i = 0; i < ast.arg.length; i++){
            
            ast.arg[i].ori = namesp.slice();

            if (ast.arg[i].key == 'atpr'){
              
              if (minargc != -1){
                mkerr('typecheck',`arguments without default cannot come after those with`,somepos(ast.arg[i]));
              }
              ast.arg[i].rhs.typ = shrinktype(ast.arg[i].rhs,0);
              ast.arg[i].typ = ast.arg[i].rhs.typ;
              ast.arg[i].rhs.typ = ast.arg[i].typ = tryfixtype(ast.arg[i].typ);

              argnames.push(ast.arg[i].lhs.val);
              avar[ast.arg[i].lhs.val] = {typ:ast.arg[i].typ,val:null};
              
            }else if (ast.arg[i].tag == 'ident'){
              // ast.arg[i].typ = 'auto';
              // avar[ast.arg[i].val] = {typ:ast.arg[i].typ,val:null};
              mkerr('typecheck',`argument type (or initial value) required`,somepos(ast.arg[i]));
            }else if (ast.arg[i].key == '='){
              doinfer(ast.arg[i].rhs);
              if (ast.arg[i].lhs.key == 'atpr'){
                doinfer(ast.arg[i].lhs.rhs);
                ast.arg[i].lhs.typ = ast.arg[i].lhs.rhs.typ;
                ast.arg[i].typ = ast.arg[i].lhs.typ;
                argnames.push(ast.arg[i].lhs.lhs.val);
                avar[ast.arg[i].lhs.lhs.val] = {typ:ast.arg[i].typ,val:null};
              }else{
                ast.arg[i].typ = ast.arg[i].rhs.typ;
                argnames.push(ast.arg[i].lhs.val);
                avar[ast.arg[i].lhs.val] = {typ:ast.arg[i].typ,val:null};
              }
              minargc = i;
              
            }
            
            o.elt[0].elt.push(ast.arg[i].typ);
          }
          // for (let i = 0; i < o.elt.length; i++){
          //   for (let j = 0; j < o.elt[i].elt.length; j++){

          //     o.elt[i].elt[j] = tryfixtype(o.elt[i].elt[j]);
          //   }
          // }
          // console.log(o.elt)
          if (ast.ano){
            ast.ano.typ = shrinktype(ast.ano,0);
            ast.ano.typ = tryfixtype(ast.ano.typ);
            o.elt.push(ast.ano.typ);
          }else{
            ast.ano = {typ:'void'}
            o.elt.push('void');
            // mkerr('typecheck',`function return type required`,somepos(ast));
          }
          if (minargc == -1) minargc = argnames.length;
          if (ast.tem && ast.tem.length){
            ast.typ = "__func_tmpl_"+shortid();
            add_func({
              typ:ast.typ,
              ipl:ast,
              ctx:scostk.slice(),
              agn:argnames,
              tty:o,
              mac:minargc
            });
    
          }else{
            ast.typ = o;
            avar.__isfun = true;
            let agt = scozoo.length;
            scozoo.push(avar);
            if (ast.nom){
              add_func({
                typ:o,
                ipl:ast,
                ctx:scostk.slice(),
                agt,
                did:false,
                mac:minargc,
              });
            }
            // add_scope(avar);
            // doinfer(ast.bdy);
            // pop_scope();
          }
        }else if (ast.key == 'call'){
          let skipfirst = 0;
          if (ast.fun.key == 'a.b'){
            doinfer(ast.fun.lhs);
            
            let typ;
            
            if (hctyps.includes(ast.fun.lhs.typ.con) || hctyps.includes(ast.fun.lhs.typ)){
              
              typ = fieldtype(ast.fun.lhs.typ,ast.fun.rhs,0);
              
              if (typ == null){
                let v0 = ast.fun.lhs;
                ast.fun.lhs = {
                  tag:'ident',
                  val:ast.fun.lhs.typ.con??ast.fun.lhs.typ,
                  pos:ast.fun.lhs.pos,
                }
                ast.arg.unshift(v0);
                doinfer(ast.fun);
                skipfirst = 1;
              }
              ast.typ = typ;
              // console.dir(ast);
              // process.exit();
            }else{
              // typ = fieldtype(ast.fun.lhs.typ,ast.fun.rhs);
              doinfer(ast.fun);
              // console.log(ast.fun.lhs.typ.con)
            }
            
          }else{
            doinfer(ast.fun);
          }
          // console.log(',,,,,,,,,,,,,,,,,',ast.fun)

          let fd = findvar(get_scope(),ast.fun,false);
          if (!fd){
            for (let i = 0; i < scozoo.length; i++){
              for (let k in scozoo[i]){
                if (scozoo[i][k].typ == ast.fun.typ){
                  fd = scozoo[i][k];
                }
              }
            }
          }
          if (!fd){
            mkerr('typecheck',`cannot find function ${printtype(ast.fun.typ)}`,somepos(ast.fun));
          }

          let arg2 = [];
          for (let i = 0; i < ast.arg.length; i++){
            if (ast.arg[i].key == '...u'){
              doinfer(ast.arg[i].val);
              if (ast.arg[i].val.typ.con == 'tup'){
                for (let j = 0; j < ast.arg[i].val.typ.elt.length; j++){
                  arg2.push({typ:ast.arg[i].val.typ.elt[i]});
                }
              }else if (ast.arg[i].val.typ.con == 'vec'){
                let n = ast.arg[i].val.typ.elt.slice(1).reduce((a,b)=>a*b,1);
                for (let j = 0; j < n; j++){
                  arg2.push({typ:ast.arg[i].val.typ.elt[0]});
                }
              }
            }else{
              if (i || !skipfirst){
                doinfer(ast.arg[i]);
              }
              realizefunc(ast.arg[i]);


              arg2.push(ast.arg[i]);
            }
          }
          // console.log(arg2)
          let ret;

          if (ast.fun.typ.con == 'func'){
            ast.fun.rty = ast.fun.typ;
            ret = ast.fun.typ.elt[1];
          }else if (typeof ast.fun.typ == 'string' && ast.fun.typ.startsWith('__func_ovld_')){
            // console.dir(ast,{depth:100000});
            ;[ret,ast.fun.rty] = matchftmpl(arg2,fd.val);
            
          }else{
            
            mkerr('typecheck',`calling a non-function (${printtype(ast.fun.typ)})`,somepos(ast));
          }
          
          
          ast.typ = ret;
          // if (!ast.fun.rty){
          //   mkerr('typecheck',`calling a non-function (${printtype(ast.fun.typ)})`,somepos(ast));
          // }
          // ast.typ = ast.fun.rty;


        }else if (ast.key == 'retn'){
          
          doinfer(ast.val);
          realizefunc(ast.val);

          if (typeof ast.val.typ == 'string' && ast.val.typ.startsWith('__func_ovld_')){
            if (ast.val.tag != 'ident'){
              mkerr('typecheck',`non-inferrable function type`,somepos(ast));
            }
            maxtypf(retyps.at(-1),ast.val.typ,ast.val);
          }else{
            maxtype(retyps.at(-1),ast.val.typ);
          }
          // ast.typ = 'void';
          ast.typ = retyps.at(-1);
        }else if (ast.key == 'noop'){
          ast.typ = 'void';
          
        }else if (ast.key == 'loop'){
          add_scope(new_scope());
          if (ast.ini) doinfer(ast.ini);
          if (ast.chk) doinfer(ast.chk);
          if (ast.ck2) doinfer(ast.ck2);
          if (ast.stp) doinfer(ast.stp);
          if (ast.bdy) doinfer(ast.bdy);
          pop_scope();

        }else if (ast.key == 'cond'){
          add_scope(new_scope());
          if (ast.chk) doinfer(ast.chk);
          if (ast.lhs) doinfer(ast.lhs);
          if (ast.rhs) doinfer(ast.rhs);
          pop_scope();
        }else if (ast.key == 'subs'){
          doinfer(ast.con);
          ast.idx.map(doinfer);
          
          if (ast.con.typ.con != 'dict'){
            for (let i = 0; i < ast.idx.length; i++){
              if (!intyps.includes(ast.idx[i].typ)){
                mkerr('typecheck',`subscript operator expects integral index for given type (got ${printtype(ast.idx[i].typ)})`,somepos(ast));
              }
            }
          }
          if (ast.con.typ == 'str'){
            
            if (ast.idx.length != 1){
              mkerr('typecheck',`subscript operator expects single index for string (got ${ast.idx.length})`,somepos(ast));
            }
            ast.idx = ast.idx[0];
            ast.typ = 'u8';

          }else if (ast.con.typ.con == 'func'){
            mkerr('typecheck',`subscript operator cannot be used on ${printtype(ast.lhs.typ)}`,somepos(ast));
          }else if (ast.con.typ.con == 'tup'){
            if (ast.idx.length != 1){
              mkerr('typecheck',`subscript operator expects single index for tuple (got ${ast.idx.length})`,somepos(ast));
            }
            let idx = compcomp(ast.idx[0],get_scope());
            // ast.idx = ast.idx[0];
            ast.idx = {
              tag: 'numbr', val: idx, pos:ast.idx[0].pos, key: 'term', typ: 'i32'
            };
            ast.typ = ast.con.typ.elt[idx];
          }else if (ast.con.typ.con == 'dict'){
            if (ast.idx.length != 1){
              mkerr('typecheck',`subscript operator expects single index for map (got ${ast.idx.length})`,somepos(ast));
            }
            ast.typ = ast.con.typ.elt[1];
            ast.idx = ast.idx[0];
            
            maxtype(ast.con.typ.elt[0],ast.idx.typ);
          }else if (ast.con.typ.con){
            ast.typ = ast.con.typ.elt[0];
            if (ast.idx.length >= Math.max(2,ast.con.typ.elt.length)){
              mkerr('typecheck',`too many indices for ${printtype(ast.con.typ)} (got ${ast.idx.length})`,somepos(ast));
            }
            if (ast.idx.length > 1){
              function trfidx(dims,idcs){
                if (dims.length == 1){
                  return idcs[0];
                }else{
                  return {key:'+',lhs:idcs[0],rhs:{key:'*',lhs:{tag:'numbr',val:dims[0],pos:idcs[0].pos,key:'term',typ:'i32'},rhs:trfidx(dims.slice(1),idcs.slice(1)),typ:'i32'},typ:'i32'};
                }
              }
              ast.idx = trfidx(ast.con.typ.elt.slice(1).reverse(),ast.idx.slice().reverse());
              // console.dir(ast.idx,{depth:10000});
              // process.exit();
            }else{
              ast.idx = ast.idx[0];
            }
          }else{
            mkerr('typecheck',`subscript operator cannot be used on ${printtype(ast.lhs.typ)}`,somepos(ast));
          }
        }else if (ast.key == 'typd' ){
          
          if (ast.rhs.key == 'type'){
            if (ast.lhs.ttp != 'smtp'){
              mkerr('typecheck',`type alias expected identifier`,somepos(ast));
            }
   
            cur_scope().__alias[scostk.at(-1)+"."+ast.lhs.val.val] = shrinktype(ast.rhs);
            
          }else{
            // console.log(ast.lhs)
            
            ast.lhs.typ = shrinktype(ast.lhs,0);

            
            
            if (ast.lhs.typ.con){
              ast.lhs.typ.con = scostk.at(-1)+'.'+ast.lhs.typ.con;
              cur_scope().__types[ast.lhs.typ.con] = {
                typ:ast.lhs.typ,
                tem:ast.rhs,
                cpy:[]
              };
            }else{
              ast.lhs.typ = scostk.at(-1)+'.'+ast.lhs.typ;

              let qq =  {typ:ast.lhs.typ};
              cur_scope().__types[ast.lhs.typ] = qq;
              add_scope(new_scope());
              qq.sig = scostk.at(-1);
              cur_scope().this = {typ:ast.lhs.typ,val:'__this'}
              for (let i = 0; i < ast.rhs.val.length; i++){
                // if (ast.rhs.val[i].key == 'func'){
                //   ast.rhs.val[i].arg.push({
                //     key: 'atpr',
                //     lhs: { tag: 'ident', val: 'this', pos:[0,0]},
                //     rhs: { key: 'type', ttp: 'smtp', val: {tag : 'ident', val: ast.lhs.typ, pos: [0,0]}},
                //     is_binop: true,
                //   })
                // }

                doinfer(ast.rhs.val[i]);
                // ast.rhs.val[i].mbr = true;
                ast.rhs.val[i].mbr = ast.lhs.typ;
                
                // realizefunc(ast.rhs.val[i]);
              }
              ast.sco = scostk.slice();
              let sig = pop_scope();
              ast.sig = sig;

              Object.assign(cur_scope().__types[ast.lhs.typ] , {typ:ast.lhs.typ,sig,cpy:[ast]});
              // console.dir(scozoo,{depth:10000000})
              
            }
          }
        }else if (ast.key == 'a.b'){
          
          doinfer(ast.lhs);

          let typ;
          if (ast.lhs.typ.con == 'vec'){
            if (/^[xyzwrgba]+$/.test(ast.rhs.val)){
              ast.rhs.val = ast.rhs.val.replace(/r/g,'x').replace(/g/g,'y').replace(/b/g,'z').replace(/a/g,'w')
              if (ast.rhs.val.length == 1){
                typ= ast.lhs.typ.elt[0];
              }else{
                typ= {con:'vec',elt:[ast.lhs.typ.elt[0],ast.rhs.val.length]}
              }
              ast.key = 'swiz';
            }else if (ast.rhs.val == 'shape'){
              let elts = ast.lhs.typ.elt.slice(1);
              // if (elts.length == 1){
              //   ast.key = 'term';
              //   ast.tag = 'numbr';
              //   ast.val = elts[0].toString();
              //   ast.typ = 'i32';
              //   typ = ast.typ;
              // }else{
                ast.key = 'tlit';
                ast.val = elts.map(x=>({key:'term',tag:'numbr',val:x.toString(),pos:somepos(ast)}));
                doinfer(ast);
                typ = ast.typ;
              // }
              
            }else{
              mkerr('typecheck',`${printtype(ast.lhs.typ.con)} does not expect field '${ast.rhs.val}'`,somepos(ast));
            }
          }else if (ast.lhs.typ.con == 'arr' && ast.rhs.val == 'shape'){
            let elts = ast.lhs.typ.elt.slice(1);
            ast.key = 'tlit';
            ast.val = elts.map(x=>({key:'term',tag:'numbr',val:x.toString(),pos:somepos(ast)}));
            doinfer(ast);
            typ = ast.typ;
          }else if (ast.lhs.typ.con == 'tup' && ast.rhs.val == 'shape'){
            let elts = ast.lhs.typ.elt;
            ast.key = 'term';
            ast.tag = 'numbr';
            ast.val = elts.length.toString();
            ast.typ = 'i32';
            typ = ast.typ;
          }else if (ast.lhs.typ.con == 'nmsp'){
            let sco = ast.lhs.typ.elt[0];
            // console.log(scozoo[sco]);
            scostk.push(sco);
            doinfer(ast.rhs);
            scostk.pop();
            typ = ast.rhs.typ;
          }else{
            typ = fieldtype(ast.lhs.typ,ast.rhs);
          }
          ast.typ = typ;
        }else if (ast.key == ','){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          ast.typ = ast.rhs.typ;
        }else if (ast.key == '+' || ast.key == '+='){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          let allow = [...matyps,'str'];
          let typ = maxtype(ast.lhs.typ,ast.rhs.typ);
          if (!allow.includes(typ) && !allow.includes(typ.con)){

            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          if (ast.key == '+='){
            ast.typ = ast.lhs.typ;
          }else{
            ast.typ = typ;
          }
        }else if (['*','/','-','%','**'].includes(ast.key)){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          let allow = matyps;
          let typ = maxtype(ast.lhs.typ,ast.rhs.typ);
          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          ast.typ = typ;
        }else if (['*=','/=','-=','%=','**=','<?=','>?='].includes(ast.key)){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          let allow = matyps;
          let typ = maxtype(ast.lhs.typ,ast.rhs.typ);

          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          ast.typ = ast.lhs.typ;
        }else if (['==','!='].includes(ast.key)){

          doinfer(ast.lhs);
          doinfer(ast.rhs);
          // let allow = [...matyps,'str'];
          let typ = maxtype(ast.lhs.typ,ast.rhs.typ);
          // if (!allow.includes(typ) && !allow.includes(typ.con)){
          //   mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          // }
          ast.typ = 'i32';

        }else if (['>','<','>=','<='].includes(ast.key)){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          let allow = nmtyps;
          let typ = maxtype(ast.lhs.typ,ast.rhs.typ);
          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          ast.typ = 'i32';

        }else if (['&&','||'].includes(ast.key)){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          let allow = intyps;
          let typ = maxtype(ast.lhs.typ,ast.rhs.typ);
          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          ast.typ = 'i32'

        }else if (['>>','<<','<<=','>>='].includes(ast.key)){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          let allow = intyps;
          let typ = maxtype(ast.lhs.typ,ast.rhs.typ);
          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          ast.typ = ast.lhs.typ;

        }else if (['&','|','^','&=','|=','^='].includes(ast.key)){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          let allow = intyps;
          let typ = maxtype(ast.lhs.typ,ast.rhs.typ);
          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          if (ast.key == '&' || ast.key == '|' || ast.key == '^'){
            ast.typ = typ;
          }else{
            ast.typ = ast.lhs.typ;
          }
        }else if (['+u','-u'].includes(ast.key)){
          doinfer(ast.val);
          let typ = ast.val.typ;
          let allow = matyps;
          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          ast.typ = typ;
        }else if (['++u','--u','u++','u--','~u'].includes(ast.key)){ 
          doinfer(ast.val);
          let typ = ast.val.typ;
          let allow = intyps;
          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          ast.typ = typ;
        }else if (['!u'].includes(ast.key)){
          doinfer(ast.val);
          let typ = ast.val.typ;
          let allow = intyps;
          if (!allow.includes(typ) && !allow.includes(typ.con)){
            mkerr('typecheck',`'${ast.key}' operator cannot be used on ${printtype(typ)}`,somepos(ast));
          }
          ast.typ = 'i32';
        }else if (ast.key == '?:'){
          doinfer(ast.lhs);
          doinfer(ast.mhs);
          doinfer(ast.rhs);
          let typ = maxtype(ast.mhs.typ,ast.rhs.typ);
          ast.typ = typ;
        }else if (['@*','@*='].includes(ast.key)){
          doinfer(ast.lhs);
          doinfer(ast.rhs);
          let typ;
          if (nmtyps.includes(ast.lhs.typ)){
            if (nmtyps.includes(ast.rhs.typ)){
              typ = maxtype(ast.lhs.typ,ast.rhs.typ);
            }else{
              typ = maxtype(ast.lhs.typ.elt[0],ast.rhs.typ);
            }
          }else if (nmtyps.includes(ast.rhs.typ)){
            typ = maxtype(ast.rhs.typ.elt[0],ast.lhs.typ);
            
          }else{
            let dim0 = ast.lhs.typ.elt.slice(1);
            let dim1 = ast.rhs.typ.elt.slice(1);
            let lisv = false, risv = false;
            let o = {con:'vec',elt:[maxtype(ast.lhs.typ.elt[0],ast.rhs.typ.elt[0])]};
            if (dim0.length > 2 || dim1.length > 2){
              mkerr('typecheck',`matrix multiply expects 1D or 2D`,somepos(ast));
            }
            if (dim0.length == 1){
              dim0.push(1);
              lisv = true;
            }
            if (dim1.length == 1){
              dim1.push(1);
              risv = true;
            }
            ast.lhs.v2m = {con:'vec',elt:[ast.lhs.typ.elt[0],...dim0]}
            ast.rhs.v2m = {con:'vec',elt:[ast.rhs.typ.elt[0],...dim1]}

            let nr = dim0[0];
            let nc = dim1[1];
            if (dim0[1] != dim1[0]){
              mkerr('typecheck',`matrix multiply dimension mismatch (${dim0[1]} != ${dim1[0]})`,somepos(ast));
            }
            // if (lisv){
            //   o.elt.push(nc);
            // }else if (risv){
            //   o.elt.push(nr);
            // }else{
              o.elt.push(nr,nc);
            // }
            typ = o;
          }
          if (ast.key == '@*='){
            ast.typ = ast.lhs.typ;
            ast.rty = typ;
          }else{
            ast.typ = typ;
          }
        }else if (ast.key == 'strit'){
          for (let i = 0; i < ast.val.length; i++){
            doinfer(ast.val[i]);
          }
          ast.typ = "str";
        }else{
          ast.typ = 'void';
        }

      }
    }
    doinfer(ast);
    return scozoo;
  }


  function compile(ast,scopes){

    let instrs = [];
    let cursco = [];
    let looplbls = [];
    let layout = {};

    let tmpvarcnt = 0;
    function tmpvar(){
      return '__r_'+(tmpvarcnt++);
    }
    function printtypeser(t){
      return printtype(t);
    }
    function _pushins(){
      let ins = Array.from(arguments);
      ins[0] = somepos(ins);
      if (pendmark == null) pendmark = ''//'_'+shortid();
      let inn = [pendmark,...ins]
      instrs.push(inn);
      pendmark = null;
      return inn;
    }
    let pendmark = null;
    function mkmark(s,dorand=true){
      if (pendmark != null){
        _pushins(ast,'nop');
      }
      pendmark = (s??'');
      if (dorand) pendmark+='_'+shortid();
      return pendmark;
    }
    _pushins(ast,'jmp','__main__');

    for (let i = 0; i < scopes.length; i++){
      for (let k in scopes[i]){
        if (typeof scopes[i][k].typ == 'string' && scopes[i][k].typ.startsWith('__func_ovld')){
          
          for (let j = 0; j < scopes[i][k].val.length; j++){
            // console.log("__________________")
            // console.dir(scopes[i][k].val[j],{depth:10000});
            if (scopes[i][k].val[j].agt != undefined && (scopes[i][k].val[j].did )){
              // console.log(scopes[scopes[i][k].val[j].agt])
              // _pushins(scopes[i][k].val[j].ipl,'func',scopes[i][k].typ,scopes[i][k].val[j].typ);
              mkmark(scopes[i][k].typ+'_'+printtypeser(scopes[i][k].val[j].typ),false);

              // if (scopes[scopes[i][k].val[j].agt].__captr){
              //   for (let l = 0; l < scopes[scopes[i][k].val[j].agt].__captr.length; l++){
              //     _pushins(scopes[i][k].val[j].ipl,'argc',scopes[scopes[i][k].val[j].agt].__captr[l].nom,scopes[scopes[i][k].val[j].agt].__captr[l].typ);
              //   }
              // }

              // for (let l in scopes[scopes[i][k].val[j].agt]){
              //   if (l.startsWith('__')){
              //     continue;
              //   }
              //   _pushins(scopes[i][k].val[j].ipl.arg,'argr',l,scopes[scopes[i][k].val[j].agt][l].typ);
              // }

              let ks = Object.keys(scopes[scopes[i][k].val[j].agt]).reverse();
              // let ori = scopes[scopes[i][k].val[j].agt].__names;//.replace(/\?\.?/g,'');
              let ori = scopes[scopes[i][k].val[j].agt].__names.split(".").at(-1);

              // console.log(scopes[i][k].typ,scopes[i][k].val[j].ipl.bdy)
              
              
              if (scopes[i][k].val[j].ipl.bdy){

                let caps = scopes[scopes[i][k].val[j].agt].__captr;
                if (caps){
                  for (let c of caps){
                    if (c.nom == 'this') continue;
                    if (typeof c.typ != 'string' || !c.typ.startsWith("__func_ovld"))
                    _pushins(scopes[i][k].val[j],'dcap',c.ori.split(".").at(-1)+"."+c.nom,c.typ)
                  }
                }

                for (let l of ks){
                  if (l.startsWith('__')){
                    continue;
                  }
                  _pushins(scopes[i][k].val[j].ipl.arg,'argr',ori+'.'+l,scopes[scopes[i][k].val[j].agt][l].typ);
                }
                if (scopes[i][k].val[j].ipl.mbr){
                  _pushins(scopes[i][k].val[j].ipl.arg,'argr','__'+i+'.this',scopes[i].this.typ);
                }
              }
              // compilefunc(scopes[i][k].val[j].ipl, scopes[scopes[i][k].val[j].ctx.at(-1)].__names.slice(2))
              compilefunc(scopes[i][k].val[j].ipl, scopes[scopes[i][k].val[j].ctx.at(-1)].__names.split(".").slice(1).join("."))
              
            }
          }
        }
      }
    }
    mkmark('__main__',false);

    function compilefunc(ast,ori){
      if (ast.bdy){
        docompile(ast.bdy);
        _pushins(ast,'ret');
      }else{
        // console.log('_______________',ast);
        // let typ = ast.typ.elt[1];
        let typ = ast.ano.typ;
        let q = tmpvar(typ);
        _pushins(ast,'decl',q,typ);
        _pushins(ast,'ccall',q,ori+"."+ast.nom.val);
        _pushins(ast,'ret',q);
      }
      
    }


    function docompile(ast,islval=false){
      // console.log("___________")
      // console.log(ast);
      function mktmpvar(typ){
        let tmp = tmpvar();
        pushins('decl',tmp,typ);
        return tmp;
      }
      function alloctmpvar(typ,cnt){
        let tmp = tmpvar();
        pushins('alloc',tmp,typ,cnt);
        return tmp;
      }
      function pushins(){
        return _pushins(ast,...arguments);
      }
      function getcaps(src,trg=null){
        let ff = findfuncbytype(scopes,src).val;
        for (let i = 0; i < ff.length; i++){
          
          if(trg == null || typeeq(ff[i].typ,trg)){
            return scopes[ff[i].agt].__captr??[];
          }
        }
      }
      function docast(nom,src,trg){
        if (trg.con == 'func'){
          // console.trace();
          let cs = getcaps(src,trg);
          pushins('fpak',nom,printtypeser(trg),src+'_'+printtypeser(trg));
          for (let i = 0; i < cs.length; i++){
            // pushins('cap',nom,cs[i].ori+'.'+cs[i].nom,cs[i].typ);
            pushins('cap',nom,cs[i].ori.split('.').at(-1)+'.'+cs[i].nom,cs[i].typ);
          }
          return nom;
        }else{
          let tmp = mktmpvar(trg);
          // console.log(trg,tmp,'<----',src,nom);
          pushins('cast',tmp,nom);
          return tmp;
        }
      }

      function sizeof(x){
        if (typeof x == 'string' && 'iuf'.includes(x[0])){
          return Number(x.slice(1))/8;
        }else if (x.con == 'vec'){
          return Math.max(sizeof(x.elt[0])*x.elt[1], 8);
        }else{
          return 8;
        }
      }

      function compiletype(tctx,tnom,tcpy){

        let typ = tcpy.elt?({con:tnom,elt:tcpy.elt}):tnom;
        let uid = printtypeser(typ);
        let id = '__typd_'+uid;

        pushins('jmp','end'+id);

        mkmark(id,false);

        let ctx = scopes[tcpy.sig];

        layout[uid] = [];

        let tmp = '.this';
        pushins('alloc',tmp,typ,1);
        for (let k in ctx){
          if (k.startsWith('__'))continue;
          if (typeof ctx[k].typ == 'string' && ctx[k].typ.startsWith('__func_ovld_')){
            // let t2 = tmpvar();
            // t2 = docast(t2,ctx[k].typ,ctx[k].val[0].typ);
            // pushins('mov',[tmp,k],t2);
          }else{
            let needcast = null;
            let {val,typ} = ctx[k];
            // if (val != '__this'){
              layout[uid].push([k,ctx[k].typ,sizeof(ctx[k].typ)]);
            if (val != '__this'){
              if (val && !typeeq(val.typ,typ)){
                needcast = [val.typ,typ];
              }
            }
            if (val == null){
              continue;
            }else if (val == '__this'){
              val = tmp;
            }else{
              val = docompile(val);
            }
            if (needcast){
              val = docast(val,...needcast);
            }
          
            pushins('mov',[tmp,k],val);
          }
        }
        pushins('ret',tmp);
        mkmark('end'+id,false);
      }



      if (ast.key == 'bloc' || ast.key == 'nmsp'){

        // pushins('bloc');
        cursco = ast.sco;

        // console.dir(ast,{depth:100000000});
        for (let k in scopes[ast.sco.at(-1)].__types){
          for (let i = 0; i < scopes[ast.sco.at(-1)].__types[k].cpy.length; i++){
            compiletype(ast.sco.at(-1),k,scopes[ast.sco.at(-1)].__types[k].cpy[i])
          }
        }

        for (let i = 0; i < ast.val.length; i++){
          docompile(ast.val[i],null);
        }
        // let o = instrs.slice(idx0).sort((a,b)=>((b[1]=='decl')-(a[1]=='decl')));
        // instrs.splice(idx0,Infinity);
        // o.forEach(x=>instrs.push(x))
        // pushins('end');

      }else if (ast.key == 'term'){
        if (ast.tag == 'ident'){
          if (ast.is_fun_alias){
            // return ast.ori.split(".").at(-1)+"."+ast.typ+ast.pos;
            return ast.ori.split(".").at(-1)+"."+ast.typ+"."+ast.pos.join(".");
          }
          return ast.ori.split(".").at(-1)+"."+ast.val;
        }else{
          return ast.val;
        }
      }else if (ast.key == 'cast'){
        let n1 = docompile(ast.lhs);
        return docast(n1,ast.lhs.typ,ast.rhs.typ);
      }else if (ast.key == 'decl'){

        // let pth = ast.ori.join('.')+".";
        let pth = ast.ori.at(-1)+".";

        if (ast.nom.key == 'tlit'){
          for (let i = 0; i < ast.nom.val.length; i++){
            let typ;
            if (ast.ano){
              typ = ast.ano.typ.elt[i];
            }else{
              typ = ast.val.typ.elt[i];
            }
            let nom = pth+ast.nom.val[i].val;
            pushins('decl',nom,typ);
          }
          if (ast.val){
            let n2 = docompile(ast.val);
            if (ast.ano && !typeeq(ast.val.typ,ast.ano.typ)){
              n2 = docast(n2,ast.val.typ,ast.ano.typ);
            }
            for (let i = 0; i < ast.nom.val.length; i++){
              let nom = pth+ast.nom.val[i].val;
              pushins('mov',nom,[n2,i]);
            }
          }
        }else if (ast.nom.key == 'vlit'){
          
          for (let i = 0; i < ast.nom.val.length; i++){
            for (let j = 0; j < ast.nom.val[i].length; j++){
              let typ;
              if (ast.ano){
                typ = ast.ano.typ.elt[0];
              }else{
                typ = ast.val.typ.elt[0];
              }
              let nom = pth+ast.nom.val[i][j].val;
              pushins('decl',nom,typ);
            }
          }
          if (ast.val){
            let n2 = docompile(ast.val);
            if (ast.ano && !typeeq(ast.val.typ,ast.ano.typ)){
              n2 = docast(n2,ast.val.typ,ast.ano.typ);
            }
            for (let i = 0; i < ast.nom.val.length; i++){
              for (let j = 0; j < ast.nom.val[i].length; j++){
                let nom = pth+ast.nom.val[i][j].val;
                pushins('mov',nom,[n2,i*ast.nom.val[i].length+j]);
              }
            }
          }
        }else{
          let typ;
          let nom = pth+ast.nom.val;
          if (ast.ano){
            typ = ast.ano.typ;
          }else{
            typ = ast.val.typ;
          }
          // if (typeof typ != 'string' || !typ.startsWith("__func_ovld_")){
            pushins('decl',nom,typ);
          // }
          if (ast.val){
            let n2 = docompile(ast.val);
            if (ast.val && !typeeq(typ,ast.val.typ)){
              n2 = docast(n2, ast.val.typ, typ);
            }
            // if (typeof n2 != 'string' || !n2.startsWith("__func_ovld_")){
              pushins('mov',nom,n2);
            // }
          }
        }
      }else if (ast.key == 'vlit'){
        let tmp = mktmpvar(ast.typ);
        let vals = ast.val.flat();
        for (let i = 0; i < vals.length; i++){
          let ni = docompile(vals[i]);
          if (!typeeq(vals[i].typ,ast.typ.elt[0])){
            ni = docast(ni, vals[i].typ, ast.typ.elt[0]);
          }
          pushins('mov',[tmp,i],ni);
        }
        return tmp;
      }else if (ast.key == 'tlit'){
        let tmp = mktmpvar(ast.typ);
        let vals = ast.val;
        for (let i = 0; i < vals.length; i++){
          let ni = docompile(vals[i]);
          if (!typeeq(vals[i].typ,ast.typ.elt[i])){
            ni = docast(ni, vals[i].typ, ast.typ.elt[0]);
          }
          pushins('mov',[tmp,i],ni);
        }
        return tmp;
      }else if (ast.key == 'alit' || ast.key == 'llit'){
        let tmp = alloctmpvar(ast.typ,ast.rhs.length);
        for (let i = 0; i < ast.rhs.length; i++){
          let ni = docompile(ast.rhs[i]);
          if (!typeeq(ast.rhs[i].typ, ast.typ.elt[0])){
            ni = docast(ni, ast.rhs[i].typ, ast.typ.elt[0]);
          }
          pushins('mov',[tmp,i],ni);
        }
        return tmp;
      }else if (ast.key == 'mlit'){
        let tmp = alloctmpvar(ast.typ,ast.rhs.length);
        for (let i = 0; i < ast.rhs.length; i++){
          let na = docompile(ast.rhs[i].lhs);
          let nb = docompile(ast.rhs[i].rhs);
          if (!typeeq(ast.rhs[i].lhs.typ, ast.typ.elt[0])){
            na = docast(na, ast.rhs[i].lhs.typ, ast.typ.elt[0]);
          }
          if (!typeeq(ast.rhs[i].rhs.typ, ast.typ.elt[1])){
            nb = docast(nb, ast.rhs[i].rhs.typ, ast.typ.elt[1]);
          }
          pushins('mov',[tmp,na],nb);
        }
        return tmp;
      }else if (ast.key == 'olit'){

        let nom = ast.typ.con ?? ast.typ;
        let id;
        for (let i = cursco.length-1; i >= 0; i--){
          if (scopes[cursco[i]].__types[nom]){
            id = '__typd_'+printtype(ast.typ);
          }
        }
        if (!id){
          for (let i = 0; i < scopes.length; i++){
            if (scopes[i].__types[nom]){
              id = '__typd_'+printtype(ast.typ);
            }
          }
        }
        // let tmp = alloctmpvar(ast.typ,1);
        
        let tmp = mktmpvar(ast.typ);
        pushins('call',tmp,id);
        let vals = ast.rhs;
        for (let i = 0; i < vals.length; i++){
          let ni = docompile(vals[i].rhs);
          if (!typeeq(vals[i].rhs.typ,vals[i].rhs.rty)){
            ni = docast(ni,vals[i].rhs.typ,vals[i].rhs.rty);
          }
          pushins('mov',[tmp,vals[i].lhs.val],ni);
        }
        return tmp;
      }else if (ast.key == 'a.b'){
        if (ast.lhs.typ.con == 'nmsp'){
          return docompile(ast.rhs);
        }else{
          if (islval){
            return [docompile(ast.lhs),ast.rhs.val];
          }else{
            let l = docompile(ast.lhs);
            let tmp = mktmpvar(ast.typ);
            pushins('mov',tmp,[l,ast.rhs.val]);
            return tmp;
          }
        }
      }else if (ast.key == 'subs'){
        // return [docompile(ast.con),ast.idx.map(docompile)];
        if (islval){
          return [docompile(ast.con),docompile(ast.idx)];
        }else{
          let con = docompile(ast.con);
          if (ast.con.tag == 'strlt'){
            let n0 = mktmpvar(ast.con.typ);
            pushins('mov',n0,con);
            con = n0;
          }
          let idx = docompile(ast.idx);
          // console.log(ast.idx,idx)
          let tmp = mktmpvar(ast.typ);
          // if (ast.idx.tag == 'strlt' || ast.idx.tag == 'numbr'){
          //   let t2 = mktmpvar(ast.idx.typ);
          //   pushins('mov',t2,idx);
          //   pushins('mov',tmp,t2);
          // }else{
          pushins('mov',tmp,[con,idx]);
          // }
          
          return tmp;
        }
      }else if (ast.key == '='){
        if (ast.lhs.key == 'tlit'){
          let n1 = docompile(ast.rhs);
          if (!typeeq(ast.rhs.typ,ast.lhs.typ)){
            n1 = docast(n1,ast.rhs.typ,ast.lhs.typ);
          }
          for (let i = 0; i < ast.lhs.val.length; i++){
            let ni = docompile(ast.lhs.val[i],true);
            pushins('mov',ni,[n1,i]);
          }
        }else if (ast.lhs.key == 'swiz'){
          let n1 = docompile(ast.rhs);
          if (!typeeq(ast.rhs.typ,ast.lhs.typ)){
            n1 = docast(n1,ast.rhs.typ,ast.lhs.typ);
          }
          let tt = [];
          for (let i = 0; i < ast.lhs.rhs.val.length; i++){
            let idx = "xyzw".indexOf(ast.lhs.rhs.val[i]);
            let t1 = mktmpvar(ast.rhs.typ.elt[0]);
            pushins('mov',t1,[n1,idx]);
            tt.push(t1);
          }
          let q = docompile(ast.lhs.lhs);
          for (let i = 0; i < ast.lhs.rhs.val.length; i++){
            let idx = "xyzw".indexOf(ast.lhs.rhs.val[i]);
            pushins('mov',[q,idx],tt[i]);
          }
        }else if (ast.lhs.key == 'vlit'){
          let n1 = docompile(ast.rhs);
          if (!typeeq(ast.rhs.typ,ast.lhs.typ)){
            n1 = docast(n1,ast.rhs.typ,ast.lhs.typ);
          }
          let tt = [];
          for (let i = 0; i < ast.lhs.val.length; i++){
            for (let j = 0; j < ast.lhs.val[i].length; j++){
              let t1 = mktmpvar(ast.rhs.typ.elt[0]);
              pushins('mov',t1,[n1,i*ast.lhs.val[i].length+j]);
              tt.push(t1);
            }
          }
          for (let i = 0; i < ast.lhs.val.length; i++){
            for (let j = 0; j < ast.lhs.val[i].length; j++){
              let idx = i*ast.lhs.val[i].length+j;
              let ni = docompile(ast.lhs.val[i][j],true);
              pushins('mov',ni,tt[idx]);
            }
          }


        }else{
          let n0 = docompile(ast.lhs,true);
          let n1 = docompile(ast.rhs);
          if (!typeeq(ast.lhs.typ,ast.rhs.typ)){
            n1 = docast(n1,ast.rhs.typ,ast.lhs.typ);
          }
          pushins('mov',n0,n1);
        }
        if (islval !== null){
          // console.log(',,,,,',instrs.length)
          let nn = docompile(ast.lhs);
          // console.log(instrs.length)
          return nn;
        }
      }else if (ast.key == 'retn'){
        if (ast.val && ast.val.key != 'noop'){
          let n0 = docompile(ast.val);
          if (!typeeq(ast.typ,ast.val.typ)){
            n0 = docast(n0,ast.val.typ,ast.typ);
          }
          pushins('ret',n0);
        }else{
          pushins('ret');
        }
      }else if (ast.key == 'call'){
        // let n1 = mktmpvar(ast.fun.rty.elt[1]);
        // for (let i = 0; i < ast.arg.length; i++){
        //   let ni = docompile(ast.arg[i]);
        //   if (!typeeq(ast.arg[i].typ,ast.fun.rty.elt[0].elt[i])){
        //     ni = docast(ni,ast.arg[i].typ,ast.fun.rty.elt[0].elt[i])
        //   }
        //   pushins('argw',ni,ast.fun.rty.elt[0].elt[i]);
        // }

        // pushins('call',ast.fun.typ,ast.fun.rty);
        // let n0 = docompile(ast.fun);
        // pushins('call',n0);
        // let n1 = tmpvar();

        // console.log(ast);

        function argw(v,t){
          // if (v.startsWith("__func_ovld_")){
          //   pushins('argf',v+'_'+printtypeser(t));
          // }else{
            pushins('argw',v,t);
          // }
          
        }

        if (ast.fun.key == 'a.b'){
          for (let i = 0; i < scopes.length; i++){
            for (let k in scopes[i]){
              if (scopes[i][k].typ == ast.fun.typ){
                // if (scopes[i][k].val.ipl){
                //   console.log(scopes[i][k].val);
                // }
                if (scopes[i].this){
                  let n1 = mktmpvar(ast.fun.lhs.typ);
                  pushins('mov',n1,docompile(ast.fun.lhs));
                  argw(n1,ast.fun.lhs.typ)
                }
              }
            }
          }
        }

        let n1 = mktmpvar(ast.fun.rty.elt[1]);
        let idx = 0;
        // pushins('bloc');
        for (let i = 0; i < ast.arg.length; i++){
          if (ast.arg[i].key == '...u'){
            let ni = docompile(ast.arg[i].val);

            if (ast.arg[i].val.typ.con == 'tup'){
              for (let j = 0; j < ast.arg[i].val.typ.elt.length; j++){
                let nj = mktmpvar(ast.arg[i].val.typ.elt[j]);
                pushins('mov',nj,[ni,j]);
                if (!typeeq(ast.fun.rty.elt[0].elt[idx],ast.arg[i].val.typ.elt[j])){
                  nj = docast(nj,ast.arg[i].val.typ.elt[j],ast.fun.rty.elt[0].elt[idx]);
                }

                argw(nj,ast.fun.rty.elt[0].elt[idx++]);
              }
            }else if (ast.arg[i].val.typ.con == 'vec'){
              let n = ast.arg[i].val.typ.elt.slice(1).reduce((a,b)=>a*b,1);
              for (let j = 0; j < n; j++){
                let nj = mktmpvar(ast.arg[i].val.typ.elt[0]);
                pushins('mov',nj,[ni,j]);
                if (!typeeq(ast.fun.rty.elt[0].elt[idx],ast.arg[i].val.typ.elt[0])){
                  nj = docast(nj,ast.arg[i].val.typ.elt[0],ast.fun.rty.elt[0].elt[idx]);
                }
                
                argw(nj,ast.fun.rty.elt[0].elt[idx++]);
              }
            }
          }else{

            let ni = docompile(ast.arg[i]);
            
            if (!typeeq(ast.arg[i].typ,ast.fun.rty.elt[0].elt[idx])){
              // console.log(ast.arg[i],ni);
              // process.exit();
              ni = docast(ni,ast.arg[i].typ,ast.fun.rty.elt[0].elt[idx])
            }
            argw(ni,ast.fun.rty.elt[0].elt[idx++]);
          }
        }

        if (ast.fun.is_fun_alias){
          pushins('call',n1,ast.fun.typ+'_'+printtypeser(ast.fun.rty));
        }else{
          try{
            pushins('rcall',n1, ast.fun.ori.split(".").at(-1)+"."+ast.fun.val);
          }catch(e){
            pushins('call',n1,ast.fun.typ+'_'+printtypeser(ast.fun.rty));
          }
        }
        // pushins('end');
        return n1;
      // }else if (ast.key == 'func'){
      //   pushins('decl',ast.nom.val,ast.mty)
      }else if (ast.key == 'loop'){
        // console.log(ast);

        // pushins('bloc');

        let lblcont = 'cont_'+shortid();
        let lblend = 'loopend_'+shortid();

        looplbls.push([lblcont,lblend]);
        let mk0,mk1,ck1,ck2,jp0,jp1;
        if (ast.ini) docompile(ast.ini);
        mk0 = mkmark('loopstart');
        // pushins('bloc');
        if (ast.chk){
          ck1 = docompile(ast.chk);
          jp0 = pushins('jeqz',ck1);
        }
        docompile(ast.bdy);
        mkmark(lblcont,false);
        if (ast.stp) docompile(ast.stp);
        if (ast.ck2){
          ck2 = docompile(ast.ck2);
          jp1 = pushins('jeqz',ck2);
        }
        // pushins('end');
        pushins('jmp',mk0);
        mk1 = mkmark(lblend,false);
        if (jp0) jp0.push(mk1);
        if (jp1) jp1.push(mk1);
        looplbls.pop();
        // pushins('end');
        // pushins('end');
      }else if (ast.key == 'continue'){
        pushins('jmp',looplbls.at(-1)[0]);
      }else if (ast.key == 'break'){
        pushins('jmp',looplbls.at(-1)[1]);
      }else if (ast.key == 'cond'){
        // console.log(ast);
        let tmp = docompile(ast.chk);
        let mk0 = mkmark('condstart');
        let in0 = pushins('jeqz',tmp);
        docompile(ast.lhs);
        
        let in1;
        if (ast.rhs){
          in1 = pushins('jmp');
        }
        let mk1 = mkmark('endif');
        in0.push(mk1);
        
        if (ast.rhs){
          docompile(ast.rhs);
          let mk2 = mkmark('condend');
          in1.push(mk2);
        }
    
      }else if (ast.key == '?:'){
        let out = mktmpvar(ast.typ);
        let tmp = docompile(ast.lhs);
        let mk0 = mkmark('ternstart')
        let in0 = pushins('jeqz',tmp);
        let o0 = docompile(ast.mhs);
        if (!typeeq(ast.mhs.typ,ast.typ)) o0 = docast(o0,ast.mhs.typ,ast.typ)
        pushins('mov',out,o0);
        let in1 = pushins('jmp');
        let mk1 = mkmark('ternelse');
        in0.push(mk1);
      
        let o1 = docompile(ast.rhs);
        if (!typeeq(ast.rhs.typ,ast.typ)) o1 = docast(o1,ast.rhs.typ,ast.typ)
        pushins('mov',out,o1);
        let mk2 = mkmark('ternend')
        in1.push(mk2);
        return out;
      
      }else if (ast.key=='||'){

        let out = mktmpvar(ast.typ);
        pushins('mov',out,'0');
        let o0 = docompile(ast.lhs);
        let in0 = pushins('jeqz',o0);
        pushins('mov',out,'1');
        let in1 = pushins('jmp');
        let mk1 = mkmark('lor');
        let o1 = docompile(ast.rhs);
        let in2 = pushins('jeqz',o1);
        pushins('mov',out,'1');
        let mk2 = mkmark('lor');
        in0.push(mk1);
        in1.push(mk2);
        in2.push(mk2);
        return out;
      } else if (ast.key=='&&'){

        let out = mktmpvar(ast.typ);
        let o0 = docompile(ast.lhs);
        let in0 = pushins('jeqz',o0);
        let o1 = docompile(ast.rhs);
        let in1 = pushins('jeqz',o1);
        pushins('mov',out,'1');
        let in2 = pushins('jmp');
        let mk1 = mkmark('land0')
        pushins('mov',out,'0');
        let mk2 = mkmark('land1')
        in0.push(mk1);
        in1.push(mk1);
        in2.push(mk2);
        return out;
        
      }else if (['<?=','>?='].includes(ast.key)){
        let ins = {
          '>':'lt',
          '<':'gt',
        }[ast.key[0]]
        let nz = docompile(ast.lhs);
        let n0 = docompile(ast.lhs,true);
        let n1 = docompile(ast.rhs);
        if (!typeeq(ast.rhs.typ,ast.typ)){
          n1 = docast(n1,ast.rhs.typ,ast.typ);
        }
        let tmp = mktmpvar(ast.typ);
        pushins(ins,tmp,nz,n1);
        let in0 = pushins('jeqz',tmp);
        pushins('mov',n0,n1);
        let mk = mkmark('meq')
        in0.push(mk);
        return n0;

      }else if (['@*','@*='].includes(ast.key)){
        // console.dir(ast,{depth:10000});
        let n0 = docompile(ast.lhs);
        let n1 = docompile(ast.rhs);
        if (!typeeq(ast.lhs.typ,ast.lhs.v2m)){
          n0 = docast(n0,ast.lhs.typ,ast.lhs.v2m);
        }
        if (!typeeq(ast.rhs.typ,ast.rhs.v2m)){
          n1 = docast(n1,ast.rhs.typ,ast.rhs.v2m);
        }
        // let tmp = (ast.key.endsWith("="))?n0:mktmpvar(ast.typ);
        // pushins('matmul',tmp,n0,n1);
        
        let tmp = mktmpvar(ast.typ);
        if (ast.key.endsWith('=')){
          pushins('matmul',tmp,n0,n1);
          if (!typeeq(ast.typ,ast.rty)){
            tmp = docast(tmp,ast.rty,ast.lhs.typ);
          }
          let n2 = docompile(ast.lhs,true);
          pushins('mov',n2,tmp);
        }else{
          pushins('matmul',tmp,n0,n1);
        }
        
        return tmp;

      }else if (['>','<','==','>=','<=','!='].includes(ast.key)){
        let ins = {
          '>':'gt',
          '<':'lt',
          '>=':'geq',
          '<=':'leq',
          '==':'eq',
          '!=':'neq',
        }[ast.key]
        let n0 = docompile(ast.lhs);
        let n1 = docompile(ast.rhs);
        let typ = maxtype(ast.lhs.typ,ast.rhs.typ);
        if (!typeeq(ast.lhs.typ,typ)){
          n0 = docast(n0,ast.lhs.typ,typ);
        }
        if (!typeeq(ast.rhs.typ,typ)){
          n1 = docast(n1,ast.rhs.typ,typ);
        }
        let tmp = mktmpvar(ast.typ);
        pushins(ins,tmp,n0,n1/*,typ*/);
        return tmp;
        
      }else if (binops.includes(ast.key)){
        let ins = {
          '+':'add',
          '-':'sub',
          '*':'mul',
          '/':'div',
          '%':'mod',
          '**':'pow',
          '<<':'shl',
          '>>':'shr',
          '|':'bor',
          '&':'band',
          '^':'xor',
        }[ast.key.replace('=','')]
        let n0 = docompile(ast.lhs);
        let n1 = docompile(ast.rhs);
        if (!typeeq(ast.lhs.typ,ast.typ)){
          n0 = docast(n0,ast.lhs.typ,ast.typ);
        }
        if (!typeeq(ast.rhs.typ,ast.typ)){
          n1 = docast(n1,ast.rhs.typ,ast.typ);
        }
        let tmp;
        if (ast.key.endsWith('=')){
          tmp = mktmpvar(ast.typ);
          pushins(ins,tmp,n0,n1);
          let n2 = docompile(ast.lhs,true);
          pushins('mov',n2,tmp);
        }else{
          tmp = (n0&&n0.startsWith&&n0.startsWith('__r_'))?n0:mktmpvar(ast.typ);
          // console.log(ast.typ);
          pushins(ins,tmp,n0,n1);
        }
        return tmp;
      }else if (['u++','u--','--u','++u'].includes(ast.key)){
        let ins = {
          '+':'add',
          '-':'sub',
        }[ast.key[1]];
        let n0 = docompile(ast.val);
        let tmp = mktmpvar(ast.typ);
        pushins(ins,tmp,n0,1);
        pushins('mov',docompile(ast.val,true),tmp);
        return (ast.key[0]=='u')?n0:tmp;
      }else if (['+u','-u'].includes(ast.key)){
        let ins = {
          '+':'add',
          '-':'sub',
        }[ast.key[0]];
        let tmp = mktmpvar(ast.typ);
        pushins(ins,tmp,0,docompile(ast.val));
        return tmp;
      }else if (['~u','!u'].includes(ast.key)){
        let ins = {
          '~':'bnot',
          '!':'lnot',
        }[ast.key[0]];
        let tmp = mktmpvar(ast.typ);
        pushins(ins,tmp,docompile(ast.val));
        return tmp;
      }else if (ast.key == 'swiz'){

        let n0 = docompile(ast.lhs);
        let tmp = mktmpvar(ast.typ);

        for (let i = 0; i < ast.rhs.val.length; i++){
          let idx = "xyzw".indexOf(ast.rhs.val[i]);
          if (ast.rhs.val.length>1){
            pushins('mov',[tmp,i],[n0,idx]);
          }else{
            pushins('mov',tmp,[n0,idx]);
          }
        }
        return tmp;
        
      }else if (ast.key == 'typd'){
        
      }else if (ast.key == 'incl'){
        pushins('incl',ast.val.val)

      }else if (ast.key == 'strit'){
        // console.log(ast.val);
        if (ast.val.length == 1){
          return ast.val[0].val;
        }else{
          let n0 = docompile(ast.val[0]);
          let tmp = mktmpvar(ast.typ);
          pushins('mov',tmp,n0);
          for (let i = 1; i < ast.val.length; i++){
            let x = docompile(ast.val[i]);
            if (!typeeq(ast.val[i].typ,'str')){
              x = docast(x,ast.val[i].typ,'str');
            }
            pushins('add',tmp,tmp,x);
          }
          return tmp;
        }
      }else{

      }
      // tmpvarcnt = 0;

    }
    
    docompile(ast);
    
    // if (pendmark){
    //   _pushins(ast,'nop');
    // }
    _pushins(ast,'eoir');
    return [instrs,layout];
  }

  function writeir(instrs){
    let o = [];
    for (let i = 0; i < instrs.length; i++){
      let l = [];
      if (instrs[i][0].length){
        o.push(instrs[i][0]+":");
      }
      for (let j = 2; j < instrs[i].length; j++){
        if (typeof instrs[i][j] == 'string'){
          if (instrs[i][j][0] == '"' || instrs[i][j][0] == "'"){
            l.push(JSON.stringify(instrs[i][j].slice(1,-1)));
          }else{
            l.push(instrs[i][j]);
          }
        }else if (typeof instrs[i][j] == 'number'){
          l.push(instrs[i][j].toString())
        }else if (Array.isArray(instrs[i][j])){
          l.push(instrs[i][j].join('+'));
        }else{
          l.push(printtype(instrs[i][j]));
        }
      }
      // o.push('\t'+l.join('\t'));
      let ll = " ".repeat(8);
      for (let j = 0; j < l.length; j++){
        if (j < l.length-1 && l[j].length < 7){
          ll+=l[j].padEnd(8,' ');
        }else{
          ll += l[j]+((j < l.length-1)?' ':'');
        }
      }
      o.push(ll);
    }
    return o.join('\n')+'\n';
  }

  function writelayout(layout){
    let o = [];
    for (let k in layout){
      
      let sum = 0;
      for (let i = 0; i < layout[k].length; i++){
        sum += layout[k][i][2];
      }
      o.push(k+'\t'+sum);
      sum = 0;
      for (let i = 0; i < layout[k].length; i++){
        let [nom,typ,siz] = layout[k][i];
        o.push(`\t${sum}\t${nom}\t${printtype(typ)}`);
        sum += siz;
      }
    }
    return o.join('\n')+'\n';
  }

  function writesrcmap(instrs){
    let o = [];
    for (let i = 0; i < state.src.length; i++){
      // o.push(`F ${i} ${state.src[i].pth}`);
      o.push(`F ${i} ${sys.path.relative(sys.process.cwd(), state.src[i].pth)}`);
    }
    for (let i = 0; i < instrs.length; i++){
      let pos = instrs[i][1];
      if (!pos) pos = [0,0];
      o.push(`P ${pos[0]} ${pos[1]}`);
    }
    return o.join('\n')+'\n';
  }

  this.tokenize = tokenize;
  this.parse = parse;
  this.abstract = abstract;
  this.infertypes = infertypes;
  this.compile = compile;
  this.writeir = writeir;
  this.writelayout = writelayout;
  this.writesrcmap = writesrcmap;
  this.state = state;
}


if (typeof module !== 'undefined'){

  const fs = require('fs');
  const path = require('path');
  let parser = new PARSER({fs,path,process,search_paths:[process.env.DITHER_ROOT??""].filter(x=>x.length)});

  let inp_pth;
  let cst_pth;
  let tok_pth;
  let ast_pth;
  let map_pth;
  let out_pth = "ir.dsm";
  for (let i = 2; i < process.argv.length; i++){
    if (process.argv[i] == '-o' || process.argv[i] == '--output'){
      out_pth = process.argv[++i];
    }else if (process.argv[i] == '--ast'){
      ast_pth = process.argv[++i];
    }else if (process.argv[i] == '--cst'){
      cst_pth = process.argv[++i];
    }else if (process.argv[i] == '--tok'){
      tok_pth = process.argv[++i];
    }else if (process.argv[i] == '--map'){
      map_pth = process.argv[++i];
    }else{
      inp_pth = process.argv[i];
    }
  }
  if (!inp_pth){
    console.log("no input file.");
    process.exit(0);
  }

  let toks = parser.tokenize(path.resolve(inp_pth));

  if (tok_pth) fs.writeFileSync(tok_pth,JSON.stringify(toks,null,2));

  let cst = parser.parse(toks);

  if (cst_pth) fs.writeFileSync(cst_pth,JSON.stringify(cst,null,2));

  let ast = parser.abstract(cst);

  if (ast_pth) fs.writeFileSync(ast_pth,JSON.stringify(ast,null,2));

  let scopes = parser.infertypes(ast);

  let [instrs,layout] = parser.compile(ast,scopes);

  let lo = parser.writelayout(layout);

  if (map_pth) fs.writeFileSync(map_pth,parser.writesrcmap(instrs));

  fs.writeFileSync(out_pth,parser.writeir(instrs)+lo);
}