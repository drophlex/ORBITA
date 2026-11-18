'use strict';
const $=id=>document.getElementById(id);
const scenes=[
 ['Anillo de Mérida','Un grave espaciado sostiene dos campanas que se cruzan.',112,0,0,7,2,2],
 ['Cristal sesgado','Ataques metálicos a contratiempo en una rosa de cinco pétalos.',136,1,1,10,5,4],
 ['Deriva lunar','Un pulso lento y resonancias abiertas sobre una escala pentatónica.',74,2,2,4,3,1.5],
 ['Ácido discreto','Una trama densa de semicorcheas con modulación brillante.',148,0,0,12,9,3.5]
];
const scales=[[0,2,3,5,7,9,10],[0,2,3,5,7,8,10],[0,3,5,7,10]], roots=[38,41,45], names=['Subgrave','Prisma','Cristal'],colors=['#ceff78','#af9af1','#f1a07d'];
let den=7,phase=2,ratio=2,mask=[],mute=[false,false,false],active=false,ctx,bus,timer,cursor=0,next=0,events=[],current=-1,access,port,rev=0;
function pattern(){mask=names.map((_,v)=>Array.from({length:16},(_,s)=>((s+phase*v)*Math.min(16,Math.max(1,den+[-3,0,2][v])))%16<Math.min(16,Math.max(1,den+[-3,0,2][v]))));tracks()}
function note(v,s){const sc=scales[$('scale').selectedIndex],mode=$('shape').selectedIndex;let a=s*Math.PI/8+phase*v*Math.PI/8;let n=Math.floor((1+[Math.sin(a*(1+v)),Math.cos(a*2)*Math.sin(a),Math.sin(a+Math.sin(3*a))][mode])*sc.length*.48);return roots[$('scale').selectedIndex]+v*12+sc[n]}
function tracks(){$('tracks').replaceChildren();names.forEach((name,v)=>{let row=document.createElement('div');row.className='track'+(mute[v]?' mute':'');row.style.setProperty('--col',colors[v]);let b=document.createElement('button');b.textContent=tr(name);b.title=tr('Silenciar o activar')+' '+tr(name);b.setAttribute('aria-pressed',!mute[v]);b.onclick=()=>{mute[v]=!mute[v];tracks()};row.append(b);mask[v].forEach((on,s)=>{let cell=document.createElement('button');cell.className=(on?'on ':'')+(s===current?'current':'');cell.setAttribute('aria-label',tr(name)+' '+tr('paso')+' '+(s+1));cell.setAttribute('aria-pressed',on);cell.onclick=()=>{mask[v][s]=!mask[v][s];tracks()};row.append(cell)});$('tracks').append(row)})}
for(const [id,label,min,max,step] of [['density','Densidad',3,14,1],['phase','Fase',0,15,1],['ratio','Relación FM',.5,8,.5]]){let l=document.createElement('label');l.innerHTML=label+'<output></output><input type="range" id="'+id+'" min="'+min+'" max="'+max+'" step="'+step+'">';$('sliders').append(l);$(id).setAttribute('aria-label',label);$(id).oninput=()=>{let n=+$(id).value;l.querySelector('output').value=n;if(id==='ratio')ratio=n;else{if(id==='phase')phase=n;else den=n;pattern()}}}
scenes.forEach(x=>$('scene').add(new Option(x[0])));
function scene(){const [,desc,bpm,geo,sc,d,p,r]=scenes[$('scene').selectedIndex];$('desc').textContent=tr(desc);$('tempo').value=bpm;$('shape').selectedIndex=geo;$('scale').selectedIndex=sc;mute.fill(false);for(const [k,val] of [['density',d],['phase',p],['ratio',r]]){$(k).value=val;$(k).oninput()}}
$('scene').onchange=scene;scene();
function stepTime(){return 15/Math.min(220,Math.max(40,+$('tempo').value||112))}
$('tempo').onchange=()=>$('tempo').value=Math.round(15/stepTime());
function voice(v,key,t,d){let c=ctx.createOscillator(),m=ctx.createOscillator(),depth=ctx.createGain(),env=ctx.createGain();let f=440*2**((key-69)/12),tail=d*[1.9,2.8,3.6][v];c.frequency.value=f;m.frequency.value=f*ratio;depth.gain.setValueAtTime(f*[.45,1.7,2.7][v],t);depth.gain.exponentialRampToValueAtTime(.01,t+tail);m.connect(depth).connect(c.frequency);c.connect(env).connect(bus);env.gain.setValueAtTime(.0001,t);env.gain.exponentialRampToValueAtTime([.26,.13,.1][v],t+.008);env.gain.exponentialRampToValueAtTime(.0001,t+tail);c.start(t);m.start(t);c.stop(t+tail+.01);m.stop(t+tail+.01);c.onended=()=>{c.disconnect();m.disconnect();depth.disconnect();env.disconnect()}}
function schedule(){while(active&&next<ctx.currentTime+.085){let d=stepTime();mask.forEach((row,v)=>{if(row[cursor]&&!mute[v]){let key=note(v,cursor);if(port){let at=performance.now()+Math.max(0,next-ctx.currentTime)*1000;port.send([144+v,key,88],at);port.send([128+v,key,0],at+d*740)}else voice(v,key,next,d)}});events.push([next,cursor]);cursor=(cursor+1)%16;next+=d}}
function silence(){if(port){port.clear();for(let v=0;v<3;v++)port.send([176+v,123,0])}}
function stop(){rev++;active=false;clearInterval(timer);silence();if(bus){bus.gain.setTargetAtTime(.0001,ctx.currentTime,.007);let old=bus;setTimeout(()=>old.disconnect(),100)}events=[];current=-1;tracks();$('play').textContent='▶  '+tr('Tocar')}
$('play').onclick=async()=>{if(active){stop();return}let ticket=++rev;try{ctx??=new AudioContext();await ctx.resume();if(ticket!==rev)return;bus=ctx.createGain();bus.gain.value=.75;bus.connect(ctx.destination);active=true;cursor=0;next=ctx.currentTime+.04;schedule();timer=setInterval(schedule,20);$('play').textContent='■  '+tr('Detener')}catch(e){stop();statusMessage('No se pudo abrir el audio.',e.message)}};
document.addEventListener('visibilitychange',()=>{if(document.hidden&&active)stop()});
$('connect').onclick=async()=>{try{if(!navigator.requestMIDIAccess)throw Error('Abre la edición web en un navegador compatible con Web MIDI.');access=await navigator.requestMIDIAccess();function refresh(){if(port&&port.state==='disconnected'){stop();port=null}let sel=$('out');sel.length=1;for(let o of access.outputs.values())if(o.state==='connected')sel.add(new Option(o.name,o.id));sel.value=port?.id||'';statusMessage(sel.length>1?'Selecciona el FM1 en Salida MIDI.':'Sin salidas MIDI detectadas. Comprueba el cable USB de datos.')}refresh();access.onstatechange=refresh}catch(e){statusMessage(translations[e.message]?e.message:'No se pudo acceder al MIDI.',translations[e.message]?'':e.message)}};
$('out').onchange=()=>{if(active)stop();port=access?.outputs.get($('out').value)};
function vlq(n){let a=[n&127];while((n>>=7)>0)a.unshift((n&127)|128);return a}
$('midi').onclick=()=>{let tempo=Math.round(stepTime()*4000000),seq=[[0,[255,81,3,(tempo>>16)&255,(tempo>>8)&255,tempo&255]]];for(let s=0;s<16;s++)mask.forEach((r,v)=>{if(r[s]&&!mute[v]){let k=note(v,s);seq.push([s*120,[144+v,k,88]],[s*120+89,[128+v,k,0]])}});seq.push([1920,[255,47,0]]);seq.sort((a,b)=>a[0]-b[0]);let data=[],last=0;for(let [t,msg] of seq){data.push(...vlq(t-last),...msg);last=t}let n=data.length,bytes=[77,84,104,100,0,0,0,6,0,0,0,1,1,224,77,84,114,107,0,0,n>>8,n&255,...data];let a=document.createElement('a'),url=URL.createObjectURL(new Blob([new Uint8Array(bytes)],{type:'audio/midi'}));a.href=url;a.download='Polaris-Orbita.mid';a.click();setTimeout(()=>URL.revokeObjectURL(url),2000)};
let lastFrameKey='';
const cv=$('plot'),g=cv.getContext('2d'),tau=2*Math.PI;
function draw(){let w=cv.clientWidth,h=cv.clientHeight,pixel=Math.min(devicePixelRatio,2);let frameKey=[w,h,pixel,phase,ratio,$('shape').selectedIndex,mute.join(''),mask.map(r=>r.join('')).join('|'),active].join('/');if(!active&&frameKey===lastFrameKey){requestAnimationFrame(draw);return}lastFrameKey=frameKey;if(cv.width!==Math.round(w*pixel)||cv.height!==Math.round(h*pixel)){cv.width=Math.round(w*pixel);cv.height=Math.round(h*pixel)}g.setTransform(pixel,0,0,pixel,0,0);g.clearRect(0,0,w,h);let ix=current;while(events.length&&events[0][0]<=ctx.currentTime)current=events.shift()[1];if(ix!==current){$('pos').value=String(current+1).padStart(2,'0')+' / 16';document.querySelectorAll('.track').forEach(row=>[...row.children].slice(1).forEach((b,s)=>b.classList.toggle('current',s===current)))}else if(!active)$('pos').value='01 / 16';
let R=Math.min(w*.35,h*.33),cx=w/2,cy=h/2;
g.strokeStyle='#343b33';g.lineWidth=.7;g.beginPath();for(let x=cx%28;x<w;x+=28){g.moveTo(x,0);g.lineTo(x,h)}for(let y=cy%28;y<h;y+=28){g.moveTo(0,y);g.lineTo(w,y)}g.stroke();
for(let v=0;v<3;v++){let geom=$('shape').selectedIndex,p=phase*tau/16+v*.44;function pt(a,k){let x,y;if(geom===0){x=Math.sin(2*a+p+k);y=Math.sin(3*a+v*.3)}else if(geom===1){let r=Math.cos(5*a+p+k);x=r*Math.cos(a);y=r*Math.sin(a)}else{x=.7*Math.cos(a)+.3*Math.cos(4*a+p+k);y=.7*Math.sin(a)-.3*Math.sin(4*a+p+k)}return [cx+x*R*1.38,cy+y*R*(1-v*.065)]}
g.strokeStyle=g.fillStyle=colors[v];for(let k=0;k<7;k++){g.globalAlpha=mute[v]?.035:(k===0?.85:.16);g.beginPath();for(let j=0;j<=320;j++){let [x,y]=pt(j*tau/320,k*.032*ratio);if(j===0)g.moveTo(x,y);else g.lineTo(x,y)}g.stroke()}g.globalAlpha=mute[v]?.1:1;if(active&&current>=0){let [x,y]=pt(current*tau/16,0);g.beginPath();g.arc(x,y,8,0,tau);g.stroke()}mask[v].forEach((on,s)=>{if(!on)return;let [x,y]=pt(s*tau/16,0);g.beginPath();g.arc(x,y,current===s?6:2.5,0,tau);g.fill()})}g.globalAlpha=1;requestAnimationFrame(draw)}draw();

let statusKey=document.getElementById('msg').textContent, statusDetail='';
function statusMessage(key,detail='') {
  statusKey=key; statusDetail=detail;
  $('msg').textContent=tr(key)+(detail?' '+detail:'');
}
bindLabels(document.querySelector('main'));
bindLabels(document.querySelector('header'));
function applyLanguage() {
  document.documentElement.lang=language;
  $('language').value=language;
  for(const [node,key,original] of textBindings) node.textContent=original.replace(key,tr(key));
  $('manual').href=language==='en'?'Orbita-EN.pdf':'Orbita-ES.pdf';
  for(const [id,key] of [['plot','Órbitas de las tres voces y posiciones de sus notas'],['scene','Escena'],['density','Densidad'],['phase','Fase'],['ratio','Relación FM']]) $(id).setAttribute('aria-label',tr(key));
  $('desc').textContent=tr(scenes[$('scene').selectedIndex][1]);
  $('play').textContent=active?'■  '+tr('Detener'):'▶  '+tr('Tocar');
  statusMessage(statusKey,statusDetail);
  tracks();
}
$('language').onchange=()=>{
  language=$('language').value;
  try {localStorage.setItem('orbita-language',language)} catch {}
  applyLanguage();
};
applyLanguage();
