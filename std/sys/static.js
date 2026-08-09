globalThis.$sys = new function(){
  var that = this;
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
}

