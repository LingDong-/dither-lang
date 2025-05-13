globalThis.$time = new function(){
  var that = this;

  function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }
  let last_time = performance.now();
  let start_time = last_time;

  that.fps = async function(){
    let [target_fps] = $pop_args(1);
    if (target_fps < 0) target_fps = Number.MAX_VALUE;
    let current_time = performance.now();
    let elapsed = (current_time - last_time) / 1000.0;
    let target_frame_time = 1.0 / target_fps;
    let sleep_time_sec = target_frame_time - elapsed;
    sleep_time_sec -= 0.001;
    if (sleep_time_sec > 0) {
      await sleep(sleep_time_sec * 1000);
      elapsed = performance.now()-last_time;
    }
    last_time = current_time;
    return 1.0 / elapsed;
  }

  that.millis = function(){
    let now = performance.now();
    return now - start_time;
  }

  that.stamp = function(){
    return new Date().getTime()/1000.0;
  }

  that.delay = async function(){
    let [ms] = $pop_args(1);
    await sleep(ms);
  }
  that.local = function(){
    let [ts] = $pop_args(1);
    let dt = new Date(ts*1000.0);
    return [dt.getFullYear(), dt.getMonth()+1, dt.getDate(), dt.getHours(), dt.getMinutes(), dt.getSeconds()]
  }

}

