WORSHIP GRAPHICS v0.6.2 - PSD + NEW CANVAS COMBINED FIX

Esta actualización une en un solo paquete:
- Importador PSD/PSB real
- Crear canvas desde cero
- Iconos lineales sofisticados
- Correcciones de compilación del importador OpenPSD para C++/MSVC

Reemplazar/agregar:
- CMakeLists.txt
- buildspec.json
- src/design-page.cpp
- src/design-page.hpp
- src/psd-importer.cpp
- src/psd-importer.hpp

Correcciones PSD incluidas:
1. Las APIs lazy de OpenPSD ahora reciben psd_document_t* no-const como exige la API.
2. Se corrige la incompatibilidad Qt6 qsizetype/int en el conteo de líneas.
3. Se añade compatibilidad MSVC para _Bool del header C de OpenPSD.
4. El checkout de OpenPSD ya no usa shallow clone con un commit SHA fijo.
5. OpenPSD se fuerza a compilar como C17 en MSVC.

No reemplazar:
- src/output-source.cpp
- src/app-state.cpp
- src/operator-page.cpp
- src/timeline-widget.cpp
- src/graphics-renderer.cpp
- src/model.hpp

Commit sugerido:
Worship Graphics v0.6.2 PSD and new canvas compile fix

Instalador esperado:
WorshipGraphics-Setup-0.6.2.exe
