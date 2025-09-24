const fs = require('fs');
const path = require('path')
const PARSER = require('../../src/parser.js');
const { execSync } = require('child_process');

let do_edit = 1;
let attr_edit = ``;
if (do_edit) attr_edit = ` contenteditable="true" `

let API = {};
if (fs.existsSync("doc/api.json")){
  API = JSON.parse(fs.readFileSync("doc/api.json").toString());
}

let parser = new PARSER(
  {fs,path,process,search_paths:[path.resolve(".")]},
  {},
);
let idens = [];
let std = fs.readdirSync("std").filter(x=>!x.startsWith("."));

function extract(out,cont){
  function printtype(x){
    if (!x){
      return 'void'
    }
    if (typeof x == 'string' || typeof x == 'number'){
      return `${x}`;
    }else{
      return `${x.con}[${x.elt.map(printtype).join(',')}]`;
    }
  }
  function shrinktype(ast){
    if (ast.ttp == 'smtp'){
      return ast.val.val;
    }else if (ast.ttp == 'cntp'){
      return {con:ast.con.val,elt:ast.elt.map(x=>shrinktype(x))};
    }else if (ast.ttp == 'nmtp'){
      return Number(ast.val.val);
    }
    return 'void';
  }
  function printval(x){
    if (typeof x == 'object'){
      if (Array.isArray(x)){
        return x.map(printval).join(',')
      }else if (x.val){
        return printval(x.val);
      }
    }
    return x??null;
  }
  let {key} = cont;
  if (key == 'decl'){
    let val = null;
    if (cont.val){
      if (cont.val.key == '-u'){
        val= '-'+printval(cont.val.val);
      }else{
        val= printval(cont.val.val)
      }
    }
    out.push({
      tag:'decl',
      nom:cont.nom.val,
      val,
      typ:cont.val?printtype(cont.val.typ):printtype(shrinktype(cont.ano)),
    })
  }else if (key == 'func'){
    let fun = cont.nom.val;
    if (fun[0] != '_'){
      out.push({
        tag:'func',
        nom:cont.nom.val,
        tem:(cont.tem??[]).map(shrinktype).map((x,y)=>(cont.pte[y]?(x+'='+printtype(shrinktype(cont.pte[y]))):x)),
        arg:cont.arg.map(x=>[x.lhs.val,printtype(shrinktype(x.rhs))]),
        ret:printtype(shrinktype(cont.ano))
      });
    }
  }else if (key == 'typd'){
    if (cont.rhs.key == 'bloc'){
      let o = {
        tag:'typd',
        nom:shrinktype(cont.lhs),
        val:[],
      }
      for (let i = 0; i < cont.rhs.val.length; i++){
        extract(o.val,cont.rhs.val[i])
      }
      o.val.sort((a, b) => a.nom.localeCompare(b.nom));
      out.push(o)
    }
  }else if (key == 'nmsp'){
    let o = {
      tag:'nmsp',
      nom:cont.nom.val,
      val:[],
    }
    for (let i = 0; i < cont.val.length; i++){
      extract(o.val,cont.val[i])
    }
    o.val.sort((a, b) => a.nom.localeCompare(b.nom));
    out.push(o)
  }
}

for (let i = 0; i < std.length; i++){
  let isdir = fs.lstatSync("std/"+std[i]).isDirectory();
  let f = "std/"+std[i]+"/header.dh";
  if (!isdir){
    continue;
  }
  let toks = parser.tokenize(f);
  let cst = parser.parse(toks);
  let ast = parser.abstract(cst);
  let scopes = parser.infertypes(ast);

  let nmsp = ast.val.filter(x=>x.key == "nmsp").at(-1);
  // if (std[i]=='gx') console.log(nmsp)
  let name = nmsp.nom.val;
  let cont = nmsp.val;
  let o = {
    tag:'nmsp',
    nom:name,
    val:[]
  }
  for (let j = 0; j < cont.length; j++){
    extract(o.val,cont[j]) 
  }
  o.val.sort((a, b) => a.nom.localeCompare(b.nom));
  idens.push(o);

}

// console.log(idens)

function get_api(id,n){
  id = id.replace(/^std-/,"");
  if (!API[id]){
    return "";
  }
  if (!API[id][n]){
    return ""
  }
  return API[id][n];
}

