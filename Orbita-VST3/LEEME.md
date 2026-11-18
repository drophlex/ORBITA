# Polaris Órbita · instrumento VST3 para Apple Silicon

Proyecto fuente, versión 0.1. Requiere macOS en un Mac M1 o posterior, herramientas de línea de comandos de Xcode, CMake 3.22 o posterior y conexión para descargar JUCE 8.0.6. Este paquete incluye el proyecto y su instalador de compilación.

1. Descomprime la carpeta y cierra Live. Instala las herramientas de Apple con `xcode-select --install` si es la primera vez que compilas software. Puedes instalar CMake desde https://cmake.org/download/.
2. Abre Terminal, escribe `bash ` con un espacio, arrastra el archivo **Construir-en-Mac.command** a Terminal y pulsa Retorno. El script descarga JUCE, construye para arm64, firma localmente e instala el VST3 en tu carpeta de usuario.
3. En Live nativo para Apple Silicon, abre Ajustes > Plug-ins. Activa la carpeta de sistema VST3 y vuelve a escanear. Arrastra **Polaris Orbita** desde Plug-ins a una pista MIDI.

## Primera escucha

Toca un acorde menor. Sube **Índice FM** para añadir parciales, mueve **Ratio FM** de 2 a 2.37 para pasar de armónicos enteros a un timbre metálico y automatiza **Fase** para desplazar el modulador. La figura del panel sigue esos parámetros. La entrada MIDI tiene doce voces.

Para escuchar un patrón sin dibujar notas, activa **Secuencia con Live** y arranca el transporte de Live. **Pulsos / 16** reparte el número de ataques elegidos en dieciséis semicorcheas. Con cinco pulsos, la secuencia toca las posiciones 1, 5, 8, 11 y 14. **Nota raíz MIDI** transpone la frase pentatónica menor. La posición musical y el tempo siguen al proyecto, incluidos los saltos de reproducción. Los parámetros se guardan en el set.

## Compilación y pruebas

El núcleo DSP se puede probar sin JUCE con `c++ -std=c++17 test.cpp -o test-orbita && ./test-orbita`. La compilación final y la carga en Ableton Live deben verificarse en macOS. La carpeta no contiene un VST3 precompilado. JUCE se obtiene de su repositorio oficial en el proceso de compilación y conserva sus propios términos.

El selector de idioma del editor cambia las etiquetas y se guarda con la instancia del instrumento.
