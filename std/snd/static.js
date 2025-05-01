globalThis.$snd = new function(){
  let that = this;

  let workletCode = `
    class MyProcessor extends AudioWorkletProcessor {
      constructor(){
        super();
        this.queue = [];
        this.port.onmessage = (e) => {
          for (let i = 0; i < e.data.length; i++){
            this.queue.push(e.data[i]);
          }
        };
      }
      process(inputs, outputs){
        const output = outputs[0];
        const frames = output[0].length;
        let idx = 0;
        for (let i = 0; i < frames; i++){
          for (let c = 0; c < output.length; c++){
            output[c][i] = idx < this.queue.length ? this.queue[idx++] : 0.0;
          }
        }
        this.queue.splice(0,idx);
        if (this.queue.length <= frames){
          this.port.postMessage(1);
        }
        return true;
      }
    }
    registerProcessor('my-processor', MyProcessor);
  `;

  
  let BUF_SIZE = 1024;
  let buffer = [];

  let audioCtx, workletNode;
  let chan,rate;
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
  function animation_frame() {
    return new Promise(resolve => requestAnimationFrame(resolve));
  }
  that.init = async function(){
    ;[rate,chan] = $pop_args(2);
    btn = document.createElement("button");
    btn.innerHTML = "📢"
    btn.style = "position:absolute;"
    document.body.appendChild(btn);
    btn.onclick = async function(){
      want_init = 0;
      btn.parentElement.removeChild(btn);
      let _audioCtx = new AudioContext({ sampleRate: rate });
      const blob = new Blob([workletCode], { type: 'application/javascript' });
      const url = URL.createObjectURL(blob);
      await _audioCtx.audioWorklet.addModule(url);
      workletNode = new AudioWorkletNode(_audioCtx, 'my-processor', {
        outputChannelCount: [chan]
      });
      workletNode.connect(_audioCtx.destination);
      audioCtx = _audioCtx;

      workletNode.port.onmessage = function(){
        workletNode.port.postMessage(buffer);
        buffer = []
      }

    }
    if (did_act){
      await btn.onclick();
    }else{
      want_init = 1;
    }
    await animation_frame();
  };

  that.put_sample = async function(){
    let [x] = $pop_args(1);
    if (!audioCtx){
      await animation_frame();
      return;
    }
    buffer.push(x);
  };

  that.buffer_full = async function(){
    if (!audioCtx){
      await animation_frame();
      return 1;
    }
    
    let o = buffer.length >= BUF_SIZE;
    if (o){
      await animation_frame();
    }
    return o;
  };


  that.exit = async function(){

    if (workletNode) workletNode.disconnect();
    if (audioCtx) audioCtx.close();
  
  };
};