#!/usr/bin/env python3
# ÓRBITA 2 · user manual (English, Spanish, and both) — HTML pages rendered to PDF by pdf.mjs; shots.mjs takes the pictures
import os, math, sys
OUT = sys.argv[1] if len(sys.argv) > 1 else '/home/claude/man/orbita2'
HERE = os.path.dirname(os.path.abspath(__file__))
L = 'en'
def T(en, es): return es if L == 'es' else en
WEB = 'https://drophlex.github.io/ORBITA/'
GUIDE = WEB + 'guide/'
REPO = 'https://github.com/drophlex/ORBITA'
DON = 'https://www.paypal.com/donate/?business=drophlex' + '%40' + 'gmail.com&amp;no_recurring=0&amp;item_name=ORBITA&amp;currency_code=USD'
IG, YT = 'https://www.instagram.com/polaris.sound.lab/', 'https://www.youtube.com/@PolarisSoundLab'
def shot(n): return f'shots/{"es" if L == "es" else "en"}_{n}.png'
def img(n, cap='', w='100%'): return f'<figure style="width:{w}"><img src="{shot(n)}">' + (f'<figcaption>{cap}</figcaption>' if cap else '') + '</figure>'
def table(head, rows): return '<table><tr>' + ''.join(f'<th>{h}</th>' for h in head) + '</tr>' + ''.join('<tr>' + ''.join(f'<td>{c}</td>' for c in r) + '</tr>' for r in rows) + '</table>'
def kick(t): return f'<div class="kick">{t}</div>'
def k(t): return f'<span class="k">{t}</span>'
def tip(t): return f'<div class="tip"><b>{T("Tip", "Consejo")}</b> {t}</div>'
def note(t): return f'<div class="note">{t}</div>'
def code(t): return f'<div class="code">{t}</div>'
def callouts(n, spots, w='100%'):
    marks = ''.join(f'<span class="dot" style="left:{x}%;top:{y}%">{i}</span>' for i, (x, y) in enumerate(spots, 1))
    return f'<div class="callout" style="width:{w}"><img src="{shot(n)}">{marks}</div>'

def orbit_svg(size=520):
    cx = cy = size / 2; R = size * .34; out = [f'<svg viewBox="0 0 {size} {size}" xmlns="http://www.w3.org/2000/svg">']
    cols = ['#ceff78', '#af9af1', '#f1a07d', '#7fd6ee']
    for v in range(4):
        for kk in range(6):
            pts = []
            for j in range(361):
                a = j * 2 * math.pi / 360; p = 1 * 2 * math.pi / 16 + v * .44 + kk * .05
                x = .7 * math.cos(a) + .3 * math.cos(4 * a + p); y = .7 * math.sin(a) - .3 * math.sin(4 * a + p)
                pts.append(f'{cx + x * R * 1.38:.1f},{cy + y * R * (1 - v * .065):.1f}')
            out.append(f'<polyline points="{" ".join(pts)}" fill="none" stroke="{cols[v]}" stroke-opacity="{.85 if kk == 0 else .13}" stroke-width="{1.4 if kk == 0 else 1}"/>')
        steps = [16, 12, 10, 7][v]; pul = [4, 7, 9, 4][v]
        for s in range(steps):
            if (s * pul) % steps < pul:
                a = s * 2 * math.pi / steps; p = 2 * math.pi / 16 + v * .44
                x = .7 * math.cos(a) + .3 * math.cos(4 * a + p); y = .7 * math.sin(a) - .3 * math.sin(4 * a + p)
                out.append(f'<circle cx="{cx + x * R * 1.38:.1f}" cy="{cy + y * R * (1 - v * .065):.1f}" r="4" fill="{cols[v]}"/>')
    out.append('</svg>'); return ''.join(out)

def euclid_svg(pulses, steps=16, col='#ceff78'):
    s = 150; c = s / 2; R = 58; out = [f'<svg viewBox="0 0 {s} {s}" xmlns="http://www.w3.org/2000/svg"><circle cx="{c}" cy="{c}" r="{R}" fill="none" stroke="#b4b7aa"/>']
    for i in range(steps):
        a = i * 2 * math.pi / steps - math.pi / 2; hit = (i * pulses) % steps < pulses
        out.append(f'<circle cx="{c + R * math.cos(a):.1f}" cy="{c + R * math.sin(a):.1f}" r="{6.5 if hit else 3}" fill="{col if hit else "#c9cbbe"}" stroke="#1a1c19" stroke-opacity="{.6 if hit else 0}"/>')
    out.append(f'<text x="{c}" y="{c + 5}" text-anchor="middle" font-family="IBM Plex Mono,monospace" font-size="15" font-weight="700">{pulses}/{steps}</text></svg>'); return ''.join(out)

PAGES = []
def page(title, body, cls=''): PAGES.append((cls, body, title))

