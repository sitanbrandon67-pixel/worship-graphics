# Worship Graphics 0.2 — Motion + Bible

Native OBS Studio plugin for church broadcast graphics. No WebSocket, local server, or browser source.

## New in 0.2

- Soft dark premium interface inspired by modern broadcast/control surfaces.
- Real layer actions: delete, duplicate, move, visibility, lock, group and ungroup.
- Image layers (PNG/JPG/WebP).
- Built-in visual template library with thumbnails.
- Save custom `.wgtpl` templates + generated thumbnails.
- `Motion Pieces` lower-third template inspired by the supplied animated reference.
- Per-layer entry/exit preset, delay and duration.
- Stagger tool (`80 ms`) and visual Motion Timeline.
- Extra motion presets: Pop, horizontal/vertical expand.
- Offline Bible engine with fast reference parser (`Juan 3:16`, `Jn 3 16`, `Salmos 23`).
- Book/chapter/verse selectors.
- Previous/next verse navigation.
- Import a local RVR1960 XML once and use it offline.
- Scripture fields `{{VERSICULO}}` and `{{REFERENCIA}}` feed directly into Preview/Program.

## Bible data

Bible text files are not bundled in this repository/package. The operator imports a local XML file they are authorized to use; Worship Graphics stores a local copy for offline operation.

## Still scheduled after 0.2

- True PSD/PSB layer parser with smart raster fallback.
- Arbitrary multi-property keyframes beyond the current layer timing/preset motion system.
- More installable Bible translation packages, subject to their licenses.
- Full prepared-service persistence and richer template category/favorite management.
