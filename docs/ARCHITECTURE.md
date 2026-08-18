# Worship Graphics architecture

## Core
- `AppState`: single in-process state for Preview/Program frames and current design.
- `GraphicsRenderer`: renders proportional 1920x1080 canvas using Qt/QPainter.
- `OutputSource`: native libobs source named `Worship Graphics Output`.
- `MainDock`: native OBS dock with separate Operator and Design modes.

## v0.1 working features
- Native OBS module; no WebSocket, local server or browser source.
- Modern Qt UI.
- Operator/Design split.
- Preview/Program workflow.
- Auto-created `Worship Graphics Output` in the current scene after OBS finishes loading.
- Basic layer model with groups, shapes and editable text.
- Add text/shape layers.
- Position, size, font size and entry animation controls.
- Basic Fade/Slide/Zoom rendering path.
- 1920x1080 logical canvas designed to scale with OBS transforms/resolution.

## Modules intentionally isolated for next phases
- `PsdImporter`: PSD/PSB, layers/groups and raster fallback for unsupported Photoshop effects.
- `BibleEngine`: RVR1960/NVI/NTV/TLA package architecture, offline search and reference parser.
- `TemplateLibrary`: thumbnails, folders, favorites and `.wgtpl` persistence.
- `ServiceManager`: prepared services, history and next/previous workflow.
- `AnimationEngine`: timeline/keyframes and custom animation presets.
