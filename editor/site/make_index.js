const fs = require('fs');

var bimg = fs.readFileSync("doc/logo.png").toString('base64');

let html = [`
<meta charset="UTF-8">
<script>${fs.readFileSync("build/dither-embed.js").toString()}</script>
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

<body>

<div style="width:720px;margin:auto;font-size:15px;font-family:sans-serif;line-height:24px">

<table style="width:720px">
<tr>
<td>
<img src="data:image/png;base64,${bimg}" style="width:400px;transform:translate(-35px,-5px)"/>
</td>
<td style="width:10px">
<a href="https://github.com/LingDong-/dither-lang/releases" style="text-decoration:none"><div class="biglink">Download</div></a>
<a href="editor.html" style="text-decoration:none"><div class="biglink">Online Editor</div></a>
<a href="blocks.html" style="text-decoration:none"><div class="biglink">Blocks Editor</div></a>
<a href="https://github.com/LingDong-/dither-lang" style="text-decoration:none"><div class="biglink">Source Code</div></a>
</td>
<td style="width:10px">
<a href="https://github.com/LingDong-/dither-lang/tree/main/examples" style="text-decoration:none"><div class="biglink">Examples</div></a>
<a href="https://github.com/LingDong-/dither-lang/blob/main/SYNTAX.md" style="text-decoration:none"><div class="biglink">Quick Start</div></a>
<a href="https://dither-doc.netlify.app/" style="text-decoration:none"><div class="biglink">Tutorials</div></a>
<a href="api.html" style="text-decoration:none"><div class="biglink">API Docs</div></a>
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

<div class="dither-embed">
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

<div class="dither-embed">
include "std/io"

io.println("hello world from dither!")
</div>

<p>
You might find the syntax of Dither familiar if you came from C-like languages.
In case you haven't noticed, you can modify example programs (like the one below)
and run them by clicking the ▶ button. If you don't feel like typing, you can try
out the Scratch-like, <a href="blocks.html">block-based editor</a>.
</p>

<div class="dither-embed">
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

<div class="dither-embed">
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

<div class="dither-embed">
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

<div class="dither-embed">

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



fs.writeFileSync("build/index.html",html.join("\n"));

