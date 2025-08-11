const fs = require('fs');

const { execSync } = require('child_process');
let do_download = 1;

function download(url) {
  if (do_download){
    console.log("downloading... "+url)
    return `<script>${execSync(`curl -sL "${url}"`, { encoding: 'utf8' })}</script>`;
  }
  return `<script src="${url}"></script>`
}

let html = [`
<meta charset="UTF-8">
<style>
  html{
    height:100%;
    overflow:hidden;
  }
  .bigbtn{
    display: inline-block; 
    width: 24px; 
    height: 24px; 
    text-align: center; 
    background: #444;
    margin: 3px;
    margin-right: 0px;
    cursor: pointer;
    border-radius: 2px;
  }
  .bigbtn:hover{
    background: #555;
  }
  .fakebigbtn{
    display:inline-block;
    transform:translate(0,6px);
    margin-top: -10px;
    margin-bottom: -10px;
    font-style: normal;
  
  }
  #doc pre{
    background: #252627;
    padding:5px;
    /* border:1px solid #222; */
    border-radius: 2px;
  }
  #doc h1{
    font-size:24px;
    margin-top:36px;
  }
  #doc h2{
    font-size:18px;
    margin-top:36px;
  }
  select {
    background: #444;
    border-radius: 2px;
    background-image: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="black"><path d="M0 5 L20 5 L10 17"/></svg>');
    background-repeat: no-repeat;
    background-position: right 5px center;
    background-size: 12px;
  }
  ::-webkit-scrollbar {
    width: 6px; 
    height: 6px;
  }
  ::-webkit-scrollbar-thumb {
    background: rgba(0,0,0,0.3);
    border-radius: 3px;
  }
  ::-webkit-scrollbar-track {
    background: transparent;
  }
  * {
    scrollbar-width: thin;
    scrollbar-color: rgba(0,0,0,0.3) transparent;
  }
</style>
<style>
.CodeMirror{font-family:monospace;height:300px;color:#000;direction:ltr}.CodeMirror-lines{padding:4px 0}.CodeMirror pre.CodeMirror-line,.CodeMirror pre.CodeMirror-line-like{padding:0 4px}.CodeMirror-gutter-filler,.CodeMirror-scrollbar-filler{background-color:#fff}.CodeMirror-gutters{border-right:1px solid #ddd;background-color:#f7f7f7;white-space:nowrap}.CodeMirror-linenumber{padding:0 3px 0 5px;min-width:20px;text-align:right;color:#999;white-space:nowrap}.CodeMirror-guttermarker{color:#000}.CodeMirror-guttermarker-subtle{color:#999}.CodeMirror-cursor{border-left:1px solid #000;border-right:none;width:0}.CodeMirror div.CodeMirror-secondarycursor{border-left:1px solid silver}.cm-fat-cursor .CodeMirror-cursor{width:auto;border:0!important;background:#7e7}.cm-fat-cursor div.CodeMirror-cursors{z-index:1}.cm-fat-cursor-mark{background-color:rgba(20,255,20,.5);-webkit-animation:blink 1.06s steps(1) infinite;-moz-animation:blink 1.06s steps(1) infinite;animation:blink 1.06s steps(1) infinite}.cm-animate-fat-cursor{width:auto;border:0;-webkit-animation:blink 1.06s steps(1) infinite;-moz-animation:blink 1.06s steps(1) infinite;animation:blink 1.06s steps(1) infinite;background-color:#7e7}@-moz-keyframes blink{50%{background-color:transparent}}@-webkit-keyframes blink{50%{background-color:transparent}}@keyframes blink{50%{background-color:transparent}}.cm-tab{display:inline-block;text-decoration:inherit}.CodeMirror-rulers{position:absolute;left:0;right:0;top:-50px;bottom:0;overflow:hidden}.CodeMirror-ruler{border-left:1px solid #ccc;top:0;bottom:0;position:absolute}.cm-s-default .cm-header{color:#00f}.cm-s-default .cm-quote{color:#090}.cm-negative{color:#d44}.cm-positive{color:#292}.cm-header,.cm-strong{font-weight:700}.cm-em{font-style:italic}.cm-link{text-decoration:underline}.cm-strikethrough{text-decoration:line-through}.cm-s-default .cm-keyword{color:#708}.cm-s-default .cm-atom{color:#219}.cm-s-default .cm-number{color:#164}.cm-s-default .cm-def{color:#00f}.cm-s-default .cm-variable-2{color:#05a}.cm-s-default .cm-type,.cm-s-default .cm-variable-3{color:#085}.cm-s-default .cm-comment{color:#a50}.cm-s-default .cm-string{color:#a11}.cm-s-default .cm-string-2{color:#f50}.cm-s-default .cm-meta{color:#555}.cm-s-default .cm-qualifier{color:#555}.cm-s-default .cm-builtin{color:#30a}.cm-s-default .cm-bracket{color:#997}.cm-s-default .cm-tag{color:#170}.cm-s-default .cm-attribute{color:#00c}.cm-s-default .cm-hr{color:#999}.cm-s-default .cm-link{color:#00c}.cm-s-default .cm-error{color:red}.cm-invalidchar{color:red}.CodeMirror-composing{border-bottom:2px solid}div.CodeMirror span.CodeMirror-matchingbracket{color:#0b0}div.CodeMirror span.CodeMirror-nonmatchingbracket{color:#a22}.CodeMirror-matchingtag{background:rgba(255,150,0,.3)}.CodeMirror-activeline-background{background:#e8f2ff}.CodeMirror{position:relative;overflow:hidden;background:#fff}.CodeMirror-scroll{overflow:scroll!important;margin-bottom:-50px;margin-right:-50px;padding-bottom:50px;height:100%;outline:0;position:relative}.CodeMirror-sizer{position:relative;border-right:50px solid transparent}.CodeMirror-gutter-filler,.CodeMirror-hscrollbar,.CodeMirror-scrollbar-filler,.CodeMirror-vscrollbar{position:absolute;z-index:6;display:none;outline:0}.CodeMirror-vscrollbar{right:0;top:0;overflow-x:hidden;overflow-y:scroll}.CodeMirror-hscrollbar{bottom:0;left:0;overflow-y:hidden;overflow-x:scroll}.CodeMirror-scrollbar-filler{right:0;bottom:0}.CodeMirror-gutter-filler{left:0;bottom:0}.CodeMirror-gutters{position:absolute;left:0;top:0;min-height:100%;z-index:3}.CodeMirror-gutter{white-space:normal;height:100%;display:inline-block;vertical-align:top;margin-bottom:-50px}.CodeMirror-gutter-wrapper{position:absolute;z-index:4;background:0 0!important;border:none!important}.CodeMirror-gutter-background{position:absolute;top:0;bottom:0;z-index:4}.CodeMirror-gutter-elt{position:absolute;cursor:default;z-index:4}.CodeMirror-gutter-wrapper ::selection{background-color:transparent}.CodeMirror-gutter-wrapper ::-moz-selection{background-color:transparent}.CodeMirror-lines{cursor:text;min-height:1px}.CodeMirror pre.CodeMirror-line,.CodeMirror pre.CodeMirror-line-like{-moz-border-radius:0;-webkit-border-radius:0;border-radius:0;border-width:0;background:0 0;font-family:inherit;font-size:inherit;margin:0;white-space:pre;word-wrap:normal;line-height:inherit;color:inherit;z-index:2;position:relative;overflow:visible;-webkit-tap-highlight-color:transparent;-webkit-font-variant-ligatures:contextual;font-variant-ligatures:contextual}.CodeMirror-wrap pre.CodeMirror-line,.CodeMirror-wrap pre.CodeMirror-line-like{word-wrap:break-word;white-space:pre-wrap;word-break:normal}.CodeMirror-linebackground{position:absolute;left:0;right:0;top:0;bottom:0;z-index:0}.CodeMirror-linewidget{position:relative;z-index:2;padding:.1px}.CodeMirror-rtl pre{direction:rtl}.CodeMirror-code{outline:0}.CodeMirror-gutter,.CodeMirror-gutters,.CodeMirror-linenumber,.CodeMirror-scroll,.CodeMirror-sizer{-moz-box-sizing:content-box;box-sizing:content-box}.CodeMirror-measure{position:absolute;width:100%;height:0;overflow:hidden;visibility:hidden}.CodeMirror-cursor{position:absolute;pointer-events:none}.CodeMirror-measure pre{position:static}div.CodeMirror-cursors{visibility:hidden;position:relative;z-index:3}div.CodeMirror-dragcursors{visibility:visible}.CodeMirror-focused div.CodeMirror-cursors{visibility:visible}.CodeMirror-selected{background:#d9d9d9}.CodeMirror-focused .CodeMirror-selected{background:#d7d4f0}.CodeMirror-crosshair{cursor:crosshair}.CodeMirror-line::selection,.CodeMirror-line>span::selection,.CodeMirror-line>span>span::selection{background:#d7d4f0}.CodeMirror-line::-moz-selection,.CodeMirror-line>span::-moz-selection,.CodeMirror-line>span>span::-moz-selection{background:#d7d4f0}.cm-searching{background-color:#ffa;background-color:rgba(255,255,0,.4)}.cm-force-border{padding-right:.1px}@media print{.CodeMirror div.CodeMirror-cursors{visibility:hidden}}.cm-tab-wrap-hack:after{content:''}span.CodeMirror-selectedtext{background:0 0}
</style>
<style>
.cm-s-theme.CodeMirror, .cm-s-theme .CodeMirror-gutters {
  background-color: #252627!important;
  color: #f8f8f8 !important;
  border: none;
}
.cm-s-theme .CodeMirror-gutters { color: #222; }
.cm-s-theme .CodeMirror-cursor { border-left: solid thin #f8f8f0; }
.cm-s-theme .CodeMirror-linenumber { color: #6D8A88; }
.cm-s-theme .CodeMirror-selected { background: rgba(255, 255, 255, 0.10); }
.cm-s-theme .CodeMirror-line::selection, .cm-s-theme .CodeMirror-line > span::selection, .cm-s-theme .CodeMirror-line > span > span::selection { background: rgba(255, 255, 255, 0.10); }
.cm-s-theme .CodeMirror-line::-moz-selection, .cm-s-theme .CodeMirror-line > span::-moz-selection, .cm-s-theme .CodeMirror-line > span > span::-moz-selection { background: rgba(255, 255, 255, 0.10); }
.cm-s-theme span.cm-comment { color:  #777; }
.cm-s-theme span.cm-string { color:  #ffb; }
.cm-s-theme span.cm-string-2 { color:  #fbb; }
.cm-s-theme span.cm-number { color:  #b9f; }
.cm-s-theme span.cm-variable { color:  #6fdfcf; }
.cm-s-theme span.cm-property { color:  #ccc; }
.cm-s-theme span.cm-fun { color:  #ccc; }
.cm-s-theme span.cm-operator { color: #fa6; }
.cm-s-theme span.cm-keyword { color: #f6a; }
.cm-s-theme span.cm-type { color:  #1bf; }
.cm-s-theme span.cm-bracket { color: #ccc; }
.cm-s-theme .CodeMirror-activeline-background { background: rgba(255,255,255,0.1); }
.cm-s-theme .CodeMirror-matchingbracket { text-decoration: underline; color: white !important; }
</style>
${download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/codemirror.min.js"              )}
${download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/mode/javascript/javascript.js"  )}
${download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/mode/simple.min.js"       )}
${download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/runmode/runmode.js"       )}
${download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/edit/matchbrackets.min.js")}
${download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/comment/comment.min.js"   )}
<style>.CodeMirror { height: 100%; }</style>

<body style="background:#141414;margin:0px;width:100%;height:100%;overflow:hidden">
  <div id="mb" style="position:absolute;left:0px;top:0px;width:100%;height:30px;overflow:visible;white-space: nowrap;">
    
    <div class="bigbtn" id="btn-compile">
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" >
        <g fill="#111">
          <rect x="8" y="3" width="8" height="4"  />
          <rect x="10" y="3" width="4" height="16"/>
          <rect x="9" y="14" width="6" height="8" />
          <rect x="3" y="2" width="5" height="6"  />
          <polygon points="16,3 22,7, 22,10, 16,7"/>
        </g>
      </svg>
    </div>
  

    <div class="bigbtn" id="btn-run">
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" >
        <polygon points="5,4 21,12 5,20" fill="#111"/>
      </svg>
    </div>

    <div class="bigbtn" id="btn-stop">
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" >
        <rect x="6" y="6" width="12" height="12" fill="#111"/>
      </svg>
    </div>


    <select id="sel-eg" style="-webkit-appearance:none;text-indent:5px;font-size:14px;display:inline-block;width:120px;margin:0px;padding:0px;transform: translateY(-7px); height:23px; color:#000; text-align: left; border:none;margin-left:4px">
      
    </select>

    <div class="bigbtn" id="btn-help">
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" >
        <text x="6" y="19" font-family="sans-serif" font-weight="bold" font-style="normal" font-size="18px" fill="#111">?</text>
      </svg>
    </div>

  </div>
  <div id="cl" style="position:absolute;left:0px;top:30px;width:50%;height:calc(100% - 30px);"></div>
  <div id="cr" style="position:absolute;left:calc(50% + 4px);top:75%;width:25%;height:25%;"></div>
  <div id="cj" style="position:absolute;left:calc(75% + 8px);top:75%;width:25%;height:25%;"></div>
  <div id="out" style="overflow:scroll;background:#252627;color:white;font-family:monospace;position:absolute;left:calc(50% + 4px);top:30px;width:calc(50% - 4px);height:calc(75% - 34px);"></div>
</body>
`];


