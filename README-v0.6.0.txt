WORSHIP GRAPHICS v0.6.0 - REAL PSD/PSB IMPORTER

Replace/add:
- CMakeLists.txt
- buildspec.json
- src/design-page.cpp
- src/design-page.hpp
- src/psd-importer.cpp   (NEW)
- src/psd-importer.hpp   (NEW)

What changes:
- IMPORTAR PSD is no longer a placeholder.
- Native PSD/PSB parsing using OpenPSD, statically linked into the plugin.
- PSD canvas dimensions are preserved.
- Group structure is read.
- Text layers become editable Worship Graphics text layers.
- Text content, base font, color, basic justification, bounds and opacity are imported when available.
- Pixel layers are exported internally to PNG and remain independent movable image layers.
- Advanced/non-pixel layers are rasterized when OpenPSD can render them; otherwise a warning lists what was omitted.
- PSD text layers named {{VERSICULO}} and {{REFERENCIA}} automatically make the imported project a Bible template.

No browser, obs-websocket, local server or runtime helper is used.

Build note:
OpenPSD source is fetched at build time from a pinned Git commit and compiled statically into worship-graphics.dll.
The installed plugin does not need OpenPSD installed separately.

Expected installer:
WorshipGraphics-Setup-0.6.0.exe
