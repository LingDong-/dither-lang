globalThis.$snd = new function(){
  let that = this;
  let BUF_SIZE = 512;
  let audioSource, audioCtx;
  let rate=44100,chan=2;
  let inp_buf,out_buf,abuf_idx,abuf_ch,audio_die;
  let did_act = 0;
  let want_init = 0;
  let btn;
  function act_init(){
    did_act = 1;
    if (want_init){
      btn.click();
    }
  }
  window.addEventListener('click',   act_init);
  window.addEventListener('keydown', function(e){
    let ignore = ['Control', 'Shift', 'Alt', 'Meta', 'CapsLock', 'Tab'];
    if (ignore.includes(e.key)) return;
    act_init();
  });
  // let t0 = new Date().getTime();
  function animation_frame() {
    // let t = new Date().getTime();
    // let dt = t-t0;
    // t0 = t;
    // console.log(dt);
    return new Promise(resolve => requestAnimationFrame(resolve));
  }
  that.init = async function(){
    ;[rate,chan] = $pop_args(2);
    btn = document.createElement("button");
    // btn.innerHTML = "start audio context"
    // btn.style = "position:absolute; left:calc(50% - 75px); top:calc(50% - 10px); width:150px;"
    btn.innerHTML = "📢"
    btn.style = "position:absolute;"
    document.body.appendChild(btn);
    btn.onclick = function(){
      want_init = 0;
      btn.parentElement.removeChild(btn);
      audioCtx = new AudioContext({sampleRate:rate});
      let buf0 = audioCtx.createBuffer(chan, BUF_SIZE, rate);
      let buf1 = audioCtx.createBuffer(chan, BUF_SIZE, rate);
      rate = rate;
      chan = chan;
      inp_buf = buf0;
      out_buf = buf1;
      abuf_idx = 0;
      abuf_ch = 0;
      audio_die = 0;
      function flipper(){
        if (audio_die) return;
        if (abuf_idx >= BUF_SIZE){
          [out_buf,inp_buf] = [inp_buf,out_buf];
          abuf_idx = 0;
          abuf_ch = 0;
        }
        audioSource = audioCtx.createBufferSource();
        audioSource.buffer = out_buf;
        audioSource.onended = flipper;
        audioSource.connect( audioCtx.destination );
        audioSource.start();
      }
      flipper();
    }
    if (did_act){
      btn.click();
    }else{
      want_init = 1;
    }
    await animation_frame();
  }
  that.buffer_full = async function(){
    if (!audioSource){
      await animation_frame();
      return 1;
    }
    let cd = inp_buf.getChannelData(chan-1);
    let o = abuf_idx >= cd.length;
    if (o){
      await animation_frame();
    }
    return o;
  }
  that.put_sample = function(){
    if (!audioSource) return;
    let [x] = $pop_args(1);
    let cd = inp_buf.getChannelData(abuf_ch);
    cd[abuf_idx++] = x;
    abuf_ch = (abuf_ch + 1) % chan;
  }
  that.exit = function(){
    if (!audioSource) return;
    audio_die = 1;
    audioCtx.close()
  }

}