function main(){
  CodeMirror.defineSimpleMode("dsm", {
    start: [
      {regex: /(?:mov|cast|bnot|lnot|decl|argr|argw|alloc|call|jeqz|fpak|cap|ret|ccall|rcall|jmp|incl|add|sub|mul|div|mod|pow|shl|shr|bor|xor|band|gt|lt|geq|leq|eq|neq|matmul|nop|bloc|end|utag|eoir|dcap)\b/,
        token: "keyword"},
      {regex: /(?:i8|i08|u8|u08|i16|u16|i32|u32|i64|u64|f32|f64|tup|list|vec|lst|arr|dict|dic|str|func|fun|void|union)\b/i,
        token: "type"},
    ],
  });
  var CMR = CodeMirror(document.getElementById("cr"), {
    theme:"theme",
    mode:  "dsm",
  });
  var CMJ = CodeMirror(document.getElementById("cj"), {
    theme:"theme",
    mode:  "javascript",
  });

  console.error = function(x){
    CMR.replaceRange(x, { line: CMR.lineCount(), ch: 0 });
  }

  function compile_from_str(str){
    CMR.setValue("");

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
    let srcmap = parser.writesrcmap(instrs);

    globalThis.__dh_intern_hooked_include = function(x){
      return STD[x];
    } 
    let to_js = new TO_JS({preclude:1});
    let irlo = ir+"\n"+lo;

    let [pir,playout] = to_js.parse_ir(irlo);
    let jj = to_js.transpile(pir,playout);

    CMR.setValue(irlo);
    CMJ.setValue(jj);
  }
  window.CMJ = CMJ;
  function run_from_str(str){
    compile_from_str(str);
    let jj = CMJ.getValue();

    const iframe = document.createElement('iframe');

    iframe.style = "width:100%;height:100%;border:none;";
    
    document.getElementById("out").appendChild(iframe);
    const iframeDoc = iframe.contentDocument || iframe.contentWindow.document;
    const htmlContent = `
      <body style="color:#bbb;font-family:monospace;">${"<"}${"/"}body>
      <script>
        (function(f,e){"object"===typeof exports&&"undefined"!==typeof module?module.exports=e():"function"===typeof define&&define.amd?define(e):f.Stats=e()})(this,function(){var f=function(){function e(a){c.appendChild(a.dom);return a}function u(a){for(var d=0;d<c.children.length;d++)c.children[d].style.display=d===a?"block":"none";l=a}var l=0,c=document.createElement("div");c.style.cssText="position:fixed;top:0;left:0;cursor:pointer;opacity:0.9;z-index:10000";c.addEventListener("click",function(a){a.preventDefault();
u(++l%c.children.length)},!1);var k=(performance||Date).now(),g=k,a=0,r=e(new f.Panel("FPS","#0ff","#002")),h=e(new f.Panel("MS","#0f0","#020"));if(self.performance&&self.performance.memory)var t=e(new f.Panel("MB","#f08","#201"));u(0);return{REVISION:16,dom:c,addPanel:e,showPanel:u,begin:function(){k=(performance||Date).now()},end:function(){a++;var c=(performance||Date).now();h.update(c-k,200);if(c>=g+1E3&&(r.update(1E3*a/(c-g),100),g=c,a=0,t)){var d=performance.memory;t.update(d.usedJSHeapSize/
1048576,d.jsHeapSizeLimit/1048576)}return c},update:function(){k=this.end()},domElement:c,setMode:u}};f.Panel=function(e,f,l){var c=Infinity,k=0,g=Math.round,a=g(window.devicePixelRatio||1),r=80*a,h=48*a,t=3*a,v=2*a,d=3*a,m=15*a,n=74*a,p=30*a,q=document.createElement("canvas");q.width=r;q.height=h;q.style.cssText="width:80px;height:48px";var b=q.getContext("2d");b.font="bold "+9*a+"px Helvetica,Arial,sans-serif";b.textBaseline="top";b.fillStyle=l;b.fillRect(0,0,r,h);b.fillStyle=f;b.fillText(e,t,v);
b.fillRect(d,m,n,p);b.fillStyle=l;b.globalAlpha=.9;b.fillRect(d,m,n,p);return{dom:q,update:function(h,w){c=Math.min(c,h);k=Math.max(k,h);b.fillStyle=l;b.globalAlpha=1;b.fillRect(0,0,r,m);b.fillStyle=f;b.fillText(g(h)+" "+e+" ("+g(c)+"-"+g(k)+")",t,v);b.drawImage(q,d+a,m,n-a,p,d,m,n-a,p);b.fillRect(d+n-a,m,a,p);b.fillStyle=l;b.globalAlpha=.9;b.fillRect(d+n-a,m,a,g((1-h/w)*p))}}};return f});
      var stats=new Stats();
      document.body.appendChild(stats.dom);
      stats.dom.style.left=null;
      stats.dom.style.right=0;
      requestAnimationFrame(function loop(){stats.update();requestAnimationFrame(loop)});
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

  document.getElementById("btn-compile").onclick = function(){
    compile_from_str(CML.getValue());
  }
  document.getElementById("btn-run").onclick = function(){
    document.getElementById("btn-stop").click();
    document.getElementById("out").innerHTML = "";
    run_from_str(CML.getValue());
  }
  document.getElementById("btn-stop").onclick = function(){
    document.getElementById("out").innerHTML = "";
  }

  document.getElementById("cl").addEventListener('keydown', function(e) {
    // console.log(e);
    if (e.key === 'Enter' && e.shiftKey) {
      e.preventDefault();
      document.getElementById("btn-run").onclick();
    }
  });
  CodeMirror.defineSimpleMode("dither", {
    meta:{
      lineComment: "//",
      comment: "//",
    },
    doubleString: [
      { regex: /[^%"\\]+/, token: "string" },         
      { regex: /\\./, token: "string-2" },            
      { regex: /%\{[^}]*\}/, token: "string-2" },         
      { regex: /"/, token: "string", next: "start" }, 
      { regex: /%/, token: "string" },                
    ],
    singleString: [
      { regex: /[^'\\]+/, token: "string" },
      { regex: /\\./, token: "string-2" },
      { regex: /'/, token: "string", next: "start" },
    ],
    start: [
      // {regex: /"(?:[^\\]|\\.)*?(?:"|$)/mi, token: "string"},
      { regex: /"/, token: "string", next: "doubleString" },
      { regex: /'/, token: "string", next: "singleString" },
      {regex: /(?:namespace|continue|typedef|include|return|break|while|const|else|func|for|if|do|as|is|embed)\b/,
        token: "keyword"},
      {regex: /(?:i8|u8|i16|u16|i32|u32|i64|u64|f32|f64|tup|list|vec|arr|dict|str|func)\b/,
        token: "type"},
      {regex: /0x[a-f\d]+|[-+]?(?:\.\d+|\d+\.?\d*)(?:e[-+]?\d+)?/i,
        token: "number"},
      {regex: /\/\*.*?\*\//, token: "comment"},
      {regex: /\/\/.*/, token: "comment"},
      {regex: /[\{\[\(]/, indent: true, token: "bracket"},
      {regex: /[\}\]\)]/, dedent: true, token: "bracket"},
      {regex: /\.(\b[A-Za-z_$][\w$]*)/, token: "property"},
      {regex: /[-+\/*=<>!\?\&\|\^\%\~\#\:\;\,\.]/, token: "operator"},
      {regex: /[A-Za-z$][\w$]*/, token: "variable"},
    ],
  });

  var CML = CodeMirror(document.getElementById("cl"), {
    lineNumbers:true,
    matchBrackets: true,
    theme:"theme",
    mode:  "dither",
    indentWithTabs: false,
    indentUnit: 2,
    tabSize: 2,
    extraKeys:{
      'Ctrl-/': 'toggleComment',
      'Cmd-/': 'toggleComment',
      "Tab": function(cm) {
        cm.replaceSelection("  ", "end");
      }
    }

  });

  CML.setSize(null,null);

  for (let k in EXAMPLES){
    let opt = document.createElement("option");
    opt.value = opt.innerHTML = k;
    document.getElementById("sel-eg").appendChild(opt);
  }

  let b64 = window.location.href.split("?code=")[1];

  if (b64){
    let k = b64.slice(0,10);
    let opt = document.createElement("option");
    opt.value = opt.innerHTML = k;
    document.getElementById("sel-eg").appendChild(opt);
    let a64 = atob(b64);
    EXAMPLES[k] = a64;
    document.getElementById("sel-eg").value = k;
    CML.setValue(a64);

  }else{
    document.getElementById("sel-eg").value = "constellation.dh";
    CML.setValue(EXAMPLES["constellation.dh"]);
  }
  document.getElementById("sel-eg").onchange = function(){
    CML.setValue(EXAMPLES[document.getElementById("sel-eg").value]);
  }

  function clone_button(fid,tid){
    const originalButton = document.getElementById(fid);
    const clonedButton = originalButton.cloneNode(true); 
    clonedButton.removeAttribute("id"); 
    clonedButton.style.cursor = "pointer"; 
    document.getElementById(tid).appendChild(clonedButton);
    clonedButton.addEventListener("click", () => originalButton.click());
  }

  function make_help(e,prolog){

    document.getElementById("out").innerHTML = `
      <div id="doc" style="line-height:20px;padding:20px;padding-left:20px;padding-right:20px;max-width:600px;margin:auto;margin-top:20px;color:#aaa;font-family:sans-serif;background:#141414;font-size:15px">

        ${prolog??""}
        ${DOC}
        </pre>

      </div>`;

    document.querySelectorAll('#doc pre').forEach(pre => {
      pre.classList.add("cm-s-theme");
      CodeMirror.runMode(pre.textContent, "dither", pre);
    })

  }

  document.getElementById("btn-help").onclick = make_help;

  make_help({},`<div style="font-size:15px;font-style:italic">
  <div style="text-align:center;width:100%;">
  <img src="${LOGO}" width="70%" style="filter:invert(92%)"/>
  </div>
  <br>
  Hi 👋, thank you for checking out Dither.
  <br>
  <br>
  You can press the <span id="hbtn-run" class="fakebigbtn"></span> button to run the code, and the output will be displayed here. To bring this document up again, you can press the 
  <span id="hbtn-help" class="fakebigbtn"></span> button.
  <br>
  <br>
  Examples can be selected from the drop down menu.
  <br>
  <br>
  Below is a syntax guide to get you started quickly!
  </div>`)
  clone_button('btn-run','hbtn-run')
  clone_button('btn-help','hbtn-help')
}

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



var unmark = (t)=>(t
  //escape tags
  .replace(/</g,"&lt")
  .replace(/>/g,"&gt")
  //tables
  .replace(/[\n\r^]((((.*\|)+.+)[\n\r$])((\||)((:|)\-+(:|)\|(:|))+\-+(:|)(\||)[\n\r])(((.*\|)+.+)[\n\r$])+)/g,'<p><table>\n$1</table></p>\n')
  .replace(/(\||)((:|)\-+(:|)\|(:|))+\-+(:|)(\||)[\n\r](?=((.*[\n\r])*<\/table>))/g,'')
  .replace(/(((.*\|)+.+))[\n\r$](?=((.*[\n\r])*<\/table>))/g,'  <tr>|$1|</tr>\n')
  .replace(/<tr>\|+(.*)\|+<\/tr>/g,'<tr> <td>$1</td> </tr>')
  .replace(/\|(?=((.+)<\/tr>))/g,'</td> <td>')
  //paragraph
  .replace(/([\n\r^](([^ !\+\*\-\=\#(\n)(\r)>(0-9)`(<.*>)].*?[\n\r^])+))(?=[\n\r$])/g,'\n<p>$2</p>\n')
  //block quote
  .replace(/[\n\r^]> {0,1}> *(.*)/g,'\n> <blockquote>$1</blockquote>')
  .replace(/<\/blockquote>\n*> *<blockquote> *<\/blockquote>\n*> *<blockquote>/g,'<br>')
  .replace(/<\/blockquote>\n*> *<blockquote>/g,' ')
  .replace(/[\n\r^]> *(.*)/g,'\n<blockquote>$1</blockquote>')
  .replace(/<\/blockquote>\n*<blockquote> *<\/blockquote>\n*<blockquote>/g,'<br>')
  .replace(/<\/blockquote>\n*<blockquote>/g,' ')
  //fence code
  // .replace(/[\n\r^]```(.*)[\n\r]((.*[\n\r])*?)```/g,'\n<pre lang="$1">$2</pre>')
  //setext header
  .replace(/(.+)[\n\r]=+[\n\r$]/g,'<h1>$1</h1>\n')
  .replace(/(.+)[\n\r]-+[\n\r$]/g,'<h2>$1</h2>\n')
  //atx header
  .replace(/###### *(.+)[\n\r$]/g,'<h6>$1</h6>\n')
  .replace(/##### *(.+)[\n\r$]/g,'<h5>$1</h5>\n')
  .replace(/#### *(.+)[\n\r$]/g,'<h4>$1</h4>\n')
  .replace(/### *(.+)[\n\r$]/g,'<h3>$1</h3>\n')
  .replace(/## *(.+)[\n\r$]/g,'<h2>$1</h2>\n')
  .replace(/# *(.+)[\n\r$]/g,'<h1>$1</h1>\n')
  //horizontal rule
  .replace(/[\n\r^]([\*\-\_] *){3,}[\n\r$]/g,'\n<hr></hr>\n')
  //unordered list
  .replace(/[\n\r^](((( ){4,}[\*\+\-] .+[\n\r$])((( ){4,}[\*\+\-] .+[\n\r$])|(( ){6,}.*[\n\r$])|[\n\r$])*))/g,'\n    <ul>\n$1    </ul>\n')
  .replace(/[\n\r^]( ){4,}([\*\+\-]) (.+)/g,'\n      <li>$3</li>')
  .replace(/[\n\r^](((( ){2,}[\*\+\-] .+[\n\r$])((( ){2,}[\*\+\-] .+[\n\r$])|(( ){4,}.*[\n\r$])|[\n\r$])*))/g,'\n  <ul>\n$1  </ul>\n')
  .replace(/[\n\r^]( ){2,}([\*\+\-]) (.+)/g,'\n    <li>$3</li>')
  .replace(/[\n\r^](((( ){0,}[\*\+\-] .+[\n\r$])((( ){0,}[\*\+\-] .+[\n\r$])|(( ){2,}.*[\n\r$])|[\n\r$])*))/g,'\n<ul>\n$1</ul>\n')
  .replace(/[\n\r^]( ){0,}([\*\+\-]) (.+)/g,'\n  <li>$3</li>')
  //ordered list
  .replace(/[\n\r^](((( ){4,}[0-9]+\. .+[\n\r$])((( ){4,}[0-9]+\. .+[\n\r$])|(( ){6,}.*[\n\r$])|[\n\r$])*))/g,'\n    <ol>\n$1    </ol>\n')
  .replace(/[\n\r^]( ){4,}([0-9]+\.) (.+)/g,'\n      <li>$3</li>')
  .replace(/[\n\r^](((( ){2,}[0-9]+\. .+[\n\r$])((( ){2,}[0-9]+\. .+[\n\r$])|(( ){4,}.*[\n\r$])|[\n\r$])*))/g,'\n  <ol>\n$1  </ol>\n')
  .replace(/[\n\r^]( ){3,}([0-9]+\.) (.+)/g,'\n    <li>$3</li>')
  .replace(/[\n\r^](((( ){0,}[0-9]+\. .+[\n\r$])((( ){0,}[0-9]+\. .+[\n\r$])|(( ){2,}.*[\n\r$])|[\n\r$])*))/g,'\n<ol>\n$1</ol>\n')
  .replace(/[\n\r^]( ){0,}([0-9]+\.) (.+)/g,'\n  <li>$3</li>')
  //em & strong & code
  .replace(/([^\\])__(.*?[^\n\r\\])__/g,'$1<strong>$2</strong>')
  .replace(/([^\\])\*\*(.*?[^\n\r\\])\*\*/g,'$1<strong>$2</strong>')
  .replace(/([^\\])\*(.*?[^\n\r\\])\*/g,'$1<em>$2</em>')
  .replace(/([^\\])`(.*?[^\n\r\\])`/g,'$1<code>$2</code>')
  //image & link
  .replace(/!\[\]\((.*?)\)/g,'<img src="$1" alt=""/>')
  .replace(/!\[(.*?)\]\((.*?)\)/g,'<figure><img src="$2" alt="$1"/><figcaption>$1</figcaption></figure>')
  .replace(/\[(.*?)\]\((.*?)\)/g,'<a href="$2">$1</a>')
  //escape
  .replace(/\\(\*|_|`)/g,'$1')
)

var bimg = fs.readFileSync("doc/logo.png").toString('base64');
html.push(`<script>var LOGO = "data:image/png;base64,${bimg}" </script>`);

let txt = fs.readFileSync("SYNTAX.md").toString().replace(/</g,'&lt;').replace(/>/g,'&gt;');
txt = txt.split("```");
for (let i = 0; i < txt.length; i+=2){
  let un = unmark(txt[i]);;
  txt[i] = "";
  if (i) txt[i] += `</pre>`;
  txt[i] += un;
  if (i != txt.length-1) txt[i] += `<pre>`
}
for (let i = 1; i < txt.length; i+=2){
  txt[i] = txt[i].trim();
}
txt = txt.join(`\n`);
html.push(`<script>var DOC = \`${txt}\`</script>`);



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



fs.writeFileSync("build/editor.html",html.join("\n"));

