WORSHIP GRAPHICS v0.4.0 - TEMPLATE LIBRARY FIX

Replace:
- src/template-library.hpp
- src/template-library.cpp
- src/design-page.cpp
- src/operator-page.cpp
- buildspec.json

Changes:
- Single click clearly selects a template.
- Selected template label shows name and INTEGRADA/MÍA.
- USAR COMO PREDETERMINADA assigns the selected template to Pastor, Scripture, Sermon, Worship or Announcement.
- The default assignment is validated and persisted.
- Personal templates are physically deleted.
- Built-in templates can be hidden from the library/operator instead of being undeletable.
- RESTAURAR INTEGRADAS brings hidden built-ins back.
- Library now includes built-in Sermon, Worship and Announcement templates too.
- Operator respects hidden built-ins and the chosen default.
- If a type has no available template, Operator tells you instead of silently falling back to a hidden/default graphic.

Expected installer: WorshipGraphics-Setup-0.4.0.exe
