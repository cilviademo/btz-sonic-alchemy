class BTZProcessor extends AudioWorkletProcessor {
  static get parameterDescriptors(){
    return [
      { name:'mix', defaultValue:1, minValue:0, maxValue:1, automationRate:'k-rate' },
      { name:'drive', defaultValue:0, minValue:0, maxValue:1, automationRate:'a-rate' },
      { name:'active', defaultValue:1, minValue:0, maxValue:1, automationRate:'k-rate' },
      { name:'clipBlend', defaultValue:.5, minValue:0, maxValue:1, automationRate:'k-rate' },
      // SPARK
      { name:'sparkMix', defaultValue:1, minValue:0, maxValue:1, automationRate:'k-rate' },
      { name:'sparkOn', defaultValue:1, minValue:0, maxValue:1, automationRate:'k-rate' },
      { name:'ceiling', defaultValue:-0.3, minValue:-3, maxValue:0, automationRate:'k-rate' },
    ];
  }
  constructor(){ super(); this.clipType='soft'; this._g=1; this.os=4;
    this.port.onmessage=e=>{ if(e.data?.type==='clipType') this.clipType=e.data.value; if(e.data?.type==='os') this.os=Math.max(1, Math.min(16, e.data.value|0)); };
    this.smooth=(a,b,c=.98)=>c*a+(1-c)*b;
  }
  process(inputs, outputs, parameters){
    const input=inputs[0], output=outputs[0]; if(!input||!output) return true;
    const chs=Math.min(input.length, output.length);
    const ceilDb=(parameters.ceiling.length>1?parameters.ceiling[0]:parameters.ceiling[0]);
    const ceilLin=Math.pow(10, (ceilDb)/20);
    for(let ch=0; ch<chs; ch++){
      const inp=input[ch], out=output[ch];
      const kMix=parameters.mix, kDrv=parameters.drive, kAct=parameters.active, kCB=parameters.clipBlend, kSM=parameters.sparkMix, kSO=parameters.sparkOn;
      let last=0;
      for(let i=0;i<out.length;i++){
        const mix=(kMix.length>1?kMix[i]:kMix[0]);
        const drv=(kDrv.length>1?kDrv[i]:kDrv[0]);
        const act=(kAct.length>1?kAct[i]:kAct[0])>0.5;
        const cb =(kCB.length>1?kCB[i]:kCB[0]);
        const sMix=(kSM.length>1?kSM[i]:kSM[0]);
        const sOn =(kSO.length>1?kSO[i]:kSO[0])>0.5;
        this._g=this.smooth(this._g, 1+drv*9, 0.98);
        const dry=inp[i], pre=dry*this._g;
        let wet=pre;
        if(act){
          if(sOn && sMix>0 && this.os>1){
            // Oversampled clipping with ceiling
            let prev=last;
            let outOvers=pre; // fallback
            for(let k=0;k<this.os;k++){
              const t=(k+1)/this.os;
              const x=prev+(pre-prev)*t;
              let y=x;
              switch(this.clipType){
                case 'hard': y=Math.max(-ceilLin, Math.min(ceilLin, x)); break;
                case 'tube': y=Math.tanh(1.2*x)*0.95; y=Math.max(-ceilLin, Math.min(ceilLin, y)); break;
                case 'tape': y=x/(1+Math.abs(x)*0.8); y=Math.max(-ceilLin, Math.min(ceilLin, y)); break;
                case 'digital': y=Math.max(-ceilLin, Math.min(ceilLin, x*0.7)); break;
                default: y=Math.tanh(x); y=Math.max(-ceilLin, Math.min(ceilLin, y)); // soft
              }
              outOvers=y; // take last sample (ZOH downsample)
            }
            wet = pre*(1-cb) + outOvers*cb;
          } else {
            // Legacy clip path
            switch(this.clipType){
              case 'hard': wet=Math.max(-0.9, Math.min(0.9, pre)); break;
              case 'tube': wet=Math.tanh(1.2*pre)*0.95; break;
              case 'tape': wet=pre/(1+Math.abs(pre)*0.8); break;
              case 'digital': wet=pre*0.7; break;
              default: wet=Math.tanh(pre); // soft
            }
            wet = pre*(1-cb) + wet*cb;
          }
        }
        const sparkOut = dry*(1-sMix) + wet*sMix;
        out[i] = dry*(1-mix) + sparkOut*mix;
        last=pre;
      }
    }
    return true;
  }
}
registerProcessor('btz-processor', BTZProcessor);
