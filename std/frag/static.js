globalThis.$frag = new function(){
  const LOC_POSITION   = 0 ;
  const LOC_COLOR      = 1 ;
  const LOC_UV         = 2 ;
  const LOC_NORMAL     = 3 ;
  const LOC_MODEL      = 4 ;
  const LOC_NM         = 8 ;

  let that = this;
  let cnv;
  let gl;
  let fbos = [null];
  let programs = [null];
  let tex_cnt = 0;

  const vertexSrc = `
precision mediump float;
attribute vec3 a_position;
attribute vec4 a_color;
attribute vec2 a_uv;
attribute vec3 a_normal;
attribute mat4 a_model;
attribute mat3 a_normal_matrix;
varying vec4 v_color;
varying vec2 v_uv;
varying vec3 v_normal;
varying vec3 v_position;
uniform mat4 view;
uniform mat4 projection;
void main() {
  v_color = a_color;
  v_uv = a_uv;
  v_normal = normalize(a_normal_matrix * a_normal);
  vec4 world_pos = a_model * vec4(a_position, 1.0);
  vec4 view_pos = view * world_pos;
  v_position = world_pos.xyz/world_pos.w;
  gl_Position = projection * view_pos;
}
  `;
  const vertices = new Float32Array([
  -1.0, -1.0, 0.0,
   1.0, -1.0, 0.0,
  -1.0,  1.0, 0.0,
   1.0,  1.0, 0.0
  ]);
  function compileShader(type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      console.error(gl.getShaderInfoLog(shader));
      gl.deleteShader(shader);
      return null;
    }
    return shader;
  }
  that.init = function(){
    let [id] = $pop_args(1);
    cnv = document.getElementById(id);
    gl = cnv.getContext('webgl',{ premultipliedAlpha: false });
  }
  
  that.program = function(){
    let [src] = $pop_args(1);
    
    gl.getExtension('OES_standard_derivatives');

    src = src.replace(/#version +\d\d\d/g,`#extension GL_OES_standard_derivatives : enable\nprecision mediump float;`);
    src = src.replace(/__/g,`GLES_DOUBLE_UNDERSCORE_REPLACEMENT`);
    src = src.replace(/\/\*ARRAY_LITERAL_BEGIN\*\/(.*?) (.*?)\[.*?\]\=.*?\[.*?\]\((.*?)\)\;\/\*ARRAY_LITERAL_END\*\//g,
      function(match,p0,p1,p2){
        return `${p0} ${p1}(int idx){${p2.split(',').map((x,i)=>`if (idx == ${i}) return ${x};`).join('\n')}}`;
      });
    src = src.replace(/\/\*ARRAY_SUBSCRIPT_BEGIN\*\/./g,'(');
    src = src.replace(/\/\*ARRAY_SUBSCRIPT_END\*\/./g,')');
    src = src.replace(/\/\*CONST\*\//g,'const ');
    console.log(src);
    const vertexShader = compileShader(gl.VERTEX_SHADER, vertexSrc);
    const fragmentShader = compileShader(gl.FRAGMENT_SHADER, src);
    const program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);

    gl.bindAttribLocation(program, LOC_POSITION,"a_position");
    gl.bindAttribLocation(program, LOC_COLOR,   "a_color");
    gl.bindAttribLocation(program, LOC_UV,      "a_uv");
    gl.bindAttribLocation(program, LOC_NORMAL,  "a_normal");
    gl.bindAttribLocation(program, LOC_MODEL,   "a_model");
    gl.bindAttribLocation(program, LOC_NM,      "a_normal_matrix");

    gl.linkProgram(program);
    programs.push(program);
    return programs.length-1;
  }
  function powerOf2(v) {
    return v && !(v & (v - 1));
  }
  that._init_texture = function(){
    let [obj,w,h,flags] = $pop_args(4);
    
    const fbo = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
    const tex = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, tex);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
  
    let po2 = powerOf2(w)&&powerOf2(h);
    if ((flags&3) == 2){
      if (po2){
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
      }else{
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
      }
    }else if ((flags&3) == 1){
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    }else{
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    }
    if ((flags&4) == 0){
      if (po2){
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
      }else{
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
      }
    }else{
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    }
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, tex, 0);
    gl.bindTexture(gl.TEXTURE_2D, null);

    let depthBuffer = gl.createRenderbuffer();
    gl.bindRenderbuffer(gl.RENDERBUFFER, depthBuffer);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, w, h);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, depthBuffer);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
       
    if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) !== gl.FRAMEBUFFER_COMPLETE) {
      console.error("FBO is not complete");
    }

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.bindFramebuffer(gl.FRAMEBUFFER,null);
    fbo._tex = tex;
    fbo._w = w;
    fbo._h = h;
    fbo._flags = flags;

    fbos.push(fbo);

    obj.fbo = fbos.length-1;
  }


  that._begin = function(){
    let [prgm, fbo] = $pop_args(2);

    gl.bindFramebuffer(gl.FRAMEBUFFER, fbos[fbo]);

    if (fbo == 0){
      gl.viewport(0,0,gl.canvas.width,gl.canvas.height);
    }else{
      gl.viewport(0,0,fbos[fbo]._w,fbos[fbo]._h);
    }

    gl.useProgram(programs[prgm]);
    tex_cnt = 0;
  }



  that.render = function(){
    let shader = gl.getParameter(gl.CURRENT_PROGRAM);
    let vbo, vbo_uvs;

    let iden = new Float32Array([1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]);

    gl.uniformMatrix4fv(gl.getUniformLocation(shader,"view"),false,iden)
    gl.uniformMatrix4fv(gl.getUniformLocation(shader,"projection"),false,iden)

    gl.disableVertexAttribArray(LOC_MODEL);
    gl.vertexAttrib4f(LOC_MODEL,   1,0,0,0);
    gl.disableVertexAttribArray(LOC_MODEL+1);
    gl.vertexAttrib4f(LOC_MODEL+1, 0,1,0,0);
    gl.disableVertexAttribArray(LOC_MODEL+2);
    gl.vertexAttrib4f(LOC_MODEL+2, 0,0,1,0);
    gl.disableVertexAttribArray(LOC_MODEL+3);
    gl.vertexAttrib4f(LOC_MODEL+3, 0,0,0,1);

    gl.disableVertexAttribArray(LOC_NM);
    gl.vertexAttrib3f(LOC_NM,   1,0,0);
    gl.disableVertexAttribArray(LOC_NM+1);
    gl.vertexAttrib3f(LOC_NM+1, 0,1,0);
    gl.disableVertexAttribArray(LOC_NM+2);
    gl.vertexAttrib3f(LOC_NM+2, 0,0,1);

    let uvs = new Float32Array(8);
    for (let i = 0; i < 4; i++){
      uvs[i*2] = (vertices[i*3]+1.0)*0.5;
      uvs[i*2+1] = (vertices[i*3+1]+1.0)*0.5;
    }
    vbo_uvs = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_uvs);
    gl.bufferData(gl.ARRAY_BUFFER, uvs, gl.STATIC_DRAW);

    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_uvs);
    gl.enableVertexAttribArray(LOC_UV);
    gl.vertexAttribPointer(LOC_UV,2,gl.FLOAT,false,0,0);

    gl.disableVertexAttribArray(LOC_COLOR);
    gl.vertexAttrib4f(LOC_COLOR,1,1,1,1);
    
    gl.disableVertexAttribArray(LOC_NORMAL);
    gl.vertexAttrib3f(LOC_NORMAL,0,0,1);
    
    vbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo);
    gl.bufferData(gl.ARRAY_BUFFER,vertices,gl.STATIC_DRAW);

    gl.enableVertexAttribArray(LOC_POSITION);
    gl.vertexAttribPointer(LOC_POSITION, 3, gl.FLOAT, false, 0, 0);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    gl.bindBuffer(gl.ARRAY_BUFFER,null);
    gl.deleteBuffer(vbo);
    gl.deleteBuffer(vbo_uvs);
  }

  that.end = function(){
    let fbo = gl.getParameter(gl.FRAMEBUFFER_BINDING);
    if (fbo){
      if (fbo._tex){
        gl.bindTexture(gl.TEXTURE_2D, fbo._tex);
        if (powerOf2(fbo._w)&&powerOf2(fbo._h)&&(fbo._flags&3)==2){
          gl.generateMipmap(gl.TEXTURE_2D);
        }
        gl.bindTexture(gl.TEXTURE_2D, null);
      }
    }
    gl.useProgram(null);
    gl.bindFramebuffer(gl.FRAMEBUFFER,null);
  }

  let glUniformMatrix = {
    "4fv":function(loc,transpose,m){
      if (transpose) {
        m = new Float32Array([
          m[0], m[4], m[8],  m[12],
          m[1], m[5], m[9],  m[13],
          m[2], m[6], m[10], m[14],
          m[3], m[7], m[11], m[15],
        ]);
      }
      return gl.uniformMatrix4fv(loc, false, m);
    },
    "3fv":function(loc,transpose,m){
      if (transpose) {
        m = new Float32Array([
          m[0], m[3], m[6], 
          m[1], m[4], m[7], 
          m[2], m[5], m[8],
        ]);
      }
      return gl.uniformMatrix3fv(loc, false, m);
    }
  }

  that.uniform = function(){
    let t = $args.at(-1).__type;
    let [s,x] = $pop_args(2);
    let prgm = gl.getParameter(gl.CURRENT_PROGRAM);
    const u = gl.getUniformLocation(prgm, s);
    
    if (t == 'f32'){
      gl.uniform1f(u, x);
    }else if (t == 'i32'){
      gl.uniform1i(u, x);
    }else if (t.con == 'vec'){
      if (t.elt.length == 2){
        gl['uniform'+t.elt[1]+t.elt[0][0]](u, ...x);
      }else{
        glUniformMatrix[t.elt[1]+t.elt[0][0]+'v'](u, true, x);
      }
    }else{
      let idx = tex_cnt;

      let fbo = fbos[x.fbo];

      gl.activeTexture(gl.TEXTURE0 + idx);
      gl.bindTexture(gl.TEXTURE_2D, fbo._tex);
      gl.uniform1i(u, idx);

      tex_cnt++;
    }
  }
  that._write_pixels = function(){
    let [fbo, pix] = $pop_args(2);
    
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbos[fbo]);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true)
    gl.bindTexture(gl.TEXTURE_2D, fbos[fbo]._tex);
    gl.texSubImage2D(
      gl.TEXTURE_2D,
      0,
      0, 0,
      fbos[fbo]._w,
      fbos[fbo]._h,
      gl.RGBA,
      gl.UNSIGNED_BYTE,
      new Uint8Array(pix)
    );
    if (powerOf2(fbos[fbo]._w)&&powerOf2(fbos[fbo]._h)&&(fbos[fbo]._flags&3)==2){
      gl.generateMipmap(gl.TEXTURE_2D);
    }
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false)
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }
}
