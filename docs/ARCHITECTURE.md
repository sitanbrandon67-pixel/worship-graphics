# Worship Graphics 0.2 architecture

## Core
- `AppState`: project, Preview/Program frames, layer operations and stagger animation.
- `GraphicsRenderer`: transparent 1920x1080 renderer with per-layer delayed animation.
- `OutputSource`: native libobs source `Worship Graphics Output`.
- `MainDock`: custom Qt interface with Operator and Design modes.

## Motion
- Each layer stores entry/exit preset, delay and duration.
- Renderer derives local progress from the global transition.
- `TimelineWidget` visualizes the first layers and their timing.
- `Motion Pieces` is a built-in multi-piece lower third designed to demonstrate staggered broadcast motion.

## Bible
- `BibleEngine` imports and parses the user's local XML.
- Supports reference search, selectors and adjacent verse navigation.
- Bible content is not bundled in the distributable source tree.

## Templates
- `TemplateFactory` provides built-in templates.
- `TemplateLibrary` persists `.wgtpl` JSON templates and thumbnail PNGs under the local app data directory.

## Next modules
- PSD/PSB layered importer.
- Full arbitrary keyframe graph/timeline.
- Prepared service persistence/history.
