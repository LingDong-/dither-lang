globalThis.$win = new function(){
  var that = this;
  let cnv;
  let evtq = [];
  let keymap = {
    F1    :0xffbe,
    F2    :0xffbf,
    F3    :0xffc0,
    F4    :0xffc1,
    F5    :0xffc2,
    F6    :0xffc3,
    F7    :0xffc4,
    F8    :0xffc5,
    F9    :0xffc6,
    F10   :0xffc7,
    F11   :0xffc8,
    F12   :0xffc9,
    ArrowLeft: 0xff51,
    ArrowUp:   0xff52,
    ArrowRight:0xff53,
    ArrowDown: 0xff54,
    Shift:     0xffe1,
    Control:   0xffe3,
    Alt:       0xffe9,
    Meta:      0xffeb,
  };
  that.init = function(){
    let [w,h,flags] = $pop_args(3);
    let par = document.getElementById("out") ?? document.body;
    cnv = document.createElement("canvas");
    let id = ~~(Math.random()*65535)
    cnv.id = id;
    cnv.width = w;
    cnv.height = h;
    par.appendChild(cnv);

    cnv.addEventListener('mousedown',function(e){
      let r = cnv.getBoundingClientRect();
      x = (e.clientX-r.left);
      y = (e.clientY-r.top);
      evtq.push({type:1,key:[1,0,2][e.button],x,y})
    });
    cnv.addEventListener('mouseup',function(e){
      let r = cnv.getBoundingClientRect();
      x = (e.clientX-r.left);
      y = (e.clientY-r.top);
      evtq.push({type:2,key:[1,0,2][e.button],x,y})
    });
    cnv.addEventListener('mousemove',function(e){
      let r = cnv.getBoundingClientRect();
      x = (e.clientX-r.left);
      y = (e.clientY-r.top);
      if (evtq.length && evtq.at(-1).type == 3){
        evtq[evtq.length-1].x = x;
        evtq[evtq.length-1],y = y;
      }else{
        evtq.push({type:3,key:0,x,y})
      }
    });
    function mapkey(k){
      let m = keymap[k];
      if (!m){
        m = k.charCodeAt(0);
      }
      return m;
    }
    document.addEventListener('keydown',function(e){
      let key = mapkey(e.key);
      evtq.push({type:4,key,x:0,y:0});
    });
    document.addEventListener('keyup',function(e){
      let key = mapkey(e.key);
      evtq.push({type:5,key,x:0,y:0});
    });
    return BigInt(id);
  }
  function animation_frame() {
    return new Promise(resolve => requestAnimationFrame(resolve));
  }
  that.poll = async function(){
    await animation_frame();
    if (evtq.length){
      return evtq.shift();
    }else{
      return {type:0,key:0,x:0,y:0};
    }
  }
  that.exit = function(){
    cnv.parentElement.removeChild(cnv);
  }
}

