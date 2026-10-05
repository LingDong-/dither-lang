globalThis.$sys = new function(){
  var that = this;
  let STDOUT_INHERIT = 0
  let STDOUT_PIPE    = 1
  let STDOUT_IGNORE  = 2
  that.gc_off = function(){}
  that.gc_on = function(){}
  that.gc = function(){}
  that.argv = function(){
    if (globalThis.__dh_intern_argv){
      return globalThis.__dh_intern_argv;
    }
    if ($argv.length){
      return $argv.slice();
    }
    if (typeof process !== 'undefined'){
      return process.argv.slice(2);
    }
    if (window?.location){
      return Array.from(new URLSearchParams(window.location.search).getAll("argv"));
    }
    return [];
  }
  that.platform = function(){
    return "js@" + navigator.userAgent;
  }
  that.getenv = function(){
    let name = $pop_args(1);
    return (globalThis.ENV ?? globalThis.__dh_intern_env ?? {})[name] ?? "";
  }
  that.command = function(){
    let [cmd,flags] = $pop_args(2);
    let o = "";
    let st = 0;
    try{
      o = eval(cmd);
    }catch(e){
      console.error(e);
      st = 1;
    }
    if (flags == STDOUT_PIPE){
      if (typeof o != 'object'){
        o = new TextEncoder().encode(o);
      }
      o = Array.from(o);
    }else{
      if (flags == STDOUT_INHERIT) (globalThis.__io_intern_hooked_print ?? console.log)(o);
      o = [];
    }
    o.__type = {con:'list',elt:['u8']};
    return [st,o];
  }
}

