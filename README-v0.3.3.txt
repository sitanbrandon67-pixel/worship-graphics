WORSHIP GRAPHICS v0.3.3 - OBS RENDER FIX

Replace:
  src/output-source.cpp
  buildspec.json

Fixes the case where the lower third is visible in the Worship Graphics
PROGRAM monitor but not visible in OBS Program or the stream.

Changes:
- Uses the gs_effect_t supplied by OBS.
- Uses gs_effect_set_texture_srgb.
- Uses premultiplied-alpha blending: GS_BLEND_ONE / GS_BLEND_INVSRCALPHA.
- Draws the sprite directly instead of opening a second effect loop.
- Resets position, scale, rotation and crop of the OBS scene item.
- Keeps Worship Graphics Output visible and on top.

Expected installer:
  WorshipGraphics-Setup-0.3.3.exe
