WORSHIP GRAPHICS v0.3.2
SERVICE PREPARED + COMPACT OPERATOR + OUTPUT HOTFIX

Replace these files in your repository:

src/app-state.hpp
src/app-state.cpp
src/operator-page.hpp
src/operator-page.cpp
src/template-factory.hpp
src/template-factory.cpp
src/main-dock.cpp
src/theme.cpp
src/output-source.hpp
src/output-source.cpp
src/plugin-main.cpp
buildspec.json

WHAT CHANGES

1. Servicio preparado is now functional:
   - selecting an entry really loads its graphic
   - Previous/Next navigate the real service queue
   - Add Pastor, Scripture, Sermon, Worship, Announcement, or Current Design
   - Delete entries
   - Move entries up/down
   - Add current Bible passage directly to the service

2. PREPARED and PROGRAM are independent:
   - selecting/preparing the next graphic does not change the graphic already on air
   - PROGRAM keeps a snapshot until Hide or Send another graphic

3. Operator is compact:
   - small Program monitor
   - main live controls
   - SERVICIO/BIBLIA tabs instead of two large columns
   - operator minimum dock size is 470x520
   - Design mode still requests 1120x720

4. OBS output hotfix included:
   - Worship Graphics Output is ensured in current Program scene
   - and in Preview scene while Studio Mode is active
   - moved to top and kept visible

Expected installer:
WorshipGraphics-Setup-0.3.2.exe
