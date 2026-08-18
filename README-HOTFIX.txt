WORSHIP GRAPHICS v0.3.9 - COMPILE HOTFIX

Replace only:
- src/operator-page.cpp
- buildspec.json

Fix:
v0.3.8 declared templateName as const QString and then called QString::replace(),
which is a mutating method. This causes MSVC compilation to fail.

No other Worship Graphics functionality is changed.
Template routing, deletion, responsive UI, Smart Text, Timeline and OBS output
remain as in v0.3.8.

Commit suggestion:
Fix v0.3.8 QString compile error

Expected installer:
WorshipGraphics-Setup-0.3.9.exe
