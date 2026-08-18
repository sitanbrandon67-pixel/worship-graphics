WORSHIP GRAPHICS v0.5.1 - PERSISTENT BIBLE TEMPLATE

Replace only:
- src/template-library.hpp
- src/template-library.cpp
- src/app-state.cpp
- src/operator-page.cpp
- src/design-page.cpp
- buildspec.json

Behavior:
- A Bible template is a reusable mold, never a frozen verse.
- Saving a valid Bible template automatically makes it the active Bible template.
- The saved .wgtpl stores {{VERSICULO}} and {{REFERENCIA}} as bindings/placeholders.
- Every new passage clones the COMPLETE saved project.
- Only the text content of the two bound layers is replaced.
- Position, bounding boxes, width/height, base/min font size, max lines, auto-fit, wrapping, alignment, colors, opacity, rotation, images, shapes, animation presets, delays and durations remain exactly as designed.
- Invalid Bible templates cannot be saved as Bible templates unless both fields exist.
- Format version bumped from template v2 to v3, while loading old templates remains compatible.

Expected installer:
WorshipGraphics-Setup-0.5.1.exe
