WORSHIP GRAPHICS v0.3.4 - SMART TEXT BIBLE LAYOUT

Replace these files in your repository:

src/model.hpp
src/graphics-renderer.hpp
src/graphics-renderer.cpp
src/template-library.hpp
src/template-library.cpp
src/template-factory.hpp
src/template-factory.cpp
src/app-state.hpp
src/app-state.cpp
src/design-page.hpp
src/design-page.cpp
buildspec.json

What this adds:
- Bible template designation from Design mode.
- Mark current text layer as {{VERSICULO}} or {{REFERENCIA}}.
- Smart text boxes that respect their bounds.
- Base font size and minimum font size per text layer.
- Auto-fit text inside the box.
- Auto wrap / split into two lines when needed.
- Horizontal text alignment: left / center / right.
- Vertical text alignment: top / center / bottom.
- Layer alignment in canvas: left / center / right / top / center / bottom.
- Saved templates persist the new text settings.

Workflow:
1. In Diseño, create or open a template.
2. Check "Plantilla bíblica".
3. Select a text layer and click "CAMPO VERSÍCULO".
4. Select another text layer and click "CAMPO REFERENCIA".
5. Set font size, min font size, max lines, alignment and auto-fit.
6. Save the template.
7. In Biblia / Operador, when a verse is loaded, the current designated bible template can receive {{VERSICULO}} and {{REFERENCIA}}.

Expected installer:
WorshipGraphics-Setup-0.3.4.exe
