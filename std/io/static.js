globalThis.$io = new function(){
  var that = this;
  var is_node = typeof module !== 'undefined';
  let buf = [];
  let fs;

  that.print = async function(){
    let [x] = $pop_args(1);
    if (globalThis.__io_intern_hooked_print){
      let o = __io_intern_hooked_print(x);
      if (o instanceof Promise){
        await o;
      }
    }else if (is_node){
      process.stdout.write(x);
    }else{
      for (let q of x){
        if (q == '\n'){
          console.log(buf.join(''));
          buf.splice(0,Infinity);
          await (function() {
            return new Promise(resolve => requestAnimationFrame(resolve));
          })();
        }else{
          buf.push(q)
        }
      }
    }
  }
  that.println = async function(){
    await that.print();
    $args.push('\n');
    await that.print();
  }
  that.read_file = async function(){
    let [pth] = $pop_args(1);
    if (globalThis.__io_intern_hooked_read_file){
      let o = __io_intern_hooked_read_file(pth);
      if (o instanceof Promise){
        o = await o;
      }
      return o;
    }else if (is_node){
      return Array.from((fs??(fs=require('fs'))).readFileSync(pth));
    }else{
      const response = await fetch(pth, { method: 'GET' });
      const arrayBuffer = await response.arrayBuffer();
      let arr = Array.from(new Uint8Array(arrayBuffer));
      return arr;
    }
  }
  that.write_file = async function(){
    let [pth,lst] = $pop_args(2);
    if (is_node){
      const buffer = Buffer.from(lst);
      (fs??(fs=require('fs'))).writeFileSync(pth, buffer);
    }else{
      const blob = new Blob([new Uint8Array(lst)], { type: 'application/octet-stream' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = pth;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }
  }
}

