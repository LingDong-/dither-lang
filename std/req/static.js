globalThis.$req = new function(){
  var that = this;

  that._http = async function(){
    let [rq,re] = $pop_args(2);
    let headers = {};
    for (let i = 0; i < rq.headers.length; i++){
      let key = rq.headers[i].split(':')[0];
      headers[key.trim()] = rq.headers[i].slice(key.length+1).trim();
    }
    let response = await fetch(rq.url,{
      method:rq.method,
      body:rq.body.length?new Uint8Array(rq.body):undefined,
      headers,
    });
    let out_body = await response.bytes();
    out_body.__type = re.body.__type;
    re.body = out_body;
    re.status = response.status;
    for (const pair of re.headers.entries()) {
      re.headers.push(`${pair[0]}: ${pair[1]}`);
    }
    re.url = response.url;
    
  }
}

