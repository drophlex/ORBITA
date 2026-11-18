#!/usr/bin/env python3
# ÓRBITA · user manual (English by default, Spanish edition, and both) — HTML pages rendered to PDF by pdf.mjs
import os, math, sys
OUT = sys.argv[1] if len(sys.argv) > 1 else '/home/claude/man/orbita'
HERE = os.path.dirname(os.path.abspath(__file__))
L = 'en'
def T(en, es): return es if L == 'es' else en
WEB = 'https://drophlex.github.io/ORBITA/'
GUIDE = WEB + 'guide/'
REPO = 'https://github.com/drophlex/ORBITA'
REL = REPO + '/releases/latest'
DON = 'https://www.paypal.com/donate/?business=drophlex' + '%40' + 'gmail.com&amp;no_recurring=0&amp;item_name=ORBITA&amp;currency_code=USD'
IG, YT = 'https://www.instagram.com/polaris.sound.lab/', 'https://www.youtube.com/@PolarisSoundLab'
def shot(n): return f'shots/{L}_{n}.png'
def img(n, cap='', w='100%'):
    return f'<figure style="width:{w}"><img src="{shot(n)}">' + (f'<figcaption>{cap}</figcaption>' if cap else '') + '</figure>'
def table(head, rows): return '<table><tr>' + ''.join(f'<th>{h}</th>' for h in head) + '</tr>' + ''.join('<tr>' + ''.join(f'<td>{c}</td>' for c in r) + '</tr>' for r in rows) + '</table>'
def kick(t): return f'<div class="kick">{t}</div>'
def k(t): return f'<span class="k">{t}</span>'
def tip(t): return f'<div class="tip"><b>{T("Tip", "Consejo")}</b> {t}</div>'
def note(t): return f'<div class="note">{t}</div>'
def code(t): return f'<div class="code">{t}</div>'

def orbit_svg(size=520):
    cx = cy = size / 2; R = size * .36; out = [f'<svg viewBox="0 0 {size} {size}" xmlns="http://www.w3.org/2000/svg">']
    cols = ['#ceff78', '#af9af1', '#f1a07d']
    for v in range(3):
        for kk in range(6):
            pts = []
            for j in range(361):
                a = j * 2 * math.pi / 360; p = v * .44 + kk * .05
                x = math.sin(2 * a + p); y = math.sin(3 * a + v * .3)
                pts.append(f'{cx + x * R:.1f},{cy + y * R * (1 - v * .07):.1f}')
            out.append(f'<polyline points="{" ".join(pts)}" fill="none" stroke="{cols[v]}" stroke-opacity="{.85 if kk == 0 else .14}" stroke-width="{1.4 if kk == 0 else 1}"/>')
        for s in range(16):
            if (s * [4, 7, 9][v]) % 16 < [4, 7, 9][v]:
                a = s * 2 * math.pi / 16; x = math.sin(2 * a + v * .44); y = math.sin(3 * a + v * .3)
                out.append(f'<circle cx="{cx + x * R:.1f}" cy="{cy + y * R * (1 - v * .07):.1f}" r="4.2" fill="{cols[v]}"/>')
    out.append('</svg>'); return ''.join(out)

def euclid_svg(pulses, col='#ceff78'):
    out = ['<svg viewBox="0 0 180 180" xmlns="http://www.w3.org/2000/svg"><circle cx="90" cy="90" r="64" fill="none" stroke="#b9bbad"/>']
    pts = []
    for s in range(16):
        a = s * 2 * math.pi / 16 - math.pi / 2; x, y = 90 + 64 * math.cos(a), 90 + 64 * math.sin(a); on = (s * pulses) % 16 < pulses
        if on: pts.append(f'{x:.1f},{y:.1f}')
        out.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{7 if on else 4}" fill="{col if on else "#d5d7cb"}" stroke="#1a1c19" stroke-width="{1 if on else 0}"/>')
        out.append(f'<text x="{90 + 82 * math.cos(a):.1f}" y="{90 + 82 * math.sin(a) + 3:.1f}" font-size="8" text-anchor="middle" fill="#606457" font-family="monospace">{s + 1}</text>')
    if len(pts) > 2: out.insert(1, f'<polygon points="{" ".join(pts)}" fill="{col}" fill-opacity=".18" stroke="{col}" stroke-width="1.2"/>')
    out.append(f'<text x="90" y="95" font-size="20" text-anchor="middle" font-weight="700" font-family="Arial" fill="#1a1c19">{pulses}/16</text></svg>')
    return ''.join(out)

def panel_svg():
    # the plug-in's panel, drawn to scale from its layout (480 x 456): a diagram, not a screenshot
    names = [T('FM ratio', 'Relación FM'), T('FM index', 'Índice FM'), T('Phase', 'Fase'), T('Pulses / 16', 'Pulsos / 16'), T('MIDI root note', 'Nota raíz MIDI'), T('Output', 'Salida'), T('Sequence with Live', 'Secuencia con Live')]
    vals = ['2.00', '1.80', '0.16', '5', '48', '0.65', '']
    fr = [(2 - .25) / 7.75, 1.8 / 8, .16, 4 / 15, 24 / 60, .65]
    o = ['<svg viewBox="0 0 480 456" xmlns="http://www.w3.org/2000/svg" font-family="Arial"><rect width="480" height="456" rx="6" fill="#102221"/>',
         '<text x="16" y="31" fill="#a5f8cd" font-size="15">POLARIS SOUND LAB</text><text x="16" y="78" fill="#a5f8cd" font-size="33">ORBITA</text>']
    pts = []
    for i in range(301):
        t = i * 2 * math.pi / 300; pts.append(f'{341 + 104 * math.sin(t):.1f},{72 + 54 * math.sin(t * 2 + .16 * 6.283 + 1.8 * .22 * math.sin(t)):.1f}')
    o.append(f'<polyline points="{" ".join(pts)}" fill="none" stroke="#a5f8cd" stroke-width="1.4"/>')
    o.append(f'<rect x="16" y="106" width="112" height="26" rx="3" fill="#1d3533" stroke="#3f6b62"/><text x="24" y="123" fill="#e6f2ec" font-size="12">English ▾</text>')
    for i, n in enumerate(names):
        y = 146 + i * 38
        o.append(f'<text x="16" y="{y + 20}" fill="#e6f2ec" font-size="13">{n}</text>')
        if i < 6:
            o.append(f'<rect x="168" y="{y + 13}" width="222" height="5" rx="2" fill="#2c4a46"/><rect x="168" y="{y + 13}" width="{222 * fr[i]:.0f}" height="5" rx="2" fill="#a5f8cd"/>')
            o.append(f'<circle cx="{168 + 222 * fr[i]:.0f}" cy="{y + 15.5}" r="7" fill="#e6f2ec"/><rect x="398" y="{y + 4}" width="66" height="24" rx="3" fill="#1d3533"/><text x="431" y="{y + 20}" fill="#e6f2ec" font-size="12" text-anchor="middle">{vals[i]}</text>')
        else:
            o.append(f'<rect x="170" y="{y + 7}" width="18" height="18" rx="3" fill="none" stroke="#a5f8cd" stroke-width="1.5"/>')
    o.append(f'<text x="14" y="440" fill="#7fb59b" font-size="10">{T("Dark Funk Durcal design · built at Polaris Sound Lab", "diseño de Dark Funk Durcal · construido en Polaris Sound Lab")}</text>'
             f'<text x="278" y="440" fill="#7fb59b" font-size="10">Instagram</text><text x="342" y="440" fill="#7fb59b" font-size="10">YouTube</text><text x="402" y="440" fill="#a5f8cd" font-size="10">♥ {T("DONATE", "DONAR")}</text>')
    for n, (x, y) in enumerate([(130, 119), (393, 162), (393, 238), (393, 314), (190, 382), (300, 72), (470, 440)], 1):
        o.append(f'<circle cx="{x}" cy="{y}" r="9" fill="#cefe65"/><text x="{x}" y="{y + 4}" font-size="11" font-weight="700" text-anchor="middle" fill="#1a1c19">{n}</text>')
    o.append('</svg>'); return ''.join(o)

