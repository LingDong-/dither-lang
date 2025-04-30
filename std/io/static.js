globalThis.$io = new function(){
  var that = this;
  var is_node = typeof module !== 'undefined';
  let buf = [];
  that.print = function(){
    let [x] = $pop_args(1);
    if (globalThis.__io_intern_hooked_print){
      __io_intern_hooked_print(x);
    }else if (is_node){
      process.stdout.write(x);
    }else{
      for (let q of x){
        if (q == '\n'){
          console.log(buf.join(''));
          buf.splice(0,Infinity);
        }else{
          buf.push(q)
        }
      }
    }
  }
  that.println = function(){
    that.print();
    $args.push('\n');
    that.print();
  }
}