let L = "";
let R = "";
function render(pth,idens){
  let dep = pth.split('-').length;
  function clean(x){
    return x.replace(/\[/g,'(').replace(/\]/g,')')
  }
  for (let i = 0; i < idens.length; i++){
    let {nom,tag,typ,val} = idens[i];
    let id = `${pth}-${nom}`;
    if (tag == 'func'){
      id += ':'+idens[i].tem.join('*');
      id += ':'+idens[i].arg.map(x=>x[1]).join('*');
    }
    id = clean(id);
    let tagstyle = `margin:2px;font-size:12px;color:royalblue; border:1px solid royalblue; border-radius:3px; background:rgba(65,105,226,0.2)`;
    let smallhd = `font-size:10px;margin-bottom:5px;margin-top:5px`

    R += `<details id="${id}" ${dep==1?`class="top-level"`:""} style="margin-left:20px"><summary><span style="line-height:20px;`;
    if (tag == 'typd' || tag == 'nmsp'){
      R += `font-size:16px;`
    }else{
      R += `font-weight:bold;`
    }

    R += `" onclick="window.location.href='#${id}'">${nom}</span>`;
    if (tag == 'typd'){
      R += `<span style="${tagstyle}">type</span>`
    }else if (tag == 'decl'){
      R += ':'
      R += `<span style="${tagstyle}">${typ}</span>`;
      if (val){
        R += '='+val;
      }
    }else if (tag == 'func'){
      if (idens[i].tem.length){
        R += '['
        for (let j = 0; j < idens[i].tem.length; j++){
          if (j) R += ','
          R += `<span style="${tagstyle}">${idens[i].tem[j]}</span>`
        }
        R += ']'
      }
      R += `(`
      for (let j = 0; j < idens[i].arg.length; j++){
        if (j) R += ','
        let [a,t] = idens[i].arg[j];
        R += `${a}:<span style="${tagstyle}">${t}</span>`
      }
      R += `):`;
      R += `<span style="${tagstyle}">${idens[i].ret}</span>`
    }
    R += `</summary>`;

    if (tag == 'nmsp' || tag == 'typd'){
      R += `<div style="margin:10px">
        <div ${attr_edit} style="background:whitesmoke" id="${id}@0">${get_api(id,0)}</div>
      </div>`
      L += `<details style="margin-left:20px"><summary><a href="#${id}">${nom}</a></summary>`;
      render(id,idens[i].val);
      L += `</details>\n`
    }else{
      if (tag == 'func'){
        if (idens[i].arg.length){
          L += `<div style="margin-left:20px"><a href="#${id}">${nom}(${idens[i].arg.length})</a>`;
        }else{
          L += `<div style="margin-left:20px"><a href="#${id}">${nom}()</a>`;
        }
      }else{
        L += `<div style="margin-left:20px"><a href="#${id}">${nom}</a>`;
      }
      R += `<div style="margin:10px">`
      if (tag == 'decl'){
        // R += `<div style="${smallhd}">DESCRIPTION</div>`
        R += `<div ${attr_edit} style="background:whitesmoke" id="${id}@0">${get_api(id,0)}</div>`
      }else if (tag == 'func'){
        // R += `<div style="${smallhd}">DESCRIPTION</div>`
        R += `<div ${attr_edit} style="background:whitesmoke" id="${id}@0">${get_api(id,0)}</div>`
        R += `<div style="${smallhd}">PARAMETERS</div>`
        R += `<table>`
        for (let j = 0; j < idens[i].arg.length; j++){
          let [a,t] = idens[i].arg[j];
          R += `<tr>`;
          R += `<td style="min-width:50px;">${a}</td>`
          R += `<td style="min-width:50px;"><span style="${tagstyle}">${t}</span></td>`
          R += `<td style="min-width:50px;background:whitesmoke" ${attr_edit} id="${id}@${j+1}">${get_api(id,j+1)}</td>`
          R += `</tr>`
        }
        R += `</table>`
        R += `<div style="${smallhd}">RETURNS</div>`
        R += `<span style="${tagstyle}">${idens[i].ret}</span>`;
        if (idens[i].ret != 'void'){
          R += `<span ${attr_edit} id="${id}@${idens[i].arg.length+1}"style="display:inline-block;background:whitesmoke;min-width:50px;">${get_api(id,idens[i].arg.length+1)}</span>`
        }
        
       
      }
      R += `</div>`
      L += `</div>\n`
    }
    
    R += `</details>\n`
  }
  
}
render("std",idens);

let html = `<html>
<style>
:target {
  border: 1px solid black;
}
a{
  cursor:pointer;
}
a:link {
  color: royalblue;
}
a:visited {
  color: royalblue;
}
a:hover {
  color: black;
}
a:active {
  color: royalblue;
}
</style>
<body style="font-family:monospace">
<div style="position:fixed;left:0px;top:0px;width:200px;height:100%;overflow:scroll;background:white">
  <div>
  <h2>&nbsp;dither std/</h2>
  ${L}
  </div>
</div>
<div style="position:absolute;left:200px;top:0px;width:calc(100% - 200px);height:100%">\n${R}</div>
</body>
<script>
  function expandTarget() {
    const targetId = location.hash.slice(1);
    if (!targetId) return;
    const target = document.getElementById(targetId);
    if (!target) return;
    if (target.tagName === 'DETAILS') {
      target.open = true;
    }
    let parent = target.parentElement;
    while (parent) {
      if (parent.tagName === 'DETAILS') parent.open = true;
      parent = parent.parentElement;
    }
  }
  expandTarget();
  window.addEventListener('hashchange', expandTarget);

  if (location.hash.slice(1) == ''){
    Array.from(document.getElementsByClassName("top-level")).forEach(x=>x.open=true)
  }

  function download_file(pth,text){
    var element = document.createElement('a');
    element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
    element.setAttribute('download', pth);
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);
  }
  function get_doc(){
    let elts = Array.from(document.querySelectorAll('[id^="std-"]')).filter(x=>x.id.includes('@'));
    let doc = {};
    for (let i = 0; i < elts.length; i++){
      let [k,v] = elts[i].id.split('@');
      k = k.replace(/^std-/,"");
      if (!doc[k]) doc[k] = [];
      doc[k][v] = elts[i].innerHTML;
    }
    return JSON.stringify(doc,null,2);
  }
  function download_doc(){
    download_file('api.json',get_doc());
  }
</script>

</html>`;
fs.writeFileSync("build/api.html",html)