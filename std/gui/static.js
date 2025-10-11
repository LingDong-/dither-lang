
globalThis.$gui = new function(){
  var that = this;
  let panel;
  let rows = [];
  that.init = function(){
    panel = document.createElement("div");
    panel.classList.add("dh-std-gui");
    panel.style = `position:absolute;right:0px;bottom:0px;width:260px;height:35px;background:whitesmoke;border:1px solid silver;color:black;font-family:sans-serif;font-size:13px;`
    document.body.appendChild(panel);
  }
  that.slider = function(){
    let [name,x,l,r] = $pop_args(4);
    let div = document.createElement("div");
    div.style=`position:absolute;left:5px;top:${5+rows.length*25};width:250px;height:25px;`;

    let lbl = document.createElement("div");
    lbl.style=`position:absolute;left:2px;top:5px;width:80px;height:20px;text-align:right;overflow:hidden;`;
    lbl.innerHTML = name;
    
    let sld = document.createElement("input");
    sld.setAttribute("type","range");
    sld.style=`position:absolute;left:85px;top:0px;width:100px;height:20px;`;
    
    let inp = document.createElement("input");
    inp.setAttribute("type","number");
    inp.style=`position:absolute;left:190px;top:2px;width:60px;height:20px`;

    sld.value = (x-l)/(r-l)*100;
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
    }
    inp.onchange = function(){
      let v = Number(inp.value);
      if (v < l) v = l;
      if (v > r) v = r;
      inp.value = v;
      sld.value = (v-l)/(r-l)*100.0;
      row.val = v;
    }


    rows.push(row);
    panel.style.height = (rows.length)*25+10;
    div.appendChild(lbl);
    div.appendChild(sld);
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

