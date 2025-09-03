const fs = require('fs');

let html = [`
<meta charset="UTF-8">
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
  }else if (ff[i].endsWith(".dh")){
    let f = ff[i].replace(/\.dh$/g,"");
    let v = JSON.stringify(fs.readFileSync("std/"+ff[i]).toString());
    html.push(`"std/${f}":`+v+",");
    html.push(`"std/${ff[i]}":`+v+",");
  }
}
html.push(`};</script>`)

function main(){
  function compile_from_str(str){
    let fs = {
      readFileSync:function(x){
        if (x == "CURRENT"){
          return new TextEncoder().encode(str);
        }
        if (x.startsWith("std/")){
          return new TextEncoder().encode(STD[x]);
        }
      },
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

    globalThis.__dh_intern_hooked_include = function(x){
      return STD[x];
    } 
    let to_js = new TO_JS({preclude:1});
    let irlo = ir+"\n"+lo;

    let [pir,playout] = to_js.parse_ir(irlo);
    let jj = to_js.transpile(pir,playout);

    return [irlo,jj];
  }

  function run_from_str(str,outdiv){
    let [irlo,jj] = compile_from_str(str);

    const iframe = document.createElement('iframe');

    iframe.style = "width:100%;height:100%;border:none;";
    
    outdiv.appendChild(iframe);
    const iframeDoc = iframe.contentDocument || iframe.contentWindow.document;
    const htmlContent = `
      <body style="color:#222;font-family:monospace;font-size:15px">${"<"}${"/"}body>
      <script>
      setInterval(function(){
        let c0 = document.getElementsByTagName("canvas")[0];
        if (c0 && c0.getContext("webgl")){
          let cnv = document.createElement("canvas");
          cnv.id = "copy";
          cnv.width = c0.width;
          cnv.height = c0.height;
          document.body.appendChild(cnv);
          let ctx = cnv.getContext("2d");
          function loop(){
            requestAnimationFrame(loop);
            ctx.drawImage(c0,0,0);
          }
          loop();
        }
      },1000);

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
          console.log(pth);
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
  let asset_url = {}
  window.addEventListener("message", async (event) => {
    if (event.data.type === "request-resource") {
      let pth = event.data.value;
      let blobUrl;
      if (asset_url[pth]){
        blobUrl = asset_url[pth];
      }else{
        let b64 = ASSETS[pth];
        let bytes = Uint8Array.from(atob(b64), c => c.charCodeAt(0))
        let blob = new Blob([bytes], { type: "application/octet-stream" });
        blobUrl = URL.createObjectURL(blob);
        asset_url[pth] = blobUrl;
      }
      event.source.postMessage({
        type: "resource-response",
        value: blobUrl
      }, "*");
    }
  });
  function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

  async function doit(){
    let ncol = 4;
    let nrow = 8;
    let imgw = 128;
    let imgh = 64;
    let idx = 0;
    let cnv = document.createElement("canvas");
    cnv.width = ncol*imgw;
    cnv.height = nrow*imgh;
    document.body.appendChild(cnv);
    let ctx = cnv.getContext("2d");
    for (let k in EXAMPLES){
      // if (k != "mousepaint.dh") continue;
      let div = document.createElement("div");
      let row = ~~(idx / ncol);
      let col = idx % ncol;
      document.body.appendChild(div);
      run_from_str(EXAMPLES[k],div);
      await sleep(2000);
      if (k == "whereami.dh" || k == "mousepaint.dh" || k == "mirror.dh"){
        await sleep(3000);  
      }
      let doc = div.getElementsByTagName("iframe")[0].contentWindow.document;
      let bdy = doc.body;
      let c0 = bdy.getElementsByTagName("canvas")[0];
      let c1 = document.createElement("canvas");
      // document.body.appendChild(c1);
      c1.width = imgw;
      c1.height = imgh;
      let c1c = c1.getContext("2d");
      if (c0){
        let fx = imgw/c0.width;
        let fy = imgh/c0.height;
        let f = Math.max(fx,fy);
        let px = (imgw - c0.width*f)/2;
        let py = (imgh - c0.height*f)/2;

        if (c0.getContext('2d')){
          c1c.drawImage(c0,px,py,c0.width*f,c0.height*f);
        }else{
          c0 = doc.getElementById("copy");
          while (!c0){
            await sleep(100);
            c0 = doc.getElementById("copy");
          }
          c1c.drawImage(c0,px,py,c0.width*f,c0.height*f);
        }

      }else{
        c1c.fillRect(0,0,imgw,imgh);
        c1c.fillStyle = "white";
        c1c.font = "10px monospace";
        let txt = bdy.innerText.split("\n");
        for (let i = 0; i < txt.length; i++){
          c1c.fillText(txt[i],0,10+i*10);
        }
      }

      ctx.drawImage(c1,col*imgw,row*imgh,imgw,imgh);
      div.innerHTML = "";

      idx++;
      // break;
    }
  }
  doit();

}

ff = fs.readdirSync("examples").filter(x=>x.endsWith('.dh'));
html.push(`<script>var EXAMPLES = {`);
for (let i = 0; i < ff.length; i++){
  let txt = fs.readFileSync("examples/"+ff[i]).toString()
  html.push(`'${ff[i]}':${JSON.stringify(txt)},`);
}
html.push(`}</script>`);

html.push(`<script>var ASSETS = {`);
ff = fs.readdirSync("examples/assets").filter(x=>!x.startsWith('.'));
for (let i = 0; i < ff.length; i++){
  html.push(`'examples/assets/${ff[i]}':"${fs.readFileSync("examples/assets/"+ff[i]).toString('base64')}",`);
}
html.push(`}</script>`);

html.push(`<script>${main.toString()};main();</script>`)

fs.writeFileSync("build/snap.html",html.join("\n"));