def callouts(n, spots, w='100%'):
    marks = ''.join(f'<span class="dot" style="left:{x}%;top:{y}%">{i}</span>' for i, (x, y) in enumerate(spots, 1))
    return f'<div class="callout" style="width:{w}"><img src="{shot(n)}">{marks}</div>'

PAGES = []
def page(title, body, cls=''): PAGES.append((cls, body, title))

def build():
    global PAGES; PAGES = []
    # 1 cover
    page('', f'''<div class="cover"><div class="cart">{orbit_svg()}</div>
<div class="ctitle">Órbita</div><div class="csub">{T('a generative instrument · FM synthesis, euclidean rhythms, phase plots', 'un instrumento generativo · síntesis FM, ritmos euclidianos, gráficos de fase')}</div>
<div class="cmeta">{T('USER MANUAL', 'MANUAL DE USUARIO')} · WEB · VST3 · AU · M-VAVE FM-1</div>
<div class="clang">{'ENGLISH · ESPAÑOL' if L == 'both' else T('ENGLISH EDITION', 'EDICIÓN EN ESPAÑOL')}</div>
<div class="cfoot">Dark Funk Durcal {T('design', 'diseño')} · {T('built at', 'creado en')} <b>POLARIS SOUND LAB</b> · Mérida, Yucatán, México · <a href="{DON}">♥ {T('DONATE', 'DONAR')}</a><br>
<a href="{IG}">Instagram @polaris.sound.lab</a> · <a href="{YT}">YouTube @PolarisSoundLab</a> · <a href="{WEB}">drophlex.github.io/ORBITA</a></div></div>''', 'coverp')
    # 2 welcome
    toc = [T('Quick start', 'Inicio rápido'), T('The screen', 'La pantalla'), T('The phase plane', 'El plano de fase'), T('Scenes', 'Escenas'),
           T('Density, phase, FM ratio', 'Densidad, fase, relación FM'), T('The sequencer and the voices', 'El secuenciador y las voces'), T('MIDI: clips and the FM-1', 'MIDI: clips y el FM-1'),
           T('Mac plug-in: install', 'Plug-in para Mac: instalar'), T('The plug-in in Ableton Live', 'El plug-in en Ableton Live'), T('FM-1 edition: install', 'Edición FM-1: instalar'),
           T('The 32 voices and 4 scenes', 'Las 32 voces y las 4 escenas'), T('The fm1-dock module', 'El módulo fm1-dock'), T('Recipes', 'Recetas'), T('Problems and answers', 'Problemas y respuestas'), T('Credits and licence', 'Créditos y licencia')]
    page(T('Welcome', 'Bienvenida'), f'''{kick(T('Welcome', 'Bienvenida'))}<h1>{T('Music drawn by <i>orbits</i>.', 'Música dibujada por <i>órbitas</i>.')}</h1>
<p class="lead">{T('ÓRBITA is a small generative instrument. Three FM voices — a sub bass, a prism and a crystal — play euclidean rhythms over a scale, and the screen draws each voice as a curve in the phase plane: every note you hear is a point on its orbit. Change the density, the phase or the FM ratio and both the sound and the figure change together.', 'ÓRBITA es un pequeño instrumento generativo. Tres voces FM — un subgrave, un prisma y un cristal — tocan ritmos euclidianos sobre una escala, y la pantalla dibuja cada voz como una curva en el plano de fase: cada nota que oyes es un punto de su órbita. Cambia la densidad, la fase o la relación FM y el sonido y la figura cambian juntos.')}</p>
<div class="grid3">
<div class="card"><h3>WEB</h3><p>{T('Open it and press Play. Computer or phone, nothing to install.', 'Ábrelo y pulsa Tocar. Computadora o teléfono, nada que instalar.')}<br><a href="{WEB}">drophlex.github.io/ORBITA</a></p></div>
<div class="card"><h3>VST3 · AU</h3><p>{T('An FM instrument for Ableton Live on Mac (Apple Silicon and Intel) that follows Live’s transport.', 'Un instrumento FM para Ableton Live en Mac (Apple Silicon e Intel) que sigue el transporte de Live.')}</p></div>
<div class="card"><h3>FM-1</h3><p>{T('32 voices installed in one click, and four MIDI scenes, for the M-VAVE FM-1.', '32 voces que se instalan en un clic, y cuatro escenas MIDI, para el M-VAVE FM-1.')}</p></div></div>
<h2>{T('Contents', 'Contenido')}</h2><ol class="toc">{''.join(f'<li>{t}</li>' for t in toc)}</ol>
{note(T(f'Everything is free and open source. Downloads, updates and the step-by-step install guide: <a href="{GUIDE}">drophlex.github.io/ORBITA/guide</a>.', f'Todo es gratis y de código abierto. Descargas, actualizaciones y la guía de instalación paso a paso: <a href="{GUIDE}">drophlex.github.io/ORBITA/guide</a>.'))}''')
    # 3 quick start
    page(T('Quick start', 'Inicio rápido'), f'''{kick('01 · ' + T('Quick start', 'Inicio rápido'))}<h1>{T('Sound in one minute', 'Sonido en un minuto')}</h1>
{img('full')}
<ol class="steps">
<li>{T(f'Open <a href="{WEB}">drophlex.github.io/ORBITA</a> in Chrome, Edge, Safari or Firefox and press {k("▶ Play")}.', f'Abre <a href="{WEB}">drophlex.github.io/ORBITA</a> en Chrome, Edge, Safari o Firefox y pulsa {k("▶ Tocar")}.')}</li>
<li>{T('Listen to a full cycle of <b>Mérida Ring</b>: a spacious bass and two bells crossing each other.', 'Escucha un ciclo completo de <b>Anillo de Mérida</b>: un grave espaciado y dos campanas que se cruzan.')}</li>
<li>{T('Tap <b>Prism</b> in the sequencer to mute it; tap again to bring it back when the bass comes in.', 'Toca <b>Prisma</b> en el secuenciador para silenciarla; vuelve a tocarla para traerla cuando entre el grave.')}</li>
<li>{T('Move <b>FM ratio</b> from 2 to 3.5: the bells turn metallic and the orbits twist.', 'Mueve <b>Relación FM</b> de 2 a 3.5: las campanas se vuelven metálicas y las órbitas se tuercen.')}</li>
<li>{T('Pick another <b>scene</b>. Each one sets tempo, geometry, scale, density, phase and ratio at once.', 'Elige otra <b>escena</b>. Cada una fija tempo, geometría, escala, densidad, fase y relación a la vez.')}</li></ol>
{tip(T('The language switch (English / Español) is in the top bar; the web remembers it, and the manual link opens the matching edition.', 'El cambio de idioma (English / Español) está en la barra superior; la web lo recuerda y el enlace al manual abre la edición del idioma elegido.'))}''')
    # 4 the screen
    rows = [('1', T('Top bar', 'Barra superior'), T('Install guide, user manual, language.', 'Guía de instalación, manual, idioma.')),
            ('2', 'BPM', T('Tempo, 40–220. Type a value and press Return.', 'Tempo, 40–220. Escribe un valor y pulsa Retorno.')),
            ('3', T('Play / Stop', 'Tocar / Detener'), T('Starts and stops the three voices.', 'Arranca y detiene las tres voces.')),
            ('4', T('Phase plane', 'Plano de fase'), T('The three orbits, the notes as dots, the step counter (01 / 16).', 'Las tres órbitas, las notas como puntos, el contador de pasos (01 / 16).')),
            ('5', T('Scene memory', 'Memoria de escena'), T('Scene, geometry, scale and the three sliders.', 'Escena, geometría, escala y los tres deslizadores.')),
            ('6', T('Sequencer', 'Secuenciador'), T('16 steps × 3 voices; tap a name to mute.', '16 pasos × 3 voces; toca un nombre para silenciar.')),
            ('7', T('Connections and downloads', 'Conexiones y descargas'), T('MIDI output, FM-1, MIDI clip, plug-in and FM-1 downloads.', 'Salida MIDI, FM-1, clip MIDI, descargas del plug-in y del FM-1.'))]
    page(T('The screen', 'La pantalla'), f'''{kick('02 · ' + T('The screen', 'La pantalla'))}<h1>{T('Everything on one page', 'Todo en una página')}</h1>
{callouts('full', [(64, 3), (75, 17), (97.5, 17), (35, 50), (96.5, 33), (24, 76), (33, 94)])}
{table(['#', T('Part', 'Parte'), T('What it does', 'Qué hace')], rows)}
{note(T('On a phone the screen stacks: the phase plane on top, then the scene panel, then the sequencer.', 'En el teléfono la pantalla se apila: arriba el plano de fase, luego el panel de escena y después el secuenciador.'))}''')
    # 5 phase plane
    page(T('The phase plane', 'El plano de fase'), f'''{kick('03 · ' + T('The phase plane', 'El plano de fase'))}<h1>{T('Three geometries', 'Tres geometrías')}</h1>
<p>{T('Each voice is a curve; its sixteen steps are sixteen points along it, and the hits are the coloured dots. While it plays, a ring marks the step sounding now. The faint copies around each curve open up as the <b>FM ratio</b> grows; a muted voice fades out of the picture.', 'Cada voz es una curva; sus dieciséis pasos son dieciséis puntos sobre ella, y los golpes son los puntos de color. Mientras suena, un anillo marca el paso que suena. Las copias tenues alrededor de cada curva se abren al subir la <b>relación FM</b>; una voz silenciada se desvanece de la figura.')}</p>
<div class="grid3 shots">{img('liss', 'Lissajous · x = sin(2t + φ), y = sin(3t)')}{img('rose', T('Polar rose · r = cos(5t + φ)', 'Rosa polar · r = cos(5t + φ)'))}{img('epi', T('Epicycle · a circle riding a circle', 'Epiciclo · un círculo sobre otro'))}</div>
<p>{T('<b>Geometry</b> does more than draw: it is also how the melody is chosen. For every step, the height of the curve picks a degree of the scale — so the same rhythm sings a different line on a Lissajous, a rose or an epicycle. The <b>Phase</b> slider turns the figure and shifts where the notes fall.', 'La <b>geometría</b> no sólo dibuja: también elige la melodía. En cada paso, la altura de la curva escoge un grado de la escala — así el mismo ritmo canta otra línea en un Lissajous, una rosa o un epiciclo. El deslizador <b>Fase</b> gira la figura y mueve dónde caen las notas.')}</p>
{table([T('Geometry', 'Geometría'), T('Shape of the line', 'Forma de la línea'), T('Good for', 'Va bien con')], [
        ('Lissajous', T('Wide arcs up and down', 'Arcos amplios de arriba abajo'), T('Bass lines that breathe, calm scenes', 'Bajos que respiran, escenas tranquilas')),
        (T('Polar rose', 'Rosa polar'), T('Jumps to the centre and back', 'Saltos al centro y de regreso'), T('Off-beat bells, syncopation', 'Campanas a contratiempo, síncopas')),
        (T('Epicycle', 'Epiciclo'), T('Small turns inside a big circle', 'Giros pequeños dentro de un círculo grande'), T('Arpeggios, dense patterns', 'Arpegios, patrones densos'))])}''')
    # 6 scenes
    sc = [(T('Mérida Ring', 'Anillo de Mérida'), 112, 'Lissajous', T('D Dorian', 'Re dórico'), 7, 2, 2, T('A spacious bass line supports two intersecting bells.', 'Un grave espaciado sostiene dos campanas que se cruzan.')),
          (T('Skewed Crystal', 'Cristal sesgado'), 136, T('Polar rose', 'Rosa polar'), T('F minor', 'Fa menor'), 10, 5, 4, T('Offbeat metallic attacks trace a five-petal rose.', 'Ataques metálicos a contratiempo en una rosa de cinco pétalos.')),
          (T('Lunar Drift', 'Deriva lunar'), 74, T('Epicycle', 'Epiciclo'), T('A pentatonic', 'La pentatónica'), 4, 3, 1.5, T('A slow pulse and open resonances over a pentatonic scale.', 'Un pulso lento y resonancias abiertas sobre una escala pentatónica.')),
          (T('Discrete Acid', 'Ácido discreto'), 148, 'Lissajous', T('D Dorian', 'Re dórico'), 12, 9, 3.5, T('A dense weave of sixteenth notes with bright modulation.', 'Una trama densa de semicorcheas con modulación brillante.'))]
    page(T('Scenes', 'Escenas'), f'''{kick('04 · ' + T('Scenes', 'Escenas'))}<h1>{T('Four starting points', 'Cuatro puntos de partida')}</h1>
<p>{T('A scene is a memory: choosing one sets the tempo, the geometry, the scale and the three sliders, and un-mutes every voice. Then change anything — the scene is only where you start.', 'Una escena es una memoria: al elegirla se fijan el tempo, la geometría, la escala y los tres deslizadores, y se activan todas las voces. Después cambia lo que quieras — la escena sólo es el punto de partida.')}</p>
{table([T('Scene', 'Escena'), 'BPM', T('Geometry', 'Geometría'), T('Scale', 'Escala'), T('Density', 'Densidad'), T('Phase', 'Fase'), T('FM ratio', 'Relación FM'), T('Character', 'Carácter')], [(a, b, c, d, e, f, g, h) for a, b, c, d, e, f, g, h in sc])}
<div class="grid3 shots">{img('scene1', sc[1][0])}{img('scene2', sc[2][0])}{img('scene3', sc[3][0])}</div>''')
    # 7 density, phase, ratio
    page(T('Density, phase, FM ratio', 'Densidad, fase, relación FM'), f'''{kick('05 · ' + T('The three sliders', 'Los tres deslizadores'))}<h1>{T('Density, phase, FM ratio', 'Densidad, fase, relación FM')}</h1>
{img('panel', '', '58%')}
<h3>{T('Density (3–14): euclidean rhythms', 'Densidad (3–14): ritmos euclidianos')}</h3>
<p>{T('Density spreads that many hits as evenly as possible over the 16 steps — the euclidean rhythms found in music all over the world. The three voices take different amounts: the <b>sub bass</b> three fewer, the <b>prism</b> exactly the density, the <b>crystal</b> two more, so they interlock instead of doubling.', 'La densidad reparte ese número de golpes lo más parejo posible en los 16 pasos — los ritmos euclidianos que aparecen en músicas de todo el mundo. Las tres voces toman cantidades distintas: el <b>subgrave</b> tres menos, el <b>prisma</b> exactamente la densidad y el <b>cristal</b> dos más, para que se entrelacen en vez de doblarse.')}</p>
<div class="grid4">{''.join(f'<div class="ring">{euclid_svg(p)}</div>' for p in (3, 5, 7, 11))}</div>
<h3>{T('Phase (0–15): rotation', 'Fase (0–15): rotación')}</h3>
<p>{T('Phase rotates each voice’s pattern by a different amount (none for the sub bass, once for the prism, twice for the crystal) and turns the figure. Small changes move the accents off the beat; big ones make a new groove from the same hits.', 'La fase rota el patrón de cada voz una cantidad distinta (nada el subgrave, una vez el prisma, dos el cristal) y gira la figura. Cambios pequeños sacan los acentos del tiempo; cambios grandes hacen un groove nuevo con los mismos golpes.')}</p>
<h3>{T('FM ratio (0.5–8): the colour', 'Relación FM (0.5–8): el color')}</h3>
<p>{T('The modulator runs at the note’s frequency times the ratio. Whole numbers (1, 2, 3, 4) give harmonic, warm tones; halves (1.5, 2.5, 3.5) give hollow, bell-like or metallic tones. The higher the ratio, the brighter the attack.', 'El modulador va a la frecuencia de la nota por la relación. Los números enteros (1, 2, 3, 4) dan tonos armónicos y cálidos; las mitades (1.5, 2.5, 3.5) dan tonos huecos, de campana o metálicos. Cuanto más alta la relación, más brillante el ataque.')}</p>''')
    # 8 sequencer and voices
    page(T('The sequencer and the voices', 'El secuenciador y las voces'), f'''{kick('06 · ' + T('Sequencer', 'Secuenciador'))}<h1>{T('Three voices, sixteen steps', 'Tres voces, dieciséis pasos')}</h1>
{img('seq')}
{table([T('Voice', 'Voz'), T('Colour', 'Color'), T('Register', 'Registro'), T('Sound', 'Sonido'), T('Hits', 'Golpes')], [
        (T('Sub bass', 'Subgrave'), '<span class="sw" style="background:#ceff78"></span> lime', T('the root octave', 'la octava de la raíz'), T('soft FM, short tail', 'FM suave, cola corta'), T('density − 3', 'densidad − 3')),
        (T('Prism', 'Prisma'), '<span class="sw" style="background:#af9af1"></span> violet', T('one octave up', 'una octava arriba'), T('bright FM, medium tail', 'FM brillante, cola media'), T('density', 'densidad')),
        (T('Crystal', 'Cristal'), '<span class="sw" style="background:#f1a07d"></span> coral', T('two octaves up', 'dos octavas arriba'), T('glassy FM, long tail', 'FM de vidrio, cola larga'), T('density + 2', 'densidad + 2'))])}
<p>{T('The highlighted column is the step playing now. Tap a voice’s <b>name</b> to mute it or bring it back; muted voices dim in the sequencer and in the phase plane. Arrangements come from muting and un-muting while it plays: start with the crystal alone, add the prism, drop in the bass.', 'La columna resaltada es el paso que suena. Toca el <b>nombre</b> de una voz para silenciarla o traerla de vuelta; las voces silenciadas se atenúan en el secuenciador y en el plano de fase. Los arreglos salen de silenciar y activar mientras suena: empieza con el cristal solo, suma el prisma, mete el grave.')}</p>
{img('mute', T('Sub bass muted: its orbit fades.', 'Subgrave silenciado: su órbita se desvanece.'), '78%')}''')
    # 9 MIDI
    page(T('MIDI: clips and the FM-1', 'MIDI: clips y el FM-1'), f'''{kick('07 · MIDI')}<h1>{T('Take it out of the browser', 'Sácalo del navegador')}</h1>
{img('conn')}
<h3>{T('Export MIDI clip', 'Exportar clip MIDI')}</h3>
<p>{T('Saves one bar of what is playing (muted voices left out) as a standard MIDI file with the tempo: the sub bass on channel 1, the prism on 2, the crystal on 3. Drop it in Ableton Live or any DAW and loop it.', 'Guarda un compás de lo que suena (sin las voces silenciadas) como archivo MIDI estándar con el tempo: el subgrave en el canal 1, el prisma en el 2, el cristal en el 3. Suéltalo en Ableton Live o cualquier DAW y ponlo en bucle.')}</p>
<h3>{T('Play your FM-1 (or any MIDI synth) from the web', 'Toca tu FM-1 (o cualquier sinte MIDI) desde la web')}</h3>
<ol class="steps"><li>{T('Use Chrome or Edge (they have Web MIDI). Connect the FM-1 with a USB <b>data</b> cable.', 'Usa Chrome o Edge (tienen Web MIDI). Conecta el FM-1 con un cable USB <b>de datos</b>.')}</li>
<li>{T('Open <b>Connections and downloads</b> and press <b>Detect FM1 / MIDI</b>; allow MIDI when the browser asks.', 'Abre <b>Conexiones y descargas</b> y pulsa <b>Detectar FM1 / MIDI</b>; permite el MIDI cuando el navegador lo pida.')}</li>
<li>{T('Choose the FM-1 under <b>MIDI output</b> and press Play: the three voices go out on channels 1, 2 and 3, and the browser goes quiet. Listen from the FM-1’s own output.', 'Elige el FM-1 en <b>Salida MIDI</b> y pulsa Tocar: las tres voces salen por los canales 1, 2 y 3 y el navegador se calla. Escucha desde la salida del FM-1.')}</li></ol>
{tip(T('Load ÓRBITA’s own voices on the FM-1 first (page 12): ORSUB on track 1, ORBELL or ORKEY on track 2, ORHALO on track 3.', 'Carga primero las voces de ÓRBITA en el FM-1 (página 12): ORSUB en la pista 1, ORBELL u ORKEY en la 2, ORHALO en la 3.'))}
{note(T('“Browser audio only” sends nothing out. If no output appears, check the cable (many USB cables only charge) and reload the page.', '“Sólo audio web” no envía nada. Si no aparece ninguna salida, revisa el cable (muchos cables USB sólo cargan) y recarga la página.'))}''')
    # 10 Mac install
    page(T('Mac plug-in: install', 'Plug-in para Mac: instalar'), f'''{kick('08 · Mac · VST3 · AU')}<h1>{T('Install in one line', 'Instala en una línea')}</h1>
<p>{T('For Ableton Live (or Logic, Bitwig, Reaper…) on a Mac with Apple Silicon or Intel, macOS 11 or later.', 'Para Ableton Live (o Logic, Bitwig, Reaper…) en una Mac con Apple Silicon o Intel, macOS 11 o posterior.')}</p>
<ol class="steps"><li>{T('Open <b>Terminal</b>: ⌘ + Space, type “Terminal”, Return.', 'Abre <b>Terminal</b>: ⌘ + Espacio, escribe “Terminal”, Retorno.')}</li>
<li>{T('Paste this line and press Return:', 'Pega esta línea y pulsa Retorno:')}{code('curl -fsSL https://drophlex.github.io/ORBITA/install.sh | bash')}</li>
<li>{T('It installs <b>Polaris Orbita</b> as VST3 and Audio Unit in your user plug-in folders, and the standalone app in ~/Applications.', 'Instala <b>Polaris Orbita</b> como VST3 y Audio Unit en tus carpetas de plug-ins de usuario, y la app independiente en ~/Applications.')}</li>
<li>{T('Ableton Live → <b>Settings → Plug-ins</b> → turn on <b>Use VST3 Plug-in System Folders</b> (or Audio Units) → <b>Rescan</b>.', 'Ableton Live → <b>Ajustes → Plug-ins</b> → activa <b>Usar carpetas de sistema VST3</b> (o Audio Units) → <b>Volver a escanear</b>.')}</li></ol>
{img('guide_mac', T('The install guide on the web has the same line with a Copy button.', 'La guía de instalación en la web tiene la misma línea con un botón Copiar.'), '88%')}
<h3>{T('Without Terminal', 'Sin Terminal')}</h3>
<p>{T(f'Download <b>ORBITA-mac.zip</b> from <a href="{REL}">the latest release</a>, unzip it, right-click <b>Install-ORBITA.command</b> → Open → Open. If macOS refuses: System Settings → Privacy &amp; Security → Open Anyway. To update, run the line or the installer again.', f'Descarga <b>ORBITA-mac.zip</b> de <a href="{REL}">la última versión</a>, descomprímelo, clic derecho en <b>Install-ORBITA.command</b> → Abrir → Abrir. Si macOS lo rechaza: Ajustes del Sistema → Privacidad y seguridad → Abrir igualmente. Para actualizar, vuelve a ejecutar la línea o el instalador.')}</p>''')
    # 11 plugin in Live
    page(T('The plug-in in Ableton Live', 'El plug-in en Ableton Live'), f'''{kick('09 · ' + T('In Ableton Live', 'En Ableton Live'))}<h1>{T('An FM instrument that follows Live', 'Un instrumento FM que sigue a Live')}</h1>
<div class="two"><div class="panelsvg">{panel_svg()}<div class="cap">{T('The panel (diagram drawn from its layout).', 'El panel (diagrama dibujado de su diseño).')}</div></div><div>
{table(['#', T('Control', 'Control'), T('Range', 'Rango')], [
        ('1', T('Language', 'Idioma'), 'English / Español'), ('2', T('FM ratio', 'Relación FM'), '0.25 – 8 · 2'), ('', T('FM index', 'Índice FM'), '0 – 8 · 1.8'),
        ('3', T('Phase', 'Fase'), '0 – 1 · 0.16'), ('', T('Pulses / 16', 'Pulsos / 16'), '1 – 16 · 5'), ('4', T('MIDI root note', 'Nota raíz MIDI'), '24 – 84 · 48'),
        ('', T('Output', 'Salida'), '0 – 1 · 0.65'), ('5', T('Sequence with Live', 'Secuencia con Live'), 'on / off'), ('6', T('Figure', 'Figura'), T('follows ratio, index, phase', 'sigue relación, índice, fase')), ('7', T('Links', 'Enlaces'), T('credits · donate', 'créditos · donar'))])}</div></div>
<ol class="steps"><li>{T('Drag <b>Plug-ins → Polaris Sound Lab → Polaris Orbita</b> onto a MIDI track. Play a minor chord: 12 voices of two-operator FM.', 'Arrastra <b>Plug-ins → Polaris Sound Lab → Polaris Orbita</b> a una pista MIDI. Toca un acorde menor: 12 voces de FM de dos operadores.')}</li>
<li>{T('Raise <b>FM index</b> for more partials; move <b>FM ratio</b> from 2 to 2.37 to go from harmonic to metallic; automate <b>Phase</b> to slide the modulator.', 'Sube <b>Índice FM</b> para más parciales; mueve <b>Relación FM</b> de 2 a 2.37 para pasar de armónico a metálico; automatiza <b>Fase</b> para desplazar el modulador.')}</li>
<li>{T('Turn on <b>Sequence with Live</b> and press play in Live: <b>Pulses / 16</b> spreads that many hits over sixteen sixteenths (5 pulses: steps 1, 5, 8, 11, 14), on a minor pentatonic phrase from the <b>root note</b>. It follows Live’s tempo, bar and jumps.', 'Activa <b>Secuencia con Live</b> y dale play en Live: <b>Pulsos / 16</b> reparte esos golpes en dieciséis semicorcheas (5 pulsos: pasos 1, 5, 8, 11, 14), sobre una frase pentatónica menor desde la <b>nota raíz</b>. Sigue el tempo, el compás y los saltos de Live.')}</li></ol>
{tip(T('Every control is a Live parameter: show automation (A) and draw ratio sweeps, or map them to your controller. Everything is saved with the set, including the panel language.', 'Cada control es un parámetro de Live: muestra la automatización (A) y dibuja barridos de relación, o asígnalos a tu controlador. Todo se guarda con el set, incluido el idioma del panel.'))}''')
    # 12 FM-1 install
    page(T('FM-1 edition: install', 'Edición FM-1: instalar'), f'''{kick('10 · M-VAVE FM-1')}<h1>{T('One click: the installer', 'Un clic: el instalador')}</h1>
<div class="two"><div>
<p>{T('The FM-1 edition is 32 voices (plus four MIDI scenes). It is <b>not a firmware</b>: the installer only adds voices.', 'La edición FM-1 son 32 voces (más cuatro escenas MIDI). <b>No es un firmware</b>: el instalador sólo añade voces.')}</p>
<ol class="steps"><li>{T('Connect the FM-1 with a USB <b>data</b> cable and turn it on.', 'Conecta el FM-1 con un cable USB <b>de datos</b> y enciéndelo.')}</li>
<li>{T(f'Open <a href="{WEB}fm1/">drophlex.github.io/ORBITA/fm1</a> in Chrome or Edge.', f'Abre <a href="{WEB}fm1/">drophlex.github.io/ORBITA/fm1</a> en Chrome o Edge.')}</li>
<li>{T('Press <b>Connect and install ÓRBITA</b> and allow MIDI. It finds the FM-1, writes the voices, reads them back and checks all 32 names.', 'Pulsa <b>Conectar e instalar ÓRBITA</b> y permite el MIDI. Encuentra el FM-1, escribe las voces, las vuelve a leer y comprueba los 32 nombres.')}</li></ol>
</div>{img('installer', '', '100%')}</div>
{table([T('Your FM-1 runs', 'Tu FM-1 tiene'), T('What the installer does', 'Qué hace el instalador'), T('Then', 'Después')], [
 ('ANDROMEDA · Dark Funk Durcal · Felucca 1.4.1+', T('writes the FM6 voice bank B1–B32 and checks it; music, projects and presets untouched. <b>Remove</b> brings the built-in voices back', 'escribe el banco de voces FM6 B1–B32 y lo comprueba; música, proyectos y presets intactos. <b>Quitar</b> regresa las voces de fábrica'),
  T('track → hold EDIT + 2nd white key (FM6) → HOME, KNOB 4 SLOT → B1…B32', 'pista → mantén EDIT + 2ª tecla blanca (FM6) → HOME, PERILLA 4 SLOT → B1…B32')),
 (T('official M-VAVE (V15, V16, FM1 Open)', 'oficial de M-VAVE (V15, V16, FM1 Open)'), T('sends the standard DX7 32-voice bank', 'envía el banco estándar DX7 de 32 voces'), T('choose on the FM-1 the bank A–D to save it in (its old voices are replaced: export them first with the Workbench to keep them)', 'elige en el FM-1 el banco A–D donde guardarlo (sus voces anteriores se reemplazan: expórtalas antes con el Workbench para conservarlas)'))])}
<h3>{T('Without the installer', 'Sin el instalador')}</h3>
<p>{T(f'<b>ORBITA-FM1.zip</b> (<a href="{REL}">latest release</a>) has <b>Orbita-32.syx</b> (the bank) and <b>voces/</b> (one file per voice) for the M-VAVE Workbench or any DX7 librarian, the <b>escenas/</b> MIDI clips and the manuals.', f'<b>ORBITA-FM1.zip</b> (<a href="{REL}">última versión</a>) trae <b>Orbita-32.syx</b> (el banco) y <b>voces/</b> (un archivo por voz) para el Workbench de M-VAVE o cualquier librería DX7, los clips MIDI de <b>escenas/</b> y los manuales.')}</p>
{note(T('Checked on a computer against the FM-1 firmware’s own code (ANDROMEDA’s): every reply, the 32 names read back, each voice loaded byte for byte and played (no clipping, no DC), and Remove. Not yet tried on a physical FM-1, nor on the official firmware — tell us how it goes.', 'Comprobado en una computadora contra el propio código del firmware del FM-1 (el de ANDROMEDA): cada respuesta, los 32 nombres leídos de vuelta, cada voz cargada byte por byte y tocada (sin saturación ni DC), y Quitar. Aún no se probó en un FM-1 físico ni en el firmware oficial — cuéntanos cómo te va.'))}''')
    # 13 voices and scenes
    fam = [('SUB', T('basses', 'bajos')), ('BELL', T('bells', 'campanas')), ('HALO', T('soft layers', 'capas suaves')), ('REED', T('reeds', 'lengüetas')),
           ('DUST', T('percussion', 'percusión')), ('KEY', T('electric keys', 'teclas eléctricas')), ('WIRE', T('metal strings', 'cuerdas metálicas')), ('TIDE', T('slow pads', 'colchones lentos'))]
    page(T('The 32 voices and 4 scenes', 'Las 32 voces y las 4 escenas'), f'''{kick('11 · ' + T('Voices and scenes', 'Voces y escenas'))}<h1>{T('Eight families, four variations', 'Ocho familias, cuatro variaciones')}</h1>
{table([T('Family', 'Familia'), T('Voices', 'Voces'), T('Numbers', 'Números'), T('Sound', 'Sonido')], [(f, f'OR{f}1 – OR{f}4', f'{i * 4 + 1:02d} – {i * 4 + 4:02d}', d) for i, (f, d) in enumerate(fam)])}
<p>{T('Each family uses three carrier/modulator pairs; its four variations change intensity, fine tuning and feedback — 1 is the gentlest, 4 the most extreme.', 'Cada familia usa tres parejas portadora/moduladora; sus cuatro variaciones cambian intensidad, afinación fina y realimentación — la 1 es la más suave, la 4 la más extrema.')}</p>
<h3>{T('The scenes (MIDI clips)', 'Las escenas (clips MIDI)')}</h3>
{table([T('Clip', 'Clip'), 'BPM', T('Hits / 16', 'Golpes / 16'), T('Try it with', 'Pruébalo con')], [('Perigeo-102.mid', 102, 5, 'ORSUB1'), ('Isoclina-124.mid', 124, 7, 'ORKEY2'), ('Lagrange-76.mid', 76, 3, 'ORHALO3'), ('Sideral-138.mid', 138, 11, 'ORDUST4')])}
<p>{T('Four bars of 4/4 in D minor; the rotation moves the first hit and the phrase changes degree every bar.', 'Cuatro compases de 4/4 en re menor; la rotación desplaza el primer golpe y la frase cambia de grado en cada compás.')}</p>
<ol class="steps"><li>{T('In Live: Settings → Link, Tempo &amp; MIDI → turn on <b>Track</b> for the FM-1’s MIDI output.', 'En Live: Ajustes → Link, Tempo y MIDI → activa <b>Track</b> en la salida MIDI del FM-1.')}</li>
<li>{T('Drag a clip from <b>escenas</b> onto a MIDI track; set its <b>MIDI To</b> to the FM-1, channel 1 (the first track on Open V14).', 'Arrastra un clip de <b>escenas</b> a una pista MIDI; pon su <b>MIDI To</b> en el FM-1, canal 1 (la primera pista en Open V14).')}</li>
<li>{T('Press play: the clip follows Live’s tempo; its name gives the suggested BPM.', 'Dale play: el clip sigue el tempo de Live; su nombre indica el BPM sugerido.')}</li></ol>''')
    # 14 fm1-dock module
    page(T('The fm1-dock module', 'El módulo fm1-dock'), f'''{kick('12 · fm1-dock')}<h1>{T('ÓRBITA on the FM-1’s screen', 'ÓRBITA en la pantalla del FM-1')}</h1>
<p>{T('The <b>modulo/orbita</b> folder is the source of a monophonic ÓRBITA instrument for <a href="https://github.com/joakimeriksson/fm1-dock">fm1-dock</a>, a community firmware for the FM-1 that loads instruments as modules. The screen draws a Lissajous figure that answers K1 and K3, with the sixteen steps of the pattern along the bottom.', 'La carpeta <b>modulo/orbita</b> es el código de un instrumento ÓRBITA monofónico para <a href="https://github.com/joakimeriksson/fm1-dock">fm1-dock</a>, un firmware comunitario para el FM-1 que carga instrumentos como módulos. La pantalla dibuja una figura de Lissajous que responde a K1 y K3, con los dieciséis pasos del patrón abajo.')}</p>
{table([T('Control', 'Control'), T('What it does', 'Qué hace'), 'CC'], [(T('Hold a key', 'Mantén una tecla'), T('starts a pattern of sixteenths; the note sets the carrier, velocity the intensity', 'arranca un patrón de semicorcheas; la nota fija la portadora y la velocidad la intensidad'), '—'),
        ('K1', T('FM ratio 1–8', 'relación FM 1–8'), '20'), ('K2', T('1–16 hits per cycle', '1–16 golpes por ciclo'), '21'), ('K3', T('rhythmic phase 0–15 steps', 'fase rítmica 0–15 pasos'), '22'), ('K4', T('internal clock 40–200 BPM', 'reloj interno 40–200 BPM'), '23'), ('EDIT', T('screen language: English / Español', 'idioma de la pantalla: English / Español'), '—')])}
<h3>{T('For developers', 'Para desarrolladores')}</h3>
<ol class="steps"><li>{T('Set up fm1-dock, its JieLi toolchain and the AC79 SDK files as its “Build it” section says.', 'Prepara fm1-dock, su toolchain JieLi y los archivos del SDK AC79 como indica su sección “Build it”.')}</li>
<li>{T('Copy the <b>orbita</b> folder to <b>modules/orbita</b> in that project.', 'Copia la carpeta <b>orbita</b> en <b>modules/orbita</b> de ese proyecto.')}</li>
<li>{T('From its root:', 'Desde su raíz:')}{code('python3 tools/hbmod.py modules/orbita')}{T('then validate <b>build/modules/orbita.dock</b> in its emulation before loading it on an instrument running that host firmware.', 'y valida <b>build/modules/orbita.dock</b> en su emulación antes de cargarlo en un instrumento con ese firmware anfitrión.')}</li></ol>
{note(T('Status: tested with GCC on Linux against hb_module.h (note and control events, non-silent stereo output, note release, screen bounds, pulse counts for all 256 density/rotation pairs). The JieLi build and the hardware test are still to do, so there is no ready-to-install file yet. Licence GPL-3.0-only.', 'Estado: probado con GCC en Linux contra hb_module.h (eventos de nota y control, salida estéreo no silenciosa, liberación de nota, límites de pantalla, conteo de pulsos para los 256 pares densidad/rotación). Faltan la compilación JieLi y la prueba en hardware, así que todavía no hay un archivo listo para instalar. Licencia GPL-3.0-only.'))}''')
    # 15 recipes
    rec = [(T('Night drive', 'Manejo nocturno'), T('Lunar Drift · Epicycle · ratio 1.5 → 2.5 over a minute · mute the crystal for 4 bars, then back.', 'Deriva lunar · Epiciclo · relación 1.5 → 2.5 durante un minuto · silencia el cristal 4 compases y regrésalo.')),
           (T('Gamelan clock', 'Reloj gamelán'), T('Skewed Crystal · Polar rose · density 9 · phase 6 · ratio 3.5 · 128 BPM.', 'Cristal sesgado · Rosa polar · densidad 9 · fase 6 · relación 3.5 · 128 BPM.')),
           (T('Acid study', 'Estudio ácido'), T('Discrete Acid · ratio 4 · then phase 0 → 15 one step per bar: the groove keeps turning.', 'Ácido discreto · relación 4 · luego fase 0 → 15 un paso por compás: el groove no deja de girar.')),
           (T('From the web to Live', 'De la web a Live'), T('Find a figure you like, Export MIDI clip, drop it on a track with Polaris Orbita, automate FM index.', 'Encuentra una figura que te guste, Exportar clip MIDI, suéltalo en una pista con Polaris Orbita, automatiza el índice FM.')),
           (T('FM-1 trio', 'Trío en el FM-1'), T('ORSUB2 on track 1, ORKEY3 on 2, ORHALO1 on 3; play the web into the FM-1 (page 9) at 102 BPM.', 'ORSUB2 en la pista 1, ORKEY3 en la 2, ORHALO1 en la 3; toca la web hacia el FM-1 (página 9) a 102 BPM.')),
           (T('Live pulses', 'Pulsos en Live'), T('Polaris Orbita with Sequence with Live on, pulses 5, root 50; a second instance with pulses 7, root 62, ratio 3.5.', 'Polaris Orbita con Secuencia con Live, pulsos 5, raíz 50; una segunda instancia con pulsos 7, raíz 62, relación 3.5.'))]
    page(T('Recipes', 'Recetas'), f'''{kick('13 · ' + T('Recipes', 'Recetas'))}<h1>{T('Six places to start', 'Seis lugares para empezar')}</h1>
<div class="grid2">{''.join(f'<div class="card"><h3>{a}</h3><p>{b}</p></div>' for a, b in rec)}</div>
{img('phone', T('ÓRBITA on a phone: everything works with touch.', 'ÓRBITA en el teléfono: todo funciona con el tacto.'), '32%')}''')
    # 16 problems
    qa = [(T('No sound in the browser', 'No suena en el navegador'), T('Press Play once more (browsers need a tap to start audio), turn the volume up, and check that “Browser audio only” is selected as MIDI output.', 'Pulsa Tocar otra vez (los navegadores necesitan un toque para iniciar el audio), sube el volumen y revisa que “Sólo audio web” esté elegido como salida MIDI.')),
          (T('The FM-1 is not listed', 'El FM-1 no aparece'), T('Use Chrome or Edge, a USB data cable, allow MIDI, then press Detect FM1 / MIDI again. Safari and Firefox on iPhone have no Web MIDI.', 'Usa Chrome o Edge, un cable USB de datos, permite el MIDI y vuelve a pulsar Detectar FM1 / MIDI. Safari y Firefox en iPhone no tienen Web MIDI.')),
          (T('Live does not show the plug-in', 'Live no muestra el plug-in'), T('Settings → Plug-ins → turn on Use VST3 Plug-in System Folders (or Audio Units) and Rescan. Live must be the native Apple Silicon or Intel version that matches your Mac.', 'Ajustes → Plug-ins → activa Usar carpetas de sistema VST3 (o Audio Units) y Volver a escanear. Live debe ser la versión nativa para tu Mac.')),
          (T('macOS says the installer is from an unidentified developer', 'macOS dice que el instalador es de un desarrollador no identificado'), T('Right-click → Open → Open, or System Settings → Privacy &amp; Security → Open Anyway. The one-line Terminal install avoids it.', 'Clic derecho → Abrir → Abrir, o Ajustes del Sistema → Privacidad y seguridad → Abrir igualmente. La instalación de una línea en Terminal lo evita.')),
          (T('Sequence with Live does nothing', 'Secuencia con Live no hace nada'), T('It plays only while Live’s transport runs. Press play in Live; check Output is up.', 'Sólo toca mientras corre el transporte de Live. Dale play en Live; revisa que Salida esté arriba.')),
          (T('The installer does not find the FM-1', 'El instalador no encuentra el FM-1'), T('Chrome or Edge on a computer, a USB data cable, the FM-1 on; close the Workbench, editors and DAWs that hold its MIDI port, reload, press again.', 'Chrome o Edge en una computadora, cable USB de datos, el FM-1 encendido; cierra el Workbench, editores y DAW que tengan su puerto MIDI, recarga y vuelve a pulsar.'))]
    page(T('Problems and answers', 'Problemas y respuestas'), f'''{kick('14 · ' + T('Help', 'Ayuda'))}<h1>{T('Problems and answers', 'Problemas y respuestas')}</h1>
{''.join(f'<div class="qa"><h3>{q}</h3><p>{a}</p></div>' for q, a in qa)}
{note(T(f'Something else? Open an issue at <a href="{REPO}/issues">github.com/drophlex/ORBITA/issues</a>.', f'¿Otra cosa? Abre un issue en <a href="{REPO}/issues">github.com/drophlex/ORBITA/issues</a>.'))}''')
    # 17 credits
    page(T('Credits and licence', 'Créditos y licencia'), f'''{kick('15 · ' + T('Credits', 'Créditos'))}<h1>{T('Made in Mérida', 'Hecho en Mérida')}</h1>
<p class="lead">ÓRBITA · Dark Funk Durcal {T('design', 'diseño')} · {T('built at', 'construido en')} <b>Polaris Sound Lab</b>, Mérida, Yucatán, México.</p>
<div class="grid2"><div class="card"><h3>{T('Follow', 'Síguenos')}</h3><p><a href="{IG}">Instagram @polaris.sound.lab</a><br><a href="{YT}">YouTube @PolarisSoundLab</a></p></div>
<div class="card"><h3>{T('Support it', 'Apóyalo')}</h3><p>{T('ÓRBITA is free. If it is useful to you, a small donation keeps it growing.', 'ÓRBITA es gratis. Si te sirve, un pequeño donativo lo mantiene creciendo.')}<br><a class="don" href="{DON}">♥ {T('DONATE (PayPal)', 'DONAR (PayPal)')}</a></p></div></div>
{table([T('Part', 'Parte'), T('Licence', 'Licencia')], [(T('ÓRBITA (web, plug-in, FM-1 edition, fm1-dock module)', 'ÓRBITA (web, plug-in, edición FM-1, módulo fm1-dock)'), 'GPL-3.0'),
        ('JUCE 8 (' + T('plug-in framework', 'marco del plug-in') + ')', T('its own terms (AGPL-3.0 / commercial)', 'sus propios términos (AGPL-3.0 / comercial)')), (T('DX7 voice format', 'Formato de voz DX7'), T('documented by the DX7Dump project', 'documentado por el proyecto DX7Dump'))])}
<p>{T(f'Source, releases and issues: <a href="{REPO}">github.com/drophlex/ORBITA</a> · Web: <a href="{WEB}">drophlex.github.io/ORBITA</a> · Guide: <a href="{GUIDE}">drophlex.github.io/ORBITA/guide</a>', f'Código, versiones e issues: <a href="{REPO}">github.com/drophlex/ORBITA</a> · Web: <a href="{WEB}">drophlex.github.io/ORBITA</a> · Guía: <a href="{GUIDE}">drophlex.github.io/ORBITA/guide</a>')}</p>
<p class="small">{T('M-VAVE and FM-1 are trademarks of their owners; Ableton and Live of Ableton AG; Yamaha and DX7 of Yamaha Corporation. ÓRBITA is not affiliated with or endorsed by them.', 'M-VAVE y FM-1 son marcas de sus dueños; Ableton y Live de Ableton AG; Yamaha y DX7 de Yamaha Corporation. ÓRBITA no está afiliado ni respaldado por ellos.')}</p>''')

def render(lang):
    global L; L = lang; build()
    css = open(os.path.join(HERE, 'orbita.css')).read()
    out = [f'<!doctype html><html lang="{lang}"><meta charset="utf-8"><style>{css}</style><body>']
    for i, (cls, body, title) in enumerate(PAGES):
        foot = '' if cls == 'coverp' else f'<footer><span>ÓRBITA · {T("USER MANUAL", "MANUAL DE USUARIO")}</span><span>{title}</span><span>{i + 1}</span></footer>'
        out.append(f'<section class="page {cls}">{body}{foot}</section>')
    return ''.join(out) + '</body></html>'

if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    for lang in ('en', 'es'): open(os.path.join(OUT, f'manual_{lang}.html'), 'w').write(render(lang))
    # both: the English pages then the Spanish ones, one cover
    L = 'both'; build(); cover = PAGES[0]
    en = render('en'); es = render('es')
    a = en.split('<section class="page ', 1); b = es.split('</section>', 1)[1]
    both = en.replace('ENGLISH EDITION', 'ENGLISH · ESPAÑOL').replace('</body></html>', '') + b
    open(os.path.join(OUT, 'manual_en_es.html'), 'w').write(both)
    print('ok', len(PAGES), 'pages per language')
