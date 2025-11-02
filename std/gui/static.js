
globalThis.$gui = new function(){
  var that = this;
  let panel;
  let rows = [];
  that.init = function(){
    panel = document.createElement("div");
    panel.classList.add("dh-std-gui");
    panel.style = `position:absolute;right:0px;bottom:0px;width:250px;height:20px;overflow:hidden;`
    document.body.appendChild(panel);

    const styleElement = document.createElement('style');
    let knob = `
      width:5px;
      height:14px;
      background:#eee;
    `
    styleElement.textContent = `
    .dh-std-gui{
      background:#555;border-radius:2px;color:#eee;font-family:sans-serif;font-size:12px;
    }
    .dh-std-gui-slider{
      -webkit-appearance: none;appearance: none;
      background:#333;
    }
    .dh-std-gui-field{
      -webkit-appearance: none;appearance: none;
      border:none;
      border-radius:2px;
      outline:none;
      background: #333;
      color:#eee;
      font-size:12px;
    }
    .dh-std-gui-slider::-webkit-slider-thumb {
      -webkit-appearance: none; appearance: none;
      ${knob}
    }

    .dh-std-gui-slider::-moz-range-thumb {
      ${knob}
    }
    .dh-std-gui-tick{
      background:#eee;
    }

    .dh-std-gui-toggle{
      -webkit-appearance: none;appearance: none;
      margin:0px;
      background: #333;
      width:16px;
      height:16px;
    }
    .dh-std-gui-toggle:checked::after{
      display:inline-block;
      content:"";
      width:8px;
      height:8px;
      margin:4px;
      background: #eee;
    }
    `;
    document.head.appendChild(styleElement);


  }
  that.slider = function(){
    let typ = $args.at(-3).__type;
    let [name,x,l,r] = $pop_args(4);
    if (typ == 'f32'){
      impl_slider(name,x,l,r,false);
    }else if (typ == 'i32'){
      impl_slider(name,x,l,r,true);
    }else if (typ.con == 'vec'){
      for (let i = 0; i < typ.elt[1]; i++){
        impl_slider(`${name}[${i}]`,x[i],l[i],r[i],false);
      }
    }
  }

  function impl_slider(name,x,l,r,is_int){ 
    let div = document.createElement("div");
    div.style=`position:absolute;left:5px;top:${rows.length*25};width:250px;height:25px;overflow:hidden;`;

    let lbl = document.createElement("div");
    lbl.style=`position:absolute;left:0px;top:5px;width:80px;height:20px;text-align:right;overflow:hidden;`;
    lbl.innerHTML = name;
    
    let sld = document.createElement("input");
    sld.classList.add("dh-std-gui-slider");
    sld.setAttribute("type","range");
    sld.style=`position:absolute;left:83px;top:2px;width:100px;height:16px;
    `;
    let inp = document.createElement("input");
    inp.classList.add("dh-std-gui-field");
    inp.setAttribute("type","number");
    inp.style=`position:absolute;left:190px;top:3px;width:52px;height:18px;
    `;

    sld.setAttribute("min",l);
    sld.setAttribute("max",r);
    sld.value = x;
    if (is_int){
      sld.setAttribute("step",1);
    }else{
      sld.setAttribute("step","any");
    }

    inp.value = x;
    let row = {
      name,
      val:x,
      min:l,
      max:r,
      div,
    }
    sld.oninput = function(){
      let v = Number(sld.value);
      inp.value = v;
      row.val = v;
    }
    inp.onchange = function(){
      let v = Number(inp.value);
      if (v < l) v = l;
      if (v > r) v = r;
      inp.value = v;
      sld.value = v;
      row.val = v;
    }
    rows.push(row);
    panel.style.height = (rows.length)*25;
    div.appendChild(lbl);
    div.appendChild(sld);
    div.appendChild(inp);
    panel.appendChild(div);

    if (is_int){
      for (let i = 0; i <= r-l; i++){
        let t = i/(r-l);
        let tick = document.createElement("div");
        tick.classList.add("dh-std-gui-tick");
        tick.style=`position:absolute;left:${87+t*95}px;top:8px;width:1px;height:8px;pointer-events:none`;
        div.appendChild(tick);
      }
    }
  }
  that.toggle = function(){
    let [name,x] = $pop_args(2);
    let div = document.createElement("div");
    div.style=`position:absolute;left:5px;top:${rows.length*25};width:250px;height:25px;overflow:hidden;`;

    let lbl = document.createElement("div");
    lbl.style=`position:absolute;left:0px;top:5px;width:80px;height:20px;text-align:right;overflow:hidden;`;
    lbl.innerHTML = name;
    
    let chk = document.createElement("input");
    chk.classList.add("dh-std-gui-toggle");
    chk.setAttribute("type","checkbox");
    chk.style=`position:absolute;left:225px;top:2px;`;

    let row = {
      name,
      val:x,
      div,
    }

    chk.checked = row.val;
    chk.onchange = function(){
      row.val = Number(chk.checked);
    }

    rows.push(row);
    panel.style.height = (rows.length)*25;
    div.appendChild(lbl);
    div.appendChild(chk);
    panel.appendChild(div);
  }
  
  that.field = function(){
    let [name,x] = $pop_args(2);

    let div = document.createElement("div");
    div.style=`position:absolute;left:5px;top:${rows.length*25};width:250px;height:25px;overflow:hidden;`;

    let lbl = document.createElement("div");
    lbl.style=`position:absolute;left:0px;top:5px;width:80px;height:20px;text-align:right;overflow:hidden;`;
    lbl.innerHTML = name;
    
    let inp = document.createElement("input");
    inp.classList.add("dh-std-gui-field");
    inp.style=`position:absolute;left:85px;top:3px;width:157px;height:18px;
    `;
    inp.value = x;
    let row = {
      name,
      val:x,
      div,
    }
    inp.oninput = function(){
      let v =inp.value;
      inp.value = v;
      row.val = v;
    }
    rows.push(row);
    panel.style.height = (rows.length)*25;
    div.appendChild(lbl);
    div.appendChild(inp);
    panel.appendChild(div);
  }

  function impl_get(name,retype){
    for (let i = 0; i < rows.length; i++){
      if (rows[i].name == name){
        if (retype == 'f32' || retype == 'i32' || retype == 'str'){
          return rows[i].val;
        }
      }
    }
    if (retype == 'f32' || retype == 'i32'){
      return 0;
    }else if (retype == 'str'){
      return "";
    }
  }

  that.get = function(retype){
    let [name] = $pop_args(1);
    let v = [];
    if (retype.con == 'vec'){
      for (let i = 0; i < retype.elt[1]; i++){
        v.push(impl_get(`${name}[${i}]`,retype.elt[0]));
      }
      return v;
    }else{
      return impl_get(name,retype);
    }
  }
  that.poll = function(){

  }
}

