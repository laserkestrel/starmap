## Copilot / AI Agent Instructions for Starmap

Purpose: Quickly orient an AI coding agent to become productive in this repository.

- **Big picture:** Starmap is a small C++ simulation that loads star data and runs a probe-exploration simulation. Core runtime lives in `src/` and configuration/data live in `content/`.

- **Major components:**
  - **Simulation / game loop:** `src/Game.cpp` drives the simulation and rendering.
  - **Config / singletons:** `src/LoadConfig.*` provides a global config via `LoadConfig::getInstance()` used in `src/Main.cpp`.
  - **Data loading:** `src/LoadCSVData.*` and `content/generate_star_data.py` / `content/star_data.json` hold the star datasets.
  - **Spatial index:** `src/GalaxyQuadTree*.cpp/h` implements the quadtree used for neighbor searches.
  - **Platform abstraction:** `src/Platform/` contains per-OS implementations; modify `Win32`, `Unix`, or `MacOS` subfolders only when OS-specific behavior needed.

- **Key files to inspect when changing behavior:**
  - [src/Main.cpp](src/Main.cpp#L1-L40) — app entry and how `LoadConfig` is used.
  - [src/LoadConfig.h](src/LoadConfig.h) / [src/LoadConfig.cpp] — config keys and defaults (see `content/config.json`).
  - [src/Game.cpp](src/Game.cpp#L1-L200) — simulation loop, input handling, and how probes/Stars are updated.
  - [content/generate_star_data.py](content/generate_star_data.py) — how star JSON/CSV is produced from `hygdata_v40.csv`.

- **Build & run (developer workflows):**
  - Preferred: use CMake. For Windows (multi-config):

    cmake -S . -B build
    cmake --build build --config Release

  - For quick local compile (single-file testing) the repo includes a g++ task; see VS Code tasks. To build then copy runtime assets, run the provided VS Code task `CMake Build And Copy Resources` or invoke the two-step tasks `CMake Build` then `Copy Resources`.

- **Runtime assets:** The `content/` directory must be copied next to the built binary (see task `Copy Resources`). Assets include `config.json`, `star_data.json`, fonts, and CSVs. Tests or runs without copying assets will fail to find `content/config.json`.

- **Project-specific conventions & patterns:**
  - `LoadConfig` is a global singleton — update defaults here when adding new config keys. `Main.cpp` expects `LoadConfig::getInstance()` with no parameter.
  - Star datasets are precomputed into `content/star_data.json`. If changing star ingestion, update `generate_star_data.py` and commit the new JSON.
  - Spatial queries use `GalaxyQuadTree*` classes; prefer updating quadtree parameters rather than performing O(N^2) searches for performance-sensitive changes.

- **Platform changes:** Keep platform-specific code in `src/Platform/*`. Prefer adding cross-platform hooks in `Platform.hpp` / `IPlatform.hpp` rather than scattering #ifdefs elsewhere.

- **Debugging & profiling hints from README:**
  - Check binaries for debug symbols: `nm -C starmap | grep ' [BD] '` (Linux/macOS).
  - Use `valgrind --tool=callgrind ./starmap` and visualize with `kcachegrind` for hotspots.

- **When modifying code, include:**
  - A reference to the concrete file(s) you changed (use repo-relative paths).
  - If behavior depends on config keys, add defaults in `LoadConfig` and update `content/config.json` and README if needed.
  - If data format changes (star JSON/CSV), update `content/generate_star_data.py` and commit sample output to `content/`.

If any section is unclear or you want more examples (e.g., common edit + build + run cycle on Windows), say which area and I will expand.
