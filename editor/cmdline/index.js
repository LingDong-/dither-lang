const os = require('os');
const _WIN32 = os.platform() == 'win32';
const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');
const PARSER = require('../../src/parser.js');
const TO_JS = require('../../src/to_js.js');
const TO_C = require('../../src/to_c.js');
const embed_glsl = require('../../src/embed_glsl.js');

function tmpth(){
  // let t = _WIN32 ? os.tmpdir() : '/tmp';
  let t = _WIN32 ? process.env.LOCALAPPDATA : '/tmp';
  return path.join(t,"dither",...arguments);
}


let version = "v0.0.1"
let help = `
The DITHER Programming Language ${version}
usage:
    dither [options] file.dh
options:
    --target,  -t name : compile backend: vm/c/js/html
    --output,  -o path : output path
    --execute, -x      : execute generated code
    --include, -I path : add include path
    --command, -c cmd  : program passed in as string
    --help,    -h      : print this help
    --version, -V      : print version
`
let args0 = process.argv.slice(2);
let args = [];
for (let i = 0; i < args0.length; i++){
  if (args0[i][0] == '-' && args0[i][1] != '-'){
    args.push(...args0[i].slice(1).split('').map(x=>'-'+x));
  }else{
    args.push(args0[i])
  }
}
let do_run = 0;
let verbose = 0;
let targ = 'vm'
let out_pth = null;
let inc_pth = [];
let inp_pth = null;
let map_pth = tmpth('ir.map');
let did_info = 0;
if (args.length == 0){
  console.log(help);
}
for (let i = 0; i < args.length; i++){
  if (args[i] == '--execute' || args[i] == '-x'){
    do_run = 1;
  }else if (args[i] == '--target' || args[i] == '-t'){
    targ = args[++i];
  }else if (args[i] == '--output' || args[i] == '-o'){
    out_pth = args[++i];
  }else if (args[i] == '--include' || args[i] == '-I'){
    inc_pth.push(args[++i])
  }else if (args[i] == '--help' || args[i] == '-h'){
    console.log(help);
  }else if (args[i] == '--version' || args[i] == '-V'){
    console.log(version);
    did_info = 1;
  }else if (args[i] == '--verbose' || args[i] == '-v'){
    verbose = 1;
  }else if (args[i] == '--command' || args[i] == '-c'){
    fs.writeFileSync(inp_pth = tmpth("in.dh"),args[++i]);
  }else{
    inp_pth = args[i];
  }
}

if (inp_pth == null){
  if (!did_info){
    console.warn("[warning] no input file.");
  }
  process.exit(0);
}
if (out_pth == null){
  if (!do_run){
    console.warn("[warning] no output file.");
  }
  if (targ == 'vm'){
    out_pth = tmpth('out.dsm');
  }else if (targ == 'c'){
    out_pth = tmpth('out.c');
  }else if (targ == 'js'){
    out_pth = tmpth('out.js');
  }else if (targ == 'html'){
    out_pth = tmpth('out.html');
  }
}
fs.mkdirSync(tmpth(), { recursive: true });

let search_paths = Array.from(new Set([...inc_pth,".",tmpth()].map(x=>path.resolve(x))));
// console.log(search_paths);
// console.log(fs.readdirSync(search_paths[1]));
// console.log(fs.readdirSync('/snapshot/dither-lang/'+fs.readdirSync(search_paths[1])[1]));

function copyDirSync(srcDir, destDir) {
  if (!fs.existsSync(destDir)) {
    fs.mkdirSync(destDir, { recursive: true });
  }
  for (const entry of fs.readdirSync(srcDir)) {
    const srcPath = path.join(srcDir, entry);
    const destPath = path.join(destDir, entry);
    const stats = fs.statSync(srcPath);

    if (stats.isDirectory()) {
      copyDirSync(srcPath, destPath);
    } else {
      const data = fs.readFileSync(srcPath);
      fs.writeFileSync(destPath, data, { mode: stats.mode });
    }
  }
}

function miniServer(ENTRY_FILE){
  const http = require('http');
  const url = require('url');
  const net = require('net');
  const BASE_DIR = path.dirname(path.resolve(ENTRY_FILE));
  const ENTRY_ABS = path.resolve(ENTRY_FILE);
  let PORT = 5555;
  function findAvailablePort(start, callback) {
    const server = net.createServer();
    server.listen(start, () => {
      server.once('close', () => callback(start));
      server.close();
    });
    server.on('error', () => findAvailablePort(start + 1, callback));
  }
  function startServer(port) {
    const server = http.createServer((req, res) => {
      let parsedUrl = url.parse(req.url);
      let reqPath = parsedUrl.pathname === '/' ? ENTRY_ABS : path.join(BASE_DIR, parsedUrl.pathname);
      let safePath = path.resolve(reqPath);
      if (!safePath.startsWith(BASE_DIR)) {
        res.writeHead(403);
        res.end('Access denied');
        return;
      }
      fs.readFile(safePath, (err, data) => {
        if (err) {
          res.writeHead(err.code === 'ENOENT' ? 404 : 500);
          res.end(err.code === 'ENOENT' ? 'Not found' : 'Server error');
        } else {
          res.writeHead(200);
          res.end(data);
        }
      });
    });
    server.listen(port, () => {
      if (verbose) console.log(`[info] serving ${ENTRY_FILE} at http://localhost:${port}/ ...`);
    });
  }
  findAvailablePort(PORT, startServer);
}

