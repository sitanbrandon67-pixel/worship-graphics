WORSHIP GRAPHICS v0.3.5 - COMPILE FIX + SMART TEXT

This patch repairs the v0.3.4 Smart Text merge without removing existing features.

REPLACE ONLY THESE FILES:

src/model.hpp
src/app-state.hpp
src/app-state.cpp
src/graphics-renderer.hpp
src/graphics-renderer.cpp
src/template-library.hpp
src/template-library.cpp
src/template-factory.hpp
src/template-factory.cpp
src/design-page.hpp
src/design-page.cpp
src/operator-page.cpp
buildspec.json

DO NOT replace output-source.cpp. Keep the v0.3.3 OBS Render Fix already in the repo.
DO NOT replace the GitHub Actions workflow.

FIXES:
- Restores AppState timelineChanged, renderPreviewAtTime, timelineDuration, setLayerTiming and scaleTimeline.
- Restores enterDurationMs and exitDurationMs used by the interactive Timeline.
- Restores sermonTitleLowerThird, worshipLowerThird and announcementLowerThird used by Operator.
- Keeps PREPARED and PROGRAM as separate projects.
- Keeps the Qt layerSelected lambda hotfix.
- Adds Smart Text: base/min font size, max lines, auto-fit, wrap, overflow split, H/V text alignment.
- Adds layer alignment left/center/right/top/middle/bottom.
- Adds Bible template designation and {{VERSICULO}} / {{REFERENCIA}} field buttons.
- Saved .wgtpl templates persist Smart Text settings.
- Operator uses the currently designated Bible template when preparing or adding Bible passages.

EXPECTED INSTALLER:
WorshipGraphics-Setup-0.3.5.exe
