const fs = require('fs');
const path = require('path')
const PARSER = require('../../src/parser.js');

let html = [`
<style>
  *::-webkit-scrollbar {
    width: 10px;
    height: 10px;
  }
  *::-webkit-scrollbar-track {
    background: #222;
  }
  *::-webkit-scrollbar-thumb {
    background-color: #444;
    border-radius: 5px;
    border: 2px solid #222;
  }
  ::-webkit-scrollbar-corner {
    background: #222;
  }
  input[type=number]::-webkit-inner-spin-button {
    opacity: 1;
    filter: invert(90%);
  }
  input[type=range],input[type=checkbox] {
    filter: invert(90%);
  }
  .menubtn{
    font-size:12px;
    background:#171717;color:white;height:24px;
    border-top:2px solid dimgray;
    border-left:2px solid dimgray;
    border-right:2px solid black;
    border-bottom:2px solid black;
    cursor:pointer;
  }
  .menubtn:active{
    border-top:2px solid black;
    border-left:2px solid black;
    border-right:2px solid dimgray;
    border-bottom:2px solid dimgray;
    cursor:pointer;
  }
</style>
<body style="color:white;background:black;min-width:960px;position:absolute;width:100%;height:100%;left:0px;top:0px;margin:0px;padding-0px;"></body>
`];

html.push(`<script>${fs.readFileSync("src/parser.js").toString()}</script>`);
html.push(`<script>${fs.readFileSync("src/to_js.js").toString()}</script>`);
html.push(`<script>${fs.readFileSync("src/embed_glsl.js").toString()}</script>`);
html.push(`<script>var STD={`)
let ff = fs.readdirSync("std");
for (let i = 0; i < ff.length; i++){
  let isdir = fs.lstatSync("std/"+ff[i]).isDirectory();
  if (isdir){
    let q = `"std/${ff[i]}/header.dh":`+JSON.stringify(fs.readFileSync("std/"+ff[i]+"/header.dh").toString())+",";
    let p = `"std/${ff[i]}/static.js":`+JSON.stringify(fs.readFileSync("std/"+ff[i]+"/static.js").toString())+`,`;
    html.push(q);
    html.push(p);
  }else if (ff[i].endsWith(".dh")){
    let f = ff[i].replace(/\.dh$/g,"");
    let v = JSON.stringify(fs.readFileSync("std/"+ff[i]).toString());
    html.push(`"std/${f}":`+v+",");
    html.push(`"std/${ff[i]}":`+v+",");
  }
}
html.push(`};</script>`)

let parser = new PARSER(
  {fs,path,process,search_paths:[path.resolve(".")]},
  {},
);
let idens = new Set();
let std = fs.readdirSync("std").filter(x=>!x.startsWith("."));
for (let i = 0; i < std.length; i++){
  let isdir = fs.lstatSync("std/"+std[i]).isDirectory();
  let f = "std/"+std[i]+"/header.dh";
  if (!isdir){
    f = "std/"+std[i];
  }
  let toks = parser.tokenize(f);
  let cst = parser.parse(toks);
  let nmsp = cst.val.filter(x=>x.key == "nmsp")[0];
  let name = nmsp.nom.val;
  let cont = nmsp.val;
  for (let j = 0; j < cont.length; j++){
    let {key} = cont[j];
    if (key == '='){
      idens.add(JSON.stringify([name,cont[j].lhs.lhs.val,'decl']))
    }else if (key == 'func'){
      let fun = cont[j].nom.val;
      if (fun[0] != '_'){
        idens.add(JSON.stringify([name,fun,'func']));
      }
    }else if (key == 'typd'){
      idens.add(JSON.stringify([name,cont[j].lhs.val,'typd']));
    }
  }
}
html.push(`<script>var Std_idens = [${Array.from(idens).sort().join(',')}]</script>`)




