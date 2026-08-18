WORSHIP GRAPHICS v0.4.1 - COMPILE HOTFIX

Replace only:
- src/design-page.cpp
- buildspec.json

Compile fix:
Two UI labels in v0.4.0 were accidentally written as literal source-code line breaks:

  b.name + "
  INTEGRADA"

  entry.name + "
  MÍA"

They are now valid C++ escaped newlines:

  b.name + "\nINTEGRADA"
  entry.name + "\nMÍA"

No template-library behavior is removed.
Selection, default-template assignment, delete/hide, restore built-ins,
Operator template routing and all previous features remain.

Expected installer:
WorshipGraphics-Setup-0.4.1.exe