copyDirSync(path.join(__dirname,'..','..','std'),tmpth("std"));

let parser = new PARSER(
  {fs,path,process,search_paths},
  Object.assign({},embed_glsl),
);

let toks = parser.tokenize(path.resolve(inp_pth));
let cst = parser.parse(toks);
let ast = parser.abstract(cst);
let scopes = parser.infertypes(ast);
let [instrs,layout] = parser.compile(ast,scopes);
let lo = parser.writelayout(layout);
let ir = parser.writeir(instrs);
fs.writeFileSync(map_pth,parser.writesrcmap(instrs));
let irlo = ir+lo;



if (targ == 'vm'){
  fs.writeFileSync(out_pth,irlo);
  if (do_run){
    if (_WIN32){
      fs.writeFileSync(tmpth('vm.exe'), fs.readFileSync(__dirname+'\\..\\..\\build\\vm.exe'), { mode: 0o755 });
      if (verbose) console.log("[info] compiled, running...");
      let cmd = `${tmpth('vm.exe')} ${out_pth} --map ${tmpth('ir.map')}`;
      execSync(cmd,{stdio:'inherit',env: { ...process.env, DITHER_ROOT: tmpth() }});
    }else{
      fs.writeFileSync(tmpth('vm'), fs.readFileSync(__dirname+'/../../build/vm'), { mode: 0o755 });
      if (verbose) console.log("[info] compiled, running...");
      let cmd = `${tmpth('vm')} ${out_pth} --map ${tmpth('ir.map')}`;
      execSync(cmd,{stdio:'inherit',env: { ...process.env, DITHER_ROOT: tmpth() }});
    }
  }
}else if (targ == 'c'){
  let to_c = new TO_C({});
  let [ir,layout] = to_c.parse_ir(irlo);
  let c = to_c.transpile(ir,layout);
  fs.writeFileSync(out_pth,c);
  if (do_run){
    if (verbose) console.log("[info] dither compiled. compiling C...");
    copyDirSync(path.join(__dirname,'..','..','third_party'),tmpth("third_party"));
    if (_WIN32){
      function get_vs() {
        let vswhere = null;
        for (let p of [
          process.env['ProgramFiles(x86)'] + '\\Microsoft Visual Studio\\Installer\\vswhere.exe',
          process.env['ProgramFiles'] + '\\Microsoft Visual Studio\\Installer\\vswhere.exe',
        ]) {
          if (fs.existsSync(p)){
            vswhere = p;
            break;
          }
        }
        if (!vswhere){
          console.error('[error] no msvc installation found.');
          process.exit(1);
        }
        let vspath = execSync(`"${vswhere}" -latest -products * -property installationPath`, { encoding: 'utf-8' }).trim();
        let vcvars = path.join(vspath, 'VC', 'Auxiliary', 'Build', 'vcvars64.bat');
        return vcvars;
      }
      if (verbose) console.log("[info] finding msvc installation...");
      let vcvars = get_vs();
      fs.writeFileSync(tmpth('config.h'), fs.readFileSync(__dirname+'\\..\\..\\build\\config.h'), { mode: 0o755 });
      fs.writeFileSync(tmpth('config.bat'), fs.readFileSync(__dirname+'\\..\\..\\build\\config.bat'), { mode: 0o755 })
      let cmd = `call ${tmpth('config.bat')} >nul && "${vcvars}" && cl /I. /FI ${tmpth('config.h')} /Fe:${tmpth('a.exe')} /Fo:"%TEMP%\\a.obj" /w /O2 ${out_pth} ${verbose?'&& echo [info] C compiled, running...':''} && ${tmpth('a.exe')}`;
      if (verbose) console.log("[info] compiling with cl.exe...");
      execSync(cmd,{stdio:'inherit',env: { ...process.env, DITHER_ROOT: tmpth() }});
    }else{
      fs.writeFileSync(tmpth('config.env'), fs.readFileSync(__dirname+'/../../config.env'), { mode: 0o755 })
      let c = `cd ${tmpth()} && source config.env && eval $(head -n 1 "${out_pth}" | cut -c 3-) && echo $CFLAGS`
      let cflags = execSync(c).toString().replace(/\n/g,' ');
      fs.writeFileSync(tmpth('config.h'), fs.readFileSync(__dirname+'/../../build/config.h'), { mode: 0o755 });
      let cmd = `gcc -include ${tmpth('config.h')} -I${tmpth()} -O3 ${cflags} ${out_pth} -o ${tmpth('a.out')} ${verbose?'&& echo "[info] C compiled, running..."':''} && ${tmpth('a.out')}`;
      execSync(cmd,{stdio:'inherit',env: { ...process.env, DITHER_ROOT: tmpth() }});
    }
  }
}else if (targ == 'js'){
  let to_js = new TO_JS({preclude:1});
  let [ir,layout] = to_js.parse_ir(irlo);
  fs.writeFileSync(out_pth,to_js.transpile(ir,layout));
  if (do_run){
    if (verbose) console.log("[info] compiled, running...");
    eval(fs.readFileSync(out_pth).toString());
  }
}else if (targ == 'html'){
  let to_js = new TO_JS({preclude:1});
  let [ir,layout] = to_js.parse_ir(irlo);
  let js = to_js.transpile(ir,layout);
  fs.writeFileSync(out_pth,`<body></body><script>${js.replace(/<\/script>/gi, '<\\/script>')}</script>`);
  if (do_run){
    if (verbose) console.log("[info] compiled, running...");
    miniServer(out_pth)
  }
}