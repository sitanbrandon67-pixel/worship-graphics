WORSHIP GRAPHICS v0.5.0 - DIRECT CANVAS + SIMPLE OPERATOR

REEMPLAZA / AGREGA:
- CMakeLists.txt
- buildspec.json
- src/app-state.cpp
- src/bible-engine.hpp
- src/bible-engine.cpp
- src/design-page.hpp
- src/design-page.cpp
- src/design-canvas-widget.hpp   [NUEVO]
- src/design-canvas-widget.cpp   [NUEVO]
- src/main-dock.cpp
- src/operator-page.hpp
- src/operator-page.cpp
- src/template-library.hpp
- src/template-library.cpp
- src/theme.cpp

NO TOCAR:
- src/output-source.cpp
- src/graphics-renderer.cpp
- src/timeline-widget.cpp
- src/model.hpp

CAMBIOS PRINCIPALES

1. CANVAS TIPO PHOTOSHOP
- Clic sobre una capa para seleccionarla.
- Arrastra dentro de la capa para moverla.
- 8 tiradores de transformación para cambiar ancho/alto.
- Las capas de texto muestran su caja delimitadora real.
- Al redimensionar la caja, Smart Text vuelve a acomodar el texto dentro del marco.
- Flechas del teclado mueven 1 px lógico; Shift + flecha mueve 10 px.
- El canvas visual siempre conserva la relación real del proyecto.
- El panel puede hacerse pequeño sin cambiar la composición.

2. CANVAS SINCRONIZADO CON OBS
- Al cargar un proyecto/plantilla se consulta el base canvas activo de OBS.
- La composición se escala proporcionalmente a ese ancho/alto.
- Si OBS usa 1920x1080, el proyecto permanece 1920x1080.
- Si OBS usa 1280x720 o 3840x2160, posiciones, tamaños y tipografía se adaptan.

3. INTERFAZ MÁS COMPACTA
- Tipografía general reducida.
- Botones y campos más pequeños.
- Botones de acciones convertidos a iconos/glyphs (papelera, abrir, guardar, flechas, etc.).
- Diseño mínimo más pequeño para caber mejor en pantallas 16:9.

4. BIBLIOTECA MÁS SIMPLE
- Un clic selecciona.
- Doble clic abre en Diseño.
- ★ marca únicamente la plantilla bíblica predeterminada.
- Papelera elimina una plantilla personal u oculta una integrada.
- Restaurar recupera integradas ocultas.
- La plantilla bíblica ★ queda persistida y Operador la usa directamente.

5. SERVICIO SIMPLIFICADO
- Se eliminó el sistema confuso Tipo + Plantilla.
- El Servicio inicia VACÍO.
- Arriba aparece una lista directa de plantillas no bíblicas.
- Un clic prepara/visualiza la plantilla.
- Doble clic o + A SERVICIO la agrega.
- Abajo solo queda la cola real del servicio con subir/bajar/eliminar.
- Ya no se agregan Pastor/Versículo/Tema/Alabanza/Anuncio automáticamente.

6. BIBLIA SIMPLIFICADA
- Se eliminó PREPARAR.
- Solo queda buscar y + A SERVICIO.
- La plantilla bíblica elegida con ★ en Diseño se usa automáticamente.
- Barra de búsqueda con sugerencias de libros y abreviaturas.
- Ejemplos aceptados: deu 6:4, deu 6 4, sal 23, juan 3:16.
- Escribir "deu" ofrece/completa "Deuteronomio".
- Enter puede completar el libro; después de escribir los números, Enter carga el pasaje.
- Flechas ← → navegan versículos.
- Al encontrar un pasaje se prepara visualmente con la plantilla bíblica ★ y + A SERVICIO lo añade a la cola.

COMMIT SUGERIDO:
Worship Graphics v0.5.0 direct canvas simple operator

INSTALADOR ESPERADO:
WorshipGraphics-Setup-0.5.0.exe
