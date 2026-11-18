const translations = {
  "INSTRUMENTOS GENERATIVOS": "GENERATIVE INSTRUMENTS",
  "PLANO DE FASE": "PHASE PLANE",
  "MEMORIA DE ESCENA": "SCENE MEMORY",
  "Geometría": "Geometry",
  "Escala": "Scale",
  "Secuenciador": "Sequencer",
  "16 PASOS / 4/4": "16 STEPS / 4/4",
  "Conexiones y descargas": "Connections and downloads",
  "Salida MIDI": "MIDI output",
  "Sólo audio web": "Browser audio only",
  "Detectar FM1 / MIDI": "Detect FM1 / MIDI",
  "Exportar clip MIDI": "Export MIDI clip",
  "Proyecto VST3 ↓": "VST3 project ↓",
  "Edición FM1 ↓": "FM1 edition ↓",
  "ÓRBITA / EDICIÓN 01": "ÓRBITA / EDITION 01",
  "Rosa polar": "Polar rose",
  "Epiciclo": "Epicycle",
  "Re dórico": "D Dorian",
  "Fa menor": "F minor",
  "La pentatónica": "A pentatonic",
  "Densidad": "Density",
  "Fase": "Phase",
  "Relación FM": "FM ratio",
  "Subgrave": "Sub bass",
  "Prisma": "Prism",
  "Cristal": "Crystal",
  "Escena": "Scene",
  "Órbitas de las tres voces y posiciones de sus notas": "Orbits of the three voices and their note positions",
  "Anillo de Mérida": "Mérida Ring",
  "Cristal sesgado": "Skewed Crystal",
  "Deriva lunar": "Lunar Drift",
  "Ácido discreto": "Discrete Acid",
  "Un grave espaciado sostiene dos campanas que se cruzan.": "A spacious bass line supports two intersecting bells.",
  "Ataques metálicos a contratiempo en una rosa de cinco pétalos.": "Offbeat metallic attacks trace a five-petal rose.",
  "Un pulso lento y resonancias abiertas sobre una escala pentatónica.": "A slow pulse and open resonances over a pentatonic scale.",
  "Una trama densa de semicorcheas con modulación brillante.": "A dense weave of sixteenth notes with bright modulation.",
  "Conecta el FM1 por USB y selecciona su salida para enviar las tres voces por los canales 1, 2 y 3. Escucha el hardware desde su propia salida.": "Connect the FM1 via USB and select its output to send the three voices on channels 1, 2 and 3. Monitor the FM1 through its audio output.",
  "Selecciona el FM1 en Salida MIDI.": "Select the FM1 under MIDI output.",
  "Sin salidas MIDI detectadas. Comprueba el cable USB de datos.": "No MIDI outputs detected. Check your USB data cable.",
  "Abre la edición web en un navegador compatible con Web MIDI.": "Open the web edition in a browser that supports Web MIDI.",
  "No se pudo abrir el audio.": "Could not start audio.",
  "No se pudo acceder al MIDI.": "Could not access MIDI.",
  "Silenciar o activar": "Mute or enable",
  "paso": "step",
  "Tocar": "Play",
  "Detener": "Stop",
  "Manual de usuario ↗": "User manual ↗",
  "Plug-in Mac (VST3 · AU) ↓": "Mac plug-in (VST3 · AU) ↓",
  "Guía de instalación": "Install guide",
  "diseño de Dark Funk Durcal · construido en Polaris Sound Lab": "Dark Funk Durcal design · built at Polaris Sound Lab · Mérida, Yucatán",
  "DONAR": "DONATE",
  "diseño de Dark Funk Durcal · creado en Polaris Sound Lab · Mérida, Yucatán": "Dark Funk Durcal design · built at Polaris Sound Lab · Mérida, Yucatán",
  "Instalar": "Install",
  "Instalar en el FM1 ▸": "Install on the FM1 ▸",
  "ÓRBITA completo en el FM1 (beta) ▸": "All of ÓRBITA on the FM1 (beta) ▸",
  "Voces para el FM1 ▸": "Voices for the FM1 ▸"
};
let language = 'en';
try { if(localStorage.getItem('orbita-language') === 'es') language = 'es'; } catch {}
function tr(key) { return language === 'en' ? translations[key] || key : key; }
const textBindings = [];
function bindLabels(root) {
  const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
  while(walker.nextNode()) {
    const node = walker.currentNode;
    const key = node.textContent.replace(/^[●\s]+|\s+$/g,'');
    if(Object.hasOwn(translations,key)) textBindings.push([node,key,node.textContent]);
  }
}