function main(){
  let EXAMPLES = [
    '["prgm","ctrl",[[["varv","decl",[["iden","iden",[],["Spread"],null],["lf32","litr",["0.5"],null,null]],null,null],["func","decl",[["iden","iden",[],["Tree"],null],["tvod","type",[],null,null],[["argr","decl",[["iden","iden",[],["X"],null],["tf32","type",[],null,null]],null,null],["argr","decl",[["iden","iden",[],["Y"],null],["tf32","type",[],null,null]],null,null],["argr","decl",[["iden","iden",[],["A"],null],["tf32","type",[],null,null]],null,null],["argr","decl",[["iden","iden",[],["L"],null],["tf32","type",[],null,null]],null,null]],[["cond","ctrl",[["oolt","oper",[["iden","iden",[],["L"],null],["lfsl","litr",["3.0","56","12.0"],null,null]],null,null],[["ret0","misc",[],null,null]]],null,null],["varv","decl",[["iden","iden",[],["X1"],null],["oadd","oper",[["iden","iden",[],["X"],null],["omul","oper",[["cal1","misc",[["stdf","stdl",[],["math: <b>cos</b>"],null],["iden","iden",[],["A"],null]],null,null],["iden","iden",[],["L"],null]],null,null]],null,null]],null,null],["varv","decl",[["iden","iden",[],["Y1"],null],["oadd","oper",[["iden","iden",[],["Y"],null],["omul","oper",[["cal1","misc",[["stdf","stdl",[],["math: <b>sin</b>"],null],["iden","iden",[],["A"],null]],null,null],["iden","iden",[],["L"],null]],null,null]],null,null]],null,null],["call","misc",[["stdf","stdl",[],["gx: <b>line</b>"],null],[["iden","iden",[],["X"],null],["iden","iden",[],["Y"],null],["iden","iden",[],["X1"],null],["iden","iden",[],["Y1"],null]]],null,null],["call","misc",[["iden","iden",[],["Tree"],null],[["iden","iden",[],["X1"],null],["iden","iden",[],["Y1"],null],["osub","oper",[["iden","iden",[],["A"],null],["iden","iden",[],["Spread"],null]],null,null],["omul","oper",[["iden","iden",[],["L"],null],["lf32","litr",["0.8"],null,null]],null,null]]],null,null],["call","misc",[["iden","iden",[],["Tree"],null],[["iden","iden",[],["X1"],null],["iden","iden",[],["Y1"],null],["oadd","oper",[["iden","iden",[],["A"],null],["iden","iden",[],["Spread"],null]],null,null],["omul","oper",[["iden","iden",[],["L"],null],["lf32","litr",["0.8"],null,null]],null,null]]],null,null]]],null,null],["cal2","misc",[["stdf","stdl",[],["gx: <b>size</b>"],null],["li32","litr",["320"],null,null],["li32","litr",["240"],null,null]],null,null],["whil","ctrl",[["li32","litr",["1"],null,null],[["cal1","misc",[["stdf","stdl",[],["gx: <b>background</b>"],null],["lf32","litr",["0.75"],null,null]],null,null],["call","misc",[["iden","iden",[],["Tree"],null],[["lf32","litr",["160.0"],null,null],["lf32","litr",["240.0"],null,null],["odiv","oper",[["stdd","stdl",[],["math: <b>PI</b>"],null],["lf32","litr",["-2.0"],null,null]],null,null],["lf32","litr",["50.0"],null,null]]],null,null],["asgn","misc",[["iden","iden",[],["Spread"],null],["cal1","misc",[["stdf","stdl",[],["math: <b>sin</b>"],null],["omul","oper",[["lf32","litr",["0.002"],null,null],["cal0","misc",[["stdf","stdl",[],["time: <b>millis</b>"],null]],null,null]],null,null]],null,null]],null,null],["cal0","misc",[["stdf","stdl",[],["gx: <b>poll</b>"],null]],null,null]]],null,null]]],null,[24,24]]',
    '["prgm","ctrl",[[["varv","decl",[["iden","iden",[],["Width"],null],["li32","litr",["256"],null,null]],null,null],["varv","decl",[["iden","iden",[],["Height"],null],["li32","litr",["256"],null,null]],null,null],["varv","decl",[["iden","iden",[],["MaxIteration"],null],["lisl","litr",["4","71","16"],null,null]],null,null],["func","decl",[["iden","iden",[],["Mandelbrot"],null],["ti32","type",[],null,null],[["argr","decl",[["iden","iden",[],["X0"],null],["tf32","type",[],null,null]],null,null],["argr","decl",[["iden","iden",[],["Y0"],null],["tf32","type",[],null,null]],null,null]],[["varv","decl",[["iden","iden",[],["Y"],null],["lf32","litr",["0.0"],null,null]],null,null],["varv","decl",[["iden","iden",[],["X"],null],["lf32","litr",["0.0"],null,null]],null,null],["varv","decl",[["iden","iden",[],["Iteration"],null],["li32","litr",["0"],null,null]],null,null],["varv","decl",[["iden","iden",[],["XX"],null],["lf32","litr",["0.0"],null,null]],null,null],["varv","decl",[["iden","iden",[],["YY"],null],["lf32","litr",["0.0"],null,null]],null,null],["whil","ctrl",[["olan","oper",[["oolt","oper",[["iden","iden",[],["Iteration"],null],["iden","iden",[],["MaxIteration"],null]],null,null],["oleq","oper",[["oadd","oper",[["asgn","misc",[["iden","iden",[],["XX"],null],["omul","oper",[["iden","iden",[],["X"],null],["iden","iden",[],["X"],null]],null,null]],null,null],["asgn","misc",[["iden","iden",[],["YY"],null],["omul","oper",[["iden","iden",[],["Y"],null],["iden","iden",[],["Y"],null]],null,null]],null,null]],null,null],["lf32","litr",["4.0"],null,null]],null,null]],null,null],[["varv","decl",[["iden","iden",[],["Xt"],null],["oadd","oper",[["osub","oper",[["iden","iden",[],["XX"],null],["iden","iden",[],["YY"],null]],null,null],["iden","iden",[],["X0"],null]],null,null]],null,null],["asgn","misc",[["iden","iden",[],["Y"],null],["oadd","oper",[["omul","oper",[["lf32","litr",["2.0"],null,null],["omul","oper",[["iden","iden",[],["X"],null],["iden","iden",[],["Y"],null]],null,null]],null,null],["iden","iden",[],["Y0"],null]],null,null]],null,null],["asgn","misc",[["iden","iden",[],["X"],null],["iden","iden",[],["Xt"],null]],null,null],["asgn","misc",[["iden","iden",[],["Iteration"],null],["oadd","oper",[["iden","iden",[],["Iteration"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null],["retn","misc",[["iden","iden",[],["Iteration"],null]],null,null]]],null,null],["cal2","misc",[["stdf","stdl",[],["gx: <b>size</b>"],null],["iden","iden",[],["Width"],null],["iden","iden",[],["Height"],null]],null,null],["whi1","ctrl",[[["forr","ctrl",[["iden","iden",[],["I"],null],["li32","litr",["0"],null,null],["iden","iden",[],["Height"],null],[["cal0","misc",[["stdf","stdl",[],["gx: <b>poll</b>"],null]],null,null],["forr","ctrl",[["iden","iden",[],["J"],null],["li32","litr",["0"],null,null],["iden","iden",[],["Width"],null],[["varv","decl",[["iden","iden",[],["Q"],null],["call","misc",[["iden","iden",[],["Mandelbrot"],null],[["osub","oper",[["omul","oper",[["odiv","oper",[["cast","misc",[["iden","iden",[],["J"],null],["tf32","type",[],null,null]],null,null],["iden","iden",[],["Width"],null]],null,null],["lf32","litr",["3.0"],null,null]],null,null],["lf32","litr",["2.0"],null,null]],null,null],["osub","oper",[["omul","oper",[["odiv","oper",[["cast","misc",[["iden","iden",[],["I"],null],["tf32","type",[],null,null]],null,null],["iden","iden",[],["Height"],null]],null,null],["lf32","litr",["3.0"],null,null]],null,null],["lf32","litr",["1.5"],null,null]],null,null]]],null,null]],null,null],["varv","decl",[["iden","iden",[],["F"],null],["opow","oper",[["odiv","oper",[["cast","misc",[["iden","iden",[],["Q"],null],["tf32","type",[],null,null]],null,null],["iden","iden",[],["MaxIteration"],null]],null,null],["lf32","litr",["0.5"],null,null]],null,null]],null,null],["call","misc",[["stdf","stdl",[],["gx: <b>stroke</b>"],null],[["iden","iden",[],["F"],null],["omul","oper",[["iden","iden",[],["F"],null],["lf32","litr",["0.8"],null,null]],null,null],["osub","oper",[["lf32","litr",["1.0"],null,null],["iden","iden",[],["F"],null]],null,null]]],null,null],["cal2","misc",[["stdf","stdl",[],["gx: <b>point</b>"],null],["iden","iden",[],["J"],null],["iden","iden",[],["I"],null]],null,null]]],null,null]]],null,null]]],null,null]]],null,[24,24]]',
    '["prgm","ctrl",[[["func","decl",[["iden","iden",[],["Factorial"],null],["ti32","type",[],null,null],[["argr","decl",[["iden","iden",[],["X"],null],["ti32","type",[],null,null]],null,null]],[["cond","ctrl",[["iden","iden",[],["X"],null],[["retn","misc",[["omul","oper",[["iden","iden",[],["X"],null],["call","misc",[["iden","iden",[],["Factorial"],null],[["osub","oper",[["iden","iden",[],["X"],null],["li32","litr",["1"],null,null]],null,null]]],null,null]],null,null]],null,null]]],null,null],["retn","misc",[["li32","litr",["1"],null,null]],null,null]]],null,null],["varv","decl",[["iden","iden",[],["A"],null],["lisl","litr",["0","50","12"],null,null]],null,null],["varv","decl",[["iden","iden",[],["B"],null],["call","misc",[["iden","iden",[],["Factorial"],null],[["iden","iden",[],["A"],null]]],null,null]],null,null],["call","misc",[["stdf","stdl",[],["io: <b>println</b>"],null],[["ocat","oper",[[["lstr","litr",["The factorial of "],null,null],["iden","iden",[],["A"],null],["lstr","litr",[" is "],null,null],["iden","iden",[],["B"],null]]],null,null]]],null,null]]],null,[24,24]]',
    '["prgm","ctrl",[[["varv","decl",[["iden","iden",[],["Width"],null],["lisl","litr",["50","14","150"],null,null]],null,null],["varv","decl",[["iden","iden",[],["Height"],null],["lisl","litr",["50","14","150"],null,null]],null,null],["varv","decl",[["iden","iden",[],["Scale"],null],["lisl","litr",["1","48","8"],null,null]],null,null],["varv","decl",[["iden","iden",[],["InitialConfig"],null],["lbmp","litr",["38","11",[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0],[0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0],[0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]],null,null]],null,null],["varv","decl",[["iden","iden",[],["Board"],null],["cal2","misc",[["stdf","stdl",[],["arr: <b>make</b>"],null],["lvec","litr",[[["iden","iden",[],["Width"],null],["iden","iden",[],["Height"],null]]],null,null],["lu08","litr",["0"],null,null]],null,null]],null,null],["varv","decl",[["iden","iden",[],["Mask"],null],["li32","litr",["1"],null,null]],null,null],["forr","ctrl",[["iden","iden",[],["I"],null],["li32","litr",["0"],null,null],["subs","misc",[["li32","litr",["0"],null,null],["cal1","misc",[["stdf","stdl",[],["arr: <b>shape</b>"],null],["iden","iden",[],["InitialConfig"],null]],null,null]],null,null],[["forr","ctrl",[["iden","iden",[],["J"],null],["li32","litr",["0"],null,null],["subs","misc",[["li32","litr",["1"],null,null],["cal1","misc",[["stdf","stdl",[],["arr: <b>shape</b>"],null],["iden","iden",[],["InitialConfig"],null]],null,null]],null,null],[["asgn","misc",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["InitialConfig"],null]],null,null]],null,null]]],null,null]]],null,null],["func","decl",[["iden","iden",[],["Step"],null],["tvod","type",[],null,null],[["lar0","litr",[],null,null]],[["forr","ctrl",[["iden","iden",[],["I"],null],["li32","litr",["0"],null,null],["iden","iden",[],["Height"],null],[["forr","ctrl",[["iden","iden",[],["J"],null],["li32","litr",["0"],null,null],["iden","iden",[],["Width"],null],[["varv","decl",[["iden","iden",[],["Alive"],null],["li32","litr",["0"],null,null]],null,null],["varv","decl",[["iden","iden",[],["LookLeft"],null],["oogt","oper",[["iden","iden",[],["J"],null],["li32","litr",["0"],null,null]],null,null]],null,null],["varv","decl",[["iden","iden",[],["LookRight"],null],["oolt","oper",[["iden","iden",[],["J"],null],["osub","oper",[["iden","iden",[],["Width"],null],["li32","litr",["1"],null,null]],null,null]],null,null]],null,null],["cond","ctrl",[["oogt","oper",[["iden","iden",[],["I"],null],["li32","litr",["0"],null,null]],null,null],[["cond","ctrl",[["oban","oper",[["subs","misc",[["lidx","litr",[[["osub","oper",[["iden","iden",[],["I"],null],["li32","litr",["1"],null,null]],null,null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null],[["asgn","misc",[["iden","iden",[],["Alive"],null],["oadd","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null],["cond","ctrl",[["olan","oper",[["iden","iden",[],["LookLeft"],null],["oban","oper",[["subs","misc",[["lidx","litr",[[["osub","oper",[["iden","iden",[],["I"],null],["li32","litr",["1"],null,null]],null,null],["osub","oper",[["iden","iden",[],["J"],null],["li32","litr",["1"],null,null]],null,null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null],[["asgn","misc",[["iden","iden",[],["Alive"],null],["oadd","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null],["cond","ctrl",[["olan","oper",[["iden","iden",[],["LookRight"],null],["oban","oper",[["subs","misc",[["lidx","litr",[[["osub","oper",[["iden","iden",[],["I"],null],["li32","litr",["1"],null,null]],null,null],["oadd","oper",[["iden","iden",[],["J"],null],["li32","litr",["1"],null,null]],null,null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null],[["asgn","misc",[["iden","iden",[],["Alive"],null],["oadd","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null]]],null,null],["cond","ctrl",[["oolt","oper",[["iden","iden",[],["I"],null],["osub","oper",[["iden","iden",[],["Height"],null],["li32","litr",["1"],null,null]],null,null]],null,null],[["cond","ctrl",[["oban","oper",[["subs","misc",[["lidx","litr",[[["oadd","oper",[["iden","iden",[],["I"],null],["li32","litr",["1"],null,null]],null,null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null],[["asgn","misc",[["iden","iden",[],["Alive"],null],["oadd","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null],["cond","ctrl",[["olan","oper",[["iden","iden",[],["LookLeft"],null],["oban","oper",[["subs","misc",[["lidx","litr",[[["oadd","oper",[["iden","iden",[],["I"],null],["li32","litr",["1"],null,null]],null,null],["osub","oper",[["iden","iden",[],["J"],null],["li32","litr",["1"],null,null]],null,null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null],[["asgn","misc",[["iden","iden",[],["Alive"],null],["oadd","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null],["cond","ctrl",[["olan","oper",[["iden","iden",[],["LookRight"],null],["oban","oper",[["subs","misc",[["lidx","litr",[[["oadd","oper",[["iden","iden",[],["I"],null],["li32","litr",["1"],null,null]],null,null],["oadd","oper",[["iden","iden",[],["J"],null],["li32","litr",["1"],null,null]],null,null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null],[["asgn","misc",[["iden","iden",[],["Alive"],null],["oadd","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null]]],null,null],["cond","ctrl",[["olan","oper",[["iden","iden",[],["LookLeft"],null],["oban","oper",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["osub","oper",[["iden","iden",[],["J"],null],["li32","litr",["1"],null,null]],null,null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null],[["asgn","misc",[["iden","iden",[],["Alive"],null],["oadd","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null],["cond","ctrl",[["olan","oper",[["iden","iden",[],["LookRight"],null],["oban","oper",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["oadd","oper",[["iden","iden",[],["J"],null],["li32","litr",["1"],null,null]],null,null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null],[["asgn","misc",[["iden","iden",[],["Alive"],null],["oadd","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["1"],null,null]],null,null]],null,null]]],null,null],["asgn","misc",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["oban","oper",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["obno","oper",[["osub","oper",[["li32","litr",["3"],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null]],null,null]],null,null],["cond","ctrl",[["oban","oper",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null],[["cond","ctrl",[["olor","oper",[["ooeq","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["2"],null,null]],null,null],["ooeq","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["3"],null,null]],null,null]],null,null],[["asgn","misc",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["obor","oper",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["osub","oper",[["li32","litr",["3"],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null]],null,null]]],null,null]]],null,null],["elif","ctrl",[["ooeq","oper",[["iden","iden",[],["Alive"],null],["li32","litr",["3"],null,null]],null,null],[["asgn","misc",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["obor","oper",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["osub","oper",[["li32","litr",["3"],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null]],null,null]]],null,null]]],null,null]]],null,null]]],null,null],["cal2","misc",[["stdf","stdl",[],["gx: <b>size</b>"],null],["omul","oper",[["iden","iden",[],["Width"],null],["iden","iden",[],["Scale"],null]],null,null],["omul","oper",[["iden","iden",[],["Height"],null],["iden","iden",[],["Scale"],null]],null,null]],null,null],["cal1","misc",[["stdf","stdl",[],["gx: <b>stroke_weight</b>"],null],["iden","iden",[],["Scale"],null]],null,null],["cal1","misc",[["stdf","stdl",[],["gx: <b>stroke</b>"],null],["lf32","litr",["1.0"],null,null]],null,null],["varv","decl",[["iden","iden",[],["MouseIsDown"],null],["li32","litr",["0"],null,null]],null,null],["whi1","ctrl",[[["cal1","misc",[["stdf","stdl",[],["time: <b>fps</b>"],null],["lf32","litr",["60.0"],null,null]],null,null],["cal1","misc",[["stdf","stdl",[],["gx: <b>background</b>"],null],["lf32","litr",["0.0"],null,null]],null,null],["forr","ctrl",[["iden","iden",[],["I"],null],["li32","litr",["0"],null,null],["iden","iden",[],["Height"],null],[["forr","ctrl",[["iden","iden",[],["J"],null],["li32","litr",["0"],null,null],["iden","iden",[],["Width"],null],[["cond","ctrl",[["oban","oper",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["I"],null],["iden","iden",[],["J"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null],[["cal2","misc",[["stdf","stdl",[],["gx: <b>point</b>"],null],["omul","oper",[["iden","iden",[],["J"],null],["iden","iden",[],["Scale"],null]],null,null],["omul","oper",[["iden","iden",[],["I"],null],["iden","iden",[],["Scale"],null]],null,null]],null,null]]],null,null]]],null,null]]],null,null],["cal0","misc",[["iden","iden",[],["Step"],null]],null,null],["asgn","misc",[["iden","iden",[],["Mask"],null],["osub","oper",[["li32","litr",["3"],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null],["varv","decl",[["iden","iden",[],["event"],null],["cal0","misc",[["stdf","stdl",[],["gx: <b>poll</b>"],null]],null,null]],null,null],["cond","ctrl",[["olor","oper",[["ooeq","oper",[["adot","misc",[["iden","iden",[],["type"],null],["iden","iden",[],["event"],null]],null,null],["stdd","stdl",[],["win: <b>MOUSE_PRESSED</b>"],null]],null,null],["olan","oper",[["ooeq","oper",[["adot","misc",[["iden","iden",[],["type"],null],["iden","iden",[],["event"],null]],null,null],["stdd","stdl",[],["win: <b>MOUSE_MOVED</b>"],null]],null,null],["iden","iden",[],["MouseIsDown"],null]],null,null]],null,null],[["asgn","misc",[["iden","iden",[],["MouseIsDown"],null],["li32","litr",["1"],null,null]],null,null],["vatv","decl",[["iden","iden",[],["J"],null],["ti32","type",[],null,null],["odiv","oper",[["adot","misc",[["iden","iden",[],["x"],null],["iden","iden",[],["event"],null]],null,null],["iden","iden",[],["Scale"],null]],null,null]],null,null],["vatv","decl",[["iden","iden",[],["I"],null],["ti32","type",[],null,null],["odiv","oper",[["adot","misc",[["iden","iden",[],["y"],null],["iden","iden",[],["event"],null]],null,null],["iden","iden",[],["Scale"],null]],null,null]],null,null],["forr","ctrl",[["iden","iden",[],["II"],null],["cast","misc",[["cal2","misc",[["stdf","stdl",[],["math: <b>max</b>"],null],["osub","oper",[["iden","iden",[],["I"],null],["li32","litr",["2"],null,null]],null,null],["li32","litr",["0"],null,null]],null,null],["ti32","type",[],null,null]],null,null],["cal2","misc",[["stdf","stdl",[],["math: <b>min</b>"],null],["oadd","oper",[["iden","iden",[],["I"],null],["li32","litr",["3"],null,null]],null,null],["iden","iden",[],["Height"],null]],null,null],[["forr","ctrl",[["iden","iden",[],["JJ"],null],["cast","misc",[["cal2","misc",[["stdf","stdl",[],["math: <b>max</b>"],null],["osub","oper",[["iden","iden",[],["J"],null],["li32","litr",["2"],null,null]],null,null],["li32","litr",["0"],null,null]],null,null],["ti32","type",[],null,null]],null,null],["cal2","misc",[["stdf","stdl",[],["math: <b>min</b>"],null],["oadd","oper",[["iden","iden",[],["J"],null],["li32","litr",["3"],null,null]],null,null],["iden","iden",[],["Width"],null]],null,null],[["cond","ctrl",[["oogt","oper",[["cal0","misc",[["stdf","stdl",[],["math: <b>random</b>"],null]],null,null],["lf32","litr",["0.5"],null,null]],null,null],[["asgn","misc",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["II"],null],["iden","iden",[],["JJ"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["obor","oper",[["subs","misc",[["lidx","litr",[[["iden","iden",[],["II"],null],["iden","iden",[],["JJ"],null]]],null,null],["iden","iden",[],["Board"],null]],null,null],["iden","iden",[],["Mask"],null]],null,null]],null,null]]],null,null]]],null,null]]],null,null]]],null,null],["elif","ctrl",[["ooeq","oper",[["adot","misc",[["iden","iden",[],["type"],null],["iden","iden",[],["event"],null]],null,null],["stdd","stdl",[],["win: <b>MOUSE_RELEASED</b>"],null]],null,null],[["asgn","misc",[["iden","iden",[],["MouseIsDown"],null],["li32","litr",["0"],null,null]],null,null]]],null,null]]],null,null]]],null,[21,12]]',
    '["prgm","ctrl",[[["varv","decl",[["iden","iden",[],["W"],null],["li32","litr",["64"],null,null]],null,null],["varv","decl",[["iden","iden",[],["H"],null],["li32","litr",["64"],null,null]],null,null],["func","decl",[["iden","iden",[],["F"],null],["ti32","type",[],null,null],[["argr","decl",[["iden","iden",[],["X"],null],["ti32","type",[],null,null]],null,null],["argr","decl",[["iden","iden",[],["Y"],null],["ti32","type",[],null,null]],null,null]],[["varv","decl",[["iden","iden",[],["Zx"],null],["cal1","misc",[["lfun","litr",[[[0,1],[0.5,1],[0.5,0],[1,0]]],null,null],["odiv","oper",[["cast","misc",[["iden","iden",[],["X"],null],["tf32","type",[],null,null]],null,null],["iden","iden",[],["W"],null]],null,null]],null,null]],null,null],["varv","decl",[["iden","iden",[],["Zy"],null],["cal1","misc",[["lfun","litr",[[[0,0],[0,1],[1,1],[1,0]]],null,null],["odiv","oper",[["cast","misc",[["iden","iden",[],["Y"],null],["tf32","type",[],null,null]],null,null],["iden","iden",[],["H"],null]],null,null]],null,null]],null,null],["retn","misc",[["omul","oper",[["omul","oper",[["iden","iden",[],["Zx"],null],["iden","iden",[],["Zy"],null]],null,null],["li32","litr",["255"],null,null]],null,null]],null,null]]],null,null],["varv","decl",[["iden","iden",[],["OrderedDitherMap"],null],["cast","misc",[["adot","misc",[["iden","iden",[],["data"],null],["cal1","misc",[["tmpl","decl",[["stdf","stdl",[],["exch: <b>decode</b>"],null],[["stdt","stdl",[],["exch: <b>JSON</b>"],null]]],null,null],["lstr","litr",["[  0,48,12,60, 3,51,15,63,\\n  32,16,44,28,35,19,47,31,\\n   8,56, 4,52,11,59, 7,55,\\n  40,24,36,20,43,27,39,23,\\n   2,50,14,62, 1,49,13,61,\\n  34,18,46,30,33,17,45,29,\\n  10,58, 6,54, 9,57, 5,53,\\n  42,26,38,22,41,25,37,21]"],null,null]],null,null]],null,null],["tlst","type",[["stdt","stdl",[],["exch: <b>JSON</b>"],null]],null,null]],null,null]],null,null],["varv","decl",[["iden","iden",[],["Scale"],null],["lf32","litr",["2.0"],null,null]],null,null],["func","decl",[["iden","iden",[],["Dither"],null],["tvod","type",[],null,null],[["argr","decl",[["iden","iden",[],["Pattern"],null],["tfun","type",[[["ti32","type",[],null,null],["ti32","type",[],null,null]],["ti32","type",[],null,null]],null,null]],null,null]],[["cal0","misc",[["stdf","stdl",[],["gx: <b>no_stroke</b>"],null]],null,null],["forr","ctrl",[["iden","iden",[],["Y"],null],["li32","litr",["0"],null,null],["iden","iden",[],["H"],null],[["forr","ctrl",[["iden","iden",[],["X"],null],["li32","litr",["0"],null,null],["iden","iden",[],["W"],null],[["varv","decl",[["iden","iden",[],["O"],null],["cal2","misc",[["iden","iden",[],["Pattern"],null],["iden","iden",[],["X"],null],["iden","iden",[],["Y"],null]],null,null]],null,null],["varv","decl",[["iden","iden",[],["D"],null],["cast","misc",[["adot","misc",[["iden","iden",[],["data"],null],["subs","misc",[["oadd","oper",[["omul","oper",[["omod","oper",[["iden","iden",[],["Y"],null],["li32","litr",["8"],null,null]],null,null],["li32","litr",["8"],null,null]],null,null],["omod","oper",[["iden","iden",[],["X"],null],["li32","litr",["8"],null,null]],null,null]],null,null],["iden","iden",[],["OrderedDitherMap"],null]],null,null]],null,null],["tf32","type",[],null,null]],null,null]],null,null],["cal1","misc",[["stdf","stdl",[],["gx: <b>fill</b>"],null],["oogt","oper",[["iden","iden",[],["O"],null],["omul","oper",[["iden","iden",[],["D"],null],["lf32","litr",["4.0"],null,null]],null,null]],null,null]],null,null],["call","misc",[["stdf","stdl",[],["gx: <b>rect</b>"],null],[["omul","oper",[["iden","iden",[],["X"],null],["iden","iden",[],["Scale"],null]],null,null],["omul","oper",[["iden","iden",[],["Y"],null],["iden","iden",[],["Scale"],null]],null,null],["iden","iden",[],["Scale"],null],["iden","iden",[],["Scale"],null]]],null,null]]],null,null]]],null,null]]],null,null],["cal2","misc",[["stdf","stdl",[],["gx: <b>size</b>"],null],["omul","oper",[["iden","iden",[],["W"],null],["iden","iden",[],["Scale"],null]],null,null],["omul","oper",[["iden","iden",[],["H"],null],["iden","iden",[],["Scale"],null]],null,null]],null,null],["cal1","misc",[["iden","iden",[],["Dither"],null],["iden","iden",[],["F"],null]],null,null],["whi1","ctrl",[[["cal0","misc",[["stdf","stdl",[],["gx: <b>poll</b>"],null]],null,null]]],null,null]]],null,[15,10]]',
  ]


  let zicnt = 0;
  let ID = 0;

  let Colormap = {
    ctrl:"turquoise",
    type:"royalblue",
    decl:"hotpink",
    oper:"mediumseagreen",
    misc:"mediumpurple",
    litr:"salmon",
    iden:"grey",
    stdl:"firebrick",
  }
  let Dmenu = document.createElement("div");
  Dmenu.style = "position:absolute;left:0px;top:0px;background:#171717;width:100%;height:24px;overflow:hidden";
  document.body.appendChild(Dmenu);

  let Dtabs = document.createElement("div");
  Dtabs.style = "position:absolute;left:0px;top:24px;background:#333;width:320px;height:55px;overflow:hidden";
  document.body.appendChild(Dtabs);
  let Dpalette = document.createElement("div");
  Dpalette.style = "position:absolute;left:0px;top:76px;background:#222;width:320px;height:calc(100% - 80px);overflow:scroll";
  document.body.appendChild(Dpalette)
  let Dprogram = document.createElement("div");
  Dprogram.style = "background:#333;position:absolute;left:320px;top:24px;border-left:1px solid black;border-right:1px solid black;width:calc(100% - 682px);height:calc(100% - 24px);overflow:scroll";
  document.body.appendChild(Dprogram)
  window.Dprogram = Dprogram;

  let Dout = document.createElement("div");
  Dout.style = "color:gainsboro;position:absolute;right:0px;top:24px;width:360px;height:70%;background:#333;"
  document.body.appendChild(Dout)

  let Dtext = document.createElement("textarea");
  Dtext.style = "color:gainsboro;white-space: pre;font-size:11px;border:none;border-top:1px solid black;position:absolute;right:0px;bottom:0px;width:360px;height:calc(30% - 24px);background:#222;"
  document.body.appendChild(Dtext)

  let btn_load = document.createElement("button");
  btn_load.classList.add("menubtn");
  btn_load.style = "position:absolute;left:0px;width:64px;"
  btn_load.innerHTML = "Load"
  Dmenu.appendChild(btn_load);
  btn_load.onclick = function(){
    upload_file(function(txt){
      btn_clear.onclick();
      deserialize_block(uncompact_serialized(JSON.parse(txt)));
      add_user_idens(uncompact_serialized(JSON.parse(txt)));
      Dout.innerHTML = "";
      Dtext.value = do_transpile();
      btn_run.onclick();
    })
  }

  let btn_save = document.createElement("button");
  btn_save.classList.add("menubtn");
  btn_save.style = "position:absolute;left:64px;width:64px;"
  btn_save.innerHTML = "Save"
  Dmenu.appendChild(btn_save);
  btn_save.onclick = function(){
    download_file("dither-blocks-program.json",
      JSON.stringify(
        compact_serialized(serialize_block(Blocks.filter(x=>(!x.parent && x.tag == 'prgm'))[0]))
      )
    );
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

  function upload_file(callback) {
    const fileInput = document.createElement('input');
    fileInput.type = 'file';
    fileInput.style.display = 'none';
    document.body.appendChild(fileInput);
    fileInput.addEventListener('change', () => {
      const file = fileInput.files[0];
      if (file) {
        const reader = new FileReader();
        reader.onload = (e) => {
          document.body.removeChild(fileInput);
          callback(e.target.result);
        };
        reader.readAsText(file);
      }
    });
    fileInput.click();

  }

  let EG_IDX = 0;

  let btn_eg = document.createElement("button");
  btn_eg.classList.add("menubtn");
  btn_eg.style = "position:absolute;left:128px;width:64px;"
  btn_eg.innerHTML = "Example"
  Dmenu.appendChild(btn_eg);
  btn_eg.onclick = function(){
    btn_clear.onclick();
    EG_IDX = (EG_IDX+1)%EXAMPLES.length;
    deserialize_block(uncompact_serialized(JSON.parse(EXAMPLES[EG_IDX])));
    add_user_idens(uncompact_serialized(JSON.parse(EXAMPLES[EG_IDX])));
    Dout.innerHTML = "";
    Dtext.value = do_transpile();
    btn_run.onclick();
  }

  let btn_run = document.createElement("button");
  btn_run.classList.add("menubtn");
  btn_run.style = "position:absolute;right:296px;width:64px;"
  btn_run.innerHTML = "Run"
  Dmenu.appendChild(btn_run);


  function compile_from_str(str){
    let fs = {
      readFileSync:function(x){
        // console.log(x);
        if (x == "CURRENT"){
          return new TextEncoder().encode(str);
        }
        if (x.startsWith("std/")){
          return new TextEncoder().encode(STD[x]);
        }else{
          return new TextEncoder().encode(EXAMPLES[x]);
        }
      }
    }
    let path = {
      resolve: (x)=>x,
      basename: (x)=>x.split('/').at(-1).split('.').slice(0,-1).join('.'),
      dirname: (x)=>x.split('/').slice(0,-1).join('/'),
      extname: (x)=>x.split('.').at(-1),
      join: (x,y)=>(x.length?(x+'/'+y):y),
      relative: (x,y)=>y,
    }
    let process = {
      cwd: ()=>'',
      exit: ()=>{throw "up"},
    }
    let parser = new PARSER({fs,path,process,search_paths:['']},{fragment:embed_glsl_frag});
    let toks = parser.tokenize("CURRENT");
    let cst = parser.parse(toks);
    let ast = parser.abstract(cst);
    let scopes = parser.infertypes(ast);
    let [instrs,layout] = parser.compile(ast,scopes);
    let ir = parser.writeir(instrs);
    let lo = parser.writelayout(layout);
    let srcmap = parser.writesrcmap(instrs);

    globalThis.__dh_intern_hooked_include = function(x){
      return STD[x];
    }
    let to_js = new TO_JS({preclude:1});
    let irlo = ir+"\n"+lo;

    let [pir,playout] = to_js.parse_ir(irlo);
    let jj = to_js.transpile(pir,playout);

    return jj;
  }

  btn_run.onclick = function(){
    Dout.innerHTML = "";
    Dtext.value = do_transpile();
    let jj = compile_from_str(Dtext.value);

    const iframe = document.createElement('iframe');

    iframe.style = "width:100%;height:100%;border:none;";
    
    Dout.appendChild(iframe);
    const iframeDoc = iframe.contentDocument || iframe.contentWindow.document;
    const htmlContent = `
      <body style="color:gainsboro;font-size:13px;font-family:monospace;margin:0px">${"<"}${"/"}body>
      <script>
      let last_call = performance.now();
      globalThis.__io_intern_hooked_print = async function(s){
        let div = document.createElement("span");
        div.innerHTML = s.replace(new RegExp("\\n", "g"),"<br>");
        document.body.appendChild(div);
        document.body.scrollTop = document.body.scrollHeight;
        if (performance.now() - last_call > 1000/60){
          await (function() {
            return new Promise(resolve => requestAnimationFrame(resolve));
          })();
          last_call = performance.now();
        }
      }
      globalThis.__io_intern_hooked_read_file = async function(pth){
        if (pth.startsWith("examples/assets/")){
          let blobUrl = await new Promise((resolve) => {
            window.addEventListener("message", function handleResponse(event) {
              if (event.data.type === "resource-response") {
                window.removeEventListener("message", handleResponse);
                resolve(event.data.value);
              }
            });
            window.parent.postMessage({type:"request-resource",value:pth}, "*");
          });
          const response = await fetch(blobUrl);            
          const arrayBuffer = await response.arrayBuffer();
          const byteArray = new Uint8Array(arrayBuffer);
          return Array.from(byteArray);
        }else{
          const response = await fetch(pth, { method: 'GET' });
          const arrayBuffer = await response.arrayBuffer();
          let arr = Array.from(new Uint8Array(arrayBuffer));
          return arr;
        }
      }
      ${jj}
      ${"<"}${"/"}script>
    `;
    iframeDoc.open();
    iframeDoc.write(htmlContent);
    iframeDoc.close();
  }

  let btn_stop = document.createElement("button");
  btn_stop.classList.add("menubtn");
  btn_stop.style = "position:absolute;right:232px;width:64px;"
  btn_stop.innerHTML = "Stop"
  Dmenu.appendChild(btn_stop);
  btn_stop.onclick = function(){
    Dout.innerHTML = "";
  }

  let btn_clear = document.createElement("button");
  btn_clear.classList.add("menubtn");
  btn_clear.style = "position:absolute;right:360px;width:64px;"
  btn_clear.innerHTML = "Clear"
  Dmenu.appendChild(btn_clear);
  btn_clear.onclick = function(){
    Blocks.splice(0,Infinity);
    Dprogram.innerHTML = "";
    Dtext.value = do_transpile();
  }

  function find_slot(me,x,y){
    let r = 8;
    for (let i = 0; i < Blocks.length; i++){
      Blocks[i].marked = false;
    }
    function mark(b){
      b.marked = true;
      for (let i = 0; i < b.slots.length; i++){
        if (b.slot_types[i] == 'L'){
          for (let j = 0; j < b.slots[i].length; j++){
            mark(b.slots[i][j]);
          }
        }else if (b.slot_types[i] == 'X' && b.slots[i]){
          mark(b.slots[i]);
        }
      }
    }
    mark(me);

    for (let i = 0; i < Blocks.length; i++){
      if (Blocks[i].marked) continue;
      if (Blocks[i].parent && Blocks[i].parent[0].slot_types[Blocks[i].parent[1]] == 'L' ){
        let rect = Blocks[i].elt.main.getBoundingClientRect();
        if (x > rect.left && x < rect.right && y > rect.bottom-r && y < rect.bottom + r){
          return [i,-1];
        }
      }
      for (let j = 0; j < Blocks[i].elt.slots.length; j++){
        let rect = Blocks[i].elt.slots[j].getBoundingClientRect();
        if (x > rect.left-r && x < rect.right+r && y > rect.top-r && y < rect.top+r*2){
          if ((Blocks[i].slot_types[j] == 'X' &&  Blocks[i].slots[j]==null) || Blocks[i].slot_types[j] == 'L'){
            return [i,j];
          }
        }
      }
    }
    return [-1,-1];
  }


  function no_highlight(){
    for (let i = 0; i < Blocks.length; i++){
      Blocks[i].elt.main.style.borderWidth = "0px 1px 1px 3px"; 
      for (let j = 0; j < Blocks[i].elt.slots.length; j++){
        Blocks[i].elt.slots[j].style.borderWidth = "1px 0px 0px 1px";
      }
    }
  }

  function pluck(that){
    that.elt.main.style.position = "absolute";
    that.elt.main.parentElement.removeChild(that.elt.main);
    Dprogram.appendChild(that.elt.main);

    if (that.prev) that.prev.next = that.next;
    if (that.next) that.next.prev = that.prev;

    if (that.parent){
      let pslot = that.parent[0].slots[that.parent[1]];
      if (that.parent[0].slot_types[that.parent[1]]=='L'){
        pslot.splice(pslot.indexOf(that),1);
      }else{
        that.parent[0].slots[that.parent[1]] = null;
      }
    }

    that.parent = null;
    that.next = null;
    that.prev = null;
  }
  function destroy(that){
    // console.log(that);
    // pluck(that);
    for (let i = 0; i < that.slots.length; i++){
      if (that.slot_types[i] == 'L'){
        that.slots[i].map(destroy)
      }else if (that.slot_types[i] == 'X' && that.slots[i]){
        destroy(that.slots[i]);
      }
    }
    Blocks.splice(Blocks.indexOf(that),1);
    that.elt.main.parentElement.removeChild(that.elt.main);
  }

  function reshape_checkbox_matrix(slot,nc,nr){
    while (slot.getElementsByTagName('div').length < nr){
      let d = document.createElement('div');
      d.style = "white-space:nowrap"
      slot.appendChild(d);
    }
    while (slot.getElementsByTagName('div').length > nr){
      slot.removeChild(slot.lastChild);
    }
    let rows = Array.from(slot.getElementsByTagName('div'));
    for (let i = 0; i < nr; i++){
      while (rows[i].getElementsByTagName('input').length < nc){
        let d = document.createElement("input");
        d.setAttribute("type","checkbox");
        d.style.margin = "0px";
        d.onmousedown = function(e){
          setTimeout(function(){
            Dtext.value = do_transpile();
          },10);
          e.stopPropagation();
        }
        d.onchange = function(e){
          setTimeout(function(){
            Dtext.value = do_transpile();
          },10);
        }
        rows[i].appendChild(d);
      }
      while (rows[i].getElementsByTagName('input').length > nc){
        rows[i].removeChild(rows[i].lastChild);
      }
    }
  }
  function render_bezier(slot){
    let cnt = slot.firstChild;
    let cs = Array.from(cnt.children);
    if (cs.length < 5){
      return;
    }
    function XY(e){
      return parseFloat(e.style.left)+","+parseFloat(e.style.top);
    }
    let [svg,p0,c0,p1,c1] = cs;
    let pth = svg.firstChild;
    let pcl = pth.nextSibling;
    let d = `M ${XY(p0)} C ${XY(c0)} ${XY(c1)} ${XY(p1)}`;
    let q = `M ${XY(p0)} L ${XY(c0)} M ${XY(p1)} L ${XY(c1)}`;
    pth.setAttribute("d",d);
    pcl.setAttribute("d",q);
  }
  function add_bezier_point(slot,x0,y0,x1,y1){
    let cnt = slot.firstChild;

    let pt = document.createElement("div");
    pt.style = `position:absolute;left:${x0}px;top:${y0}px;width:7px;height:7px;background:white;border: 1px solid black;border-radius:4px;cursor:move`
    let cc = document.createElement("div");
    cc.style = `position:absolute;left:${x1}px;top:${y1}px;width:7px;height:7px;background:white;border: 1px solid black;cursor:move`
    cnt.appendChild(pt);
    cnt.appendChild(cc);

    function render(){
      render_bezier(slot);
    }
    
    let drag = [0,0];
    pt.addEventListener("mousedown",function(e){
      drag[0] = 1;
      e.stopPropagation();
    })
    cc.addEventListener("mousedown",function(e){
      drag[1] = 1;
      e.stopPropagation();
    })
    document.addEventListener("mousemove",function(e){
      let r = pt.parentElement.getBoundingClientRect();
      if (drag[0]){
        let y = e.clientY - r.top-4;
        y = Math.min(Math.max(y,0),100);
        pt.style.top = y+"px";
      }else if (drag[1]){
        let x = e.clientX - r.left-4;
        let y = e.clientY - r.top-4;
        x = Math.min(Math.max(x,0),100);
        y = Math.min(Math.max(y,0),100);
        cc.style.left = (x)+"px";
        cc.style.top = (y)+"px";
      }
      render();
    });
    document.addEventListener("mouseup",function(e){
      drag[0] = 0;
      drag[1] = 0;
      render();
    });
    render();
  }

  function make_block(cfg){
    let div = document.createElement("div");
    let that = Object.assign({
      is_template:false,
      is_dragging:false,
      drag_x:0,
      drag_y:0,
      x:0,y:0,
    },cfg,{
      elt:{
        main:div,
        slots:[],
      },
      id:ID++,
      next:null,
      prev:null,
      parent:null,
    });
    

    if (!that.slots) that.slots = [];
    if (!that.slots.length){
      // console.log(that);
      for (let i = 0; i < that.slot_types.length; i++){
        if (that.slot_types[i] == 'L'){
          that.slots.push([])
        }else{
          that.slots.push(null)
        }
      }
    }

    div.style="color:gainsboro;display:block;font-family:sans-serif;font-size:12px;min-width:32px;min-height:16px;border:1px solid black;border-radius:2px 3px 3px 2px;cursor:grab;-webkit-user-select: none;-ms-user-select: none;user-select: none;padding-top:2px;padding-left:1px";
    div.style.borderWidth = "0px 1px 1px 0px"
    div.style.borderLeft = "3px solid "+Colormap[cfg.page]
    div.style.position = 'absolute';
    div.style.background = "#444";
    // div.style.color = Colormap[cfg.page];
    div.style.left = that.x+'px';
    div.style.top = that.y+'px';


    // div.innerHTML = `<div style="font-size:8px;color:dimgray;position:relative;"><div style="position:absolute;right:0px;top:-2px">#${that.id}</div></div>`;

    
    if (cfg.is_template){
      div.addEventListener("mousedown", function(e) {
        that.is_dragging = true;
        let rect = div.getBoundingClientRect();
        that.drag_x = e.clientX - rect.left;
        that.drag_y = e.clientY - rect.top;
        e.preventDefault();
      });
      document.addEventListener("mousemove", function(e) {
        if (that.is_dragging) {
          let other = make_block(Object.assign({},JSON.parse(JSON.stringify(Object.assign({},that,{elt:null}))),{
            is_template:false,
            x: div.getBoundingClientRect().left,
            y: div.getBoundingClientRect().top,
          }));
          
          document.body.appendChild(other.elt.main);

          other.elt.main.style.zIndex = ++zicnt;
          Blocks.push(other);
          that.is_dragging = false;
        }
      });
    }else{
      div.addEventListener("mousedown", function(e) {
        let [x,y] = [e.clientX, e.clientY];
        let r = div.getBoundingClientRect();
        // console.log(r);
        that.is_dragging = true;

        that.drag_x = x - r.left;
        that.drag_y = y- r.top;

        div.style.left = (x - that.drag_x - Dprogram.offsetLeft + Dprogram.scrollLeft) + "px";
        div.style.top = (y - that.drag_y -  Dprogram.offsetTop  + Dprogram.scrollTop ) + "px";
        // console.log(div.style.left,div.style.top);
        // console.log(e.clientX,e.clientY,r.left,r.top, e.clientX-r.left, e.clientY-r.top);
        div.style.zIndex = ++zicnt;

        if (that.parent){
          if (that.next){

            if (that.prev) that.prev.next = null;
            that.prev = null;

            let pslot = that.parent[0].slots[that.parent[1]];

            let drop = pslot.splice(pslot.indexOf(that),Infinity);

            let par = that.parent;
            if (par[0].tag == "snip"){
              if (par[0].slot_types[par[1]] == 'X' || (par[0].slot_types[par[1]] == 'L' && par[0].slots[par[1]].length == 0)){ 
                pluck(par[0]);
                Blocks.splice(Blocks.indexOf(par[0]),1);
                par[0].elt.main.parentElement.removeChild(par[0].elt.main);
              }
            }
            
            let other = make_block({

              drag_x:that.drag_x,drag_y:that.drag_y,
              tag:'snip',
              page:'ctrl',
              texts:['',''],
              slot_types:['L'],
            });
            other.slots[0] = drop;
            other.elt.main.style.left = div.style.left;
            other.elt.main.style.top = div.style.top;

            Dprogram.appendChild(other.elt.main);
            other.elt.main.style.zIndex = ++zicnt;
            Blocks.push(other);
            that.is_dragging = false;
            
            drop.map(x=>(x.parent=[other,0]));
            drop.forEach(node=>{
              node.parent = [other,0];
              other.elt.slots[0].appendChild(node.elt.main);
              node.elt.main.style.position = "static";
            })
            
            other.is_dragging = true;

          }else{
            let par = that.parent;
            pluck(that);
    
            if (par[0].tag == "snip"){
              if (par[0].slot_types[par[1]] == 'X' || (par[0].slot_types[par[1]] == 'L' && par[0].slots[par[1]].length == 0)){ 
                pluck(par[0]);
                Blocks.splice(Blocks.indexOf(par[0]),1);
                par[0].elt.main.parentElement.removeChild(par[0].elt.main);
              }
            }
            
          }


        }
        


        
        e.stopPropagation();
        

      });
      document.addEventListener("mousemove", function(e) {
        let [x,y] = [e.clientX, e.clientY];
        // console.log(x,y)
        if (that.is_dragging) {
          div.style.left = (x - that.drag_x - div.parentElement.offsetLeft +  div.parentElement.scrollLeft) + "px";
          div.style.top = (y - that.drag_y -  div.parentElement.offsetTop   + div.parentElement.scrollTop ) + "px";
          div.style.opacity = "0.75"
          div.style.boxShadow = "2px 2px 2px rgba(0,0,0,0.3)"
          
          no_highlight();

          let [i,j] = find_slot(that,x,y);
          if (i != -1){
            if (j == -1){
              Blocks[i].elt.main.style.borderWidth = "0px 1px 4px 3px";
            }else{
              Blocks[i].elt.slots[j].style.borderWidth= "4px 0px 0px 1px";
            }
          }
        }
      });
      
      document.addEventListener("mouseup", function(e) {
        let [x,y] = [e.clientX, e.clientY];
        if (!that.is_dragging) return;

        if (div.parentElement = document.body){
          Dprogram.appendChild(div);
        }
        div.style.left = (x - that.drag_x - div.parentElement.offsetLeft +  div.parentElement.scrollLeft) + "px";
        div.style.top = (y - that.drag_y -  div.parentElement.offsetTop   + div.parentElement.scrollTop ) + "px";

        if (div.parentElement == Dprogram){
          if (x-div.parentElement.offsetLeft+div.parentElement.scrollLeft<0){
            destroy(that);
          }
        }

        div.style.opacity = "1"
        div.style.boxShadow = "none"
        that.is_dragging = false;
        
        let [i,j] = find_slot(that,x,y);
        no_highlight();

        if ( i != -1){
          if (j == -1){
            let L = Blocks[i];
            let R = Blocks[i].next;
            if (R) R.prev = that;
            if (L) L.next = that;
            that.prev = L;
            that.next = R;
            that.parent = L.parent;
            that.elt.main.parentElement.removeChild(that.elt.main);
            let pslot = that.parent[0].slots[that.parent[1]];

            pslot.splice(pslot.indexOf(that.prev)+1,0,that);
            
            if (that.next){
              that.next.elt.main.parentElement.insertBefore(that.elt.main, that.next.elt.main)
            }else{
              that.prev.elt.main.parentElement.appendChild(that.elt.main)
            }

            that.elt.main.style.position = "static";
            
          }else{
            if (Blocks[i].slot_types[j] == 'L'){
              if (Blocks[i].slots[j][0]){
                Blocks[i].slots[j][0].prev = that;
                that.next = Blocks[i].slots[j][0];
              }
              Blocks[i].slots[j].unshift(that);
              
            }else{
              Blocks[i].slots[j] = that;
            }
            that.parent = [Blocks[i],j];
            that.elt.main.parentElement.removeChild(that.elt.main);
            Blocks[i].elt.slots[j].prepend(that.elt.main);
            that.elt.main.style.position = "static";

          }
          Blocks.unshift(...Blocks.splice(Blocks.indexOf(that),1));

          if (that.tag == "snip"){
            let par = that.parent;
            if (par[0].slot_types[par[1]] == 'L'){
              for (let k = 0; k < that.slots[0].length; k++){
                let node = that.slots[0][k];
                if (k == 0){
                  node.prev = that.prev;
                  if (node.prev) node.prev.next = node;
                  
                }
                that.prev = node;
                node.parent = par;
    
                par[0].slots[par[1]].splice(par[0].slots[par[1]].indexOf(that),0,node);
                
                that.elt.slots[0].removeChild(node.elt.main);
                that.elt.main.parentElement.insertBefore(node.elt.main, that.elt.main);
              }
              pluck(that);
              Blocks.splice(Blocks.indexOf(that),1);
              that.elt.main.parentElement.removeChild(that.elt.main);
            }
          }

        }

        e.stopPropagation();
        setTimeout(function(){
          Dtext.value = do_transpile();
        },10);
      });
    }
    
    for (let i = 0; i < that.texts.length; i++){
      let span = document.createElement("span");
      span.innerHTML = that.texts[i];
      span.style = "margin:3px;"
      if (that.texts[i].length){
        div.appendChild(span);
      }
      if (i == that.texts.length-1){
        break;
      }
      let slot; 

      if ('XLRB'.includes(that.slot_types[i])){
        slot = document.createElement("span");
      }else if ('NI'.includes(that.slot_types[i])){
        slot = document.createElement("input");
      }else if ('S'.includes(that.slot_types[i])){
        slot = document.createElement("textarea");
      }else if ('F'.includes(that.slot_types[i])){
        slot = document.createElement("div");
      }
      slot.style="color:gainsboro;min-width:32px;min-height:16px;background:#333;border:1px solid black;border-radius:3px;vertical-align: middle;"
      slot.style.borderWidth="1px 0px 0px 1px"
      // slot.style.margin = "3px 3px 3px 3px";
      slot.style.margin = "0px 2px 2px 2px";
      if (that.slot_types[i] == 'X'){
        slot.style.display = "inline-block";
        
      }else if (that.slot_types[i] == 'L'){
        slot.style.display = "block";
        slot.style.marginLeft = "6px";
      }else if ('NI'.includes(that.slot_types[i])){
        
        slot.style.display = "inline-block";
        slot.setAttribute("type","number");
        
        if (that.slot_types[i] == 'N'){
          slot.setAttribute("step","0.1");
          slot.value = "0.0"
        }else{
          slot.value = "0"
        }
        slot.style.width="64px";
        
      }else if (that.slot_types[i] == 'S'){
        slot.style.display = "inline-block";
      }else if (that.slot_types[i] == 'R'){
        

        let sl = document.createElement("input");
        sl.setAttribute("type","range");
        sl.value = "50"
        sl.style="width:100px;";
        let lb = document.createElement("div");
        lb.style="width:104px;text-align:center;"
        
        slot.style = "display:inline-block;width:105px;vertical-align: middle;"

        slot.appendChild(lb);
        slot.appendChild(sl);
        
        let is_int = that.slot_types[i-1] == 'I';

        function linklbl(){
          let nl = slot.previousSibling;
          let nr = slot.nextSibling;
          nl.addEventListener('change',setlbl);
          nr.addEventListener('change',setlbl);
        }

        function setlbl(){
          let nl = slot.previousSibling;
          let nr = slot.nextSibling;
          let t = Number(sl.value)/100;
          let a = Number(nl.value);
          let b = Number(nr.value);
          let v = a * (1-t) + b * t;

          function formatThreeSigAfterDecimal(num) {
            const EPS = Number.EPSILON * Math.max(1, Math.abs(num));
            if (Math.abs(num - Math.round(num)) < EPS) {
              return Math.round(num);
            }
            const decimalPart = num.toString().split('.')[1];
            if (!decimalPart || decimalPart.replace(/^0+/, '').length <= 3) {
              return num;
            }
            let intPartLength = Math.trunc(num).toString().length;
            let totalSigDigits = intPartLength + 3;
            return Number(num.toPrecision(totalSigDigits));
          }
          if (is_int){
            lb.innerHTML = Math.round(v);
          }else{
            lb.innerHTML = formatThreeSigAfterDecimal(v);
          }

        }

        setTimeout(setlbl,10);
        setTimeout(linklbl,10);

        sl.addEventListener("input",setlbl)

        sl.onmousedown = function(e){
          setTimeout(function(){
            Dtext.value = do_transpile();
          },10);
          e.stopPropagation();
        }
        sl.onchange = function(e){
          setTimeout(function(){
            Dtext.value = do_transpile();
          },10);
        }

      }else if (that.slot_types[i] == 'B'){
        slot.style = "";
        slot.style.display = "block";
        let nh = div.lastChild;
        let nw = nh.previousSibling.previousSibling;

        function reshape(){
          let nr = Number(nh.value);
          let nc = Number(nw.value);
          reshape_checkbox_matrix(slot,nc,nr);
        }
        nw.addEventListener('change',reshape);
        nh.addEventListener('change',reshape);

      }else if (that.slot_types[i] == 'F'){
        slot.style = "";
        slot.style.display = "block";
        let cnt = document.createElement("div");
        cnt.style = "position:relative;width:100px;height:100px;margin:4px;padding:4px;overflow:hidden;cursor:crosshair;";
        slot.appendChild(cnt);

        let svg = document.createElementNS("http://www.w3.org/2000/svg","svg");
        svg.setAttribute("width",100);
        svg.setAttribute("height",100);
        let pth = document.createElementNS("http://www.w3.org/2000/svg","path");
        pth.setAttribute("fill","none");
        pth.setAttribute("stroke","gainsboro");
        pth.setAttribute("stroke-width","2");
        pth.setAttribute("d","");

        let pcl = document.createElementNS("http://www.w3.org/2000/svg","path");
        pcl.setAttribute("fill","none");
        pcl.setAttribute("stroke","gainsboro");
        pcl.setAttribute("stroke-width","1");
        pcl.setAttribute("stroke-dasharray","2");
        pcl.setAttribute("d","");

        svg.style="border:1px solid black; border-radius:2px; pointer-events:none";
        svg.appendChild(pth);
        svg.appendChild(pcl);
        cnt.appendChild(svg);

        add_bezier_point(slot,0,100,50,100);
        add_bezier_point(slot,100,0,50,0);

        cnt.onmousedown = function(e){
          e.stopPropagation();
        }
      }

      if ('NIS'.includes(that.slot_types[i])){
        slot.onmousedown = function(e){
          setTimeout(function(){
            Dtext.value = do_transpile();
          },10);
          e.stopPropagation();
        }
        slot.onchange = function(e){
          setTimeout(function(){
            Dtext.value = do_transpile();
          },10);
        }
      }
      that.elt.slots.push(slot);
      div.appendChild(slot);
    }




    for (let i = 0; i < that.slots.length; i++){
      if (that.slot_types[i] == 'L'){
        let prev = null;
        for (let j = 0; j < that.slots[i].length; j++){
          let other = make_block(that.slots[i][j]);
          if (prev) prev.next = other;
          other.prev = prev;
          other.parent = [that,i];
          Blocks.push(other);
          other.elt.main.style.position = "static";
          that.elt.slots[i].appendChild(other.elt.main);
          that.slots[i][j] = other;
          prev = other;
        }
      }else if (that.slot_types[i] == 'X' && that.slots[i]){
        let other = make_block(that.slots[i]);
        other.parent = [that,i];
        Blocks.push(other);
        other.elt.main.style.position = "static";
        that.elt.slots[i].appendChild(other.elt.main);
        that.slots[i] = other;
      }else if ("NIS".includes(that.slot_types[i]) && that.slots[i] !== null){
        that.elt.slots[i].value = that.slots[i];
      }else if (that.slot_types[i] == 'R' && that.slots[i] !== null){
        that.elt.slots[i].firstChild.nextSibling.value = that.slots[i];
      }else if (that.slot_types[i] == 'B' && that.slots[i] !== null){
        reshape_checkbox_matrix(that.elt.slots[i],(that.slots[i][0]??[]).length,that.slots[i].length);

        let rows = Array.from(that.elt.slots[i].getElementsByTagName('div'));
        for (let y = 0; y < rows.length; y++){
          let cols = Array.from(rows[y].getElementsByTagName('input'));
          for (let x = 0; x < cols.length; x++){
            cols[x].checked = that.slots[i][y][x]!=0;
          }
        }
      }else if (that.slot_types[i] == 'F' && that.slots[i] !== null){
        let [svg,p0,c0,p1,c1] = that.elt.slots[i].firstChild.children;
        p0.style.left = (that.slots[i][0][0]*100)+"px";
        p0.style.top  = (that.slots[i][0][1]*100)+"px";
        c0.style.left = (that.slots[i][1][0]*100)+"px";
        c0.style.top  = (that.slots[i][1][1]*100)+"px";
        p1.style.left = (that.slots[i][3][0]*100)+"px";
        p1.style.top  = (that.slots[i][3][1]*100)+"px";
        c1.style.left = (that.slots[i][2][0]*100)+"px";
        c1.style.top  = (that.slots[i][2][1]*100)+"px";
        render_bezier(that.elt.slots[i]);
      }
    }

    return that;
  }

  function serialize_block(b){
    // console.log(b)
    let o = {
      tag: b.tag,
      page: b.page,
      texts: b.texts,
      slot_types: b.slot_types.slice(),
      x: b.elt.main.offsetLeft,
      y: b.elt.main.offsetTop,
      slots: [],
    }
    for (let i = 0; i < b.slots.length; i++){
      if (b.slot_types[i] == 'L'){
        o.slots.push([])
        for (let j = 0; j < b.slots[i].length; j++){
          o.slots[i].push(serialize_block(b.slots[i][j]));
        }
      }else if (b.slot_types[i] == 'X'){
        if (b.slots[i] == null){
          o.slots.push(null);
        }else{
          o.slots.push(serialize_block(b.slots[i]));
        }
      }else if ("NIS".includes(b.slot_types[i])){

        o.slots.push(b.elt.slots[i].value);
      }else if (b.slot_types[i] == 'R'){
        // console.log(b.elt.slots[i])
        // console.log(b.elt.slots[i].firstChild)
        // console.log(b.elt.slots[i].firstChild.nextSibling)
        o.slots.push(b.elt.slots[i].firstChild.nextSibling.value);
      }else if (b.slot_types[i] == 'B'){
        // o.slots.push(Array.from(b.elt.slots[i].getElementsByTagName("input")).map(x=>Number(x.checked)));
        o.slots.push(Array.from(b.elt.slots[i].getElementsByTagName("div")).map(x=>Array.from(x.getElementsByTagName("input")).map(y=>Number(y.checked))));
      }else if (b.slot_types[i] == 'F'){
        let [svg,p0,c0,p1,c1] = b.elt.slots[i].firstChild.children;
        o.slots.push([
          [parseFloat(p0.style.left)/100,parseFloat(p0.style.top)/100],
          [parseFloat(c0.style.left)/100,parseFloat(c0.style.top)/100],
          [parseFloat(c1.style.left)/100,parseFloat(c1.style.top)/100],
          [parseFloat(p1.style.left)/100,parseFloat(p1.style.top)/100],
        ])
      }else{
        o.slots.push(null);
      }
    }
    return o;
  }
  window.serialize_block = serialize_block;

  function deserialize_block(b){
    let block = make_block(b);
    Blocks.push(block);
    Dprogram.appendChild(block.elt.main);
  }

  function get_block_by_id(id){
    return Blocks.filter(x=>x.id==id)[0];
  }

  let NL = `<br><span style="display:inline-block;width:3px"></span>`;

  let Templates = [
    {page:"ctrl",tag:'snip',texts:['',''],slot_types:['L']},
    {page:"ctrl",tag:'prgm',texts:['<b>Program</b>','End'],slot_types:['L']},
    {page:"ctrl",tag:'cond',texts:['<b>If</b>','then',''],slot_types:['X','L'],},
    {page:"ctrl",tag:'elif',texts:['<b>Else if</b>','then',''],slot_types:['X','L'],},
    {page:"ctrl",tag:'else',texts:['<b>Else</b>',''],slot_types:['L'],},
    {page:"ctrl",tag:'whil',texts:['Repeat <b>while</b>','do','end'],slot_types:['X','L'],},
    {page:"ctrl",tag:'whi1',texts:['Repeat <b>forever</b>','end'],slot_types:['L'],},
    {page:"ctrl",tag:'forr',texts:['Repeat <b>for</b>','from','till','do','end'],slot_types:['X','X','X','L'],},
    {page:"ctrl",tag:'forl',texts:['First do',`then repeat${NL}Before each time check`,`${NL}After each time`,`${NL}<b>For</b> each time do`,'end'],slot_types:['X','X','X','L'],},
    {page:"ctrl",tag:'brek',texts:['<b>Break</b> out of the loop'],slot_types:[],},
    {page:"ctrl",tag:'cont',texts:['<b>Continue</b> to the next iteration'],slot_types:[],},
    {page:"decl",tag:'func',texts:['Define <b>function</b>','that returns a','and takes','Begin','End'],slot_types:['X','X','L','L'],},
    {page:"decl",tag:'argr',texts:['<b>Argument</b>','of type',''],slot_types:['X','X'],},
    {page:"decl",tag:'vart',texts:['Declare <b>variable</b>','of type',''],slot_types:['X','X'],},
    {page:"decl",tag:'varv',texts:['Declare <b>variable</b>','of value',''],slot_types:['X','X'],},
    {page:"decl",tag:'vatv',texts:['Declare <b>variable</b>','of type','and value',''],slot_types:['X','X','X'],},
    {page:"decl",tag:'tmpl',texts:['Named','with <b>generic</b> types',''],slot_types:['X','L'],},
    {page:"decl",tag:'typd',texts:['Define <b>type</b>','','end'],slot_types:['X','L'],},
    {page:"type",tag:'tvod',texts:['<b>Nothing</b>'],slot_types:[],},
    {page:"type",tag:'ti32',texts:['<b>Integer</b>'],slot_types:[],},
    {page:"type",tag:'tf32',texts:['<b>Number</b>'],slot_types:[],},
    {page:"type",tag:'tu08',texts:['<b>Byte<b>'],slot_types:[],},
    {page:"type",tag:'tstr',texts:['<b>Text</b>'],slot_types:[],},
    {page:"type",tag:'tvec',texts:['','-D <b>vector</b> of type',''],slot_types:['I','X'],},
    {page:"type",tag:'tv3f',texts:['3D <b>vector</b>'],slot_types:[],},
    {page:"type",tag:'tv2f',texts:['2D <b>vector</b>'],slot_types:[],},
    {page:"type",tag:'tlst',texts:['<b>List</b> of type',''],slot_types:['X'],},
    {page:"type",tag:'tarr',texts:['','-D <b>array</b> of type',''],slot_types:['I','X'],},
    {page:"type",tag:'tdic',texts:['<b>Dictionary</b> from','to',''],slot_types:['X','X'],},
    {page:"type",tag:'tuon',texts:['<b>Union</b> of types',''],slot_types:['L'],},
    {page:"type",tag:'ttup',texts:['<b>Tuple</b> of',''],slot_types:['L'],},
    {page:"type",tag:'tfun',texts:['<b>Function</b> from','to',''],slot_types:['L','X'],},
    {page:"type",tag:'ttmt',texts:['Generic type <b>T</b>'],slot_types:[],},
    {page:"type",tag:'ttmu',texts:['Generic type <b>U</b>'],slot_types:[],},
    {page:"type",tag:'ttmv',texts:['Generic type <b>V</b>'],slot_types:[],},
    {page:"oper",tag:'oadd',texts:['','<b>plus</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'osub',texts:['','<b>minus</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'omul',texts:['','<b>times</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'odiv',texts:['','<b>divided</b> by',''],slot_types:['X','X'],},
    {page:"oper",tag:'omod',texts:['<b>remainder</b> of','divided by',''],slot_types:['X','X'],},
    {page:"oper",tag:'opow',texts:['','to the <b>power</b> of',''],slot_types:['X','X'],},
    {page:"oper",tag:'ooeq',texts:['','<b>equals</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'oneq',texts:['','does <b>not equal</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'oolt',texts:['','is <b>less</b> than',''],slot_types:['X','X'],},
    {page:"oper",tag:'oogt',texts:['','is <b>greater</b> than',''],slot_types:['X','X'],},
    {page:"oper",tag:'oleq',texts:['','is <b>at most</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'ogeq',texts:['','is <b>at least</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'olan',texts:['','<b>and</b> also',''],slot_types:['X','X'],},
    {page:"oper",tag:'olor',texts:['','<b>or</b> else',''],slot_types:['X','X'],},
    {page:"oper",tag:'olno',texts:['logically <b>not</b>',''],slot_types:['X'],},
    {page:"oper",tag:'ogeq',texts:['','is <b>at least</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'ocat',texts:['The following texts <b>joined</b>',''],slot_types:['L'],},
    {page:"oper",tag:'oban',texts:['bits of','<b>and</b> bits of',''],slot_types:['X','X'],},
    {page:"oper",tag:'obor',texts:['bits of','<b>or</b> bits of' ,''],slot_types:['X','X'],},
    {page:"oper",tag:'obno',texts:['','with bits <b>flipped</b>'],slot_types:['X'],},
    {page:"oper",tag:'obxo',texts:['bits of','that <b>differ</b> from',''],slot_types:['X','X'],},
    {page:"oper",tag:'obsl',texts:['','with bits <b>left shifted</b> by',''],slot_types:['X','X'],},
    {page:"oper",tag:'obsr',texts:['','with bits <b>right shifted</b> by',''],slot_types:['X','X'],},
    {page:"litr",tag:'lstr',texts:['Text "','"'],slot_types:['S'],},
    {page:"litr",tag:'lf32',texts:['Number',''],slot_types:['N'],},
    {page:"litr",tag:'lfsl',texts:['Number','','',''],slot_types:['N','R','N'],},
    {page:"litr",tag:'lisl',texts:['Integer','','',''],slot_types:['I','R','I'],},
    {page:"litr",tag:'li32',texts:['Integer',''],slot_types:['I'],},
    {page:"litr",tag:'lu08',texts:['Byte',''],slot_types:['I'],},
    {page:"litr",tag:'lvec',texts:['Vector',''],slot_types:['L'],},
    {page:"litr",tag:'llst',texts:['List of','',''],slot_types:['X','L'],},
    {page:"litr",tag:'lar0',texts:['Empty'],slot_types:[],},
    {page:"litr",tag:'ltup',texts:['Tuple',''],slot_types:['L'],},
    {page:"litr",tag:'ldic',texts:['Dictionary',''],slot_types:['L'],},
    {page:"litr",tag:'lkvp',texts:['Entry with <b>key</b>','and <b>value</b>',''],slot_types:['X','X'],},
    {page:"litr",tag:'lidx',texts:['Dimensional index',''],slot_types:['L'],},
    {page:"litr",tag:'lbmp',texts:['Bitmap','x','',''],slot_types:['I','I','B'],},
    {page:"litr",tag:'lfun',texts:[`Shaping function`,''],slot_types:['F'],},
    {page:"litr",tag:'lcmt',texts:['Comment ',''],slot_types:['S'],},
    {page:"misc",tag:'asgn',texts:['<b>Set</b>','to',''],slot_types:['X','X'],},
    {page:"misc",tag:'retn',texts:['<b>Return</b>',''],slot_types:['X'],},
    {page:"misc",tag:'ret0',texts:['<b>Return</b> nothing'],slot_types:[],},
    {page:"misc",tag:'subs',texts:['The','-th <b>item</b> of',''],slot_types:['X','X'],},
    {page:"misc",tag:'subs',texts:['<b>Entry</b>','of',''],slot_types:['X','X'],},
    {page:"misc",tag:'adot',texts:['<b>Attribute</b>','of',''],slot_types:['X','X'],},
    {page:"misc",tag:'subx',texts:['<b>X</b> component of',''],slot_types:['X'],},
    {page:"misc",tag:'suby',texts:['<b>Y</b> component of',''],slot_types:['X'],},
    {page:"misc",tag:'subz',texts:['<b>Z</b> component of',''],slot_types:['X'],},
    {page:"misc",tag:'cal0',texts:['<b>Call</b> function',''],slot_types:['X'],},
    {page:"misc",tag:'cal1',texts:['<b>Call</b> function','with',''],slot_types:['X','X'],},
    {page:"misc",tag:'cal2',texts:['<b>Call</b> function','with','and',''],slot_types:['X','X','X'],},
    {page:"misc",tag:'call',texts:['<b>Call</b> function','with arguments',''],slot_types:['X','L'],},
    {page:"misc",tag:'cast',texts:['','converted to <b>type</b>',''],slot_types:['X','X'],},
  ];

  for (let i = 0; i < 26; i++){
    Templates.push({page:"iden",tag:'iden',texts:[String.fromCharCode(65+i)],slot_types:[],});
  }

  for (let i = 0; i < Std_idens.length; i++){
    Templates.push({page:"stdl",tag:'std'+Std_idens[i][2][0],texts:[Std_idens[i][0]+': <b>'+Std_idens[i][1]+'</b>'],slot_types:[],});
  }

  let Tabs = {
    "ctrl":{name:"Control"},
    "decl":{name:"Declarations"},
    "type":{name:"Types"},
    "oper":{name:"Operations"},
    "litr":{name:"Literals"},
    "misc":{name:"Misc"},
    "iden":{name:"Identifiers"},
    "stdl":{name:"Standard Library"},
  }

  for (let k in Tabs){
    let btn = document.createElement("button");
    btn.style = "color:gainsboro;height:21.5px;margin:2px;font-size:13px;border:1px solid black;border-radius: 2px;background:#444;cursor:pointer"
    btn.style.borderTop = `3px solid ${Colormap[k]}`
    // btn.innerHTML = `<span><span style="display:inline-block;width:8px;height:8px;border-radius:8px;vertical-align:middle;background:${Colormap[k]}"></span>&nbsp;${Tabs[k].name}</span>`;
    btn.innerHTML = `${Tabs[k].name}`;
  
    btn.onclick = function(){
      let tems = Templates.filter(x=>(x.page == k));
      Dpalette.innerHTML = "";
      if (k == "iden"){
        let inp = document.createElement("input");
        inp.style = "margin:5px;filter:invert(80%);"
        Dpalette.appendChild(inp);
        let btm = document.createElement("button");
        btm.style="filter:invert(80%);"
        btm.innerHTML = "Add"
        Dpalette.appendChild(btm);
        Dpalette.appendChild(document.createElement("br"))
        function add(){
          let val = inp.value.replace(/[^a-zA-Z0-9]/g, "_");
          let tem = {page:"iden",tag:'iden',texts:[val],slot_types:[],}
          Templates.push(tem);
          let b = make_block(Object.assign({},tem,{is_template:true,x:0,y:0}));
          b.elt.main.style.position = "static";
          b.elt.main.style.display = "inline-block";
          b.elt.main.style.margin = "5px";
          b.elt.main.style.verticalAlign = "middle";
          Dpalette.appendChild(b.elt.main);

          inp.value = "";
        }
        btm.onclick = add;
        inp.onkeyup = function(e){
          if (e.key == 'Enter'){
            add();
          }
        }

      }
      for (let i = 0; i < tems.length; i++){
        let b = make_block(Object.assign({},tems[i],{is_template:true,x:0,y:0}));
        b.elt.main.style.position = "static";
        b.elt.main.style.display = "inline-block";
        b.elt.main.style.margin = "5px";
        b.elt.main.style.verticalAlign = "middle";
        Dpalette.appendChild(b.elt.main);
      }

    }
    Tabs[k].button = btn;  
    Dtabs.appendChild(btn);
  }


  function formatter(text, indentSize = 2) {
    const result = [];
    let indentLevel = 0;
    let i = 0;
    let inQuote = false;
    let quoteChar = '';
    let escaped = false;
    let currentLine = '';
    const flushLine = () => {
      if (currentLine.trim()) {
        result.push(' '.repeat(indentLevel * indentSize) + currentLine.trim());
      }
      currentLine = '';
    };
    while (i < text.length) {
      const char = text[i];
      if (inQuote) {
        currentLine += char;
        if (escaped) {
          escaped = false;
        } else if (char === '\\') {
          escaped = true;
        } else if (char === quoteChar) {
          inQuote = false;
        }
      } else {
        if (char === '"' || char === "'") {
          inQuote = true;
          quoteChar = char;
          currentLine += char;
        } else if (char === '{') {
          currentLine += ' {';
          flushLine();
          indentLevel++;
        } else if (char === '}') {
          flushLine();
          indentLevel = Math.max(indentLevel - 1, 0);
          result.push(' '.repeat(indentLevel * indentSize) + '}');
        } else if (char === ';'){
          if (currentLine.length){
            currentLine += char;
            flushLine();
          }
        } else if (char === '\n') {
          flushLine();
        } else {
          currentLine += char;
        }
      }
      i++;
    }
    flushLine();
    return result.join('\n');
  }


  
  deserialize_block(uncompact_serialized(JSON.parse(EXAMPLES[EG_IDX])));

  Tabs.ctrl.button.onclick();

  function add_user_idens(tree){
    
    if (!tree) return;
    if (tree.tag == 'iden'){
      let same =Templates.filter(x=>(x.tag=='iden'&&x.texts[0]==tree.texts[0]));
      // console.log(tree.texts[0],same);
      if (!same.length){
        let tem = {page:"iden",tag:'iden',texts:[tree.texts[0]],slot_types:[]}
        Templates.push(tem);
      }
    }else{
      for (let i = 0; i < tree.slot_types.length; i++){
        if (tree.slot_types[i] == 'L'){
          tree.slots[i].map(add_user_idens);
        }else if (tree.slot_types[i] == 'X'){
          add_user_idens(tree.slots[i])
        }
      }
    }
  }

  function compact_serialized(tree){
    function _compact(tree){
      if (!tree) return tree;
      let o = [];
      o[3] = null;
      if (tree.page == 'iden' || tree.page == 'stdl'){
        o[3] = tree.texts;
      }
      o[0] = tree.tag;
      o[1] = tree.page;
      o[2] = [];
      for (let i = 0; i < tree.slots.length; i++){
        if (tree.slot_types[i] == 'L'){
          o[2].push(tree.slots[i].map(_compact));
        }else if (tree.slot_types[i] == 'X'){
          o[2].push(_compact(tree.slots[i]))
        }else{
          o[2].push(tree.slots[i]);
        }
      }
      o[4] = null;
      return o;
    }
    let o = _compact(tree);
    o[4] = [tree.x,tree.y];
    return o;
  }
  function uncompact_serialized(tree){
    if (!tree) return tree;
    let o = {tag:tree[0],page:tree[1],x:0,y:0};
    let tem = Templates.filter(x=>(x.tag == tree[0] && x.page == tree[1]))[0];
    o.texts = tree[3];
    if (!o.texts){
      o.texts = tem.texts;
    }
    if (tree[4]){
      o.x = tree[4][0];
      o.y = tree[4][1];
    }
    o.slot_types = tem.slot_types;
    o.slots = [];
    for (let i = 0; i < tree[2].length; i++){
      if (o.slot_types[i] == 'L'){
        o.slots.push(tree[2][i].map(uncompact_serialized));
      }else if (o.slot_types[i] == 'X'){
        o.slots.push(uncompact_serialized(tree[2][i]))
      }else{
        o.slots.push(tree[2][i]);
      }
    }
    return o;
  }


  add_user_idens(uncompact_serialized(JSON.parse(EXAMPLES[EG_IDX])));
  btn_run.onclick();

  function transpile(tree){
    
    let includes = new Set();
    function _transpile(tree){
      // console.log(tree);
      if (tree.tag == 'prgm'){
        return tree.slots[0].map(_transpile).join(';');
      }else if (tree.tag == 'iden'){
        return tree.texts[0];
      }else if (tree.tag == 'func'){
        return `func ${_transpile(tree.slots[0])}(${tree.slots[2].map(_transpile).join(',')}):${_transpile(tree.slots[1])}{${tree.slots[3].map(_transpile).join(';')}}`;
      }else if (tree.tag == 'argr'){
        return _transpile(tree.slots[0])+':'+_transpile(tree.slots[1]);
      }else if (tree.tag == 'ti32'){
        return 'i32';
      }else if (tree.tag == 'tf32'){
        return 'f32';
      }else if (tree.tag == 'tvod'){
        return 'void';
      }else if (tree.tag == 'tlst'){
        return `list[${_transpile(tree.slots[0])}]`;
      }else if (tree.tag == 'tfun'){
        return `func[tup[${tree.slots[0].map(_transpile)}],${_transpile(tree.slots[1])}]`;
      }else if (tree.tag == 'cond'){
        return `if (${_transpile(tree.slots[0])}){${tree.slots[1].map(_transpile).join(';')}}`
      }else if (tree.tag == 'elif'){
        return `else if (${_transpile(tree.slots[0])}){${tree.slots[1].map(_transpile).join(';')}}`
      }else if (tree.tag == 'else'){
        return `else {${tree.slots[0].map(_transpile).join(';')}}`
      }else if (tree.tag == 'whil'){
        return `while (${_transpile(tree.slots[0])}){${tree.slots[1].map(_transpile).join(';')}}`
      }else if (tree.tag == 'whi1'){
        return `while (1){${tree.slots[0].map(_transpile).join(';')}}`
      }else if (tree.tag == 'forr'){
        let i = _transpile(tree.slots[0]);
        return `for (${i}:=${_transpile(tree.slots[1])}; ${i}<${_transpile(tree.slots[2])}; ${i}++){${tree.slots[3].map(_transpile).join(';')}}`
      }else if (tree.tag == 'retn'){
        return `return ${_transpile(tree.slots[0])};`
      }else if (tree.tag == 'li32'){
        return `(${tree.slots[0]} as i32)`
      }else if (tree.tag == 'lf32'){
        return `(${tree.slots[0]} as f32)`
      }else if (tree.tag == 'lu08'){
        return `(${tree.slots[0]} as u8)`
      }else if (tree.tag == 'lisl'){
        let a = Number(tree.slots[0]);
        let t = Number(tree.slots[1])/100.0;
        let b = Number(tree.slots[2]);
        let n = Math.round(a * (1-t) + b * t);
        return `(${n} as i32)`
      }else if (tree.tag == 'lfsl'){
        let a = Number(tree.slots[0]);
        let t = Number(tree.slots[1])/100.0;
        let b = Number(tree.slots[2]);
        let n = a * (1-t) + b * t;
        return `(${n} as f32)`
      }else if (tree.tag == 'lstr'){
        return `${JSON.stringify(tree.slots[0])}`
      }else if (tree.tag == 'lidx'){
        return `${tree.slots[0].map(_transpile).join(',')}`
      }else if (tree.tag == 'lvec'){
        return `{${tree.slots[0].map(_transpile).join(',')}}`
      }else if (tree.tag == 'lar0'){
        return ``
      }else if (tree.tag == 'lbmp'){
        return `arr[u8,2]{${tree.slots[2].map(x=>x.join(',')).join(';')}}`
      }else if (tree.tag == 'lfun'){
        return `(func(x:f32):f32{
          p0 := {${tree.slots[0][0].map(x=>x.toFixed(3))}};
          p1 := {${tree.slots[0][1].map(x=>x.toFixed(3))}};
          p2 := {${tree.slots[0][2].map(x=>x.toFixed(3))}};
          p3 := {${tree.slots[0][3].map(x=>x.toFixed(3))}};
          func bez(t:f32):vec[f32,2] {
            u := 1 - t;
            tt := t * t;
            uu := u * u;
            return u*uu*p0 + 3*uu*t*p1 + 3*u*tt*p2 + t*tt*p3;
          }
          func x_to_t(targ:f32):f32{
            tolerance := 0.001;
            t := 0.5;
            iter := 0;
            while (iter < 100) {
              p := bez(t);
              diff := p.x - targ;
              if (-tolerance < diff && diff < tolerance) {
                return t;
              } else {
                t -= diff / 2;
              }
              iter++;
            }
            return t;
          }
          return bez(x_to_t(x)).y;
        })`
      }else if (tree.tag == 'lcmt'){
        return `/*${tree.slots[0]}*/`
      }else if (tree.tag == 'omul'){
        return `((${_transpile(tree.slots[0])}) * (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'osub'){
        return `((${_transpile(tree.slots[0])}) - (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'oadd'){
        return `((${_transpile(tree.slots[0])}) + (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'odiv'){
        return `((${_transpile(tree.slots[0])}) / (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'omod'){
        return `((${_transpile(tree.slots[0])}) % (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'opow'){
        return `((${_transpile(tree.slots[0])}) ** (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'oolt'){
        return `((${_transpile(tree.slots[0])}) < (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'ooeq'){
        return `((${_transpile(tree.slots[0])}) == (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'oogt'){
        return `((${_transpile(tree.slots[0])}) > (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'oleq'){
        return `((${_transpile(tree.slots[0])}) <= (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'olan'){
        return `((${_transpile(tree.slots[0])}) && (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'olor'){
        return `((${_transpile(tree.slots[0])}) || (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'oban'){
        return `((${_transpile(tree.slots[0])}) & (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'obor'){
        return `((${_transpile(tree.slots[0])}) | (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'obno'){
        return `(~(${_transpile(tree.slots[0])}))`
      }else if (tree.tag == 'subs'){
        return `((${_transpile(tree.slots[1])})[${_transpile(tree.slots[0])}])`
      }else if (tree.tag == 'tmpl'){
        return `${_transpile(tree.slots[0])}[${tree.slots[1].map(_transpile).join(',')}]`
      }else if (tree.tag == 'adot'){
        return `((${_transpile(tree.slots[1])}).${_transpile(tree.slots[0])})`
      }else if (tree.tag == 'cast'){
        return `((${_transpile(tree.slots[0])}) as ${_transpile(tree.slots[1])})`
      }else if (tree.tag == 'ret0'){
        return `return;`
      }else if (tree.tag == 'ocat'){
        return `(${tree.slots[0].map(_transpile).join('+')})`
      }else if (tree.tag == 'call'){
        return `${_transpile(tree.slots[0])}(${tree.slots[1].map(_transpile).join(',')})`
      }else if (tree.tag == 'cal0'){
        return `${_transpile(tree.slots[0])}()`
      }else if (tree.tag == 'cal1'){
        return `${_transpile(tree.slots[0])}(${_transpile(tree.slots[1])})`
      }else if (tree.tag == 'cal2'){
        return `${_transpile(tree.slots[0])}(${_transpile(tree.slots[1])},${_transpile(tree.slots[2])})`
      }else if (tree.tag == 'varv'){
        return `${_transpile(tree.slots[0])} := ${_transpile(tree.slots[1])};`
      }else if (tree.tag == 'vatv'){
        return `${_transpile(tree.slots[0])} : ${_transpile(tree.slots[1])} = ${_transpile(tree.slots[2])};`
      }else if (tree.tag == 'asgn'){
        return `${_transpile(tree.slots[0])} = ${_transpile(tree.slots[1])}`
      }else if (tree.tag == 'stdf' || tree.tag == 'stdd' || tree.tag == 'stdt'){
        let [a,b] = tree.texts[0].slice(0,-4).split(`: <b>`);
        includes.add(`include "std/${a}"`);
        return `${a}.${b}`
      }else{
        return tree.tag
      }
    }
    let o = _transpile(tree);
    return Array.from(includes).join(';')+';'+o;
  }

  function do_transpile(){
    let blk = Blocks.filter(x=>(!x.parent && x.tag == 'prgm'))[0];
    
    if (!blk) return `/* No "Program" block found! */`;
    let bls = JSON.parse(JSON.stringify(serialize_block(blk)));
    return formatter(transpile(bls));
  }
  Dtext.value = do_transpile();

}


html.push(`<script>var Blocks = [];${main.toString()};main();</script>`);


fs.writeFileSync("build/blocks.html",html.join("\n"));