def build():
    global PAGES; PAGES = []
    # 1 cover
    page('', f'''<div class="cover"><div class="cart">{orbit_svg()}</div>
<div class="ctitle">Órbita 2</div><div class="csub">{T('a generative FM instrument · four voices, euclidean rhythms, orbits you can hear', 'un instrumento FM generativo · cuatro voces, ritmos euclidianos, órbitas que se escuchan')}</div>
<div class="cmeta">{T('USER MANUAL', 'MANUAL DE USUARIO')} · WEB · VST3 · AU · APP</div>
<div class="clang">{'ENGLISH · ESPAÑOL' if L == 'both' else T('ENGLISH EDITION', 'EDICIÓN EN ESPAÑOL')}</div>
<div class="cfoot">Dark Funk Durcal {T('design', 'diseño')} · {T('built at', 'desarrollado en')} <b>POLARIS SOUND LAB</b> · Mérida, Yucatán, México · <a href="{DON}">♥ {T('DONATE', 'DONAR')}</a><br>
<a href="{IG}">Instagram @polaris.sound.lab</a> · <a href="{YT}">YouTube @PolarisSoundLab</a> · <a href="{WEB}">drophlex.github.io/ORBITA</a></div></div>''', 'coverp')
    # 2 welcome + what's new
    toc = [T('Welcome and what is new', 'Bienvenida y novedades'), T('Get it', 'Consíguelo'), T('The screen, piece by piece', 'La pantalla, pieza por pieza'), T('Playing: scenes, density, phase', 'Tocar: escenas, densidad, fase'),
           T('GRAVITY', 'GRAVEDAD'), T('EVOLVE, CHAOS and the time machine', 'EVOLUCIÓN, CAOS y la máquina del tiempo'), T('The lanes and polymeter', 'Los carriles y la polimetría'), T('Sound: the six voice models', 'Sonido: los seis modelos de voz'),
           T('Harmony and keys', 'Armonía y teclas'), T('In your DAW', 'En tu DAW'), T('Effects, mix and recipes', 'Efectos, mezcla y recetas'), T('Answers, credits and licence', 'Respuestas, créditos y licencia')]
    page(T('Welcome', 'Bienvenida'), f'''{kick(T('Welcome', 'Bienvenida'))}<h1>{T('Music drawn by <i>orbits</i>.', 'Música dibujada por <i>órbitas</i>.')}</h1>
<p class="lead">{T('ÓRBITA 2 is a generative instrument. Four FM voices play euclidean rhythms over a scale, and the screen draws each voice as an orbit: every bright dot on a curve is a note you hear, and the ring is where the voice is right now. You don\'t write notes one by one: you shape the orbit, and it plays.', 'ÓRBITA 2 es un instrumento generativo. Cuatro voces FM tocan ritmos euclidianos sobre una escala, y la pantalla dibuja cada voz como una órbita: cada punto brillante de una curva es una nota que escuchas, y el anillo es donde está la voz ahora. No escribes notas una por una: das forma a la órbita y ella toca.')}</p>
{img('full', T('ÓRBITA 2 in the browser. The plug-in shows exactly the same page.', 'ÓRBITA 2 en el navegador. El plug-in muestra exactamente la misma página.'), '66%')}
<div class="two"><div><h2>{T('New in ÓRBITA 2', 'Novedades de ÓRBITA 2')}</h2><ul>
<li>{T('<b>One engine, one interface.</b> The web and the plug-in run the same code: they sound and look the same.', '<b>Un motor, una interfaz.</b> La web y el plug-in ejecutan el mismo código: suenan y se ven igual.')}</li>
<li>{T('<b>GRAVITY</b>: one knob glides the whole sound from one scene to another.', '<b>GRAVEDAD</b>: una perilla desliza todo el sonido de una escena a otra.')}</li>
<li>{T('<b>EVOLVE</b> and the <b>TIME MACHINE</b>: the orbit wanders by itself, and every moment is kept.', '<b>EVOLUCIÓN</b> y la <b>MÁQUINA DEL TIEMPO</b>: la órbita vaga sola y cada momento se guarda.')}</li>
<li>{T('A fourth voice (HALO), six voice models, polymeter, an echo, a deep reverb, chorus and warmth.', 'Una cuarta voz (HALO), seis modelos de voz, polimetría, eco, reverberación profunda, coro y calidez.')}</li>
<li>{T('<b>HARMONY keys</b>, <b>MIDI OUT</b> and dragging a clip onto your tracks.', '<b>Teclas de ARMONÍA</b>, <b>MIDI OUT</b> y arrastrar un clip a tus pistas.')}</li></ul>
{note(T('The four classic scenes play ÓRBITA 1\'s notes and rhythms exactly (all 82,944 cases are checked by the tests), so the FM-1 firmware and your old clips still match.', 'Las cuatro escenas clásicas tocan exactamente las notas y ritmos de ÓRBITA 1 (las 82,944 combinaciones se comprueban en las pruebas), así que el firmware del FM-1 y tus clips anteriores siguen coincidiendo.'))}</div>
<div><h2>{T('Contents', 'Contenido')}</h2><ol class="toc" style="columns:1">{''.join(f'<li>{t}</li>' for t in toc)}</ol>
<div class="card"><h3>{T('Free, and it stays free', 'Gratis, y seguirá gratis')}</h3><p>{T('Everything is open source on GitHub. If ÓRBITA is useful to you, a donation keeps it growing.', 'Todo es código abierto en GitHub. Si ÓRBITA te sirve, un donativo lo mantiene creciendo.')}<br><a class="don" href="{DON}">♥ {T('DONATE (PayPal)', 'DONAR (PayPal)')}</a></p></div></div></div>''')
    # 3 get it
    page(T('Get it', 'Consíguelo'), f'''{kick(T('Get it', 'Consíguelo'))}<h1>{T('Three ways to <i>play</i>.', 'Tres maneras de <i>tocar</i>.')}</h1>
{table([T('Where', 'Dónde'), T('How', 'Cómo')], [
  ('🌐 Web', T(f'Open <a href="{WEB}">drophlex.github.io/ORBITA</a> on a computer or a phone and press ▶ Play. Nothing to install. Chrome or Edge also send the notes to MIDI gear (the FM-1, other synths).', f'Abre <a href="{WEB}">drophlex.github.io/ORBITA</a> en una computadora o un teléfono y pulsa ▶ Tocar. No hay nada que instalar. Chrome o Edge además envían las notas a equipos MIDI (el FM-1, otros sintes).')),
  ('🎛 Mac · VST3 · AU · App', T('Open Terminal (⌘ Space, type Terminal) and paste the line below. It downloads the latest release, installs the VST3, the AU and the app, and clears the download quarantine.', 'Abre Terminal (⌘ Espacio, escribe Terminal) y pega la línea de abajo. Descarga la versión más reciente, instala el VST3, el AU y la app, y quita la cuarentena de la descarga.')),
  ('🪐 M-VAVE FM-1', T(f'The ÓRBITA firmware (beta) and the 32 voices are installed from the browser: <a href="{WEB}firmware/">/firmware</a> and <a href="{WEB}fm1/">/fm1</a>. They are ÓRBITA 1\'s, explained in the ÓRBITA 1 manual (Orbita-1-EN.pdf).', f'El firmware ÓRBITA (beta) y las 32 voces se instalan desde el navegador: <a href="{WEB}firmware/">/firmware</a> y <a href="{WEB}fm1/">/fm1</a>. Son los de ÓRBITA 1 y se explican en el manual de ÓRBITA 1 (Orbita-1-ES.pdf).'))])}
{code('curl -fsSL https://drophlex.github.io/ORBITA/install.sh | bash')}
<h2>{T('Ableton Live (and other DAWs)', 'Ableton Live (y otros DAW)')}</h2>
<ol class="steps"><li>{T('Live → Settings → Plug-ins → turn on <b>Use VST3 Plug-in System Folders</b> (and Audio Units if you prefer the AU) → <b>Rescan</b>.', 'Live → Ajustes → Plug-ins → activa <b>Usar carpetas de sistema VST3</b> (y Audio Units si prefieres el AU) → <b>Volver a escanear</b>.')}</li>
<li>{T('Drag <b>Polaris Orbita</b> (Plug-ins → Polaris Sound Lab) onto a MIDI track.', 'Arrastra <b>Polaris Orbita</b> (Plug-ins → Polaris Sound Lab) a una pista MIDI.')}</li>
<li>{T('Press play in Live: with <b>SYNC</b> on (the default) ÓRBITA follows Live\'s tempo and starts on the beat. With Live stopped, ÓRBITA\'s own ▶ plays at its own tempo.', 'Pulsa play en Live: con <b>SYNC</b> activado (lo normal) ÓRBITA sigue el tempo de Live y arranca a tiempo. Con Live detenido, el ▶ de ÓRBITA toca a su propio tempo.')}</li></ol>
{tip(T(f'ÓRBITA 2 replaces ÓRBITA 1 in your plug-in list (same name). Old Live sets open with ÓRBITA 2 starting from its first scene. A step-by-step guide with pictures: <a href="{GUIDE}">drophlex.github.io/ORBITA/guide</a>.', f'ÓRBITA 2 reemplaza a ÓRBITA 1 en tu lista de plug-ins (mismo nombre). Los sets anteriores se abren con ÓRBITA 2 empezando en su primera escena. Guía paso a paso con imágenes: <a href="{GUIDE}">drophlex.github.io/ORBITA/guide</a>.'))}
<div class="two"><div>{img('coach', T('The first time, three moves get you playing.', 'La primera vez, tres movimientos te ponen a tocar.'), '86%')}</div><div>{img('phone', T('On a phone', 'En un teléfono'), '40%')}</div></div>''')
    # 4 the screen
    page(T('The screen', 'La pantalla'), f'''{kick(T('The screen, piece by piece', 'La pantalla, pieza por pieza'))}<h1>{T('Everything on <i>one page</i>.', 'Todo en <i>una página</i>.')}</h1>
{callouts('full', [(23, 3), (37, 3), (44, 3), (75, 3), (6, 11), (22, 60), (80, 18), (80, 41), (80, 64), (40, 82), (30, 97)], '92%')}
{table(['#', T('Part', 'Parte'), T('What it does', 'Qué hace')], [
  ('1', '▶ ' + T('Play · BPM', 'Tocar · BPM'), T('Start and stop (or the space bar). The tempo. In the plug-in, SYNC follows the DAW.', 'Arranca y detiene (o la barra espaciadora). El tempo. En el plug-in, SYNC sigue al DAW.')),
  ('2', '✦ ' + T('NEW ORBIT', 'NUEVA ÓRBITA'), T('A new orbit: scale, geometry, rhythm and sound, chosen to be musical.', 'Una órbita nueva: escala, geometría, ritmo y sonido, elegidos para ser musicales.')),
  ('3', '≈ ' + T('MUTATE', 'MUTAR') + ' · ↶', T('Change two or three things and keep the feel · go back one step.', 'Cambia dos o tres cosas y conserva la sensación · un paso atrás.')),
  ('4', '⇣ MIDI · ◐ · ? · EN/ES · ♥', T('A 4-bar clip · paper or night · quick guide · language · donate.', 'Un clip de 4 compases · papel o noche · guía rápida · idioma · donar.')),
  ('5', T('Phase plane', 'Plano de fase'), T('The four orbits. Click a dot to add or remove that note. The chips below mute a voice.', 'Las cuatro órbitas. Haz clic en un punto para poner o quitar esa nota. Las fichas de abajo silencian una voz.')),
  ('6', T('The core', 'El núcleo'), T('The sound itself, drawn as an XY figure (left against right).', 'El sonido mismo, dibujado como figura XY (izquierda contra derecha).')),
  ('7', T('Scenes', 'Escenas'), T('Eight scenes and four slots of your own. B picks GRAVITY\'s target.', 'Ocho escenas y cuatro ranuras propias. B elige el destino de GRAVEDAD.')),
  ('8', T('Macros', 'Macros'), T('The ten big moves; FX & MIX opens eleven more.', 'Los diez movimientos grandes; EFECTOS Y MEZCLA abre once más.')),
  ('9', T('Harmony', 'Armonía'), T('Root, scale, geometry, and what your keys do.', 'Tónica, escala, geometría y lo que hacen tus teclas.')),
  ('10', T('Lanes', 'Carriles'), T('One row per voice: its steps, pulses, length, rotation, octave, level. Its name opens its sound.', 'Una fila por voz: sus pasos, pulsos, longitud, rotación, octava, nivel. Su nombre abre su sonido.')),
  ('11', T('Time machine · keys', 'Máquina del tiempo · teclado'), T('Every moment you can go back to · two octaves to play (and the computer keys A–K).', 'Cada momento al que puedes volver · dos octavas para tocar (y las teclas A–K).'))])}
{tip(T('Every knob: drag up or down (Shift for fine), use the mouse wheel or the arrow keys; double-click returns it to its default.', 'Cada perilla: arrastra hacia arriba o abajo (Shift para afinar), usa la rueda o las flechas; doble clic la regresa a su valor inicial.'))}''')
    # 5 playing
    page(T('Playing', 'Tocar'), f'''{kick(T('Playing: scenes, density, phase', 'Tocar: escenas, densidad, fase'))}<h1>{T('Shape the orbit, <i>not the notes</i>.', 'Dale forma a la órbita, <i>no a las notas</i>.')}</h1>
<div class="two"><div>{img('scenes', T('Click a scene to load it. Right-click (or B, then click) to make it GRAVITY\'s target.', 'Clic en una escena para cargarla. Clic derecho (o B y luego clic) para hacerla destino de GRAVEDAD.'))}</div>
<div>{table([T('Scene', 'Escena'), T('The idea', 'La idea')], [
 (T('Mérida Ring', 'Anillo de Mérida'), T('A spaced sub bass and two bells that cross (ÓRBITA 1).', 'Un grave espaciado y dos campanas que se cruzan (ÓRBITA 1).')),
 (T('Skewed Crystal', 'Cristal sesgado'), T('Metallic off-beats in a five-petal rose (ÓRBITA 1).', 'Contratiempos metálicos en una rosa de cinco pétalos (ÓRBITA 1).')),
 (T('Lunar Drift', 'Deriva lunar'), T('A slow, open pentatonic pulse (ÓRBITA 1).', 'Un pulso lento y abierto, pentatónico (ÓRBITA 1).')),
 (T('Discrete Acid', 'Ácido discreto'), T('Dense sixteenths, bright modulation (ÓRBITA 1).', 'Semicorcheas densas, modulación brillante (ÓRBITA 1).')),
 ('Cenote', T('Glass drops in deep water; a spiral that keeps evolving.', 'Gotas de vidrio en agua profunda; una espiral que sigue evolucionando.')),
 (T('Nebula Polymeter', 'Nebulosa polimétrica'), T('Cycles of 16, 12, 10 and 7 steps that drift and meet again.', 'Ciclos de 16, 12, 10 y 7 pasos que se desfasan y se reencuentran.')),
 (T('Hijaz Ritual', 'Ritual Hijaz'), T('Phrygian dominant, dust and a turning rose.', 'Frigio dominante, polvo y una rosa que gira.')),
 (T('Deep Orbit', 'Órbita profunda'), T('Sub-heavy techno at 126 BPM with eighth-note echoes.', 'Techno con mucho grave a 126 BPM, ecos en corcheas.'))])}</div></div>
<h2>{T('The three moves that matter', 'Los tres movimientos que importan')}</h2>
<div class="grid3"><div class="card"><div class="ring">{euclid_svg(4)}</div><h3>{T('DENSITY', 'DENSIDAD')}</h3><p>{T('How many notes fall in each cycle, spread as evenly as possible (a euclidean rhythm). Each voice adds its own PULSES to it: the sub plays fewer, the crystal more.', 'Cuántas notas caen en cada ciclo, repartidas lo más parejo posible (un ritmo euclidiano). Cada voz le suma sus PULSOS: el grave toca menos, el cristal más.')}</p></div>
<div class="card"><div class="ring">{euclid_svg(7, 16, '#af9af1')}</div><h3>{T('PHASE', 'FASE')}</h3><p>{T('Where the notes fall, and which notes they are. Each voice is shifted by PHASE × its number, so the four voices answer each other instead of hitting together.', 'Dónde caen las notas, y qué notas son. Cada voz se desplaza FASE × su número, así las cuatro voces se responden en lugar de sonar juntas.')}</p></div>
<div class="card"><div class="ring">{euclid_svg(9, 16, '#f1a07d')}</div><h3>{T('FM RATIO', 'RELACIÓN FM')}</h3><p>{T('The modulator\'s pitch against the note: whole numbers (1, 2, 3) are warm and in tune, halves are hollow, odd values like 3.5 or 7 are bells and metal.', 'La altura del modulador contra la nota: los enteros (1, 2, 3) son cálidos y afinados, las mitades son huecas, valores como 3.5 o 7 son campanas y metal.')}</p></div></div>
{tip(T('Click a dot on the screen or a cell in a lane to add or remove a note; the lane shows ✎ while it has your own steps. Moving DENSITY, PHASE, PULSES, STEPS or ROTATE draws the rhythm again.', 'Haz clic en un punto de la pantalla o en una celda de un carril para poner o quitar una nota; el carril muestra ✎ mientras tiene tus pasos. Mover DENSIDAD, FASE, PULSOS, PASOS o ROTAR dibuja el ritmo de nuevo.'))}''')
    # 6 gravity
    page(T('GRAVITY', 'GRAVEDAD'), f'''{kick('GRAVITY · ' + T('the morph', 'la transformación'))}<h1>{T('From one world <i>to another</i>.', 'De un mundo <i>a otro</i>.')}</h1>
<p class="lead">{T('GRAVITY glides everything from the scene you are playing (A) to a second one (B): the tempo, the FM ratio, every level, filter, envelope and send move smoothly; DENSITY, PHASE and the pulses step; the scale, the geometry and the voice models switch half way. One knob, a whole transition.', 'GRAVEDAD desliza todo de la escena que tocas (A) a una segunda (B): el tempo, la relación FM, cada nivel, filtro, envolvente y envío se mueven suavemente; DENSIDAD, FASE y los pulsos avanzan por pasos; la escala, la geometría y los modelos de voz cambian a la mitad. Una perilla, toda una transición.')}</p>
{img('gravity', T('GRAVITY at 35% towards Cenote: the target\'s orbits are drawn dashed, and the bar shows where you are.', 'GRAVEDAD al 35% hacia Cenote: las órbitas del destino se dibujan punteadas y la barra muestra dónde estás.'))}
<ol class="steps"><li>{T('Play a scene (A).', 'Toca una escena (A).')}</li><li>{T('Press <b>B</b> and click another scene, or right-click it. It shows a B and stripes.', 'Pulsa <b>B</b> y haz clic en otra escena, o clic derecho sobre ella. Muestra una B y rayas.')}</li>
<li>{T('Turn <b>GRAVITY</b>. At 0% you hear A, at 100% you hear B. <b>NO TARGET</b> lets go of B.', 'Gira <b>GRAVEDAD</b>. Al 0% oyes A, al 100% oyes B. <b>SIN DESTINO</b> suelta B.')}</li></ol>
{tip(T('In a DAW, automate <b>Gravity</b> over 8 or 16 bars for a build or a breakdown. Your own slots (★) work as A or B too, so you can morph between two versions of the same idea.', 'En un DAW, automatiza <b>Gravity</b> durante 8 o 16 compases para una subida o un corte. Tus ranuras (★) también sirven como A o B, así puedes transformar entre dos versiones de la misma idea.'))}''')
    # 7 evolve + time machine
    page(T('EVOLVE and the time machine', 'EVOLUCIÓN y la máquina del tiempo'), f'''{kick(T('EVOLVE, CHAOS and the time machine', 'EVOLUCIÓN, CAOS y la máquina del tiempo'))}<h1>{T('Let it wander. <i>Keep the best</i>.', 'Déjala vagar. <i>Quédate con lo mejor</i>.')}</h1>
<div class="two"><div><h2>EVOLVE</h2><p>{T('Once a bar, with the chance EVOLVE sets, the orbit takes a small step: PHASE moves one place, DENSITY one note, or the FM ratio drifts a little. It\'s a random walk on a short leash: it never goes more than four phases, two notes or half a ratio from the scene, so it stays recognisable. The screen shows ✦ while it evolves.', 'Una vez por compás, con la probabilidad que fija EVOLUCIÓN, la órbita da un pequeño paso: la FASE se mueve un lugar, la DENSIDAD una nota o la relación FM se desplaza un poco. Es una caminata al azar con correa corta: nunca se aleja más de cuatro fases, dos notas o media relación de la escena, así sigue siendo reconocible. La pantalla muestra ✦ mientras evoluciona.')}</p>
<h2>CHAOS · HUMAN</h2><p>{T('CHAOS adds ghost notes, drops a few and, above 30%, throws some notes an octave away. HUMAN loosens the velocities. Both leave the orbit\'s shape alone.', 'CAOS añade notas fantasma, quita algunas y, arriba del 30%, lanza algunas notas una octava. HUMANO suelta las velocidades. Ninguno cambia la forma de la órbita.')}</p></div>
<div><h2>{T('The time machine', 'La máquina del tiempo')}</h2><p>{T('Every change you make (after a short pause), every scene, every NEW ORBIT and MUTATE, and every step EVOLVE takes is kept as a small picture with its time. Click one to go back to it exactly; the last 40 are kept. ↶ steps back one.', 'Cada cambio que haces (tras una pausa corta), cada escena, cada NUEVA ÓRBITA y MUTAR, y cada paso de EVOLUCIÓN se guarda como una pequeña imagen con su hora. Haz clic en una para volver exactamente a ella; se guardan las últimas 40. ↶ retrocede una.')}</p>
{note(T('Going back turns EVOLVE off, so the moment you came back for stays. Turn it up again to let it wander from there. ★ SAVE TO SLOT keeps a moment for good (in the browser, or in the DAW project).', 'Volver apaga EVOLUCIÓN, para que el momento al que volviste se quede. Súbela otra vez para dejarla vagar desde ahí. ★ GUARDAR EN RANURA guarda un momento para siempre (en el navegador o en el proyecto del DAW).'))}</div></div>
{img('bottom', T('The time machine (✦ marks the moments EVOLVE made) and the keys.', 'La máquina del tiempo (✦ marca los momentos que hizo EVOLUCIÓN) y el teclado.'))}
{tip(T('A generative jam: EVOLVE at 40%, record the audio (or MIDI OUT) for ten minutes, then come back through the time machine to the moments you liked and drag them out as clips.', 'Una improvisación generativa: EVOLUCIÓN al 40%, graba el audio (o MIDI OUT) diez minutos, luego vuelve con la máquina del tiempo a los momentos que te gustaron y arrástralos como clips.'))}''')
    # 8 lanes + polymeter
    page(T('Lanes and polymeter', 'Carriles y polimetría'), f'''{kick(T('The lanes and polymeter', 'Los carriles y la polimetría'))}<h1>{T('Four voices, <i>four cycles</i>.', 'Cuatro voces, <i>cuatro ciclos</i>.')}</h1>
{img('lanes', T('Nebula Polymeter: Prism has 12 steps, Crystal 10, Halo 7.', 'Nebulosa polimétrica: Prisma tiene 12 pasos, Cristal 10, Halo 7.'))}
{table([T('Control', 'Control'), T('What it does', 'Qué hace')], [
  ('M · S', T('Mute the voice · solo it (S again brings the others back).', 'Silencia la voz · tócala sola (S otra vez regresa las demás).')),
  (T('Name', 'Nombre'), T('Opens the voice\'s sound (next page). Under it, its model, and ✎ when it has your own steps.', 'Abre el sonido de la voz (página siguiente). Debajo, su modelo, y ✎ cuando tiene tus pasos.')),
  (T('Cells', 'Celdas'), T('Its steps. Click to add or remove a note. A dot marks each beat.', 'Sus pasos. Clic para poner o quitar una nota. Un punto marca cada tiempo.')),
  (T('PULSES', 'PULSOS'), T('Notes per cycle: DENSITY plus this voice\'s offset (the number shown is the total).', 'Notas por ciclo: DENSIDAD más el ajuste de esta voz (el número que ves es el total).')),
  (T('STEPS', 'PASOS'), T('The length of its cycle, 2 to 16. Different lengths make a polymeter.', 'La longitud de su ciclo, de 2 a 16. Longitudes distintas hacen una polimetría.')),
  (T('ROTATE', 'ROTAR'), T('Turns its rhythm around the circle, on top of PHASE.', 'Gira su ritmo alrededor del círculo, además de la FASE.')),
  ('OCT · ' + T('LEVEL', 'NIVEL'), T('Its octave (±2) · its volume.', 'Su octava (±2) · su volumen.'))])}
<h2>{T('Why polymeter feels alive', 'Por qué la polimetría se siente viva')}</h2>
<p>{T('With 16, 12, 10 and 7 steps, the voices line up again only after 1,680 sixteenth notes: at 120 BPM that is three and a half minutes of patterns that never quite repeat, from four simple rhythms. Short cycles (5, 7) against 16 make syncopation; 12 against 16 feels like a triplet swing.', 'Con 16, 12, 10 y 7 pasos, las voces vuelven a alinearse sólo después de 1,680 semicorcheas: a 120 BPM son tres minutos y medio de patrones que nunca se repiten del todo, a partir de cuatro ritmos sencillos. Ciclos cortos (5, 7) contra 16 hacen síncopa; 12 contra 16 se siente como un swing de tresillos.')}</p>''')
    # 9 sound
    page(T('Sound', 'Sonido'), f'''{kick(T('Sound: the six voice models', 'Sonido: los seis modelos de voz'))}<h1>{T('FM that <i>breathes</i>.', 'FM que <i>respira</i>.')}</h1>
{img('drawer', T('A voice\'s sound: click its name in its lane. RESET VOICE brings its default back.', 'El sonido de una voz: clic en su nombre en su carril. RESTABLECER VOZ regresa su valor inicial.'))}
<div class="two"><div>{table([T('Model', 'Modelo'), T('Sound', 'Sonido')], [
  (T('SUB', 'SUBGRAVE'), T('A soft FM bass with a sine an octave below.', 'Un bajo FM suave con una senoidal una octava abajo.')),
  (T('PRISM', 'PRISMA'), T('A clear bell with a high sparkle.', 'Una campana clara con un brillo agudo.')),
  (T('CRYSTAL', 'CRISTAL'), T('An inharmonic, glassy bell.', 'Una campana inarmónica, de vidrio.')),
  ('HALO', T('A slow, detuned pad that holds the harmony.', 'Un pad lento y desafinado que sostiene la armonía.')),
  (T('GLASS', 'VIDRIO'), T('Bright and metallic, like struck glass.', 'Brillante y metálico, como vidrio golpeado.')),
  (T('DUST', 'POLVO'), T('A breathy FM tone with noise in its attack.', 'Un tono FM con aire, con ruido en su ataque.'))])}</div>
<div>{table([T('Knob', 'Perilla'), T('What it does', 'Qué hace')], [
  (T('RATIO · INDEX', 'RELACIÓN · ÍNDICE'), T('This voice\'s FM ratio (× the global one) · how much modulation.', 'La relación FM de esta voz (× la global) · cuánta modulación.')),
  (T('FEEDBACK · MOD ENV', 'REPETICIONES · ENV MOD'), T('Grit in the modulator · how much the brightness falls with the note.', 'Aspereza en el modulador · cuánto cae el brillo con la nota.')),
  (T('ATTACK … RELEASE', 'ATAQUE … LIBERAR'), T('The volume envelope.', 'La envolvente de volumen.')),
  (T('GATE', 'DURACIÓN'), T('How long each note is held, in steps.', 'Cuánto dura cada nota, en pasos.')),
  (T('CUTOFF · RESONANCE', 'CORTE · RESONANCIA'), T('A low-pass filter on the voice.', 'Un filtro pasa bajos en la voz.')),
  (T('DETUNE', 'DESAFINAR'), T('Two copies, left and right, slightly apart: width.', 'Dos copias, izquierda y derecha, un poco separadas: amplitud.')),
  (T('ECHO · SPACE SEND', 'ENVÍO ECO · ESPACIO'), T('How much goes to the echo and the reverb.', 'Cuánto va al eco y a la reverberación.')),
  (T('CHANCE · ACCENT', 'PROBABILIDAD · ACENTO'), T('How likely each note plays · how much the orbit\'s shape sets its velocity.', 'Qué tan probable es que suene cada nota · cuánto marca la forma de la órbita su velocidad.'))])}</div></div>
{note(T('Gain staging is built in: the voices are summed with headroom, a clean soft-knee glue keeps the peaks round and a safety limiter holds −1 dBFS. WARMTH adds tape-like drive gradually; at 0% there is none.', 'La ganancia ya viene cuidada: las voces se suman con margen, un pegamento suave y limpio redondea los picos y un limitador de seguridad mantiene −1 dBFS. CALIDEZ añade saturación tipo cinta poco a poco; al 0% no hay ninguna.'))}''')
    # 10 harmony & keys
    scales = [('Dorian', 'Dórico', '0 2 3 5 7 9 10'), ('Minor', 'Menor', '0 2 3 5 7 8 10'), ('Minor pentatonic', 'Pentatónica menor', '0 3 5 7 10'), ('Phrygian', 'Frigio', '0 1 3 5 7 8 10'), ('Lydian', 'Lidio', '0 2 4 6 7 9 11'),
              ('Major pentatonic', 'Pentatónica mayor', '0 2 4 7 9'), ('Phrygian dominant', 'Frigio dominante', '0 1 4 5 7 8 10'), ('Whole tone', 'Tonos enteros', '0 2 4 6 8 10'), ('Harmonic minor', 'Menor armónica', '0 2 3 5 7 8 11'), ('Hirajoshi', 'Hirajoshi', '0 2 3 7 8')]
    page(T('Harmony and keys', 'Armonía y teclas'), f'''{kick(T('Harmony and keys', 'Armonía y teclas'))}<h1>{T('Play it like an <i>instrument</i>.', 'Tócala como un <i>instrumento</i>.')}</h1>
<div class="two"><div>{img('harmony')}{table([T('Scale', 'Escala'), T('Semitones', 'Semitonos')], [(T(a, b), c) for a, b, c in scales])}</div>
<div><h2>{T('What your keys do', 'Lo que hacen tus teclas')}</h2>
<p>{T('The on-screen keys, the computer\'s keys (A W S E D F T G Y H U J K…, Z and X change octave) and, in the plug-in, your MIDI keyboard:', 'Las teclas en pantalla, las de la computadora (A W S E D F T G Y H U J K…, Z y X cambian de octava) y, en el plug-in, tu teclado MIDI:')}</p>
{table([T('Mode', 'Modo'), T('What happens', 'Qué pasa')], [
  (T('PLAY', 'TOCAR'), T('You play the voice chosen in KEYS PLAY (Halo by default) like a synth, over the orbit.', 'Tocas la voz elegida en LAS TECLAS TOCAN (Halo por defecto) como un sinte, sobre la órbita.')),
  (T('TRANSPOSE', 'TRANSPONER'), T('The key you press becomes the root: the whole orbit moves to it. Great for following a bass line or a chord change.', 'La tecla que pulsas se vuelve la tónica: toda la órbita se mueve a ella. Ideal para seguir una línea de bajo o un cambio de acorde.')),
  (T('HARMONY', 'ARMONÍA'), T('Hold notes: while you hold them they <b>are</b> the scale. Hold a chord and the four voices play only its notes, in the orbit\'s rhythm. Let go and the scale comes back.', 'Mantén notas: mientras las sostienes <b>son</b> la escala. Mantén un acorde y las cuatro voces tocan sólo sus notas, al ritmo de la órbita. Suelta y la escala regresa.'))])}
<h2>{T('Geometry', 'Geometría')}</h2><p>{T('The geometry is the curve each voice follows. It sets the shape on the screen and, with PHASE, which degree of the scale each step plays: Lissajous climbs and falls in waves, the polar rose returns to the root often, the epicycle circles slowly, the spiral (new) wanders the most.', 'La geometría es la curva que sigue cada voz. Fija la forma en la pantalla y, con la FASE, qué grado de la escala toca cada paso: Lissajous sube y baja en olas, la rosa polar regresa seguido a la tónica, el epiciclo gira despacio, la espiral (nueva) es la que más vaga.')}</p>
<div class="grid3 shots">{img('geo1', T('Polar rose', 'Rosa polar'))}{img('geo2', T('Spiral', 'Espiral'))}{img('geo3', T('Hijaz Ritual', 'Ritual Hijaz'))}</div></div></div>''')
    # 11 DAW
    page(T('In your DAW', 'En tu DAW'), f'''{kick(T('In your DAW', 'En tu DAW'))}<h1>{T('A generator for <i>your whole studio</i>.', 'Un generador para <i>todo tu estudio</i>.')}</h1>
{table([T('Feature', 'Función'), T('How', 'Cómo')], [
  ('SYNC', T('On: ÓRBITA follows the DAW\'s tempo and position (it locks to the bar when you press play or jump). Off: its own tempo and ▶.', 'Activado: ÓRBITA sigue el tempo y la posición del DAW (se alinea al compás cuando pulsas play o saltas). Desactivado: su propio tempo y ▶.')),
  ('MIDI OUT', T('The notes ÓRBITA plays also leave the plug-in: Sub on channel 1, Prism 2, Crystal 3, Halo 4. In Live, put another MIDI track\'s input on ÓRBITA (MIDI From: the ÓRBITA track) to play any instrument with an orbit.', 'Las notas que toca ÓRBITA también salen del plug-in: Subgrave en el canal 1, Prisma 2, Cristal 3, Halo 4. En Live, pon la entrada de otra pista MIDI en ÓRBITA (MIDI From: la pista de ÓRBITA) para tocar cualquier instrumento con una órbita.')),
  ('⇣ MIDI', T('Press and drag it onto a MIDI track: a 4-bar clip with one track per voice, at the current tempo and swing. On the web it downloads the .mid file.', 'Púlsalo y arrástralo a una pista MIDI: un clip de 4 compases con una pista por voz, al tempo y swing actuales. En la web descarga el archivo .mid.')),
  (T('Automation', 'Automatización'), T('63 parameters: Gravity, Density, Phase, FM ratio, Evolve, Chaos, Bloom, Space, Echo, Warmth and the rest of the mix, root, scale, geometry, keys mode, and for each voice On, Level, Pulses, Steps, Rotate, FM index, Cutoff, Ratio, Echo and Space sends.', '63 parámetros: Gravity, Density, Phase, FM ratio, Evolve, Chaos, Bloom, Space, Echo, Warmth y el resto de la mezcla, tónica, escala, geometría, modo de teclas y, por voz, On, Level, Pulses, Steps, Rotate, FM index, Cutoff, Ratio y los envíos de Echo y Space.')),
  (T('The project', 'El proyecto'), T('Everything is saved with your set: every value, GRAVITY\'s target, the steps you clicked, your four slots, the language and the theme.', 'Todo se guarda con tu set: cada valor, el destino de GRAVEDAD, los pasos que pulsaste, tus cuatro ranuras, el idioma y el tema.'))])}
{img('top', T('The plug-in adds SYNC and MIDI OUT to the top bar.', 'El plug-in añade SYNC y MIDI OUT a la barra superior.'))}
{tip(T('Layer it: MIDI OUT into a drum rack (Sub as kick, Crystal as hats) and keep ÓRBITA\'s own sound for the bells. Or set its MASTER to 0 and use it only as a generator.', 'En capas: MIDI OUT a un drum rack (Subgrave como bombo, Cristal como hats) y deja el sonido de ÓRBITA para las campanas. O pon su SALIDA en 0 y úsalo sólo como generador.'))}
{note(T('Tested: the engine (ÓRBITA 1\'s notes and rhythms in every case, levels, silence after stop, every note on with its note off, steady sync) and the plug-in with pluginval at strictness 8 and auval. It hasn\'t been played inside Live by us yet: tell us how it goes.', 'Probado: el motor (las notas y ritmos de ÓRBITA 1 en todos los casos, niveles, silencio al detener, cada nota con su apagado, sincronía estable) y el plug-in con pluginval en rigor 8 y auval. Todavía no lo hemos tocado dentro de Live: cuéntanos cómo te va.'))}''')
    # 12 fx + recipes
    page(T('Effects and recipes', 'Efectos y recetas'), f'''{kick(T('Effects, mix and recipes', 'Efectos, mezcla y recetas'))}<h1>{T('Space, echo, <i>warmth</i>.', 'Espacio, eco, <i>calidez</i>.')}</h1>
<div class="two"><div>{img('macros')}</div><div>{table([T('Knob', 'Perilla'), T('What it does', 'Qué hace')], [
  (T('BLOOM', 'BRILLO FM'), T('More FM on every voice at once: from round to glittering.', 'Más FM en todas las voces a la vez: de redondo a brillante.')),
  (T('SPACE · SIZE · DAMP', 'ESPACIO · TAMAÑO · AMORTIGUAR'), T('An eight-line reverb: how much, how big (up to ~10 s), how dark.', 'Una reverberación de ocho líneas: cuánta, qué tan grande (hasta ~10 s), qué tan oscura.')),
  (T('ECHO · TIME · FEEDBACK · TONE', 'ECO · TIEMPO · REPETICIONES · TONO'), T('A ping-pong echo in time with the tempo (1/16 to 1/2, triplets), warm in its loop.', 'Un eco ping-pong a tempo (1/16 a 1/2, tresillos), cálido en su bucle.')),
  (T('CHORUS · WIDTH', 'CORO · ANCHURA'), T('A slow stereo shimmer · from mono to extra wide.', 'Un brillo estéreo lento · de mono a muy ancho.')),
  (T('WARMTH · TILT · MASTER', 'CALIDEZ · TONO · SALIDA'), T('Tape-like drive · darker or brighter · the output.', 'Saturación tipo cinta · más oscuro o más brillante · la salida.')),
  (T('SWING · HUMAN', 'SWING · HUMANO'), T('Delays every other sixteenth · loosens the velocities.', 'Retrasa cada segunda semicorchea · suelta las velocidades.'))])}</div></div>
<h2>{T('Recipes', 'Recetas')}</h2><div class="grid2">
<div class="card"><h3>{T('Ambient that never repeats', 'Ambient que nunca se repite')}</h3><p>{T('Cenote · EVOLVE 40% · SPACE 70%, SIZE 90% · Halo STEPS 10 · HARMONY mode, hold a minor seventh chord.', 'Cenote · EVOLUCIÓN 40% · ESPACIO 70%, TAMAÑO 90% · Halo PASOS 10 · modo ARMONÍA, mantén un acorde menor séptima.')}</p></div>
<div class="card"><h3>{T('Club transition', 'Transición de club')}</h3><p>{T('Play Deep Orbit, B on Discrete Acid, automate Gravity 0 → 100% over 16 bars, WARMTH 30%.', 'Toca Órbita profunda, B en Ácido discreto, automatiza Gravity de 0 a 100% en 16 compases, CALIDEZ 30%.')}</p></div>
<div class="card"><h3>{T('Bells over your bass line', 'Campanas sobre tu línea de bajo')}</h3><p>{T('TRANSPOSE mode, send your bass track\'s MIDI to ÓRBITA: the orbit follows every root. Mute Sub.', 'Modo TRANSPONER, manda el MIDI de tu pista de bajo a ÓRBITA: la órbita sigue cada tónica. Silencia Subgrave.')}</p></div>
<div class="card"><h3>{T('Generative drums', 'Batería generativa')}</h3><p>{T('MIDI OUT to a drum rack, Sub = kick (STEPS 16), Prism = snare (PULSES −5, ROTATE +4), Crystal = hats (STEPS 12), CHAOS 15%.', 'MIDI OUT a un drum rack, Subgrave = bombo (PASOS 16), Prisma = tarola (PULSOS −5, ROTAR +4), Cristal = hats (PASOS 12), CAOS 15%.')}</p></div></div>''')
    # 13 answers + credits
    qa = [(T('I hear nothing on the web.', 'No oigo nada en la web.'), T('Press ▶ (browsers start audio only after a click), check the volume, and that the phone isn\'t on silent.', 'Pulsa ▶ (los navegadores sólo inician el audio tras un clic), revisa el volumen y que el teléfono no esté en silencio.')),
          (T('Live doesn\'t show the plug-in.', 'Live no muestra el plug-in.'), T('Turn on "Use VST3 Plug-in System Folders" and Rescan. If you installed by hand, run the curl line again: it clears the quarantine.', 'Activa "Usar carpetas de sistema VST3" y vuelve a escanear. Si instalaste a mano, ejecuta otra vez la línea curl: quita la cuarentena.')),
          (T('It doesn\'t follow Live\'s tempo.', 'No sigue el tempo de Live.'), T('SYNC must be on (top bar). With Live stopped, ÓRBITA uses its own tempo.', 'SYNC debe estar activado (barra superior). Con Live detenido, ÓRBITA usa su propio tempo.')),
          (T('My clicked steps disappeared.', 'Mis pasos pulsados desaparecieron.'), T('Moving DENSITY, PHASE, PULSES, STEPS or ROTATE draws the rhythm again. Go back with the time machine.', 'Mover DENSIDAD, FASE, PULSOS, PASOS o ROTAR dibuja el ritmo de nuevo. Vuelve con la máquina del tiempo.')),
          (T('Too loud, or distorted?', '¿Muy fuerte o distorsionado?'), T('Lower MASTER and WARMTH. With WARMTH at 0% ÓRBITA is clean; the limiter only catches peaks.', 'Baja SALIDA y CALIDEZ. Con CALIDEZ al 0% ÓRBITA es limpio; el limitador sólo atrapa picos.'))]
    page(T('Answers and credits', 'Respuestas y créditos'), f'''{kick(T('Answers, credits and licence', 'Respuestas, créditos y licencia'))}<h1>{T('Questions, <i>answered</i>.', 'Preguntas, <i>respondidas</i>.')}</h1>
{''.join(f'<div class="qa"><h3>{q}</h3><p>{a}</p></div>' for q, a in qa)}
<div class="two" style="margin-top:14px"><div class="card"><h3>{T('Credits', 'Créditos')}</h3><p>{T('Dark Funk Durcal design · built at Polaris Sound Lab · Mérida, Yucatán, México.', 'Diseño de Dark Funk Durcal · desarrollado en Polaris Sound Lab · Mérida, Yucatán, México.')}<br><a href="{IG}">Instagram @polaris.sound.lab</a> · <a href="{YT}">YouTube @PolarisSoundLab</a></p></div>
<div class="card"><h3>{T('Support it', 'Apóyalo')}</h3><p>{T('ÓRBITA is free and complete. A donation keeps new instruments coming.', 'ÓRBITA es gratis y completo. Un donativo mantiene llegando instrumentos nuevos.')}<br><a class="don" href="{DON}">♥ {T('DONATE (PayPal)', 'DONAR (PayPal)')}</a></p></div></div>
{table([T('Part', 'Parte'), T('Licence', 'Licencia')], [(T('ÓRBITA 2 (engine, web, plug-in)', 'ÓRBITA 2 (motor, web, plug-in)'), 'GPL-3.0'), ('JUCE 8 (' + T('plug-in framework', 'marco del plug-in') + ')', T('its own terms (AGPL-3.0 / commercial)', 'sus propios términos (AGPL-3.0 / comercial)'))])}
<p>{T(f'Source and releases: <a href="{REPO}">github.com/drophlex/ORBITA</a> · Web: <a href="{WEB}">drophlex.github.io/ORBITA</a>', f'Código y versiones: <a href="{REPO}">github.com/drophlex/ORBITA</a> · Web: <a href="{WEB}">drophlex.github.io/ORBITA</a>')}</p>
<p class="small">{T('M-VAVE and FM-1 are trademarks of their owners; Ableton and Live of Ableton AG. ÓRBITA is not affiliated with or endorsed by them.', 'M-VAVE y FM-1 son marcas de sus dueños; Ableton y Live de Ableton AG. ÓRBITA no está afiliado ni respaldado por ellos.')}</p>''')

def render(lang):
    global L; L = lang; build()
    css = open(os.path.join(HERE, '../manual/orbita.css')).read() + 'ul{padding-left:18px;margin:6px 0}ul li{margin:4px 0}.k{font-family:IBM Plex Mono,monospace}'
    out = [f'<!doctype html><html lang="{lang}"><meta charset="utf-8"><style>{css}</style><body>']
    for i, (cls, body, title) in enumerate(PAGES):
        foot = '' if cls == 'coverp' else f'<footer><span>ÓRBITA 2 · {T("USER MANUAL", "MANUAL DE USUARIO")}</span><span>{title}</span><span>{i + 1}</span></footer>'
        out.append(f'<section class="page {cls}">{body}{foot}</section>')
    return ''.join(out) + '</body></html>'

if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    for lang in ('en', 'es'): open(os.path.join(OUT, f'manual_{lang}.html'), 'w').write(render(lang))
    en = render('en'); es = render('es')
    both = en.replace('ENGLISH EDITION', 'ENGLISH · ESPAÑOL').replace('</body></html>', '') + es.split('</section>', 1)[1]
    open(os.path.join(OUT, 'manual_en_es.html'), 'w').write(both)
    print('ok', len(PAGES), 'pages per language')
