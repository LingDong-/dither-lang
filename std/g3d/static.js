globalThis.$g3d = new function(){
  const DIRTY_VERTICES = 1 ;
  const DIRTY_INDICES  = 2 ;
  const DIRTY_COLORS   = 4 ;
  const DIRTY_UVS      = 8 ;
  const DIRTY_NORMALS  = 16;

  let that = this;
  let cnv;
  let gl;
  let shader;
  let currentProgram = null;
  const vertexSrc = `
precision mediump float;
attribute vec3 a_position;
attribute vec4 a_color;
attribute vec2 a_uv;
attribute vec3 a_normal;
varying vec4 v_color;
varying vec2 v_uv;
varying vec3 v_normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_matrix;
void main() {
  v_color = a_color;
  v_uv = a_uv;
  v_normal = normalize(normal_matrix * a_normal);
  gl_Position = projection*view*model*vec4(a_position, 1.0);
}
  `;
  const fragmentSrc = `
precision mediump float;
varying vec4 v_color;
varying vec2 v_uv;
varying vec3 v_normal;
void main() {
  gl_FragColor = v_color;
}
  `;
  let vaos = [];
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
    let [_] = $pop_args(1);
    cnv = document.getElementById("canvas");
    gl = cnv.getContext('webgl');

    const vertexShader = compileShader(gl.VERTEX_SHADER, vertexSrc);
    const fragmentShader = compileShader(gl.FRAGMENT_SHADER, fragmentSrc);
    shader = gl.createProgram();
    gl.attachShader(shader, vertexShader);
    gl.attachShader(shader, fragmentShader);
    gl.linkProgram(shader);

    gl.enable(gl.DEPTH_TEST);
  }
  function copy_list_vec_pack(lst,vn){
    let data = new Float32Array(lst.length*vn);
    for (let i = 0; i < lst.length; i++){
      for (let j = 0; j < vn; j++){
        data[i*vn+j] = lst[i][j];
      }
    }
    return data;
  }

  that._update_mesh = function(){
    let [vao,flags,vertices,indices,colors,uvs,normals] = $pop_args(7);
    let p_vertices, p_colors, p_uvs, p_normals;
    if (flags & DIRTY_VERTICES) p_vertices = copy_list_vec_pack(vertices,3);
    if (flags & DIRTY_COLORS) p_colors = copy_list_vec_pack(colors,4);
    if (flags & DIRTY_UVS) p_uvs = copy_list_vec_pack(uvs,2);
    if (flags & DIRTY_NORMALS) p_normals = copy_list_vec_pack(normals,3);

    let mesh;
    if (vao == -1){
      mesh = {
        vbo_vertices:gl.createBuffer(),
        vbo_colors:gl.createBuffer(),
        vbo_uvs:gl.createBuffer(),
        vbo_normals:gl.createBuffer(),
        ebo_indices:gl.createBuffer(),
      }
      // let dummy = new Float32Array(8);
      // gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_uvs);
      // gl.bufferData(gl.ARRAY_BUFFER, dummy, gl.STATIC_DRAW);

      vao = vaos.length;
      vaos.push(mesh);
    }else{
      mesh = vaos[vao];
    }
    if ((flags & DIRTY_VERTICES) && vertices.length){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_vertices);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(p_vertices), gl.STATIC_DRAW);
    }
    if ((flags & DIRTY_INDICES) && indices.length){
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, mesh.ebo_indices);
      gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);
    }
    if ((flags & DIRTY_COLORS) && colors.length){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_colors);
      gl.bufferData(gl.ARRAY_BUFFER, p_colors, gl.STATIC_DRAW);
    }
    if ((flags & DIRTY_UVS) && uvs.length){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_uvs);
      gl.bufferData(gl.ARRAY_BUFFER, p_uvs, gl.STATIC_DRAW);
    }
    if ((flags & DIRTY_NORMALS) && normals.length){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_normals);
      gl.bufferData(gl.ARRAY_BUFFER, p_normals, gl.STATIC_DRAW);
    }
    vaos[vao].n_vertices = vertices.length;
    vaos[vao].n_indices = indices.length;
    vaos[vao].n_normals = normals.length;
    vaos[vao].n_uvs = uvs.length;
    vaos[vao].n_colors = colors.length;
    return vao;
  }
  function compute_normal_mat(modelMatrix) {
    let m = [
      modelMatrix[0], modelMatrix[4], modelMatrix[8],
      modelMatrix[1], modelMatrix[5], modelMatrix[9],
      modelMatrix[2], modelMatrix[6], modelMatrix[10],
    ];
    let a00 = m[0], a01 = m[3], a02 = m[6];
    let a10 = m[1], a11 = m[4], a12 = m[7];
    let a20 = m[2], a21 = m[5], a22 = m[8];
    let b01 = a22 * a11 - a12 * a21;
    let b11 = -a22 * a10 + a12 * a20;
    let b21 = a21 * a10 - a11 * a20;
    let det = a00 * b01 + a01 * b11 + a02 * b21;
    if (Math.abs(det) < Number.EPSILON){
      return new Float32Array(m);
    }
    let invDet = 1.0 / det;
    let out = new Float32Array(9);
    out[0] = b01 * invDet;
    out[1] = (-a22 * a01 + a02 * a21) * invDet;
    out[2] = (a12 * a01 - a02 * a11) * invDet;
    out[3] = b11 * invDet;
    out[4] = (a22 * a00 - a02 * a20) * invDet;
    out[5] = (-a12 * a00 + a02 * a10) * invDet;
    out[6] = b21 * invDet;
    out[7] = (-a21 * a00 + a01 * a20) * invDet;
    out[8] = (a11 * a00 - a01 * a10) * invDet;
    return out;
  }
  function glUniformMatrix4fv(loc,transpose,m){
    if (transpose) {
      m = new Float32Array([
        m[0], m[4], m[8],  m[12],
        m[1], m[5], m[9],  m[13],
        m[2], m[6], m[10], m[14],
        m[3], m[7], m[11], m[15],
      ]);
    }
    return gl.uniformMatrix4fv(loc, false, m);
  }
  that._draw_mesh = function(){
    let [vao,mode,model_matrix] = $pop_args(3);
    let mesh = vaos[vao];
    let program = gl.getParameter(gl.CURRENT_PROGRAM);

    let loc_model = gl.getUniformLocation(program, "model");
    glUniformMatrix4fv(loc_model, true, model_matrix);

    let nm = compute_normal_mat(model_matrix);
    let loc_nm = gl.getUniformLocation(program, "normal_matrix");
    gl.uniformMatrix3fv(loc_nm, false, nm);
    
    let loc_uv = gl.getAttribLocation(program, "a_uv");
    if (mesh.n_uvs){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_uvs);
      gl.enableVertexAttribArray(loc_uv);
      gl.vertexAttribPointer(loc_uv, 2, gl.FLOAT, false, 0, 0);
    }else{
      gl.disableVertexAttribArray(loc_uv);
      gl.vertexAttrib2f(loc_uv, 0.0, 0.0);
    }

    let loc_color = gl.getAttribLocation(program, "a_color");
    if (mesh.n_colors){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_colors);
      gl.enableVertexAttribArray(loc_color);
      gl.vertexAttribPointer(loc_color, 4, gl.FLOAT, false, 0, 0);
    }else{
      gl.disableVertexAttribArray(loc_color);
      gl.vertexAttrib4f(loc_color, 1.0, 1.0, 1.0, 1.0);
    }
    
    let loc_norm = gl.getAttribLocation(program, "a_normal");
    if (loc_norm >= 0) {
      if (mesh.n_normals){
        gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_normals);
        gl.enableVertexAttribArray(loc_norm);
        gl.vertexAttribPointer(loc_norm, 3, gl.FLOAT, false, 0, 0);
      }else{
        gl.disableVertexAttribArray(loc_norm);
        gl.vertexAttrib3f(loc_norm, 0.0, 0.0, 1.0);
      }
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_vertices);
    let loc_pos = gl.getAttribLocation(program, "a_position");
    gl.enableVertexAttribArray(loc_pos);
    gl.vertexAttribPointer(loc_pos, 3, gl.FLOAT, false, 0, 0);


    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, mesh.ebo_indices);
    
    gl.drawElements(mode,mesh.n_indices,gl.UNSIGNED_SHORT,0);

    // if (loc_pos)   gl.disableVertexAttribArray(loc_pos);
    // if (loc_norm)  gl.disableVertexAttribArray(loc_norm);
    // if (loc_uv)    gl.disableVertexAttribArray(loc_uv);
    // if (loc_color) gl.disableVertexAttribArray(loc_color);

  }


  that._look_at = function(){
    let [eye,center,up] = $pop_args(3);
    let f = [ center[0]-eye[0], center[1]-eye[1], center[2]-eye[2] ];
    let fn = Math.sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
    for (let i = 0; i < 3; ++i) f[i] /= fn;
    let s = [
      f[1]*up[2] - f[2]*up[1],
      f[2]*up[0] - f[0]*up[2],
      f[0]*up[1] - f[1]*up[0]
    ];
    let sn = Math.sqrt(s[0]*s[0] + s[1]*s[1] + s[2]*s[2]);
    for (let i = 0; i < 3; ++i) s[i] /= sn;
    let u = [
      s[1]*f[2] - s[2]*f[1],
      s[2]*f[0] - s[0]*f[2],
      s[0]*f[1] - s[1]*f[0]
    ];
    let out = new Float32Array(16);
    out[0] = out[5] = out[10] = out[15] = 1;

    out[0 ] = s[0];
    out[1 ] = s[1];
    out[2 ] = s[2];
    out[4 ] = u[0];
    out[5 ] = u[1];
    out[6 ] = u[2];
    out[8 ] =-f[0];
    out[9 ] =-f[1];
    out[10] =-f[2];
    out[3 ] =-(s[0]*eye[0]+s[1]*eye[1]+s[2]*eye[2]);
    out[7 ] =-(u[0]*eye[0]+u[1]*eye[1]+u[2]*eye[2]);
    out[11] = (f[0]*eye[0]+f[1]*eye[1]+f[2]*eye[2]);
    return out;
  }
  that._perspective = function(){
    let [fov,aspect,nearZ,farZ] = $pop_args(4);
    fov = fov * Math.PI/180.0;
    let tanHalfFov = Math.tan(fov / 2.0);
    let out = new Float32Array(16);
    out[0]  = 1.0 / (tanHalfFov * aspect);
    out[5]  = 1.0 / tanHalfFov;
    out[10] = - (farZ+nearZ) / (farZ-nearZ);
    out[14] = - 1.0;
    out[11] = -(2.0 * farZ * nearZ) / (farZ-nearZ);
    return out;
  }
  that.background = function(){
    let [r,g,b,a] = $pop_args(4);
    gl.clearColor(r,g,b,a);
    gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);
  }
  that._camera_begin = function(){
    let [view,proj] = $pop_args(2);
    currentProgram = gl.getParameter(gl.CURRENT_PROGRAM);
    if (currentProgram === null){
      gl.useProgram(shader);
    }
    let program = gl.getParameter(gl.CURRENT_PROGRAM);
    glUniformMatrix4fv(
      gl.getUniformLocation(program,"view"),true,view
    );
    glUniformMatrix4fv(
      gl.getUniformLocation(program,"projection"),true,proj
    );
  }
  that._camera_end = function(){
    if (!currentProgram){
      gl.useProgram(null);
    }
    currentProgram = null;
  }
  that.mat = {
    rotate_deg: function(){
      let [axis,ang] = $pop_args(2);
      ang = ang * Math.PI/180.0;
      let x = axis[0], y = axis[1], z = axis[2];
      let len = Math.sqrt(x * x + y * y + z * z);
      let out = new Float32Array(16);
      if (len == 0.0) {
        for (let i = 0; i < 16; ++i)
          out[i] = (i % 5 == 0) ? 1 : 0;
        return;
      }
      x /= len;
      y /= len;
      z /= len;
      let c = Math.cos(ang);
      let s = Math.sin(ang);
      let t = 1.0 - c;
      out[0 ] = t*x*x + c;
      out[4 ] = t*x*y + s*z;
      out[8 ] = t*x*z - s*y;
      out[12] = 0.0;
      out[1 ] = t*x*y - s*z;
      out[5 ] = t*y*y + c;
      out[9 ] = t*y*z + s*x;
      out[13] = 0.0;
      out[2 ] = t*x*z + s*y;
      out[6 ] = t*y*z - s*x;
      out[10] = t*z*z + c;
      out[14] = 0.0;
      out[3 ] = 0.0;
      out[7 ] = 0.0;
      out[11] = 0.0;
      out[15] = 1.0;
      return out;
    }
  }
  that.flush = function(){

  }
}
