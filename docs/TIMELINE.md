# Interactive Motion Timeline

The Design timeline is an editable time sequencer inspired by motion-graphics editors.

## Scrubbing and preview
Drag the vertical playhead or click the ruler to render that exact moment into the Design canvas. Playback does not affect PROGRAM.

## Layer timing
Each non-group layer has a timing bar. The two circular handles define start and end. Drag:
- start handle: trims the start while keeping the current end time,
- end handle: changes animation duration,
- center of bar: moves the complete animation in time.

Entry and exit have independent delay and duration values.

## Time stretching
`½ TIEMPO` multiplies every delay/duration in the active phase by 0.5. `×2 TIEMPO` multiplies them by 2.0.

## Operator safety
Operator mode intentionally displays only PROGRAM. The application still maintains an internal prepared frame used by `ENVIAR AL AIRE`, preventing design or Bible preparation from immediately changing the live output.
