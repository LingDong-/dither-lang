
globalThis.$img = new function(){
  var that = this;

  function getImageMimeType(byteArray) {
    if (!byteArray || byteArray.length < 4) return null;
    const bytes = new Uint8Array(byteArray);
    if (bytes[0] === 0x89 && bytes[1] === 0x50 && bytes[2] === 0x4E && bytes[3] === 0x47) {
      return 'image/png';
    }
    if (bytes[0] === 0xFF && bytes[1] === 0xD8 && bytes[2] === 0xFF) {
      return 'image/jpeg';
    }
    if (bytes[0] === 0x47 && bytes[1] === 0x49 && bytes[2] === 0x46) {
      return 'image/gif';
    }
    if (bytes[0] === 0x42 && bytes[1] === 0x4D) {
      return 'image/bmp';
    }
    if (
      bytes[0] === 0x52 && bytes[1] === 0x49 && bytes[2] === 0x46 && bytes[3] === 0x46 && 
      bytes[8] === 0x57 && bytes[9] === 0x45 && bytes[10] === 0x42 && bytes[11] === 0x50  
    ) {
      return 'image/webp';
    }
  }
  that.decode = async function(){
    let [byteArray] = $pop_args(1);
    return await new Promise((resolve, reject) => {
      const blob = new Blob([new Uint8Array(byteArray)], { type: getImageMimeType(byteArray) });
      const url = URL.createObjectURL(blob);
      const img = new Image();
      img.onload = () => {
        const canvas = document.createElement('canvas');
        canvas.width = img.width;
        canvas.height = img.height;
        const ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0);
        let data = ctx.getImageData(0, 0, canvas.width, canvas.height).data;
        
        const out = Object.assign(
          data,
          {__dims:[img.height,img.width,4]}
        );
        URL.revokeObjectURL(url);
        resolve(out);
      };
      img.src = url;
    });
  }
  that.encode = async function(){
    let [type,pixels] = $pop_args(2);

    return await new Promise((resolve, reject) => {
      const canvas = document.createElement('canvas');
      canvas.width = pixels.__dims[1];
      canvas.height = pixels.__dims[0];
      const ctx = canvas.getContext('2d');
      const imageData = new ImageData(new Uint8ClampedArray(pixels), pixels.__dims[1], pixels.__dims[0]);
      ctx.putImageData(imageData, 0, 0);
      canvas.toBlob((blob) => {
        const reader = new FileReader();
        reader.onloadend = () => {
          const arrayBuffer = reader.result;
          resolve(Array.from(new Uint8Array(arrayBuffer)));
        };
        reader.onerror = reject;
        reader.readAsArrayBuffer(blob);
      }, 'image/'+type.replace('jpg','jpeg'), 0.92);
    });
  }

}

