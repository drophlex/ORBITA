/* i18n.js — English by default, Español one click away */
'use strict';
const ES = {
  'Play': 'Tocar', 'Stop': 'Detener', 'NEW ORBIT': 'NUEVA ÓRBITA', 'MUTATE': 'MUTAR', 'DONATE': 'DONAR',
  'PHASE PLANE': 'PLANO DE FASE', 'STEP': 'PASO', 'SCENES': 'ESCENAS', 'click: load · B: gravity target': 'clic: cargar · B: destino de gravedad',
  'SAVE TO SLOT': 'GUARDAR EN RANURA', 'NO TARGET': 'SIN DESTINO', 'MACROS': 'MACROS', 'drag · double-click resets': 'arrastra · doble clic restablece',
  'HARMONY': 'ARMONÍA', 'ROOT': 'TÓNICA', 'SCALE': 'ESCALA', 'GEOMETRY': 'GEOMETRÍA', 'KEYS PLAY': 'LAS TECLAS TOCAN', 'PLAY': 'TOCAR', 'TRANSPOSE': 'TRANSPONER',
  'RESET VOICE': 'RESTABLECER VOZ', 'TIME MACHINE': 'MÁQUINA DEL TIEMPO', 'every change and every evolution is kept · click to go back': 'cada cambio y cada evolución se guarda · clic para volver',
  'KEYS': 'TECLADO', 'computer keys A–K · MIDI in': 'teclas A–K de la computadora · entrada MIDI',
  'Connections, downloads and the other editions': 'Conexiones, descargas y las otras ediciones', 'MIDI OUTPUT': 'SALIDA MIDI', 'Browser audio only': 'Sólo audio del navegador',
  'Find FM-1 / MIDI': 'Detectar FM-1 / MIDI', 'Mac plug-in (VST3 · AU)': 'Plug-in Mac (VST3 · AU)', 'ÓRBITA on the FM-1 (beta)': 'ÓRBITA en el FM-1 (beta)', 'Voices for the FM-1': 'Voces para el FM-1',
  'Install guide': 'Guía de instalación', 'Manual': 'Manual', 'ÓRBITA 1 (classic)': 'ÓRBITA 1 (clásica)',
  'Connect the FM-1 by USB and pick its output: the four voices go out on channels 1 to 4.': 'Conecta el FM-1 por USB y elige su salida: las cuatro voces salen por los canales 1 a 4.',
  'FREE AND OPEN SOURCE': 'LIBRE Y GRATUITO', 'BUILT AT POLARIS SOUND LAB · MÉRIDA, YUCATÁN, MÉXICO': 'DESARROLLADO EN POLARIS SOUND LAB · MÉRIDA, YUCATÁN, MÉXICO', 'EDITION': 'EDICIÓN',
  'WELCOME': 'BIENVENIDA', "Three moves and it's music.": 'Tres movimientos y ya es música.', 'Press Play.': 'Pulsa Tocar.',
  'Four voices start orbiting: every dot on a curve is a note you hear.': 'Cuatro voces empiezan a orbitar: cada punto de una curva es una nota que escuchas.',
  'Pick a scene, or press ✦ NEW ORBIT.': 'Elige una escena o pulsa ✦ NUEVA ÓRBITA.', 'Each one is a scale, a geometry, a rhythm and a sound.': 'Cada una es una escala, una geometría, un ritmo y un sonido.',
  'Press B on another scene and turn GRAVITY.': 'Pulsa B en otra escena y gira GRAVEDAD.',
  'The music glides from one world to the other. EVOLVE lets it wander by itself; the TIME MACHINE keeps every moment.': 'La música se desliza de un mundo al otro. EVOLUCIÓN la deja vagar sola; la MÁQUINA DEL TIEMPO guarda cada momento.',
  'Play and start': 'Tocar y empezar', 'Close': 'Cerrar',
  // controls
  'DENSITY': 'DENSIDAD', 'PHASE': 'FASE', 'FM RATIO': 'RELACIÓN FM', 'GRAVITY': 'GRAVEDAD', 'EVOLVE': 'EVOLUCIÓN', 'CHAOS': 'CAOS', 'BLOOM': 'BRILLO FM',
  'SPACE': 'ESPACIO', 'ECHO': 'ECO', 'WARMTH': 'CALIDEZ', 'ECHO TIME': 'TIEMPO ECO', 'FEEDBACK': 'REPETICIONES', 'ECHO TONE': 'TONO ECO', 'SIZE': 'TAMAÑO', 'DAMP': 'AMORTIGUAR',
  'CHORUS': 'CORO', 'WIDTH': 'ANCHURA', 'SWING': 'SWING', 'HUMAN': 'HUMANO', 'TILT': 'TONO', 'MASTER': 'SALIDA', 'FX & MIX': 'EFECTOS Y MEZCLA',
  'PULSES': 'PULSOS', 'STEPS': 'PASOS', 'ROTATE': 'ROTAR', 'OCT': 'OCT', 'LEVEL': 'NIVEL', 'PAN': 'PANEO',
  'RATIO': 'RELACIÓN', 'INDEX': 'ÍNDICE', 'MOD ENV': 'ENV MOD', 'ATTACK': 'ATAQUE', 'DECAY': 'CAÍDA', 'SUSTAIN': 'SOSTÉN', 'RELEASE': 'LIBERAR', 'CUTOFF': 'CORTE',
  'RESONANCE': 'RESONANCIA', 'ECHO SEND': 'ENVÍO ECO', 'SPACE SEND': 'ENVÍO ESPACIO', 'DETUNE': 'DESAFINAR', 'GATE': 'DURACIÓN', 'CHANCE': 'PROBABILIDAD', 'ACCENT': 'ACENTO',
  // lanes and models
  'SUB': 'SUBGRAVE', 'PRISM': 'PRISMA', 'CRYSTAL': 'CRISTAL', 'HALO': 'HALO', 'GLASS': 'VIDRIO', 'DUST': 'POLVO',
  'Sub bass': 'Subgrave', 'Prism': 'Prisma', 'Crystal': 'Cristal', 'Halo': 'Halo', 'mute': 'silenciar', 'solo': 'solo',
  // scales, geometries, notes
  'Dorian': 'Dórico', 'Minor': 'Menor', 'Minor pentatonic': 'Pentatónica menor', 'Phrygian': 'Frigio', 'Lydian': 'Lidio', 'Major pentatonic': 'Pentatónica mayor',
  'Phrygian dominant': 'Frigio dominante', 'Whole tone': 'Tonos enteros', 'Harmonic minor': 'Menor armónica', 'Hirajoshi': 'Hirajoshi',
  'Lissajous': 'Lissajous', 'Polar rose': 'Rosa polar', 'Epicycle': 'Epiciclo', 'Spiral': 'Espiral',
  // messages
  'Saved in slot': 'Guardado en la ranura', 'Gravity target': 'Destino de gravedad', 'Target cleared': 'Sin destino', 'MIDI clip saved': 'Clip MIDI guardado',
  'Drag the clip onto a MIDI track': 'Arrastra el clip a una pista MIDI', 'Pick the scene for B': 'Elige la escena para B', 'Back in time': 'De vuelta en el tiempo',
  'empty': 'vacía', 'slot': 'ranura', 'evolving': 'evolucionando', 'held': 'sostenidas',
  'The keys play': 'Las teclas tocan', 'A key sets the root: the whole orbit follows it.': 'Una tecla fija la tónica: toda la órbita la sigue.',
  'Hold notes: they become the scale the orbit plays.': 'Mantén notas: se vuelven la escala que toca la órbita.',
  'No MIDI outputs found. Check the USB data cable.': 'No hay salidas MIDI. Revisa el cable USB de datos.', 'Pick the FM-1 in MIDI output.': 'Elige el FM-1 en Salida MIDI.',
  'Open the web edition in Chrome or Edge for Web MIDI.': 'Abre la edición web en Chrome o Edge para usar Web MIDI.', 'The audio could not start.': 'No se pudo iniciar el audio.',
  'Follow the DAW\'s tempo and transport': 'Seguir el tempo y el transporte del DAW', 'A new orbit: scale, geometry, rhythm and sound, all musical': 'Una órbita nueva: escala, geometría, ritmo y sonido, todo musical',
  'Change a few things, keep the feel': 'Cambia algunas cosas y conserva la sensación', 'Back one step in the time machine': 'Un paso atrás en la máquina del tiempo',
  'Drag it onto a MIDI track (plug-in) or download it (web): 4 bars, one track per voice': 'Arrástralo a una pista MIDI (plug-in) o descárgalo (web): 4 compases, una pista por voz',
  'Send the notes out of the plug-in too, to play your other instruments (channel 1)': 'Envía también las notas fuera del plug-in, para tocar tus otros instrumentos (canal 1)',
  'Paper or night': 'Papel o noche', 'Quick guide': 'Guía rápida', 'The next scene you click becomes GRAVITY\'s target': 'La siguiente escena que pulses será el destino de GRAVEDAD',
  'Drag it onto a track in your DAW': 'Arrástralo a una pista de tu DAW', 'Audio loop saved': 'Loop de audio guardado', 'From the clip': 'Del clip', 'Drop a .mid file': 'Suelta un archivo .mid',
  'Drag it onto a MIDI track (plug-in) or download it (web): 4 bars, all the voices on one channel': 'Arrástralo a una pista MIDI (plug-in) o descárgalo (web): 4 compases, todas las voces en un canal',
  'Drag the audio onto a track (plug-in) or download it (web): a 4-bar loop': 'Arrastra el audio a una pista (plug-in) o descárgalo (web): un loop de 4 compases',
  'MIDI out on': 'Salida MIDI activada', 'MIDI out off': 'Salida MIDI desactivada', 'Following the DAW': 'Siguiendo al DAW', 'Own tempo': 'Tempo propio'
};
let LANG = 'en';
function T(s) { return LANG === 'es' && ES[s] !== undefined ? ES[s] : s; }
function applyLang(root = document) {
  document.documentElement.lang = LANG;
  root.querySelectorAll('[data-t]').forEach(el => { if (el.dataset.en === undefined) el.dataset.en = el.textContent.trim(); el.textContent = T(el.dataset.en); });
  root.querySelectorAll('[data-t-title]').forEach(el => { el.title = T(el.dataset.tTitle); el.setAttribute('aria-label', el.title); });
}
