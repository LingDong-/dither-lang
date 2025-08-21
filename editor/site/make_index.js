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

var bimg = fs.readFileSync("doc/logo.png").toString('base64');

let html = [`
<meta charset="UTF-8">
<style>

a:link {
  color: #e51;
}
a:visited {
  color: #e51;
}
a:hover {
  color: #e51;
}
a:active {
  color: #e51;
}
.biglink{
  color:black;
  text-decoration: none;
  border: 1px solid black;
  width:120px;
  font-size:14px;
  height:18px;
  line-height:20px;
  text-align:center;
  cursor:pointer;
  margin:4px;
}
.biglink:hover{
  color:white;
  background:black;
}

</style>
<style>
.CodeMirror{font-family:monospace;height:300px;color:#000;direction:ltr}.CodeMirror-lines{padding:4px 0}.CodeMirror pre.CodeMirror-line,.CodeMirror pre.CodeMirror-line-like{padding:0 4px}.CodeMirror-gutter-filler,.CodeMirror-scrollbar-filler{background-color:#fff}.CodeMirror-gutters{border-right:1px solid #ddd;background-color:#f7f7f7;white-space:nowrap}.CodeMirror-linenumber{padding:0 3px 0 5px;min-width:20px;text-align:right;color:#999;white-space:nowrap}.CodeMirror-guttermarker{color:#000}.CodeMirror-guttermarker-subtle{color:#999}.CodeMirror-cursor{border-left:1px solid #000;border-right:none;width:0}.CodeMirror div.CodeMirror-secondarycursor{border-left:1px solid silver}.cm-fat-cursor .CodeMirror-cursor{width:auto;border:0!important;background:#7e7}.cm-fat-cursor div.CodeMirror-cursors{z-index:1}.cm-fat-cursor-mark{background-color:rgba(20,255,20,.5);-webkit-animation:blink 1.06s steps(1) infinite;-moz-animation:blink 1.06s steps(1) infinite;animation:blink 1.06s steps(1) infinite}.cm-animate-fat-cursor{width:auto;border:0;-webkit-animation:blink 1.06s steps(1) infinite;-moz-animation:blink 1.06s steps(1) infinite;animation:blink 1.06s steps(1) infinite;background-color:#7e7}@-moz-keyframes blink{50%{background-color:transparent}}@-webkit-keyframes blink{50%{background-color:transparent}}@keyframes blink{50%{background-color:transparent}}.cm-tab{display:inline-block;text-decoration:inherit}.CodeMirror-rulers{position:absolute;left:0;right:0;top:-50px;bottom:0;overflow:hidden}.CodeMirror-ruler{border-left:1px solid #ccc;top:0;bottom:0;position:absolute}.cm-s-default .cm-header{color:#00f}.cm-s-default .cm-quote{color:#090}.cm-negative{color:#d44}.cm-positive{color:#292}.cm-header,.cm-strong{font-weight:700}.cm-em{font-style:italic}.cm-link{text-decoration:underline}.cm-strikethrough{text-decoration:line-through}.cm-s-default .cm-keyword{color:#708}.cm-s-default .cm-atom{color:#219}.cm-s-default .cm-number{color:#164}.cm-s-default .cm-def{color:#00f}.cm-s-default .cm-variable-2{color:#05a}.cm-s-default .cm-type,.cm-s-default .cm-variable-3{color:#085}.cm-s-default .cm-comment{color:#a50}.cm-s-default .cm-string{color:#a11}.cm-s-default .cm-string-2{color:#f50}.cm-s-default .cm-meta{color:#555}.cm-s-default .cm-qualifier{color:#555}.cm-s-default .cm-builtin{color:#30a}.cm-s-default .cm-bracket{color:#997}.cm-s-default .cm-tag{color:#170}.cm-s-default .cm-attribute{color:#00c}.cm-s-default .cm-hr{color:#999}.cm-s-default .cm-link{color:#00c}.cm-s-default .cm-error{color:red}.cm-invalidchar{color:red}.CodeMirror-composing{border-bottom:2px solid}div.CodeMirror span.CodeMirror-matchingbracket{color:#0b0}div.CodeMirror span.CodeMirror-nonmatchingbracket{color:#a22}.CodeMirror-matchingtag{background:rgba(255,150,0,.3)}.CodeMirror-activeline-background{background:#e8f2ff}.CodeMirror{position:relative;overflow:hidden;background:#fff}.CodeMirror-scroll{overflow:scroll!important;margin-bottom:-50px;margin-right:-50px;padding-bottom:50px;height:100%;outline:0;position:relative}.CodeMirror-sizer{position:relative;border-right:50px solid transparent}.CodeMirror-gutter-filler,.CodeMirror-hscrollbar,.CodeMirror-scrollbar-filler,.CodeMirror-vscrollbar{position:absolute;z-index:6;display:none;outline:0}.CodeMirror-vscrollbar{right:0;top:0;overflow-x:hidden;overflow-y:scroll}.CodeMirror-hscrollbar{bottom:0;left:0;overflow-y:hidden;overflow-x:scroll}.CodeMirror-scrollbar-filler{right:0;bottom:0}.CodeMirror-gutter-filler{left:0;bottom:0}.CodeMirror-gutters{position:absolute;left:0;top:0;min-height:100%;z-index:3}.CodeMirror-gutter{white-space:normal;height:100%;display:inline-block;vertical-align:top;margin-bottom:-50px}.CodeMirror-gutter-wrapper{position:absolute;z-index:4;background:0 0!important;border:none!important}.CodeMirror-gutter-background{position:absolute;top:0;bottom:0;z-index:4}.CodeMirror-gutter-elt{position:absolute;cursor:default;z-index:4}.CodeMirror-gutter-wrapper ::selection{background-color:transparent}.CodeMirror-gutter-wrapper ::-moz-selection{background-color:transparent}.CodeMirror-lines{cursor:text;min-height:1px}.CodeMirror pre.CodeMirror-line,.CodeMirror pre.CodeMirror-line-like{-moz-border-radius:0;-webkit-border-radius:0;border-radius:0;border-width:0;background:0 0;font-family:inherit;font-size:inherit;margin:0;white-space:pre;word-wrap:normal;line-height:inherit;color:inherit;z-index:2;position:relative;overflow:visible;-webkit-tap-highlight-color:transparent;-webkit-font-variant-ligatures:contextual;font-variant-ligatures:contextual}.CodeMirror-wrap pre.CodeMirror-line,.CodeMirror-wrap pre.CodeMirror-line-like{word-wrap:break-word;white-space:pre-wrap;word-break:normal}.CodeMirror-linebackground{position:absolute;left:0;right:0;top:0;bottom:0;z-index:0}.CodeMirror-linewidget{position:relative;z-index:2;padding:.1px}.CodeMirror-rtl pre{direction:rtl}.CodeMirror-code{outline:0}.CodeMirror-gutter,.CodeMirror-gutters,.CodeMirror-linenumber,.CodeMirror-scroll,.CodeMirror-sizer{-moz-box-sizing:content-box;box-sizing:content-box}.CodeMirror-measure{position:absolute;width:100%;height:0;overflow:hidden;visibility:hidden}.CodeMirror-cursor{position:absolute;pointer-events:none}.CodeMirror-measure pre{position:static}div.CodeMirror-cursors{visibility:hidden;position:relative;z-index:3}div.CodeMirror-dragcursors{visibility:visible}.CodeMirror-focused div.CodeMirror-cursors{visibility:visible}.CodeMirror-selected{background:#d9d9d9}.CodeMirror-focused .CodeMirror-selected{background:#d7d4f0}.CodeMirror-crosshair{cursor:crosshair}.CodeMirror-line::selection,.CodeMirror-line>span::selection,.CodeMirror-line>span>span::selection{background:#d7d4f0}.CodeMirror-line::-moz-selection,.CodeMirror-line>span::-moz-selection,.CodeMirror-line>span>span::-moz-selection{background:#d7d4f0}.cm-searching{background-color:#ffa;background-color:rgba(255,255,0,.4)}.cm-force-border{padding-right:.1px}@media print{.CodeMirror div.CodeMirror-cursors{visibility:hidden}}.cm-tab-wrap-hack:after{content:''}span.CodeMirror-selectedtext{background:0 0}
</style>
<style>
.cm-s-theme.CodeMirror, .cm-s-theme .CodeMirror-gutters {
  background-color: #fff!important;
  color: #000 !important;
  border: none;
}
.cm-s-theme .CodeMirror-cursor { border-left: solid thin #000; }
.cm-s-theme .CodeMirror-linenumber { color: #777; }
.cm-s-theme .CodeMirror-selected { background: #def); }
.cm-s-theme span.cm-comment { color:  #777; }
.cm-s-theme span.cm-string { color:  #765; }
.cm-s-theme span.cm-string-2 { color:  #765; }
.cm-s-theme span.cm-number { color:  #671; }
.cm-s-theme span.cm-variable { color:  #000; font-style:italic }
.cm-s-theme span.cm-property { color:  #333; }
.cm-s-theme span.cm-operator { color: #000; font-weight:bold}
.cm-s-theme span.cm-keyword { color: #e51; font-weight:bold}
.cm-s-theme span.cm-type { color:  #37d; }
.cm-s-theme span.cm-bracket { color: #000; }
</style>
${download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/codemirror.min.js"              )}
${download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/mode/simple.min.js"       )}
<style>.CodeMirror { height: 100%; }</style>

<div style="width:720px;margin:auto;font-size:15px;font-family:sans-serif;line-height:24px">

<table style="width:720px">
<tr>
<td>
<img src="data:image/png;base64,${bimg}" style="width:360px;transform:translate(-20px,0)"/>
</td>
<td style="width:10px">
<a href="https://github.com/LingDong-/dither-lang/releases" style="text-decoration:none"><div class="biglink">Download</div></a>
<a href="editor.html" style="text-decoration:none"><div class="biglink">Online Editor</div></a>
<a href="blocks.html" style="text-decoration:none"><div class="biglink">Blocks Editor</div></a>
</td>
<td style="width:10px">
<a href="https://github.com/LingDong-/dither-lang/tree/main/examples" style="text-decoration:none"><div class="biglink">Examples</div></a>
<a href="https://github.com/LingDong-/dither-lang/blob/main/SYNTAX.md" style="text-decoration:none"><div class="biglink">Documentation</div></a>
<a href="https://github.com/LingDong-/dither-lang" style="text-decoration:none"><div class="biglink">Source Code</div></a>
</td>
</tr>
</table>

<p>
Dither is a programming language for creative coding.
<ul>
<li><b>Compiled<sup><a href="https://en.wikipedia.org/wiki/Compiler">?</a></sup> or interpreted<sup><a href="https://en.wikipedia.org/wiki/Interpreter_(computing)">?</a></sup>: your choice.</b>
Use interpreted mode for fast iteration, and compile your code when
you're done for fast execution.
</li>
<li><b>CPU and GPU, web or native: write once.</b>
Dither code can be directly compiled to shaders<sup><a href="https://en.wikipedia.org/wiki/Shader">?</a></sup>.
Dither and its standard libraries runs natively on windows, mac, linux and the web.
</li>
<li><b>Automatic inference of variable types.</b><sup><a href="https://en.wikipedia.org/wiki/Type_inference">?</a></sup>
The merits of types without the hassle of writing them.
</li>
<li><b>Vector math, built-in.</b>
Vector and matrix operations are integrated into the syntax of Dither.
</li>
<li><b>Pixels-oriented.</b>
Standard libraries
focused on getting pixels onto your screen (or sound to your speakers), ASAP.
</li>
</ul>
</p>

<p>
Here's a quick taste of Dither's syntax:
</p>

<div class="dh">
include "std/math"
include "std/time"
include "std/vec"
include "std/win"
include "std/frag"

// initialize window and context
frag.init(win.init(200,200,win.CONTEXT_3D))

// ordered dither pattern lookup
ord_dith_map := arr[i32,2]{
   0,48,12,60, 3,51,15,63; 32,16,44,28,35,19,47,31;
   8,56, 4,52,11,59, 7,55; 40,24,36,20,43,27,39,23;
   2,50,14,62, 1,49,13,61; 34,18,46,30,33,17,45,29;
  10,58, 6,54, 9,57, 5,53; 42,26,38,22,41,25,37,21;
};

// write your shader logic in dither
shader := frag.program(embed (func(
  @builtin frag_coord : vec[f32,4],
  @varying uv         : vec[f32,2],
  @uniform t          : f32
                     ): vec[f32,4]{
  c:= (frag_coord.xy as vec[i32,2])/2;
  u:= (uv-0.5)*5.0;
  o:= (math.sin(u.dot(u)**0.5*u.x+t*0.01)*0.35+0.5)
  d:= (ord_dith_map[c.y%8,c.x%8] as f32)/64.0
  return {o&lt;d,o&lt;d,o&lt;d,1.0};
}) as "fragment")

// render loop
while (1){
  frag.begin(shader);
  frag.uniform("t", time.millis() as f32);
  frag.render();
  frag.end();
  win.poll();
}
</div>

<p>
But let's start from the basics! Here's a "hello world" program in Dither.
</p>

<div class="dh">
include "std/io"

io.println("hello world from dither!")
</div>

<p>
You might find the syntax of Dither familiar if you came from C-like languages.
In case you haven't noticed, you can modify example programs (like the one below)
and run them by clicking the ▶ button. If you don't feel like typing, you can try
out the Scratch-like, <a href="blocks.html">block-based editor</a>.
</p>

<div class="dh">
include "std/io"

func fact(x:i32):i32{
  if (x) return x * fact(x-1);
  return 1;
}

x := 7
y := fact(x)
io.println("factorial of %{x} is %{y}");

</div>

<p>
The standard libraries give us options
when it comes to delivering pixels onto the screen, 
one of which allows you to draw things the easy way, à la Processing-style.
Welcome back, <code>pushMatrix()</code>!
</p>

<div class="dh">
include "std/gx"

gx.size(200,200);

func tree(l:f32){
  if (l < 8) return;
  gx.line(0,0,l,0);
  gx.translate(l,0);
  gx.rotate_deg(-15);
  for (i := 0; i < 2; i++){
    gx.push_matrix();
    tree(l*0.8);
    gx.pop_matrix();
    gx.rotate_deg(30);
  }
}

while (1){
  gx.background(0.9);
  gx.stroke(0.);
  gx.push_matrix();
  gx.translate(100,200);
  gx.rotate_deg(-90);
  tree(42);
  gx.pop_matrix();
  gx.poll();
}

</div>

<p>
Now let's make some sound! 
Below we play a chromatic scale by directly putting data into your sound card.
(Click the little speaker button).
</p>

<div class="dh">
include "std/snd"
include "std/math"

rate := 10000
snd.init(rate,1);

func note(freq:f32, sec:f32){
  t := 0.0;
  while (t < sec){
    if (!snd.buffer_full()){
      x := math.sin( t * 2.0 * math.PI * freq ) 
      env := math.sin( t * math.PI / sec);
      snd.put_sample(x*env);
      t += 1.0/rate;
    }
  }
}
for (i:=0; i<12; i++){
  freq := 440 * 2**(i/12.0);
  note(freq, 0.25);
}
</div>

<p>
We can also easily create GPU-accelerated 3D graphics that work across different platforms.
Below we generate a simple terrain and render it from a camera.
We can also combine it with the fragment module to create
more interesting shading effects -- check out <a href="https://github.com/LingDong-/dither-lang/tree/main/examples">examples</a>!
</p>

<div class="dh">

include "std/g3d"
include "std/win"
include "std/list"
include "std/rand"

W := 200;
H := 200;
nx := 20;
nz := 20;

g3d.init(win.init(W,H,win.CONTEXT_3D));

mesh := g3d.Mesh{};

for (i:=0; i&lt;nz; i++){
  for (j:=0; j&lt;nx; j++){
    h := rand.noise(i*0.2,j*0.2);
    mesh.vertices.push(
      {j-nx*0.5, h*10.0-5.0, i-nz*0.5}
    );
    mesh.colors.push({0.5+h*0.5,h,0.5-h*0.5,1});
    if (i&&j){
      mesh.indices.push(i*nx+j);
      mesh.indices.push(i*nx+j-1);
      mesh.indices.push((i-1)*nx+j);
      mesh.indices.push(i*nx+j-1);
      mesh.indices.push((i-1)*nx+j-1);
      mesh.indices.push((i-1)*nx+j);
    }
  }
}
cam := g3d.Camera{}
cam.look_at({0,15,15},{0,0,0},g3d.AXIS_Y);
cam.perspective(45,W/(H as f32),0.1,100.0);

frame := 0;
while (1){
  g3d.background(0.2,0.0,0.1);

  cam.begin();
  mesh.draw(g3d.mat.rotate_deg(g3d.AXIS_Y,frame++));
  cam.end();
  
  win.poll();
}
</div>

<p>
Now that we've gotten a sneak peek of what Dither is capable of, feel free to 
give it a try in the <a href="editor.html">online editor</a>,
<a href="https://github.com/LingDong-/dither-lang/releases">download Dither</a>,
or check out even more <a href="https://github.com/LingDong-/dither-lang/tree/main/examples">examples</a>!
</p>


<p style="text-align:right">
<br><br><br>
<small>Lingdong Huang, 2025-?, Future Sketches Group, MIT Media Lab</small>
</p>

</div>
</body>
`];


