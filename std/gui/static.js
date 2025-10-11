
globalThis.$gui = new function(){
  var that = this;
  let panel;
  let rows = [];
  that.init = function(){
    panel = document.createElement("div");
    panel.classList.add("dh-std-gui");
    panel.style = `position:absolute;right:0px;bottom:0px;width:250px;height:20px;background:#555;border-radius:2px;color:#eee;font-family:sans-serif;font-size:12px;overflow:hidden;`
    document.body.appendChild(panel);

    const styleElement = document.createElement('style');
    let knob = `
      width:5px;
      height:14px;
      background:#eee;
    `
    styleElement.textContent = `
    .dh-std-gui-slider::-webkit-slider-thumb {
      -webkit-appearance: none; appearance: none;
      ${knob}
    }

    .dh-std-gui-slider::-moz-range-thumb {
      ${knob}
    }
    `;
    document.head.appendChild(styleElement);


  }
  that.slider = function(){
    let [name,x,l,r] = $pop_args(4);
    let div = document.createElement("div");
    div.style=`position:absolute;left:5px;top:${rows.length*25};width:250px;height:25px;overflow:hidden;`;

    let lbl = document.createElement("div");
    lbl.style=`position:absolute;left:0px;top:5px;width:80px;height:20px;text-align:right;overflow:hidden;`;
    lbl.innerHTML = name;
    
    let sld = document.createElement("input");
    sld.classList.add("dh-std-gui-slider");
    sld.setAttribute("type","range");
    sld.style=`position:absolute;left:83px;top:2px;width:100px;height:16px;
    -webkit-appearance: none;appearance: none;
    background:#333;
    `;


    let inp = document.createElement("input");
    inp.setAttribute("type","number");
    inp.style=`position:absolute;left:190px;top:3px;width:52px;height:18px;
    -webkit-appearance: none;appearance: none;
    border:none;
    border-radius:2px;
    outline:none;
    background: #333;
    color:#eee;
    font-size:12px;
    `;

    sld.value = (x-l)/(r-l)*100;

    // let sll = document.createElement("div");
    // sll.style=`position:absolute;left:85px;top:11px;width:${sld.value}px;height:1px;
    // background:#eee;
    // `;

    inp.value = x;
    let row = {
      name,
      val:x,
      min:l,
      max:r,
      div,
    }

    sld.oninput = function(){
      let v= Number(sld.value)/100.0*(r-l)+l;
      inp.value = v;
      row.val = v;
      // sll.style.width = sld.value+'px';
    }
    inp.onchange = function(){
      let v = Number(inp.value);
      if (v < l) v = l;
      if (v > r) v = r;
      inp.value = v;
      sld.value = (v-l)/(r-l)*100.0;
      row.val = v;
      // sll.style.width = sld.value+'px';
    }


    rows.push(row);
    panel.style.height = (rows.length)*25;
    div.appendChild(lbl);
    div.appendChild(sld);
    // div.appendChild(sll);
    div.appendChild(inp);
    panel.appendChild(div);
  }
  that.get = function(){
    let [name] = $pop_args(1);
    for (let i = 0; i < rows.length; i++){
      if (rows[i].name == name){
        return rows[i].val;
      }
    }
    return 0;
  }
  that.poll = function(){

  }
}

