const fs = require('fs');

const { execSync } = require('child_process');
let do_download = 1;

function download(url) {
  if (do_download){
    console.log("downloading... "+url)
    return `${execSync(`curl -sL "${url}"`, { encoding: 'utf8' })}`;
  }
  return `{
let s = document.createElement("script");
s.src = "${url}";
document.body.appendChild(s);
}`;
}

let js = ["(function dither_embed_all(){"];
let css = `
.CodeMirror{font-family:monospace;height:300px;color:#000;direction:ltr}.CodeMirror-lines{padding:4px 0}.CodeMirror pre.CodeMirror-line,.CodeMirror pre.CodeMirror-line-like{padding:0 4px}.CodeMirror-gutter-filler,.CodeMirror-scrollbar-filler{background-color:#fff}.CodeMirror-gutters{border-right:1px solid #ddd;background-color:#f7f7f7;white-space:nowrap}.CodeMirror-linenumber{padding:0 3px 0 5px;min-width:20px;text-align:right;color:#999;white-space:nowrap}.CodeMirror-guttermarker{color:#000}.CodeMirror-guttermarker-subtle{color:#999}.CodeMirror-cursor{border-left:1px solid #000;border-right:none;width:0}.CodeMirror div.CodeMirror-secondarycursor{border-left:1px solid silver}.cm-fat-cursor .CodeMirror-cursor{width:auto;border:0!important;background:#7e7}.cm-fat-cursor div.CodeMirror-cursors{z-index:1}.cm-fat-cursor-mark{background-color:rgba(20,255,20,.5);-webkit-animation:blink 1.06s steps(1) infinite;-moz-animation:blink 1.06s steps(1) infinite;animation:blink 1.06s steps(1) infinite}.cm-animate-fat-cursor{width:auto;border:0;-webkit-animation:blink 1.06s steps(1) infinite;-moz-animation:blink 1.06s steps(1) infinite;animation:blink 1.06s steps(1) infinite;background-color:#7e7}@-moz-keyframes blink{50%{background-color:transparent}}@-webkit-keyframes blink{50%{background-color:transparent}}@keyframes blink{50%{background-color:transparent}}.cm-tab{display:inline-block;text-decoration:inherit}.CodeMirror-rulers{position:absolute;left:0;right:0;top:-50px;bottom:0;overflow:hidden}.CodeMirror-ruler{border-left:1px solid #ccc;top:0;bottom:0;position:absolute}.cm-s-default .cm-header{color:#00f}.cm-s-default .cm-quote{color:#090}.cm-negative{color:#d44}.cm-positive{color:#292}.cm-header,.cm-strong{font-weight:700}.cm-em{font-style:italic}.cm-link{text-decoration:underline}.cm-strikethrough{text-decoration:line-through}.cm-s-default .cm-keyword{color:#708}.cm-s-default .cm-atom{color:#219}.cm-s-default .cm-number{color:#164}.cm-s-default .cm-def{color:#00f}.cm-s-default .cm-variable-2{color:#05a}.cm-s-default .cm-type,.cm-s-default .cm-variable-3{color:#085}.cm-s-default .cm-comment{color:#a50}.cm-s-default .cm-string{color:#a11}.cm-s-default .cm-string-2{color:#f50}.cm-s-default .cm-meta{color:#555}.cm-s-default .cm-qualifier{color:#555}.cm-s-default .cm-builtin{color:#30a}.cm-s-default .cm-bracket{color:#997}.cm-s-default .cm-tag{color:#170}.cm-s-default .cm-attribute{color:#00c}.cm-s-default .cm-hr{color:#999}.cm-s-default .cm-link{color:#00c}.cm-s-default .cm-error{color:red}.cm-invalidchar{color:red}.CodeMirror-composing{border-bottom:2px solid}div.CodeMirror span.CodeMirror-matchingbracket{color:#0b0}div.CodeMirror span.CodeMirror-nonmatchingbracket{color:#a22}.CodeMirror-matchingtag{background:rgba(255,150,0,.3)}.CodeMirror-activeline-background{background:#e8f2ff}.CodeMirror{position:relative;overflow:hidden;background:#fff}.CodeMirror-scroll{overflow:scroll!important;margin-bottom:-50px;margin-right:-50px;padding-bottom:50px;height:100%;outline:0;position:relative}.CodeMirror-sizer{position:relative;border-right:50px solid transparent}.CodeMirror-gutter-filler,.CodeMirror-hscrollbar,.CodeMirror-scrollbar-filler,.CodeMirror-vscrollbar{position:absolute;z-index:6;display:none;outline:0}.CodeMirror-vscrollbar{right:0;top:0;overflow-x:hidden;overflow-y:scroll}.CodeMirror-hscrollbar{bottom:0;left:0;overflow-y:hidden;overflow-x:scroll}.CodeMirror-scrollbar-filler{right:0;bottom:0}.CodeMirror-gutter-filler{left:0;bottom:0}.CodeMirror-gutters{position:absolute;left:0;top:0;min-height:100%;z-index:3}.CodeMirror-gutter{white-space:normal;height:100%;display:inline-block;vertical-align:top;margin-bottom:-50px}.CodeMirror-gutter-wrapper{position:absolute;z-index:4;background:0 0!important;border:none!important}.CodeMirror-gutter-background{position:absolute;top:0;bottom:0;z-index:4}.CodeMirror-gutter-elt{position:absolute;cursor:default;z-index:4}.CodeMirror-gutter-wrapper ::selection{background-color:transparent}.CodeMirror-gutter-wrapper ::-moz-selection{background-color:transparent}.CodeMirror-lines{cursor:text;min-height:1px}.CodeMirror pre.CodeMirror-line,.CodeMirror pre.CodeMirror-line-like{-moz-border-radius:0;-webkit-border-radius:0;border-radius:0;border-width:0;background:0 0;font-family:inherit;font-size:inherit;margin:0;white-space:pre;word-wrap:normal;line-height:inherit;color:inherit;z-index:2;position:relative;overflow:visible;-webkit-tap-highlight-color:transparent;-webkit-font-variant-ligatures:contextual;font-variant-ligatures:contextual}.CodeMirror-wrap pre.CodeMirror-line,.CodeMirror-wrap pre.CodeMirror-line-like{word-wrap:break-word;white-space:pre-wrap;word-break:normal}.CodeMirror-linebackground{position:absolute;left:0;right:0;top:0;bottom:0;z-index:0}.CodeMirror-linewidget{position:relative;z-index:2;padding:.1px}.CodeMirror-rtl pre{direction:rtl}.CodeMirror-code{outline:0}.CodeMirror-gutter,.CodeMirror-gutters,.CodeMirror-linenumber,.CodeMirror-scroll,.CodeMirror-sizer{-moz-box-sizing:content-box;box-sizing:content-box}.CodeMirror-measure{position:absolute;width:100%;height:0;overflow:hidden;visibility:hidden}.CodeMirror-cursor{position:absolute;pointer-events:none}.CodeMirror-measure pre{position:static}div.CodeMirror-cursors{visibility:hidden;position:relative;z-index:3}div.CodeMirror-dragcursors{visibility:visible}.CodeMirror-focused div.CodeMirror-cursors{visibility:visible}.CodeMirror-selected{background:#d9d9d9}.CodeMirror-focused .CodeMirror-selected{background:#d7d4f0}.CodeMirror-crosshair{cursor:crosshair}.CodeMirror-line::selection,.CodeMirror-line>span::selection,.CodeMirror-line>span>span::selection{background:#d7d4f0}.CodeMirror-line::-moz-selection,.CodeMirror-line>span::-moz-selection,.CodeMirror-line>span>span::-moz-selection{background:#d7d4f0}.cm-searching{background-color:#ffa;background-color:rgba(255,255,0,.4)}.cm-force-border{padding-right:.1px}@media print{.CodeMirror div.CodeMirror-cursors{visibility:hidden}}.cm-tab-wrap-hack:after{content:''}span.CodeMirror-selectedtext{background:0 0}
.cm-s-dither-light-theme.CodeMirror, .cm-s-dither-light-theme .CodeMirror-gutters {
  background-color: #fff!important;
  color: #000 !important;
  border: none;
}
.cm-s-dither-light-theme .CodeMirror-cursor { border-left: solid thin #000; }
.cm-s-dither-light-theme .CodeMirror-linenumber { color: #777; }
.cm-s-dither-light-theme .CodeMirror-selected { background: #def); }
.cm-s-dither-light-theme span.cm-comment { color:  #777; }
.cm-s-dither-light-theme span.cm-string { color:  #765; }
.cm-s-dither-light-theme span.cm-string-2 { color:  #765; }
.cm-s-dither-light-theme span.cm-number { color:  #671; }
.cm-s-dither-light-theme span.cm-variable { color:  #000; font-style:italic }
.cm-s-dither-light-theme span.cm-property { color:  #333; }
.cm-s-dither-light-theme span.cm-operator { color: #000; font-weight:bold}
.cm-s-dither-light-theme span.cm-keyword { color: #e51; font-weight:bold}
.cm-s-dither-light-theme span.cm-type { color:  #37d; }
.cm-s-dither-light-theme span.cm-bracket { color: #000; }
.CodeMirror { height: 100%; }
`;

