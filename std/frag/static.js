globalThis.$frag = new function(){
  let that = this;
  let cnv;
  let gl;
  let fbos = [null];
  let programs = [null];
  let tex_cnt = 0;

  const vertexSrc = `
    attribute vec2 a_position;
    void main() {
      gl_Position = vec4(a_position, 0.0, 1.0);
    }`;
  const vertices = new Float32Array([
    -1, -1,
     1, -1,
    -1,  1,
    -1,  1,
     1, -1,
     1,  1,
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
  that._size = function(){
    let [w,h,_] = $pop_args(3);
    cnv = document.getElementById("canvas");
    gl = cnv.getContext('webgl');
    width = w;
    height = h;
  }
  
  that.program = function(){
    let [src] = $pop_args(1);
    src = src.replace(/#version +\d\d\d/g,`precision mediump float;`);
    console.log(src);
    const vertexShader = compileShader(gl.VERTEX_SHADER, vertexSrc);
    const fragmentShader = compileShader(gl.FRAGMENT_SHADER, src);
    const program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);
    programs.push(program);
    return programs.length-1;
  }

  that._init_texture = function(){
    let [obj] = $pop_args(1);
    
    const fbo = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
    const tex = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, tex);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, tex, 0);
   
    if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) !== gl.FRAMEBUFFER_COMPLETE) {
      console.error("FBO is not complete");
    }

    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER,null);
    fbo._tex = tex;

    fbos.push(fbo);

    obj.fbo = fbos.length-1;
  }

  that._begin = function(){
    let [prgm, fbo] = $pop_args(2);

    const buffer = gl.createBuffer();

    gl.bindFramebuffer(gl.FRAMEBUFFER, fbos[fbo]);

    gl.useProgram(programs[prgm]);
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    const posAttrib = gl.getAttribLocation(programs[prgm], 'a_position');
    gl.enableVertexAttribArray(posAttrib);
    gl.vertexAttribPointer(posAttrib, 2, gl.FLOAT, false, 0, 0);

    tex_cnt = 0;
  }
  that.end = function(){
    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
    gl.clearColor(0, 0, 0, 1);
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.drawArrays(gl.TRIANGLES, 0, 6);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
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
      gl['uniform'+t.elt[1]+t.elt[0][0]](u, ...x);
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
      width,
      height,
      gl.RGBA,
      gl.UNSIGNED_BYTE,
      new Uint8Array(pix)
    );
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false)
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }
}
