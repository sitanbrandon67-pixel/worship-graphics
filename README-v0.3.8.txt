WORSHIP GRAPHICS v0.3.8 - TEMPLATE ROUTING

Replace:
- src/template-library.hpp
- src/template-library.cpp
- src/design-page.cpp
- src/operator-page.hpp
- src/operator-page.cpp
- buildspec.json

Features:
- In Design > Template Library: select a template and assign it as the active/default template for Pastor, Scripture, Sermon, Worship or Announcement.
- User templates can be deleted from the library. Built-in templates are protected.
- Default template choices persist using QSettings.
- Operator > Service now has a Template selector for every service item.
- The selected template is snapshotted into the prepared service item, so later default changes do not alter already-prepared items.
- Bible tab has its own Bible Template selector.
- Scripture lists only templates marked as BibleText.
- Existing responsive layout, timeline, mini operator and OBS output render fix are preserved.

Expected installer:
WorshipGraphics-Setup-0.3.8.exe
