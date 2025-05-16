globalThis.$guts = new function(){
  var that = this;
  that.gc_off = function(){}
  that.gc_on = function(){}
  that.gc = function(){}
}