function main(){
  function compile_from_str(str){
    let fs = {
      readFileSync:function(x){
        // console.log(x);
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
    let srcmap = parser.writesrcmap(instrs);

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
        const response = await fetch(pth, { method: 'GET' });
        const arrayBuffer = await response.arrayBuffer();
        let arr = Array.from(new Uint8Array(arrayBuffer));
        return arr;
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
      {regex: /[-+\/*=<>!\?\&\|\^\%\~\#\:\;\,\.\@]/, token: "operator"},
      {regex: /[A-Za-z$][\w$]*/, token: "variable"},
    ],
  });

  function make_widget(par,text){
    let div = document.createElement("div");
    div.style="font-size:14px;line-height:18px;width:720px;height:240px;border-radius:5px;border:1px solid silver;box-shadow: 2px 2px 2px rgba(0,0,0,0.3);overflow:hidden;"
    div.innerHTML = `
      <div style="position:relative">
        <div class="edit" style="width:480px;height:240px;position:absolute;"></div>
        <div class="out" style="position:absolute;left:480px;top:0px;width:240px;height:240px;overflow:hidden;border-left:1px solid silver"></div>
        <button class="play" style="position:absolute;left:430px;top:2px;width:20px;height:20px;font-size:16px;line-height:16px;color:#222;padding:0px;z-index:1000;text-align:center;">▶</button>
      </div>
    `
    par.appendChild(div);
    let cml = CodeMirror(div.getElementsByClassName("edit")[0], {
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
    cml.setValue(text)
    cml.setSize(null,null);
    let btn = div.getElementsByClassName("play")[0];
    let out = div.getElementsByClassName("out")[0];
    btn.onclick = function(){
      out.innerHTML = "";
      run_from_str(cml.getValue(),out);
    }
    btn.onclick();
    div.addEventListener('keydown', function(e) {
      if (e.key === 'Enter' && e.shiftKey) {
        e.preventDefault();
        btn.onclick();
      }
    });
    
  }
  Array.from(document.getElementsByClassName("dh")).forEach(elem => {
    text = elem.textContent.trim();
    elem.innerHTML = "";
    make_widget(elem,text);
  });

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


html.push(`<script>${main.toString()};main();</script>`)


fs.writeFileSync("build/index.html",html.join("\n"));

