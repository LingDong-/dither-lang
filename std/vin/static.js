
globalThis.$vin = new function(){
  var that = this;

  let SOURCE_WEBCAM =1
  let SOURCE_FREAD  =2
  let SOURCE_FWRITE =4
  let RESO_VGA  =8
  let RESO_HD   =16
  let RESO_FHD  =32
  let EFFECT_MIRROR  =64
  
  let vids = [];

  async function waitForEvent(element, eventName, fun) {
    return new Promise((resolve) => {
      const handler = (event) => {
        fun(event);
        element.removeEventListener(eventName, handler);
        resolve(event);
      };
      element.addEventListener(eventName, handler);
    });
  }

  that.create = async function(){
    let [flag,path] = $pop_args(2);
    let v = {flag};
    if (flag & SOURCE_WEBCAM){
      if (flag & RESO_VGA){
        v.w = 640;
        v.h = 480;
      }else if (flag & RESO_HD){
        v.w = 1280;
        v.h = 720;
      }else if (flag & RESO_FHD){
        v.w = 1920;
        v.h = 1080;
      }
      const devices = await navigator.mediaDevices.enumerateDevices();
      const videoDevices = devices.filter(device => device.kind === "videoinput");
      const chosen = videoDevices.find(d => d.label.includes(path));
      const stream = await navigator.mediaDevices.getUserMedia({
        video: {
          deviceId: chosen?.deviceId || videoDevices[0].deviceId,
          width: { ideal: v.w },
          height: { ideal: v.h }
        }
      });
      const videoElement = document.createElement("video");
      videoElement.autoplay = true;
      videoElement.playsInline = true;
      videoElement.muted = true;   
      videoElement.style.position = "absolute";
      videoElement.style.opacity = "0.01"; 
      videoElement.style.pointerEvents = "none";
      videoElement.srcObject = stream;
      document.body.appendChild(videoElement);
      const canvas = document.createElement("canvas");
      canvas.width = v.w;
      canvas.height = v.h;
      // document.body.appendChild(canvas);
      v.canvas = canvas;
      v.videoElement = videoElement;
    }else if (flag & SOURCE_FREAD){
      const videoElement = document.createElement("video");
      videoElement.autoplay = true;
      videoElement.playsInline = true;
      videoElement.muted = true;  
      videoElement.loop = true;   
      videoElement.style.position = "absolute";
      videoElement.style.opacity = "0.01"; 
      videoElement.style.pointerEvents = "none";
      videoElement.src = "fingers.mp4";
      document.body.appendChild(videoElement);
      const canvas = document.createElement("canvas");
      canvas.width = v.w;
      canvas.height = v.h;
      // document.body.appendChild(canvas);
      await waitForEvent(videoElement,"loadedmetadata", () => {
        v.w = canvas.width = videoElement.videoWidth;
        v.h = canvas.height = videoElement.videoHeight;
      });
      v.canvas = canvas;
      v.videoElement = videoElement;
    }
    vids.push(v);
    return {
      id:vids.length-1,
      w:v.w,
      h:v.h,
    }
  }
  that._read_pixels = function(){
    let [id] = $pop_args(1);
    let v = vids[id];
    let ctx = v.canvas.getContext('2d',{willReadFrequently:true});
    let fx = v.canvas.width/v.videoElement.videoWidth;
    let fy = v.canvas.height/v.videoElement.videoHeight;
    let f = Math.max(fx,fy);
    ctx.save();
    if (v.flag & EFFECT_MIRROR){
      ctx.translate(v.w,0);
      ctx.scale(-1, 1);
    }
    ctx.drawImage(v.videoElement, 0, 0, f*v.videoElement.videoWidth, f*v.videoElement.videoHeight);
    ctx.restore();
    let imdata = ctx.getImageData(0,0,v.w,v.h).data;
    return Object.assign(Array.from(imdata),{__dims:[v.h,v.w,4]});
  }
}

