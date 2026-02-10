globalThis.$rdr = new function(){
  const DIRTY_VERTICES = 1 ;
  const DIRTY_INDICES  = 2 ;
  const DIRTY_COLORS   = 4 ;
  const DIRTY_UVS      = 8 ;
  const DIRTY_NORMALS  = 16;
  const DIRTY_BOUNDS   = 32;

  const CULL_FRUSTUM   = 16;

  const LOC_POSITION   = 0 ;
  const LOC_COLOR      = 1 ;
  const LOC_UV         = 2 ;
  const LOC_NORMAL     = 3 ;
  const LOC_MODEL      = 4 ;
  const LOC_NM         = 8 ;

  function MAT_TFRM(A,v) {return [((A)[0]*(v)[0]+(A)[1]*(v)[1]+(A)[2]*(v)[2]+(A)[3])/((A)[12]*(v)[0]+(A)[13]*(v)[1]+(A)[14]*(v)[2]+(A)[15]),((A)[4]*(v)[0]+(A)[5]*(v)[1]+(A)[6]*(v)[2]+(A)[7])/((A)[12]*(v)[0]+(A)[13]*(v)[1]+(A)[14]*(v)[2]+(A)[15]),((A)[8]*(v)[0]+(A)[9]*(v)[1]+(A)[10]*(v)[2]+(A)[11])/((A)[12]*(v)[0]+(A)[13]*(v)[1]+(A)[14]*(v)[2]+(A)[15])];}
  function MAT_MULT(A,B) {return [(A)[0]*(B)[0]+(A)[1]*(B)[4]+(A)[2]*(B)[8]+(A)[3]*(B)[12],(A)[0]*(B)[1]+(A)[1]*(B)[5]+(A)[2]*(B)[9]+(A)[3]*(B)[13],(A)[0]*(B)[2]+(A)[1]*(B)[6]+(A)[2]*(B)[10]+(A)[3]*(B)[14],(A)[0]*(B)[3]+(A)[1]*(B)[7]+(A)[2]*(B)[11]+(A)[3]*(B)[15],(A)[4]*(B)[0]+(A)[5]*(B)[4]+(A)[6]*(B)[8]+(A)[7]*(B)[12],(A)[4]*(B)[1]+(A)[5]*(B)[5]+(A)[6]*(B)[9]+(A)[7]*(B)[13],(A)[4]*(B)[2]+(A)[5]*(B)[6]+(A)[6]*(B)[10]+(A)[7]*(B)[14],(A)[4]*(B)[3]+(A)[5]*(B)[7]+(A)[6]*(B)[11]+(A)[7]*(B)[15],(A)[8]*(B)[0]+(A)[9]*(B)[4]+(A)[10]*(B)[8]+(A)[11]*(B)[12],(A)[8]*(B)[1]+(A)[9]*(B)[5]+(A)[10]*(B)[9]+(A)[11]*(B)[13],(A)[8]*(B)[2]+(A)[9]*(B)[6]+(A)[10]*(B)[10]+(A)[11]*(B)[14],(A)[8]*(B)[3]+(A)[9]*(B)[7]+(A)[10]*(B)[11]+(A)[11]*(B)[15],(A)[12]*(B)[0]+(A)[13]*(B)[4]+(A)[14]*(B)[8]+(A)[15]*(B)[12],(A)[12]*(B)[1]+(A)[13]*(B)[5]+(A)[14]*(B)[9]+(A)[15]*(B)[13],(A)[12]*(B)[2]+(A)[13]*(B)[6]+(A)[14]*(B)[10]+(A)[15]*(B)[14],(A)[12]*(B)[3]+(A)[13]*(B)[7]+(A)[14]*(B)[11]+(A)[15]*(B)[15]];}

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
attribute mat4 a_model;
attribute mat3 a_normal_matrix;
varying vec4 v_color;
varying vec2 v_uv;
varying vec3 v_normal;
varying vec3 v_position;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_matrix;
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
  const fragmentSrc = `
precision mediump float;
varying vec4 v_color;
varying vec2 v_uv;
varying vec3 v_normal;
void main() {
  gl_FragColor = v_color;
}
  `;
  let meshes = [];
  let index_cons = Uint16Array;
  let index_type = 5123;//gl.UNSIGNED_SHORT;
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
  function bindShaderAttribLocs(shader){
    gl.bindAttribLocation(shader, LOC_POSITION,"a_position");
    gl.bindAttribLocation(shader, LOC_COLOR,   "a_color");
    gl.bindAttribLocation(shader, LOC_UV,      "a_uv");
    gl.bindAttribLocation(shader, LOC_NORMAL,  "a_normal");
    gl.bindAttribLocation(shader, LOC_MODEL,   "a_model");
    gl.bindAttribLocation(shader, LOC_NM,      "a_normal_matrix");
  }
  that.init = function(){
    let [id] = $pop_args(1);
    cnv = document.getElementById(id);
    gl = cnv.getContext('webgl',{ premultipliedAlpha: false });

    const vertexShader = compileShader(gl.VERTEX_SHADER, vertexSrc);
    const fragmentShader = compileShader(gl.FRAGMENT_SHADER, fragmentSrc);
    shader = gl.createProgram();
    gl.attachShader(shader, vertexShader);
    gl.attachShader(shader, fragmentShader);
    bindShaderAttribLocs(shader);
    gl.linkProgram(shader);

    gl.enable(gl.DEPTH_TEST);
    gl.enable( gl.BLEND );
    // gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
    gl.blendFuncSeparate(
      gl.SRC_ALPHA,
      gl.ONE_MINUS_SRC_ALPHA,
      gl.ONE,
      gl.ONE_MINUS_SRC_ALPHA
    );

    let ext = gl.getExtension('OES_element_index_uint');
    if (ext){
      index_type = gl.UNSIGNED_INT;
      index_cons = Uint32Array;
    }
    let ext1 = gl.getExtension('OES_vertex_array_object');
    gl.bindVertexArray = function(){return ext1.bindVertexArrayOES(...arguments)};
    gl.createVertexArray = function(){return ext1.createVertexArrayOES(...arguments)};
    gl.deleteVertexArray = function(){return ext1.deleteVertexArrayOES(...arguments)};
    let ext2 = gl.getExtension('ANGLE_instanced_arrays');
    gl.vertexAttribDivisor = function(){return ext2.vertexAttribDivisorANGLE(...arguments)};
    gl.drawArraysInstanced = function(){return ext2.drawArraysInstancedANGLE(...arguments)};
    gl.drawElementsInstanced = function(){return ext2.drawElementsInstancedANGLE(...arguments)};
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

  function compute_bounding_sphere(vertices){
    let mx = Infinity;
    let my = Infinity;
    let mz = Infinity;
    let Mx =-Infinity;
    let My =-Infinity;
    let Mz =-Infinity;
    for (let i = 0; i < vertices.length; i++){
      let x = vertices[i][0];
      let y = vertices[i][1];
      let z = vertices[i][2];
      mx = Math.min(x,mx);
      my = Math.min(y,my);
      mz = Math.min(z,mz);
      Mx = Math.max(x,Mx);
      My = Math.max(y,My);
      Mz = Math.max(z,Mz);
    }
    let out = [
      (mx+Mx)*0.5,
      (my+My)*0.5,
      (mz+Mz)*0.5,
      0
    ];
    let dx = Mx-out[0];
    let dy = My-out[1];
    let dz = Mz-out[2];
    out[3] = Math.sqrt(dx*dx+dy*dy+dz*dz);
    return out;
  }

  that._update_mesh = function(){
    let [mesh_id,flags,mode,vertices,indices,colors,uvs,normals] = $pop_args(8);
    let p_vertices, p_colors, p_uvs, p_normals;
    if (flags & DIRTY_VERTICES) p_vertices = copy_list_vec_pack(vertices,3);
    if (flags & DIRTY_COLORS) p_colors = copy_list_vec_pack(colors,4);
    if (flags & DIRTY_UVS) p_uvs = copy_list_vec_pack(uvs,2);
    if (flags & DIRTY_NORMALS) p_normals = copy_list_vec_pack(normals,3);

    let mesh;
    if (mesh_id == -1){
      mesh = {
        vbo_vertices:gl.createBuffer(),
        vbo_colors:gl.createBuffer(),
        vbo_uvs:gl.createBuffer(),
        vbo_normals:gl.createBuffer(),
        ebo_indices:gl.createBuffer(),
        vbo_models: gl.createBuffer(),
        vbo_normal_matrices : gl.createBuffer(),
        VAO:gl.createVertexArray(),
        bsphere:[0,0,0,Infinity],
      }
      
      gl.bindVertexArray(mesh.VAO);
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_models);
      for (let i = 0; i < 4; i++){
        gl.enableVertexAttribArray(LOC_MODEL + i);
        gl.vertexAttribPointer(
          LOC_MODEL + i,
          4,
          gl.FLOAT,
          false,
          4*16,
          i*4*4
        );
        gl.vertexAttribDivisor(LOC_MODEL + i, 1);
      }
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_normal_matrices);
      for (let i = 0; i < 3; i++){
        gl.enableVertexAttribArray(LOC_NM + i);
        gl.vertexAttribPointer(
          LOC_NM + i,
          3,
          gl.FLOAT,
          false,
          4*9,
          i*4*3
        )
        gl.vertexAttribDivisor(LOC_NM+i, 1);
      }
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_vertices);
      gl.enableVertexAttribArray(LOC_POSITION);
      gl.vertexAttribPointer(LOC_POSITION,3,gl.FLOAT,false,0,0);

      mesh_id = meshes.length;
      meshes.push(mesh);
    }else{
      mesh = meshes[mesh_id];
      gl.bindVertexArray(mesh.VAO);
    }
    if ((flags & DIRTY_VERTICES) && vertices.length){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_vertices);
      gl.bufferData(gl.ARRAY_BUFFER, p_vertices, gl.STATIC_DRAW);
    }
    if (((flags & DIRTY_BOUNDS) || (flags & DIRTY_VERTICES)) && vertices.length){
      mesh.bsphere = compute_bounding_sphere(vertices);
    }
  
    if ((flags & DIRTY_INDICES) && indices.length){
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, mesh.ebo_indices);
      gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new index_cons(indices), gl.STATIC_DRAW);
    }
    if (flags & DIRTY_COLORS){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_colors);
      if (colors && colors.length > 0) {
        gl.bufferData(gl.ARRAY_BUFFER, p_colors, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(LOC_COLOR);
        gl.vertexAttribPointer(LOC_COLOR, 4, gl.FLOAT, false, 0, 0);
      }else{
        gl.disableVertexAttribArray(LOC_COLOR);
        gl.vertexAttrib4f(LOC_COLOR, 1.0, 1.0, 1.0, 1.0);
      }
    }
    if (flags & DIRTY_UVS){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_uvs);
      if (uvs && uvs.length > 0){
        gl.bufferData(gl.ARRAY_BUFFER, p_uvs, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(LOC_UV);
        gl.vertexAttribPointer(LOC_UV,2,gl.FLOAT,false,0,0);
      }else{
        gl.disableVertexAttribArray(LOC_UV);
        gl.vertexAttrib2f(LOC_UV, 0.0, 0.0);
      }
    }
    if (flags & DIRTY_NORMALS){
      gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_normals);
      if (normals && normals.length > 0){
        gl.bufferData(gl.ARRAY_BUFFER, p_normals, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(LOC_NORMAL);
        gl.vertexAttribPointer(LOC_NORMAL,3,gl.FLOAT,false,0,0);
      }else{
        gl.disableVertexAttribArray(LOC_NORMAL);
        gl.vertexAttrib3f(LOC_NORMAL, 0.0, 0.0, 1.0);
      }
    }
    meshes[mesh_id].n_vertices = vertices.length;
    meshes[mesh_id].n_indices = indices.length;
    meshes[mesh_id].n_normals = normals.length;
    meshes[mesh_id].n_uvs = uvs.length;
    meshes[mesh_id].n_colors = colors.length;
    gl.bindVertexArray(null);
    return mesh_id;
  }
  function compute_normal_mat(out, outIndex, modelMatrix) {
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
    out[outIndex+0] = b01 * invDet;
    out[outIndex+1] = (-a22 * a01 + a02 * a21) * invDet;
    out[outIndex+2] = (a12 * a01 - a02 * a11) * invDet;
    out[outIndex+3] = b11 * invDet;
    out[outIndex+4] = (a22 * a00 - a02 * a20) * invDet;
    out[outIndex+5] = (-a12 * a00 + a02 * a10) * invDet;
    out[outIndex+6] = b21 * invDet;
    out[outIndex+7] = (-a21 * a00 + a01 * a20) * invDet;
    out[outIndex+8] = (a11 * a00 - a01 * a10) * invDet;
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
  function sphere_test(bsphere, m, planes){
    let center = MAT_TFRM(m, bsphere);
    let sx = m[0]*m[0]+m[1]*m[1]+m[2]*m[2];
    let sy = m[4]*m[4]+m[5]*m[5]+m[6]*m[6];
    let sz = m[8]*m[8]+m[9]*m[9]+m[10]*m[10];
    let radius = Math.sqrt(Math.max(sx,Math.max(sy,sz))) * bsphere[3];
    for (let i = 0; i < 6; i++) {
      let dist =  planes[i*4+0]*center[0] +
                  planes[i*4+1]*center[1] +
                  planes[i*4+2]*center[2] +
                  planes[i*4+3];
      if (dist < - radius) return 0;
    }
    return 1;
  }

  let buf_nm = new ArrayBuffer(36);
  let buf_model = new ArrayBuffer(64)
  function impl_draw_instances(mesh_id,mode,model_matrices){
    let mesh = meshes[mesh_id];
    let n_models = model_matrices.length;
    gl.bindVertexArray(mesh.VAO);

    let nm_sz = n_models*9*4;
    if (buf_nm.byteLength < nm_sz){
      buf_nm = new ArrayBuffer(nm_sz);
    }
    let md_sz = n_models*16*4;
    if (buf_model.byteLength < md_sz){
      buf_model = new ArrayBuffer(md_sz);
    }
    let p_models = new Float32Array(buf_model, 0, 16*n_models);
    let p_normal_matrices = new Float32Array(buf_nm, 0, 9*n_models);
    let n_draw = 0;
    for (let i = 0; i < n_models; i++){
      let vis = 1;
      if (mode & CULL_FRUSTUM){
        vis = sphere_test(mesh.bsphere, model_matrices[i], cached_frust_planes);
      }
      if (vis){
        p_models[n_draw*16+ 0] = model_matrices[i][ 0];
        p_models[n_draw*16+ 1] = model_matrices[i][ 4];
        p_models[n_draw*16+ 2] = model_matrices[i][ 8];
        p_models[n_draw*16+ 3] = model_matrices[i][12];
        p_models[n_draw*16+ 4] = model_matrices[i][ 1];
        p_models[n_draw*16+ 5] = model_matrices[i][ 5];
        p_models[n_draw*16+ 6] = model_matrices[i][ 9];
        p_models[n_draw*16+ 7] = model_matrices[i][13];
        p_models[n_draw*16+ 8] = model_matrices[i][ 2];
        p_models[n_draw*16+ 9] = model_matrices[i][ 6];
        p_models[n_draw*16+10] = model_matrices[i][10];
        p_models[n_draw*16+11] = model_matrices[i][14];
        p_models[n_draw*16+12] = model_matrices[i][ 3];
        p_models[n_draw*16+13] = model_matrices[i][ 7];
        p_models[n_draw*16+14] = model_matrices[i][11];
        p_models[n_draw*16+15] = model_matrices[i][15];
        compute_normal_mat(p_normal_matrices, n_draw*9, model_matrices[i]);
        n_draw ++;
      }
    }
    p_models = new Float32Array(buf_model, 0, 16*n_draw);
    p_normal_matrices = new Float32Array(buf_nm, 0, 9*n_draw);

    gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_normal_matrices);
    gl.bufferData(gl.ARRAY_BUFFER, p_normal_matrices, gl.DYNAMIC_DRAW);

    gl.bindBuffer(gl.ARRAY_BUFFER, mesh.vbo_models);
    gl.bufferData(gl.ARRAY_BUFFER, p_models, gl.DYNAMIC_DRAW);

    if (mesh.n_indices){
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, mesh.ebo_indices);
      gl.drawElementsInstanced(mode&0xf,mesh.n_indices,index_type,0,n_draw);
    }else{
      gl.drawArraysInstanced(mode&0xf,0,mesh.n_vertices,n_draw);
    }
    gl.bindVertexArray(null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
  }
  that._draw_mesh = function(){
    let [mesh_id,mode,model_matrix] = $pop_args(3);
    impl_draw_instances(mesh_id,mode,[model_matrix]);
  }
  that._draw_instances = function(){
    let [mesh_id,mode,model_matrices] = $pop_args(3);
    impl_draw_instances(mesh_id,mode,model_matrices);
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
  that._ortho = function(){
    let [left,right,bottom,top,nearZ,farZ] = $pop_args(6);
    let rl = 1.0 / (right - left);
    let tb = 1.0 / (top - bottom);
    let fn = 1.0 / (farZ - nearZ);
    let out = new Float32Array(16);
    out[0]  = 2.0 * rl;
    out[5]  = 2.0 * tb;
    out[10] = -2.0 * fn;
    out[3] = -(right + left) * rl;
    out[7] = -(top + bottom) * tb;
    out[11] = -(farZ + nearZ) * fn;
    out[15] = 1.0;
    return out;
  }
  that.background = function(){
    let [r,g,b,a] = $pop_args(4);
    gl.clearColor(r,g,b,a);
    gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);
  }

  function extract_frustum_planes(out, m){
    let p = [
      m[3*4+0]+m[0*4+0], m[3*4+1]+m[0*4+1], m[3*4+2]+m[0*4+2], m[3*4+3]+m[0*4+3], // left
      m[3*4+0]-m[0*4+0], m[3*4+1]-m[0*4+1], m[3*4+2]-m[0*4+2], m[3*4+3]-m[0*4+3], // right
      m[3*4+0]+m[1*4+0], m[3*4+1]+m[1*4+1], m[3*4+2]+m[1*4+2], m[3*4+3]+m[1*4+3], // bottom
      m[3*4+0]-m[1*4+0], m[3*4+1]-m[1*4+1], m[3*4+2]-m[1*4+2], m[3*4+3]-m[1*4+3], // top
      m[3*4+0]+m[2*4+0], m[3*4+1]+m[2*4+1], m[3*4+2]+m[2*4+2], m[3*4+3]+m[2*4+3], // near
      m[3*4+0]-m[2*4+0], m[3*4+1]-m[2*4+1], m[3*4+2]-m[2*4+2], m[3*4+3]-m[2*4+3], // far
    ];
    for (let i = 0; i < 6; i++) {
      let a = p[i*4+0], b = p[i*4+1], c = p[i*4+2];
      let inv = 1.0 / Math.sqrt(a*a + b*b + c*c);
      out[i*4+0] = a * inv;
      out[i*4+1] = b * inv;
      out[i*4+2] = c * inv;
      out[i*4+3] = p[i*4+3] * inv;
    }
  }

  let cached_view = new Float32Array([1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]);
  let cached_proj = new Float32Array([1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]);
  let cached_frust_planes = new Float32Array(24);
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
    cached_view = view;
    cached_proj = proj;
    let vp = MAT_MULT(proj,view);
    extract_frustum_planes(cached_frust_planes,vp);
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
  
  let FONT_W = 8;
  let FONT_H = 16;
  let FONT_COLS = 16;
  let FONT_ROWS = 8;
  let FONT_N = 128;
  let FONT_TEX_W = (FONT_COLS * FONT_W);
  let FONT_TEX_H = (FONT_ROWS * FONT_H);
  let font_texture = -1;
  let text_vbo = 0;
  let text_uv_vbo = 0;
  let vao_text = 0;
  let text_shader = 0;
  let font_bitmap = new Uint8Array([
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x08,0x08,0x00,0x00,
    0x00,0x00,0x22,0x22,0x22,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x12,0x12,0x12,0x7E,0x24,0x24,0x7E,0x48,0x48,0x48,0x00,0x00,
    0x00,0x00,0x00,0x00,0x08,0x3E,0x49,0x48,0x38,0x0E,0x09,0x49,0x3E,0x08,0x00,0x00,
    0x00,0x00,0x00,0x00,0x31,0x4A,0x4A,0x34,0x08,0x08,0x16,0x29,0x29,0x46,0x00,0x00,
    0x00,0x00,0x00,0x00,0x1C,0x22,0x22,0x14,0x18,0x29,0x45,0x42,0x46,0x39,0x00,0x00,
    0x00,0x00,0x08,0x08,0x08,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x04,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x08,0x08,0x04,0x00,
    0x00,0x00,0x00,0x20,0x10,0x10,0x08,0x08,0x08,0x08,0x08,0x08,0x10,0x10,0x20,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x49,0x2A,0x1C,0x2A,0x49,0x08,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x7F,0x08,0x08,0x08,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x08,0x08,0x10,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,
    0x00,0x00,0x00,0x00,0x02,0x02,0x04,0x08,0x08,0x10,0x10,0x20,0x40,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,0x18,0x24,0x42,0x46,0x4A,0x52,0x62,0x42,0x24,0x18,0x00,0x00,
    0x00,0x00,0x00,0x00,0x08,0x18,0x28,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x02,0x0C,0x10,0x20,0x40,0x40,0x7E,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x02,0x1C,0x02,0x02,0x42,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x04,0x0C,0x14,0x24,0x44,0x44,0x7E,0x04,0x04,0x04,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x7C,0x02,0x02,0x02,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x1C,0x20,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7E,0x02,0x02,0x04,0x04,0x04,0x08,0x08,0x08,0x08,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x3E,0x02,0x02,0x02,0x04,0x38,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x08,0x08,0x10,0x00,
    0x00,0x00,0x00,0x00,0x00,0x02,0x04,0x08,0x10,0x20,0x10,0x08,0x04,0x02,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x40,0x20,0x10,0x08,0x04,0x08,0x10,0x20,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x02,0x04,0x08,0x08,0x00,0x08,0x08,0x00,0x00,
    0x00,0x00,0x00,0x00,0x1C,0x22,0x4A,0x56,0x52,0x52,0x52,0x4E,0x20,0x1E,0x00,0x00,
    0x00,0x00,0x00,0x00,0x18,0x24,0x24,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x7C,0x42,0x42,0x42,0x42,0x7C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x40,0x40,0x40,0x40,0x42,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x78,0x44,0x42,0x42,0x42,0x42,0x42,0x42,0x44,0x78,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x7C,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x7C,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x40,0x40,0x4E,0x42,0x42,0x46,0x3A,0x00,0x00,
    0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3E,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
    0x00,0x00,0x00,0x00,0x1F,0x04,0x04,0x04,0x04,0x04,0x04,0x44,0x44,0x38,0x00,0x00,
    0x00,0x00,0x00,0x00,0x42,0x44,0x48,0x50,0x60,0x60,0x50,0x48,0x44,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,
    0x00,0x00,0x00,0x00,0x42,0x42,0x66,0x66,0x5A,0x5A,0x42,0x42,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x42,0x62,0x62,0x52,0x52,0x4A,0x4A,0x46,0x46,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x7C,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x5A,0x66,0x3C,0x03,0x00,
    0x00,0x00,0x00,0x00,0x7C,0x42,0x42,0x42,0x7C,0x48,0x44,0x44,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x40,0x30,0x0C,0x02,0x42,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7F,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x00,
    0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x41,0x41,0x41,0x22,0x22,0x22,0x14,0x14,0x08,0x08,0x00,0x00,
    0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x5A,0x5A,0x66,0x66,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x42,0x42,0x24,0x24,0x18,0x18,0x24,0x24,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x41,0x41,0x22,0x22,0x14,0x08,0x08,0x08,0x08,0x08,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7E,0x02,0x02,0x04,0x08,0x10,0x20,0x40,0x40,0x7E,0x00,0x00,
    0x00,0x00,0x00,0x0E,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x0E,0x00,
    0x00,0x00,0x00,0x00,0x40,0x40,0x20,0x10,0x10,0x08,0x08,0x04,0x02,0x02,0x00,0x00,
    0x00,0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x70,0x00,
    0x00,0x00,0x18,0x24,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,
    0x00,0x20,0x10,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x02,0x3E,0x42,0x42,0x46,0x3A,0x00,0x00,
    0x00,0x00,0x00,0x40,0x40,0x40,0x5C,0x62,0x42,0x42,0x42,0x42,0x62,0x5C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x40,0x40,0x40,0x40,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x02,0x02,0x02,0x3A,0x46,0x42,0x42,0x42,0x42,0x46,0x3A,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x7E,0x40,0x40,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x0C,0x10,0x10,0x10,0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x02,0x3A,0x44,0x44,0x44,0x38,0x20,0x3C,0x42,0x42,0x3C,
    0x00,0x00,0x00,0x40,0x40,0x40,0x5C,0x62,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x08,0x08,0x00,0x18,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
    0x00,0x00,0x00,0x04,0x04,0x00,0x0C,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x48,0x30,
    0x00,0x00,0x00,0x40,0x40,0x40,0x44,0x48,0x50,0x60,0x50,0x48,0x44,0x42,0x00,0x00,
    0x00,0x00,0x00,0x18,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x76,0x49,0x49,0x49,0x49,0x49,0x49,0x49,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x5C,0x62,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x5C,0x62,0x42,0x42,0x42,0x42,0x62,0x5C,0x40,0x40,
    0x00,0x00,0x00,0x00,0x00,0x00,0x3A,0x46,0x42,0x42,0x42,0x42,0x46,0x3A,0x02,0x02,
    0x00,0x00,0x00,0x00,0x00,0x00,0x5C,0x62,0x42,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x40,0x30,0x0C,0x02,0x42,0x3C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x7C,0x10,0x10,0x10,0x10,0x10,0x0C,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x46,0x3A,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x24,0x24,0x24,0x18,0x18,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x49,0x49,0x49,0x49,0x49,0x49,0x36,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x24,0x18,0x18,0x24,0x42,0x42,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x26,0x1A,0x02,0x02,0x3C,
    0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x02,0x04,0x08,0x10,0x20,0x40,0x7E,0x00,0x00,
    0x00,0x00,0x00,0x0C,0x10,0x10,0x08,0x08,0x10,0x20,0x10,0x08,0x08,0x10,0x10,0x0C,
    0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
    0x00,0x00,0x00,0x30,0x08,0x08,0x10,0x10,0x08,0x04,0x08,0x10,0x10,0x08,0x08,0x30,
    0x00,0x00,0x00,0x31,0x49,0x46,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  ]);
  const text_vertex_src = `precision mediump float;
    attribute vec3 a_position;
    attribute vec4 a_color;
    attribute vec2 a_uv;
    attribute vec3 a_normal;
    attribute mat4 a_model;
    attribute mat3 a_normal_matrix;
    varying vec2 v_uv;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
      v_uv = a_uv;
      vec4 p = projection * view * a_model * vec4(a_position, 1.0);
      gl_Position = p;
    }`;

  const text_fragment_src = `precision mediump float;
    varying vec2 v_uv;
    uniform sampler2D font_atlas;
    void main() {
      gl_FragColor = texture2D(font_atlas,v_uv);
    }`;
  function build_font_texture() {
    let tex_data = new Uint8Array(FONT_TEX_H*FONT_TEX_W);
    for (let ch = 0; ch < FONT_N; ++ch) {
      let gx = (ch % FONT_COLS) * FONT_W;
      let gy =~~(ch / FONT_COLS) * FONT_H;
      for (let row = 0; row < FONT_H; ++row) {
        let bits = font_bitmap[ch*FONT_H+row];
        for (let col = 0; col < FONT_W; ++col) {
          if (bits & (1 << (7 - col))) {
            tex_data[ (gy + row) * FONT_TEX_W + gx + col] = 255;
          }
        }
      }
    }
    font_texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, font_texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, FONT_TEX_W, FONT_TEX_H, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, tex_data);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

    vao_text = gl.createVertexArray();
    gl.bindVertexArray(vao_text);

    text_vbo = gl.createBuffer();
    text_uv_vbo = gl.createBuffer();

    gl.bindBuffer(gl.ARRAY_BUFFER, text_uv_vbo);
    gl.enableVertexAttribArray(LOC_UV);
    gl.vertexAttribPointer(LOC_UV, 2, gl.FLOAT, false, 0,0);

    gl.bindBuffer(gl.ARRAY_BUFFER, text_vbo);
    gl.enableVertexAttribArray(LOC_POSITION);
    gl.vertexAttribPointer(LOC_POSITION, 3, gl.FLOAT, false, 0,0);

    gl.disableVertexAttribArray(LOC_COLOR);
    gl.vertexAttrib4f(LOC_COLOR, 1,1,1,1);
    gl.disableVertexAttribArray(LOC_NORMAL);
    gl.vertexAttrib3f(LOC_NORMAL, 0,0,1);

    gl.bindVertexArray(null);

    const vertexShader = compileShader(gl.VERTEX_SHADER, text_vertex_src);
    const fragmentShader = compileShader(gl.FRAGMENT_SHADER, text_fragment_src);
    text_shader = gl.createProgram();
    gl.attachShader(text_shader, vertexShader);
    gl.attachShader(text_shader, fragmentShader);
    bindShaderAttribLocs(text_shader);
    gl.linkProgram(text_shader);

  }

  that.text = function(){
    let [str,model_matrix] = $pop_args(2);
    if (font_texture == -1){
      build_font_texture();
    }
    let prev_prog = gl.getParameter(gl.CURRENT_PROGRAM);
    let program = 0;
    if (prev_prog == 0 || prev_prog == shader){
      gl.useProgram(text_shader);
      program = text_shader;
      let loc_view = gl.getUniformLocation(program, "view");
      glUniformMatrix4fv(loc_view, true, cached_view);
      let loc_proj = gl.getUniformLocation(program, "projection");
      glUniformMatrix4fv(loc_proj, true, cached_proj);
    }else{
      program = prev_prog;
    }

    gl.bindVertexArray(vao_text);

    let len = str.length;
    let vertices = new Float32Array(len*18);
    let uvs = new Float32Array(len*12);
    for (let i = 0; i < len; i++){
      let ch = str.charCodeAt(i)-32;
      let cx = ch % FONT_COLS;
      let cy =~~(ch / FONT_COLS);
      let u0 = cx * (FONT_W / FONT_TEX_W);
      let v0 = cy * (FONT_H / FONT_TEX_H);
      let u1 = u0 + (FONT_W / FONT_TEX_W);
      let v1 = v0 + (FONT_H / FONT_TEX_H);
      let x0 = i * FONT_W;
      let y0 = 0;
      let x1 = x0 + FONT_W;
      let y1 = y0 + FONT_H;
      vertices[i*18+0]=x0;vertices[i*18+1]=y0;vertices[i*18+2]=0;
      vertices[i*18+3]=x1;vertices[i*18+4]=y0;vertices[i*18+5]=0;
      vertices[i*18+6]=x1;vertices[i*18+7]=y1;vertices[i*18+8]=0;
      vertices[i*18+9]=x0;vertices[i*18+10]=y0;vertices[i*18+11]=0;
      vertices[i*18+12]=x1;vertices[i*18+13]=y1;vertices[i*18+14]=0;
      vertices[i*18+15]=x0;vertices[i*18+16]=y1;vertices[i*18+17]=0;
      uvs[i*12+0]=u0;uvs[i*12+1]=v0;
      uvs[i*12+2]=u1;uvs[i*12+3]=v0;
      uvs[i*12+4]=u1;uvs[i*12+5]=v1;
      uvs[i*12+6]=u0;uvs[i*12+7]=v0;
      uvs[i*12+8]=u1;uvs[i*12+9]=v1;
      uvs[i*12+10]=u0;uvs[i*12+11]=v1;
    }

    for (let i = 0; i < 4; i++){
      gl.disableVertexAttribArray(LOC_MODEL + i);
      gl.vertexAttrib4f(LOC_MODEL + i, 
        model_matrix[0*4+i],
        model_matrix[1*4+i],
        model_matrix[2*4+i],
        model_matrix[3*4+i]
      );
    }

    gl.disableVertexAttribArray(LOC_NM);
    gl.vertexAttrib3f(LOC_NM,   1,0,0);
    gl.disableVertexAttribArray(LOC_NM+1);
    gl.vertexAttrib3f(LOC_NM+1, 0,1,0);
    gl.disableVertexAttribArray(LOC_NM+2);
    gl.vertexAttrib3f(LOC_NM+2, 0,0,1);

    gl.bindBuffer(gl.ARRAY_BUFFER, text_vbo);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.DYNAMIC_DRAW);

    gl.bindBuffer(gl.ARRAY_BUFFER, text_uv_vbo);
    gl.bufferData(gl.ARRAY_BUFFER, uvs, gl.DYNAMIC_DRAW);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, font_texture);
    let tex_loc = gl.getUniformLocation(program, "font_atlas");
    gl.uniform1i(tex_loc, 0);
    gl.drawArrays(gl.TRIANGLES, 0, len*6);

    gl.bindVertexArray(null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.useProgram(prev_prog);

  }
}
