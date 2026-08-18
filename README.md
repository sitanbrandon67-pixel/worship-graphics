# Worship Graphics 0.1 Foundation

Worship Graphics is a native OBS Studio plugin for church lower thirds, sermon graphics and scripture workflows. It runs inside OBS and does **not** use obs-websocket, a local web server or a browser source.

## What is implemented in this foundation

- Native OBS source: `Worship Graphics Output`.
- Native dock: `Docks -> Worship Graphics`.
- Two UI modes: **Operator** and **Design**.
- Preview / Program workflow with visible buttons.
- Basic layer model: groups, text and shapes.
- Editable text, position, size, font size and entry preset.
- Basic Fade / Slide / Zoom animation renderer.
- Demo lower third with `{{NOMBRE}}` and `{{CARGO}}` fields.
- Automatic creation of `Worship Graphics Output` in the current scene on first load.
- Scalable logical 1920x1080 canvas.
- Module boundaries for PSD, Bible, templates, services and advanced animation.

## Important scope note

This is the first compilable foundation, not the finished 1.0 product. PSD parsing, licensed Bible text packages, template persistence, image layers and the full keyframe timeline are the next modules to implement.

## Build target

The code uses the current OBS native APIs (`libobs` + `obs-frontend-api`) and Qt 6. The dock API used by the plugin exists in OBS 30+.

### Windows prerequisites

- Windows 10/11 x64
- Visual Studio 2022 with C++ desktop workload
- CMake 3.28+
- An OBS development prefix/build exposing CMake packages for `libobs`, `obs-frontend-api`, and the Qt 6 used by OBS

### Build

```powershell
.\scripts\build-windows.ps1 -ObsPrefix "C:\path\to\obs-development-prefix"
```

Or manually:

```powershell
cmake -S . -B build_x64 -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\path\to\obs-development-prefix"
cmake --build build_x64 --config RelWithDebInfo
cmake --install build_x64 --config RelWithDebInfo
```

The install tree will contain the OBS plugin layout:

```text
WorshipGraphics/
  obs-plugins/64bit/worship-graphics.dll
  data/obs-plugins/worship-graphics/...
```

Copy those two folders into the matching OBS installation folders for development testing.

### Installer

If NSIS is installed, CPack can create an `.exe` package after a successful configure/build:

```powershell
.\scripts\package-windows.ps1
```

## Next implementation order

1. `.wgtpl` template save/load and thumbnail library.
2. True layer tree with drag/reorder, visibility, lock and nested groups.
3. PSD/PSB importer with raster fallback.
4. Advanced animation timeline/keyframes and reusable presets.
5. Bible package manager + reference parser + offline search.
6. Prepared Service manager and history.
7. Resolution/aspect-ratio adaptation rules and safe areas.
8. Signed Windows installer and compatibility test matrix.

## GitHub Actions Windows build

This package includes `.github/workflows/build-windows.yml`.
After uploading the project to GitHub, open **Actions → Build Worship Graphics Windows → Run workflow**.
The workflow downloads the official OBS plugin build infrastructure, compiles with MSVC on `windows-2022`, builds an NSIS installer, and uploads the resulting artifact.

The current CI bootstrap uses the official OBS plugin-template dependency set based on OBS 31.1.1 as a conservative compatibility baseline. The next compatibility pass will move the buildspec to the current OBS branch after the first clean Windows build is established.
