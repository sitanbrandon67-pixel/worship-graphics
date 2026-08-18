WORSHIP GRAPHICS v0.6.3 - PSD C BRIDGE + NEW CANVAS

Esta versión mantiene juntos:
- Importador PSD/PSB
- Nuevo Canvas desde cero
- Iconos sofisticados

Cambio técnico principal:
OpenPSD se mantiene 100% C y C++ ya no incluye <openpsd/psd.h>.
La comunicación se hace mediante:
- src/psd-bridge.c
- src/psd-bridge.h

Esto evita que los tipos C de OpenPSD (_Bool, APIs mutables, etc.)
entren directamente al compilador C++ de MSVC.

Reemplazar/agregar:
- CMakeLists.txt
- buildspec.json
- src/design-page.cpp
- src/design-page.hpp
- src/psd-importer.cpp
- src/psd-importer.hpp
- src/psd-bridge.c       NUEVO
- src/psd-bridge.h       NUEVO

No tocar:
- src/output-source.cpp
- src/app-state.cpp
- src/operator-page.cpp
- src/timeline-widget.cpp
- src/graphics-renderer.cpp
- src/model.hpp

Commit:
Worship Graphics v0.6.3 PSD C bridge fix

Instalador esperado:
WorshipGraphics-Setup-0.6.3.exe