js.push(download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/codemirror.min.js"));
js.push(download("https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/mode/simple.min.js"));
js.push(`{
  let css = document.createElement("style");
  css.textContent = \`${css}\`;
  document.head.appendChild(css);
}`);

function main(){
  let olderror = console.error;

  function compile_from_str(str,outdiv){
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
    console.error = function(x){
      let errdiv;
      if (!(errdiv = outdiv.getElementsByClassName("dh-err")[0])){
        errdiv = document.createElement("div");
        errdiv.style = "width:100%;overflow:scroll";
        errdiv.classList.add("dh-err");
        outdiv.appendChild(errdiv);
      }
      olderror(x);
      let div = document.createElement("pre");
      div.style = "margin:0px;color:#e51;font-size:13px;"
      div.innerHTML = x
      errdiv.appendChild(div);
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
    console.error = olderror;
    return [irlo,jj];
  }

  function run_from_str(str,outdiv){
    let irlo,jj;

    try{
      ;[irlo,jj] = compile_from_str(str,outdiv);
    }catch(e){
      console.error = olderror;
      throw e;
    }
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

  let states = {};
  const callback = (entries, observer) => {
    entries.forEach(entry => {
      if (entry.isIntersecting) {
        states[entry.target.id] = 1;
        entry.target.getElementsByClassName("dh-play")[0].onclick();
        // observer.unobserve(entry.target);
      }else if (states[entry.target.id]){
        states[entry.target.id] = 0;
        entry.target.getElementsByClassName("dh-out").innerHTML="";
      }
    });
  };
  const observer = new IntersectionObserver(callback, {
    root: null,
    rootMargin: '0px',
    threshold: 0.01
  });

  function make_widget(par,text,options){
    let div = document.createElement("div");
    div.id = "embed-"+Math.random().toString().slice(2);
    let h0 = 240;
    if (options.bleed){
      h0 = 640;
      div.style=`font-size:14px;line-height:18px;width:100vw;margin-left:calc(50% - 50vw);margin-right:calc(50% - 50vw);height:${h0}px;overflow:hidden;`;
      div.innerHTML = `
        <div style="position:relative;height:100%;width:100%;">
          <div class="dh-edit" style="width:50%;height:100%;position:absolute;"></div>
          <div class="dh-out" style="position:absolute;left:50%;top:0px;width:50%;height:100%;overflow:hidden;border-left:1px solid silver;background:white;"></div>
          <button class="dh-play" style="position:absolute;left:calc(50% - 50px);top:4px;width:20px;height:20px;font-size:16px;line-height:16px;color:#222;padding:0px;z-index:1000;text-align:center;">▶</button>
          <button class="dh-menu" style="position:absolute;left:calc(50% - 75px);top:4px;width:20px;height:20px;font-size:16px;line-height:16px;color:#222;padding:0px;z-index:1000;text-align:center;"><span style="position:relative;top:-1px;">☰</span></button>
          <div class="dh-drawer" style="display:none;position:absolute;left:calc(50% - 75px);top:28px;width:45px;z-index:1000;">
            <button class="dh-stop" style="width:72px;height:20px;color:#222;padding:0px;text-align:left;">&nbsp;stop</button>
            <button class="dh-reset" style="width:72px;height:20px;color:#222;padding:0px;text-align:left;">&nbsp;reset</button>
            <button class="dh-expand" style="width:72px;height:20px;color:#222;padding:0px;text-align:left;">&nbsp;expand</button>
          </div>
          <div style="box-shadow: inset 0px 0px 10px rgba(0,0,0,0.3), inset 0px 0px 10px rgba(0,0,0,0.3);position:absolute;left:-10%;top:0px;width:120%;height:100%;pointer-events:none;z-index:1000;"></div>
        </div>
        
      `
    }else{
      h0 = 240;
      div.style="font-size:14px;line-height:18px;width:720px;height:240px;border-radius:5px;border:1px solid silver;box-shadow: 2px 2px 2px rgba(0,0,0,0.3);overflow:hidden;"
      div.innerHTML = `
        <div style="position:relative;height:100%">
          <div class="dh-edit" style="width:480px;height:100%;position:absolute;"></div>
          <div class="dh-out" style="position:absolute;left:480px;top:0px;width:240px;height:100%;overflow:hidden;border-left:1px solid silver;background:white;"></div>
          <button class="dh-play" style="position:absolute;left:430px;top:2px;width:20px;height:20px;font-size:16px;line-height:16px;color:#222;padding:0px;z-index:1000;text-align:center;">▶</button>
          <button class="dh-menu" style="position:absolute;left:405px;top:2px;width:20px;height:20px;font-size:16px;line-height:16px;color:#222;padding:0px;z-index:1000;text-align:center;"><span style="position:relative;top:-1px;">☰</span></button>
          <div class="dh-drawer" style="display:none;position:absolute;left:405px;top:25px;width:45px;z-index:1000;">
            <button class="dh-stop" style="width:72px;height:20px;color:#222;padding:0px;text-align:left;">&nbsp;stop</button>
            <button class="dh-reset" style="width:72px;height:20px;color:#222;padding:0px;text-align:left;">&nbsp;reset</button>
            <button class="dh-expand" style="width:72px;height:20px;color:#222;padding:0px;text-align:left;">&nbsp;expand</button>
          </div>
        </div>
      `
    }
    par.appendChild(div);
    let cml = CodeMirror(div.getElementsByClassName("dh-edit")[0], {
      lineNumbers:true,
      matchBrackets: true,
      theme:"dither-light-theme",
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
    let btn = div.getElementsByClassName("dh-play")[0];
    let out = div.getElementsByClassName("dh-out")[0];
    btn.onclick = function(){
      out.innerHTML = "";
      run_from_str(cml.getValue(),out);
    }
    if (options.lazy){
      observer.observe(div);
    }else{
      btn.onclick();
    }
    div.addEventListener('keydown', function(e) {
      if (e.key === 'Enter' && e.shiftKey) {
        e.preventDefault();
        btn.onclick();
      }
    });
    let drawer = div.getElementsByClassName("dh-drawer")[0]

    div.getElementsByClassName("dh-menu")[0].onclick = function(){
      
      if (drawer.style.display != "none"){
        drawer.style.display = "none"
      }else{
        drawer.style.display = "block"
      }
    }
    div.getElementsByClassName("dh-reset")[0].onclick = function(){
      cml.setValue(text);
      drawer.style.display = "none"
    }
    div.getElementsByClassName("dh-stop")[0].onclick = function(){
      out.innerHTML = "";
      drawer.style.display = "none"
    }
    let exp = div.getElementsByClassName("dh-expand")[0];
    exp.onclick = function(){
      if (div.style.height){
        div.style.height = "";
        exp.innerHTML = "&nbsp;collapse"
      }else{
        div.style.height = h0+"px"
        exp.innerHTML = "&nbsp;expand"
      }
      cml.setSize(null,null);
      drawer.style.display = "none"
    }
  }
  window.dither_make_embed = make_widget;
  window.dither_run = run_from_str;
  window.addEventListener("load",function(){
    Array.from(document.getElementsByClassName("dither-embed")).forEach(elem => {
      text = elem.textContent.trim();
      elem.innerHTML = "";
      make_widget(elem,text,{
        lazy:elem.classList.contains("lazy"),
        bleed:elem.classList.contains("bleed"),
      });
    });
    document.querySelectorAll('script[type="text/dither"]').forEach(script => {
      const source = script.textContent;
      let div = document.createElement("div");
      // console.log(script.style);
      div.style = script.getAttribute("style")
      script.parentElement.insertBefore(div,script);
      run_from_str(source, div);
    });
  });

}

js.push(`${fs.readFileSync("src/parser.js").toString()}`);
js.push(`${fs.readFileSync("src/to_js.js").toString()}`);
js.push(`${fs.readFileSync("src/embed_glsl.js").toString()}`);

js.push(`var STD={`)
let ff = fs.readdirSync("std");
for (let i = 0; i < ff.length; i++){
  let isdir = fs.lstatSync("std/"+ff[i]).isDirectory();
  if (isdir){
    let q = `"std/${ff[i]}/header.dh":`+JSON.stringify(fs.readFileSync("std/"+ff[i]+"/header.dh").toString())+",";
    let p = `"std/${ff[i]}/static.js":`+JSON.stringify(fs.readFileSync("std/"+ff[i]+"/static.js").toString())+`,`;
    js.push(q);
    js.push(p);
  }else if (ff[i].endsWith(".dh")){
    let f = ff[i].replace(/\.dh$/g,"");
    let v = JSON.stringify(fs.readFileSync("std/"+ff[i]).toString());
    js.push(`"std/${f}":`+v+",");
    js.push(`"std/${ff[i]}":`+v+",");
  }
}
js.push(`}`)

js.push(`${main.toString()};main();`)

js.push(`})();`);

fs.writeFileSync("build/dither-embed.js",js.join("\n"));

