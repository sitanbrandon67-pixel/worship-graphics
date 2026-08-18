Worship Graphics v0.3.1 OUTPUT HOTFIX

Replace these files in the repository:
- src/output-source.hpp
- src/output-source.cpp
- src/plugin-main.cpp
- buildspec.json

What this fixes:
- Worship Graphics Output is now ensured in the active OBS Program scene.
- In Studio Mode it is also ensured in the current Preview scene.
- The overlay is re-attached when Program/Preview scenes change.
- The overlay is forced visible, locked and moved to the top of the scene.
- Existing output sources are reused instead of causing an early return.

After replacing the files, commit to main and let GitHub Actions build
WorshipGraphics-Setup-0.3.1.exe.
