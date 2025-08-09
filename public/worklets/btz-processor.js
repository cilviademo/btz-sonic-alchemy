class BTZProcessor extends AudioWorkletProcessor {
  static get parameterDescriptors(){
    return [
      { name:'mix', defaultValue:1, minValue:0, maxValue:1, automationRate:'k-rate' },
      { name:'drive', defaultValue:0, minValue:0, maxValue:1, automationRate:'a-rate' },
      { name:'active', defaultValue:1, minValue:0, maxValue:1, automationRate:'k-rate' },
      { name:'clipBlend', defaultValue:.5, minValue:0, maxValue:1, automationRate:'k-rate' },
    ];
  }
  constructor(){ super(); this.clipType='soft'; this._g=1;
    this.port.onmessage=e=>{ if(e.data?.type==='clipType') this.clipType=e.data.value; };
    this.smooth=(a,b,c=.98)=>c*a+(1-c)*b;
  }
  process(inputs, outputs, parameters){
    const input=inputs[0], output=outputs[0]; if(!input||!output) return true;
    const chs=Math.min(input.length, output.length);
    for(let ch=0; ch<chs; ch++){
      const inp=input[ch], out=output[ch];
      const kMix=parameters.mix, kDrv=parameters.drive, kAct=parameters.active, kCB=parameters.clipBlend;
      for(let i=0;i<out.length;i++){
        const mix=(kMix.length>1?kMix[i]:kMix[0]);
        const drv=(kDrv.length>1?kDrv[i]:kDrv[0]);
        const act=(kAct.length>1?kAct[i]:kAct[0])>0.5;
        const cb =(kCB.length>1?kCB[i]:kCB[0]);
        this._g=this.smooth(this._g, 1+drv*9, 0.98);
        const dry=inp[i], pre=dry*this._g;
        let wet=pre;
        if(act){
          switch(this.clipType){
            case 'hard': wet=Math.max(-0.9, Math.min(0.9, pre)); break;
            case 'tube': wet=Math.tanh(1.2*pre)*0.95; break;
            case 'tape': wet=pre/(1+Math.abs(pre)*0.8); break;
            case 'digital': wet=pre*0.7; break;
            default: wet=Math.tanh(pre); // soft
          }
          wet = pre*(1-cb) + wet*cb;
        }
        out[i] = dry*(1-mix) + wet*mix;
      }
    }
    return true;
  }
}
registerProcessor('btz-processor', BTZProcessor);
