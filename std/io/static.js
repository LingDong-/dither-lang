globalThis.$io = new function(){
  var that = this;
  var is_node = typeof module !== 'undefined';
  let buf = [];
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
}

