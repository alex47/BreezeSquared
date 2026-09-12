# Decoration regression tests

These tests exercise the compiled decoration through KDecoration3 with synthetic
windows and settings. Qt runs with the offscreen platform; the test creates no
windows, starts no GUI apps, captures no desktop images, and changes no session
settings. Temporary XDG configuration/cache/data directories and an unavailable
session-bus address isolate it from the desktop.

## Run from a clean checkout

Install the development dependencies listed in [BUILDING.md](../BUILDING.md),
plus Python 3.12 or newer, Git, CMake, Ninja, and the DejaVu Sans font (on Ubuntu,
`fonts-dejavu-core`). KDecoration3's private development target is needed by the
synthetic bridge. A complete clone must contain the tag and commit recorded in
[upstream-base.json](../upstream-base.json).

For a new checkout, clone the fork and add KDE as the upstream remote:

```sh
git clone --branch breezesquared https://github.com/alex47/BreezeSquared.git
cd BreezeSquared
git remote add upstream https://invent.kde.org/plasma/breeze.git
git config remote.pushDefault origin
```

From the repository root:

```sh
python3 tests/run_tests.py
```

The script can also be invoked by its absolute path from any directory. It:

1. Checks that the upstream tag resolves to its recorded commit.
2. Extracts that commit from local Git objects into the ignored test build area.
3. Configures and builds the fork decoration, settings module, and test executable
   with system libraries, then runs the CTest matrix.
4. Builds the original upstream decoration with the same libraries and compares
   its title-bar geometry and rendered icon fingerprints with the fork.

No existing `build/` files or KDE Builder installation are needed for this test
command. It performs no package installation, network fetch, plugin installation,
or desktop activation. Missing dependencies or Git objects produce a failure;
obtain them before retrying. Reference extraction/build output is cached by commit
under the build directory; use a new directory when a fully fresh build is needed.

```sh
python3 tests/run_tests.py --build-dir build/regression-clean --jobs 4
```

For a proposed upgrade, `--upstream-ref` selects a different **local** Git tag or
commit for comparison without changing the recorded base. The reference target
and plugin path still come from `upstream-base.json` and must match that release.

## Faster local checks

After the full runner has configured `build/regression/current`:

```sh
cmake --build build/regression/current --parallel 4 \
  --target breezesquareddecoration kcm_breezesquareddecoration breezesquared_geometry_test
ctest --test-dir build/regression/current --output-on-failure -R '^breezesquared\.'
```

CTest alone runs the fork's behavior checks. The Python runner additionally
compares with upstream. `./build.sh` retains its build-only KDE Builder workflow
with `BUILD_TESTING=OFF`; it does not run this suite.

## Coverage

Each CTest case spans 3 font sizes, 4 scales (1, 1.25, 1.5, 2), 2 border settings
(None/Tiny), and 6 states (normal, maximized, three screen-edge combinations, and
shaded): 144 configurations.

Five cases cover all button-size presets. Five further cases cover disabled alpha
support, borders on maximized windows, gradients, a different rightmost button,
and inactive windows. Total: **1,440 configurations**. The five size cases also
provide **720 comparisons** against the recorded upstream release.

Checks cover:

- Full-height square button geometry and zero automatic group spacing.
- Right-edge alignment, adjoining boundary hover, top/bottom/side clicks, action
  dispatch, cancellation after release outside, and cursor sections.
- Window-width and title-bar-height changes.
- Rounded-corner hover/press membership and agreement with fully opaque or fully
  transparent raster samples. Partially covered antialiased pixels are not
  treated as binary geometry.
- Identical outline alpha when idle, hovered, and pressed; preservation of an
  existing backdrop outside the outline; translated partial repaints.
- Upstream title-bar height/interaction rectangle and original rendered symbols,
  compared in separate processes with identical settings and fonts. Snapshot
  hashes are generated during each run, not stored as platform-specific fixtures.

## Results and limits

The default output is `build/regression/summary.json`, with per-case reports in
`build/regression/current/tests/reports/` and upstream reports alongside the
summary. Reports include the upstream commit and whether the tested working tree
was dirty. A failed run removes the previous success summary and returns nonzero.
Logs and compiled artifacts are not committed.

The harness depends on KDecoration3's private bridge interfaces and must be
reviewed when those APIs change. It does not exercise a running KWin, plugin/KCM
discovery, installation, real multi-monitor transitions, arbitrary user layouts,
all button types, or the timing of animations. The automated font/scale/border
matrix is representative, not exhaustive.

After an authorized install/preview, manually check normal and maximized windows,
tiling, shading, screen-edge pointing, dragging/resizing outside buttons, menu
icons, explicit layout spacers, reordered/hidden buttons, animation transitions,
and settings persistence. Check that original Breeze remains independently
selectable. Live desktop checks require user participation or explicit approval
to operate GUI apps; the automated suite does not perform them.
