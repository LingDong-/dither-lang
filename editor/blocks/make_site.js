const fs = require('fs');
const path = require('path')
const PARSER = require('../../src/parser.js');

let html = [`
<style>
  input[type=number]::-webkit-inner-spin-button {
    opacity: 1
  }
  .menubtn{
    font-size:12px;
    background:#222;color:white;height:24px;
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
<body></body>
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
  let f = "std/"+std[i]+"/header.dh";
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

  // let test_program = '{"tag":"prgm","page":"ctrl","texts":["<b>Program</b>","End"],"slot_types":["L"],"x":17,"y":104,"slots":[[{"tag":"func","page":"decl","texts":["Define <b>function</b>","that returns a","and takes","Begin","End"],"slot_types":["X","X","L","L"],"x":4,"y":19,"slots":[{"tag":"iden","page":"iden","texts":["Factorial"],"slot_types":[],"x":117,"y":22,"slots":[]},{"tag":"ti32","page":"type","texts":["<b>Integer</b>"],"slot_types":[],"x":275,"y":22,"slots":[]},[{"tag":"argr","page":"decl","texts":["<b>Argument</b>","of type",""],"slot_types":["X","X"],"x":11,"y":44,"slots":[{"tag":"iden","page":"iden","texts":["X"],"slot_types":[],"x":90,"y":47,"slots":[]},{"tag":"ti32","page":"type","texts":["<b>Integer</b>"],"slot_types":[],"x":180,"y":47,"slots":[]}]}],[{"tag":"cond","page":"ctrl","texts":["<b>If</b>","then",""],"slot_types":["X","L"],"x":11,"y":88,"slots":[{"tag":"iden","page":"iden","texts":["X"],"slot_types":[],"x":33,"y":91,"slots":[]},[{"tag":"retn","page":"misc","texts":["<b>Return</b>",""],"slot_types":["X"],"x":18,"y":113,"slots":[{"tag":"omul","page":"oper","texts":["","<b>times</b>",""],"slot_types":["X","X"],"x":25,"y":132,"slots":[{"tag":"iden","page":"iden","texts":["X"],"slot_types":[],"x":38,"y":154,"slots":[]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":123,"y":135,"slots":[{"tag":"iden","page":"iden","texts":["Factorial"],"slot_types":[],"x":214,"y":138,"slots":[]},[{"tag":"osub","page":"oper","texts":["","<b>minus</b>",""],"slot_types":["X","X"],"x":130,"y":160,"slots":[{"tag":"iden","page":"iden","texts":["X"],"slot_types":[],"x":143,"y":165,"slots":[]},{"tag":"li32","page":"litr","texts":["Integer",""],"slot_types":["I"],"x":232,"y":163,"slots":["1"]}]}]]}]}]}]]},{"tag":"retn","page":"misc","texts":["<b>Return</b>",""],"slot_types":["X"],"x":11,"y":202,"slots":[{"tag":"li32","page":"litr","texts":["Integer",""],"slot_types":["I"],"x":69,"y":205,"slots":["1"]}]}]]},{"tag":"varv","page":"decl","texts":["Declare <b>variable</b>","of value",""],"slot_types":["X","X"],"x":4,"y":250,"slots":[{"tag":"iden","page":"iden","texts":["A"],"slot_types":[],"x":122,"y":255,"slots":[]},{"tag":"li32","page":"litr","texts":["Integer",""],"slot_types":["I"],"x":219,"y":253,"slots":["6"]}]},{"tag":"varv","page":"decl","texts":["Declare <b>variable</b>","of value",""],"slot_types":["X","X"],"x":4,"y":280,"slots":[{"tag":"iden","page":"iden","texts":["B"],"slot_types":[],"x":122,"y":283,"slots":[]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":11,"y":305,"slots":[{"tag":"iden","page":"iden","texts":["Factorial"],"slot_types":[],"x":103,"y":308,"slots":[]},[{"tag":"iden","page":"iden","texts":["A"],"slot_types":[],"x":18,"y":330,"slots":[]}]]}]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":4,"y":355,"slots":[{"tag":"stdf","page":"stdl","texts":["io: <b>println</b>"],"slot_types":[],"x":96,"y":358,"slots":[]},[{"tag":"ocat","page":"oper","texts":["The following texts <b>joined</b>",""],"slot_types":["L"],"x":11,"y":380,"slots":[[{"tag":"lstr","page":"litr","texts":["Text \\"","\\""],"slot_types":["S"],"x":18,"y":399,"slots":["The factorial of "]},{"tag":"iden","page":"iden","texts":["A"],"slot_types":[],"x":18,"y":440,"slots":[]},{"tag":"lstr","page":"litr","texts":["Text \\"","\\""],"slot_types":["S"],"x":18,"y":459,"slots":[" is "]},{"tag":"iden","page":"iden","texts":["B"],"slot_types":[],"x":18,"y":500,"slots":[]}]]}]]}]]}'
  let test_program = '{"tag":"prgm","page":"ctrl","texts":["<b>Program</b>","End"],"slot_types":["L"],"x":30,"y":8,"slots":[[{"tag":"varv","page":"decl","texts":["Declare <b>variable</b>","of value",""],"slot_types":["X","X"],"x":8,"y":18,"slots":[{"tag":"iden","page":"iden","texts":["Spread"],"slot_types":[],"x":119,"y":23,"slots":[]},{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":228,"y":21,"slots":["0.5"]}]},{"tag":"func","page":"decl","texts":["Define <b>function</b>","that returns a","and takes","Begin","End"],"slot_types":["X","X","L","L"],"x":8,"y":48,"slots":[{"tag":"iden","page":"iden","texts":["Tree"],"slot_types":[],"x":113,"y":51,"slots":[]},{"tag":"tvod","page":"type","texts":["<b>Nothing</b>"],"slot_types":[],"x":238,"y":51,"slots":[]},[{"tag":"argr","page":"decl","texts":["<b>Argument</b>","of type",""],"slot_types":["X","X"],"x":19,"y":73,"slots":[{"tag":"iden","page":"iden","texts":["X"],"slot_types":[],"x":93,"y":76,"slots":[]},{"tag":"tf32","page":"type","texts":["<b>Number</b>"],"slot_types":[],"x":180,"y":76,"slots":[]}]},{"tag":"argr","page":"decl","texts":["<b>Argument</b>","of type",""],"slot_types":["X","X"],"x":19,"y":98,"slots":[{"tag":"iden","page":"iden","texts":["Y"],"slot_types":[],"x":93,"y":101,"slots":[]},{"tag":"tf32","page":"type","texts":["<b>Number</b>"],"slot_types":[],"x":180,"y":101,"slots":[]}]},{"tag":"argr","page":"decl","texts":["<b>Argument</b>","of type",""],"slot_types":["X","X"],"x":19,"y":123,"slots":[{"tag":"iden","page":"iden","texts":["A"],"slot_types":[],"x":93,"y":126,"slots":[]},{"tag":"tf32","page":"type","texts":["<b>Number</b>"],"slot_types":[],"x":180,"y":126,"slots":[]}]},{"tag":"argr","page":"decl","texts":["<b>Argument</b>","of type",""],"slot_types":["X","X"],"x":19,"y":148,"slots":[{"tag":"iden","page":"iden","texts":["L"],"slot_types":[],"x":93,"y":151,"slots":[]},{"tag":"tf32","page":"type","texts":["<b>Number</b>"],"slot_types":[],"x":180,"y":151,"slots":[]}]}],[{"tag":"cond","page":"ctrl","texts":["<b>If</b>","then",""],"slot_types":["X","L"],"x":19,"y":191,"slots":[{"tag":"oolt","page":"oper","texts":["","is <b>less</b> than",""],"slot_types":["X","X"],"x":40,"y":194,"slots":[{"tag":"iden","page":"iden","texts":["L"],"slot_types":[],"x":53,"y":199,"slots":[]},{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":168,"y":197,"slots":["8.0"]}]},[{"tag":"ret0","page":"misc","texts":["<b>Return</b> nothing"],"slot_types":[],"x":30,"y":226,"slots":[]}]]},{"tag":"varv","page":"decl","texts":["Declare <b>variable</b>","of value",""],"slot_types":["X","X"],"x":19,"y":248,"slots":[{"tag":"iden","page":"iden","texts":["X1"],"slot_types":[],"x":130,"y":251,"slots":[]},{"tag":"oadd","page":"oper","texts":["","<b>plus</b>",""],"slot_types":["X","X"],"x":26,"y":273,"slots":[{"tag":"iden","page":"iden","texts":["X"],"slot_types":[],"x":39,"y":276,"slots":[]},{"tag":"omul","page":"oper","texts":["","<b>times</b>",""],"slot_types":["X","X"],"x":33,"y":298,"slots":[{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":46,"y":301,"slots":[{"tag":"stdf","page":"stdl","texts":["math: <b>cos</b>"],"slot_types":[],"x":132,"y":304,"slots":[]},[{"tag":"iden","page":"iden","texts":["A"],"slot_types":[],"x":57,"y":326,"slots":[]}]]},{"tag":"iden","page":"iden","texts":["L"],"slot_types":[],"x":344,"y":315,"slots":[]}]}]}]},{"tag":"varv","page":"decl","texts":["Declare <b>variable</b>","of value",""],"slot_types":["X","X"],"x":19,"y":357,"slots":[{"tag":"iden","page":"iden","texts":["Y1"],"slot_types":[],"x":130,"y":360,"slots":[]},{"tag":"oadd","page":"oper","texts":["","<b>plus</b>",""],"slot_types":["X","X"],"x":26,"y":382,"slots":[{"tag":"iden","page":"iden","texts":["Y"],"slot_types":[],"x":39,"y":385,"slots":[]},{"tag":"omul","page":"oper","texts":["","<b>times</b>",""],"slot_types":["X","X"],"x":33,"y":407,"slots":[{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":46,"y":410,"slots":[{"tag":"stdf","page":"stdl","texts":["math: <b>sin</b>"],"slot_types":[],"x":132,"y":413,"slots":[]},[{"tag":"iden","page":"iden","texts":["A"],"slot_types":[],"x":57,"y":435,"slots":[]}]]},{"tag":"iden","page":"iden","texts":["L"],"slot_types":[],"x":340,"y":424,"slots":[]}]}]}]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":19,"y":466,"slots":[{"tag":"stdf","page":"stdl","texts":["gx: <b>line</b>"],"slot_types":[],"x":105,"y":469,"slots":[]},[{"tag":"iden","page":"iden","texts":["X"],"slot_types":[],"x":30,"y":491,"slots":[]},{"tag":"iden","page":"iden","texts":["Y"],"slot_types":[],"x":30,"y":510,"slots":[]},{"tag":"iden","page":"iden","texts":["X1"],"slot_types":[],"x":30,"y":529,"slots":[]},{"tag":"iden","page":"iden","texts":["Y1"],"slot_types":[],"x":30,"y":548,"slots":[]}]]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":19,"y":570,"slots":[{"tag":"iden","page":"iden","texts":["Tree"],"slot_types":[],"x":105,"y":573,"slots":[]},[{"tag":"iden","page":"iden","texts":["X1"],"slot_types":[],"x":30,"y":595,"slots":[]},{"tag":"iden","page":"iden","texts":["Y1"],"slot_types":[],"x":30,"y":614,"slots":[]},{"tag":"osub","page":"oper","texts":["","<b>minus</b>",""],"slot_types":["X","X"],"x":30,"y":633,"slots":[{"tag":"iden","page":"iden","texts":["A"],"slot_types":[],"x":43,"y":636,"slots":[]},{"tag":"iden","page":"iden","texts":["Spread"],"slot_types":[],"x":129,"y":636,"slots":[]}]},{"tag":"omul","page":"oper","texts":["","<b>times</b>",""],"slot_types":["X","X"],"x":30,"y":658,"slots":[{"tag":"iden","page":"iden","texts":["L"],"slot_types":[],"x":43,"y":663,"slots":[]},{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":125,"y":661,"slots":["0.8"]}]}]]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":19,"y":691,"slots":[{"tag":"iden","page":"iden","texts":["Tree"],"slot_types":[],"x":105,"y":694,"slots":[]},[{"tag":"iden","page":"iden","texts":["X1"],"slot_types":[],"x":30,"y":716,"slots":[]},{"tag":"iden","page":"iden","texts":["Y1"],"slot_types":[],"x":30,"y":735,"slots":[]},{"tag":"oadd","page":"oper","texts":["","<b>plus</b>",""],"slot_types":["X","X"],"x":30,"y":754,"slots":[{"tag":"iden","page":"iden","texts":["A"],"slot_types":[],"x":43,"y":757,"slots":[]},{"tag":"iden","page":"iden","texts":["Spread"],"slot_types":[],"x":118,"y":757,"slots":[]}]},{"tag":"omul","page":"oper","texts":["","<b>times</b>",""],"slot_types":["X","X"],"x":30,"y":779,"slots":[{"tag":"iden","page":"iden","texts":["L"],"slot_types":[],"x":43,"y":784,"slots":[]},{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":125,"y":782,"slots":["0.8"]}]}]]}]]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":8,"y":829,"slots":[{"tag":"stdf","page":"stdl","texts":["gx: <b>size</b>"],"slot_types":[],"x":94,"y":832,"slots":[]},[{"tag":"li32","page":"litr","texts":["Integer",""],"slot_types":["I"],"x":19,"y":854,"slots":["320"]},{"tag":"li32","page":"litr","texts":["Integer",""],"slot_types":["I"],"x":19,"y":878,"slots":["240"]}]]},{"tag":"whil","page":"ctrl","texts":["Repeat <b>while</b>","do","end"],"slot_types":["X","L"],"x":8,"y":904,"slots":[{"tag":"li32","page":"litr","texts":["Integer",""],"slot_types":["I"],"x":99,"y":907,"slots":["1"]},[{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":19,"y":934,"slots":[{"tag":"stdf","page":"stdl","texts":["gx: <b>background</b>"],"slot_types":[],"x":105,"y":937,"slots":[]},[{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":30,"y":959,"slots":["0.75"]}]]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":19,"y":985,"slots":[{"tag":"iden","page":"iden","texts":["Tree"],"slot_types":[],"x":105,"y":988,"slots":[]},[{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":30,"y":1010,"slots":["160.0"]},{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":30,"y":1034,"slots":["240.0"]},{"tag":"odiv","page":"oper","texts":["","<b>divided</b> by",""],"slot_types":["X","X"],"x":30,"y":1057,"slots":[{"tag":"stdd","page":"stdl","texts":["math: <b>PI</b>"],"slot_types":[],"x":43,"y":1062,"slots":[]},{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":176,"y":1060,"slots":["-2.0"]}]},{"tag":"lf32","page":"litr","texts":["Number",""],"slot_types":["N"],"x":30,"y":1087,"slots":["50.0"]}]]},{"tag":"call","page":"misc","texts":["<b>Call</b> function","with arguments",""],"slot_types":["X","L"],"x":19,"y":1113,"slots":[{"tag":"stdf","page":"stdl","texts":["gx: <b>poll</b>"],"slot_types":[],"x":105,"y":1116,"slots":[]},[]]}]]}]]}'
  let zicnt = 0;
  let ID = 0;

  let Colormap = {
    ctrl:"turquoise",
    type:"royalblue",
    decl:"hotpink",
    oper:"mediumseagreen",
    misc:"mediumpurple",
    litr:"salmon",
    iden:"dimgray",
    stdl:"firebrick",
  }
  let Dmenu = document.createElement("div");
  Dmenu.style = "position:absolute;left:0px;top:0px;background:#222;width:100%;height:24px;overflow:hidden";
  document.body.appendChild(Dmenu);

  let Dtabs = document.createElement("div");
  Dtabs.style = "position:absolute;left:0px;top:24px;background:white;width:320px;height:55px;overflow:hidden";
  document.body.appendChild(Dtabs);
  let Dpalette = document.createElement("div");
  Dpalette.style = "position:absolute;left:0px;top:76px;background:whitesmoke;width:320px;height:calc(100% - 80px);overflow:scroll";
  document.body.appendChild(Dpalette)
  let Dprogram = document.createElement("div");
  Dprogram.style = "position:absolute;left:320px;top:24px;border-left:1px solid black;border-right:1px solid black;width:calc(100% - 682px);height:calc(100% - 24px);overflow:scroll";
  document.body.appendChild(Dprogram)
  window.Dprogram = Dprogram;

  let Dout = document.createElement("div");
  Dout.style = "position:absolute;right:0px;top:24px;width:360px;height:70%;background:whitesmoke;"
  document.body.appendChild(Dout)

  let Dtext = document.createElement("textarea");
  Dtext.style = "font-size:11px;border:none;border-top:1px solid black;position:absolute;right:0px;bottom:0px;width:360px;height:calc(30% - 24px);background:whitesmoke;"
  document.body.appendChild(Dtext)

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
      <body style="font-size:13px;font-family:monospace;">${"<"}${"/"}body>
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

    div.style="display:block;font-family:sans-serif;font-size:13px;min-width:32px;min-height:16px;border:1px solid black;border-radius:2px;cursor:grab;-webkit-user-select: none;-ms-user-select: none;user-select: none;padding-top:2px;padding-left:1px";
    div.style.borderWidth = "0px 1px 1px 0px"
    div.style.borderLeft = "3px solid "+Colormap[cfg.page]
    div.style.position = 'absolute';
    div.style.background = "lightgray";
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
          div.style.left = (x - that.drag_x - div.parentElement.offsetLeft) + "px";
          div.style.top = (y - that.drag_y -  div.parentElement.offsetTop ) + "px";
        }
        div.style.left = (x - that.drag_x - div.parentElement.offsetLeft +  div.parentElement.scrollLeft) + "px";
        div.style.top = (y - that.drag_y -  div.parentElement.offsetTop   + div.parentElement.scrollTop ) + "px";


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
      div.appendChild(span);
      if (i == that.texts.length-1){
        break;
      }
      let slot; 

      if ('XL'.includes(that.slot_types[i])){
        slot = document.createElement("span");
      }else if ('NI'.includes(that.slot_types[i])){
        slot = document.createElement("input");
      }else if ('S'.includes(that.slot_types[i])){
        slot = document.createElement("textarea");
      }
      slot.style="min-width:32px;min-height:16px;background:white;border:1px solid black;border-radius:2px;vertical-align: middle;"
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
        
      }else if (that.slot_types[i] == 'I'){
        
        slot.style.display = "inline-block";
        slot.setAttribute("type","number");
        slot.value = "0"
      }else if (that.slot_types[i] == 'S'){
        slot.style.display = "inline-block";
      }

      if ('NIS'.includes(that.slot_types[i])){
        slot.onmousedown = function(e){
          setTimeout(function(){
            Dtext.value = do_transpile();
          },10);
          e.stopPropagation();
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
    {page:"oper",tag:'ooeq',texts:['','<b>equals</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'oneq',texts:['','does <b>not equal</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'oolt',texts:['','is <b>less</b> than',''],slot_types:['X','X'],},
    {page:"oper",tag:'oogt',texts:['','is <b>greater</b> than',''],slot_types:['X','X'],},
    {page:"oper",tag:'oleq',texts:['','is <b>at most</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'ogeq',texts:['','is <b>at least</b>',''],slot_types:['X','X'],},
    {page:"oper",tag:'ocat',texts:['The following texts <b>joined</b>',''],slot_types:['L'],},
    {page:"litr",tag:'lstr',texts:['Text "','"'],slot_types:['S'],},
    {page:"litr",tag:'lf32',texts:['Number',''],slot_types:['N'],},
    {page:"litr",tag:'li32',texts:['Integer',''],slot_types:['I'],},
    {page:"litr",tag:'lvec',texts:['Vector',''],slot_types:['L'],},
    {page:"litr",tag:'llst',texts:['List',''],slot_types:['L'],},
    {page:"litr",tag:'lls0',texts:['Empty list'],slot_types:[],},
    {page:"litr",tag:'ltup',texts:['Tuple',''],slot_types:['L'],},
    {page:"litr",tag:'ldic',texts:['Dictionary',''],slot_types:['L'],},
    {page:"litr",tag:'lkvp',texts:['Entry with <b>key</b>','and <b>value</b>',''],slot_types:['X','X'],},
    {page:"misc",tag:'asgn',texts:['<b>Set</b>','to',''],slot_types:['X','X'],},
    {page:"misc",tag:'retn',texts:['<b>Return</b>',''],slot_types:['X'],},
    {page:"misc",tag:'ret0',texts:['<b>Return</b> nothing'],slot_types:[],},
    {page:"misc",tag:'subs',texts:['The','-th <b>item</b> of',''],slot_types:['X','X'],},
    {page:"misc",tag:'subs',texts:['<b>Entry</b>','of',''],slot_types:['X','X'],},
    {page:"misc",tag:'subx',texts:['<b>X</b> component of',''],slot_types:['X'],},
    {page:"misc",tag:'suby',texts:['<b>Y</b> component of',''],slot_types:['X'],},
    {page:"misc",tag:'subz',texts:['<b>Z</b> component of',''],slot_types:['X'],},
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
    btn.style = "margin:2px;font-size:13px;border:1px solid black;border-radius: 2px;background:gainsboro;cursor:pointer"
    btn.style.borderTop = `3px solid ${Colormap[k]}`
    // btn.innerHTML = `<span><span style="display:inline-block;width:8px;height:8px;border-radius:8px;vertical-align:middle;background:${Colormap[k]}"></span>&nbsp;${Tabs[k].name}</span>`;
    btn.innerHTML = `${Tabs[k].name}`;
  
    btn.onclick = function(){
      let tems = Templates.filter(x=>(x.page == k));
      Dpalette.innerHTML = "";
      if (k == "iden"){
        let inp = document.createElement("input");
        inp.style = "margin:5px;"
        Dpalette.appendChild(inp);
        let btm = document.createElement("button");
        btm.innerHTML = "Add"
        Dpalette.appendChild(btm);
        Dpalette.appendChild(document.createElement("br"))
        function add(){
          let tem = {page:"iden",tag:'iden',texts:[inp.value],slot_types:[],}
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


  deserialize_block(JSON.parse(test_program));

  Tabs.ctrl.button.onclick();


  function transpile(tree){
    let includes = new Set();
    function _transpile(tree){
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
      }else if (tree.tag == 'cond'){
        return `if (${_transpile(tree.slots[0])}){${tree.slots[1].map(_transpile).join(';')}}`
      }else if (tree.tag == 'whil'){
        return `while (${_transpile(tree.slots[0])}){${tree.slots[1].map(_transpile).join(';')}}`
      }else if (tree.tag == 'retn'){
        return `return ${_transpile(tree.slots[0])};`
      }else if (tree.tag == 'li32'){
        return `(${tree.slots[0]} as i32)`
      }else if (tree.tag == 'lf32'){
        return `(${tree.slots[0]} as f32)`
      }else if (tree.tag == 'lstr'){
        return `${JSON.stringify(tree.slots[0])}`
      }else if (tree.tag == 'omul'){
        return `((${_transpile(tree.slots[0])}) * (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'osub'){
        return `((${_transpile(tree.slots[0])}) - (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'oadd'){
        return `((${_transpile(tree.slots[0])}) + (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'odiv'){
        return `((${_transpile(tree.slots[0])}) / (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'oolt'){
        return `((${_transpile(tree.slots[0])}) < (${_transpile(tree.slots[1])}))`
      }else if (tree.tag == 'ret0'){
        return `return;`
      }else if (tree.tag == 'ocat'){
        return `(${tree.slots[0].map(_transpile).join('+')})`
      }else if (tree.tag == 'call'){
        return `${_transpile(tree.slots[0])}(${tree.slots[1].map(_transpile).join(',')})`
      }else if (tree.tag == 'varv'){
        return `${_transpile(tree.slots[0])} := ${_transpile(tree.slots[1])};`
      }else if (tree.tag == 'asgn'){
        return `${_transpile(tree.slots[0])} = ${_transpile(tree.slots[1])}`
      }else if (tree.tag == 'stdf' || tree.tag == 'stdd'){
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

