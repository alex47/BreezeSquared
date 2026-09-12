# BreezeSquared: fork changes and upstream porting

This guide describes the maintained differences from Breeze, their purpose, and
where to inspect them when upgrading. Git contains the exact patches; this guide
records the intended behavior so it can survive changes in upstream architecture.

## Upstream base

| Item | Recorded value |
| --- | --- |
| Source repository | <https://invent.kde.org/plasma/breeze.git> |
| Release tag | `v6.6.5` |
| Commit | `bfe46a06e92a196fe3051706051d5c5e2d497e92` |
| Current fork branch | `breezesquared` |

[upstream-base.json](upstream-base.json) is the machine-readable reference used by
the tests. It also records the upstream decoration target and output path. These
are source versions, not distribution package revisions or the KWin version.
The initial `origin` remote points to KDE upstream; there is no requirement for
a separate hosted fork to build or test locally.

Inspect the complete changes from the current base with:

```sh
git log --reverse --oneline v6.6.5..HEAD
git diff --find-renames --stat v6.6.5..HEAD
git diff --find-renames v6.6.5..HEAD -- kdecoration CMakeLists.txt
```

After an upgrade, substitute the newly recorded base tag. To inspect one change,
use `git show <commit>`; use `git log -p -- <path>` to trace later adjustments.

## Behavior to preserve

- Keep Breeze's title-bar height calculation. Set each ordinary button's layout
  width and height to that existing `borderTop()` value, with `y = 0`.
- Preserve upstream icons and their sizing, centering them inside the larger
  buttons. A larger hit area must not enlarge an icon or the title bar.
- Keep zero automatic spacing between adjacent buttons. Explicit spacers in the
  user's KWin layout retain their preferred width.
- Position the right button group flush with the outer window edge in every
  state. The left group deliberately retains Breeze's normal inset:
  `2 * smallSpacing + borderLeft()`. It becomes flush when `isLeftEdge()` is true.
  Removing that left inset has been discussed, but has not been implemented.
- Match button highlights, hover, and clicks to the same geometric region. A
  square at a rounded corner is limited by the title-bar outline. Antialiased
  boundary pixels have partial coverage; input is a binary geometric test.
- Preserve Breeze's corner-state rules: normal and tiled title bars retain their
  existing upper curves, shaded title bars can also have lower curves, and the
  usual maximized/no-alpha case is rectangular. Respect the option to draw
  borders on maximized windows; it changes the maximized/edge helpers.
- Preserve normal frame/resize handling outside button regions. A rounded cutout
  must not activate a button; this does not make it click through to another app.
- Keep independent BreezeSquared settings, Breeze colors and animations, and
  the existing user-configured button layout. The Windows reference used during
  brainstorming did not authorize replacing Breeze's icons or copying sizes.

## Patch history

| Commit | Purpose |
| --- | --- |
| `46a5066d` | Build only the decoration and KCM through KDE Builder; keep build state local. |
| `dec02671` | Give the fork separate plugin, configuration, UI, and CMake identities. |
| `e33acb12` | Replace circular button backgrounds with rectangles covering button geometry. |
| `8c2b8707` | Use full-height squares, remove automatic gaps, center original icons, and correct fractional input routing. |
| `065030c6` | Remove the right outer inset and preserve rounded outlines in painting and input. |

The current implementation combines these patches. Replaying only the first
rectangle patch does not reproduce the final behavior.

## Code map and reasons

### Build workflow

[build.sh](build.sh) and [kde-builder.yaml](kde-builder.yaml) select the decoration
and settings-module targets, skip updates and installation, and keep generated
output and Git URL aliases under `build/`. [BUILDING.md](BUILDING.md) documents the
dependencies and usage. The fork retains the full upstream source tree, including
unmodified styles, colors, cursors, and wallpapers; a whole-tree installation
would have a broader scope than installing BreezeSquared's two plugins.

The optional tests are registered in the Qt 6 decoration branch of
[CMakeLists.txt](CMakeLists.txt), controlled by `BUILD_TESTING`.
[tests/CMakeLists.txt](tests/CMakeLists.txt) registers the isolated test cases;
[tests/run_tests.py](tests/run_tests.py) builds the fork and upstream reference.

### Independent identity

Check these locations together when upstream renames or reorganizes plugins:

| Concern | Files and relevant details |
| --- | --- |
| CMake package | Root `CMakeLists.txt` and `BreezeSquaredConfig.cmake.in`: project/package name, version-file prefix, plugin ID export. Upstream `BreezeConfig.cmake.in` was replaced. |
| Decoration plugin | `kdecoration/CMakeLists.txt`, `breezedecoration.cpp` factory macro, and `breezesquared.json`: target `breezesquareddecoration`, output `org.kde.breezesquared`, display name and KCM reference. Upstream `breeze.json` was renamed. |
| Settings plugin | `kdecoration/config/CMakeLists.txt` and `kcm_breezesquareddecoration.{cpp,json}`: target/factory/metadata identity. The upstream `kcm_breezedecoration` files were replaced. |
| Settings storage | `kdecoration/breezesettingsdata.kcfg`, `breezesettingsprovider.cpp`, and `config/breezeconfigwidget.cpp` all use `breezesquaredrc`. |
| Settings UI | `config/breezeexceptionlistwidget.cpp` and `config/ui/breezeconfigurationui.ui` use the fork's name in window titles. JSON display names were updated without retaining stale localized Breeze names. |
| Settings notification | `ConfigWidget::save()` in `config/breezeconfigwidget.cpp` still notifies KWin. Its signal asking the separate Breeze application style to reload shadows was removed. |

The C++ namespace `Breeze`, most source filenames, and the translation domain
`breeze_kwin_deco` remain upstream names. This is intentional; broad renaming
would add porting work. See `BUILDING.md` for the installation identities.

### Button geometry and icons

In [kdecoration/breezedecoration.cpp](kdecoration/breezedecoration.cpp),
`Decoration::updateButtonsGeometry()` sets full-height square geometry, preserves
explicit spacer widths, sets both group spacings to zero, and positions the groups.
The added `bordersChanged` connection in `init()` updates geometry when the title
bar's measured height changes. The function guards against groups not yet created.

`bordersFor()`, `buttonSize()`, and `updateTitleBar()` retain the upstream size and
interaction-rectangle calculations. Do not derive `borderTop()` from the new button
square: the dependency runs from existing title-bar height to button geometry.

In [kdecoration/breezebutton.cpp](kdecoration/breezebutton.cpp), `Button::iconRect()`
centers `preferredSize()` inside the hit rectangle. Both menu icons and the normal
symbol renderer use it. `drawIcon()` fills the button geometry before painting the
original symbol. The former padding fields/setters were removed from
[breezebutton.h](kdecoration/breezebutton.h), and the unused automatic spacing
constant was removed from [breeze.h](kdecoration/breeze.h).

### Fractional coordinates and pointer routing

`Button::containsPoint()` uses half-open rectangle boundaries and the shared
title-bar outline. Half-open boundaries give an adjoining edge one owner.

`Button::event()` maps press, release, and movement into or outside KDecoration's
button rectangle while retaining its action/pressed-state machinery.
`Decoration::hoverMoveEvent()` similarly maps a valid button position into its
intersection with `titleBar()`. This keeps the normal cursor over the upper button
area that used to be a resize strip. Outside buttons it retains the base handler
and clears hover false positives.

These adapters address KDecoration3's conversion of fractional button geometry
and pointer coordinates to integers. That conversion produced small dead strips
and incorrect boundary ownership in testing. During an upgrade, inspect the new
KDecoration event routing and containment code. If it now handles the exact
geometry correctly, simplify the adapters only after the boundary, click, cursor,
and cancellation checks still pass.

### Rounded outlines and painting

`Decoration::titleBarPath()` in `breezedecoration.cpp` represents the existing
title-bar shape using its size, corner radius, alpha support, and window state.
`containsPoint()` uses this outline for input.

`paintTitleBar()` draws into a transparent title-bar image using the caller's
device transform and logical DPI. It paints the outline, caption, and buttons;
`CompositionMode_SourceAtop` constrains the buttons to the painted alpha. It then
composites the image back in device coordinates, retaining the caller's clip.

The independent image matters: the settings preview paints a shadow underneath
the decoration. Masking against the caller's existing alpha could tint that shadow
outside the window. Preserving alpha also keeps hover/press from making the
antialiased corner more opaque. Keep the backdrop, fractional translation, and
partial-repaint tests when porting this code.

## Upgrade procedure

1. Choose the upstream release explicitly. Check its Qt, Frameworks, KDecoration,
   and KWin requirements against the environment where the plugin will run. A
   decoration built for a different plugin ABI may need new system dependencies.
2. Start from a clean, committed fork. Record a pre-upgrade branch or tag if useful.
   Fetch the selected release from the verified KDE remote, and create a separate
   upgrade branch from the current BreezeSquared branch. Preserve any sibling
   reference checkout.
3. Merge the selected upstream tag into the upgrade branch, retaining fork
   history. Review conflicts and upstream edits in every area listed above,
   including renamed files and changes that merged without a conflict.
4. Port by purpose: preserve the behavior contract using the new upstream APIs.
   Pay particular attention to event routing, frame geometry/state updates,
   painting transforms, plugin metadata, and settings storage. Retain upstream
   licensing and attribution.
5. Run `python3 tests/run_tests.py --upstream-ref <selected-tag>`. The reference is
   built from that tag using the installed development libraries. If upstream
   changes the target/output names, update those fields in `upstream-base.json`;
   if its private bridge API changes, adapt the test bridge too. Inspect failures
   instead of weakening assertions to get a pass.
6. Verify the new release in settings and on real windows with user authorization.
   Follow the manual checks in `tests/README.md`. Building/tests alone do not prove
   the installed KWin will discover or correctly run a new plugin ABI.
7. Record the verified new tag and full commit in `upstream-base.json` and this
   guide, update build requirements and the code map, and run the default test
   command again. Document any intentional behavior or test changes with reasons.
8. Review and commit the upgrade. Installation and activation remain separate
   steps. Keep the preceding working version available until the new one is
   confirmed on the desktop.

The default is a merge on an upgrade branch. Rewriting history or publishing to a
remote is not part of this preparation and requires an explicit request.

## Verification and maintenance

[tests/README.md](tests/README.md) is the reproducible test specification. The
runner builds an untouched upstream snapshot from Git, sets identical isolated
settings for both plugins, and compares geometry and rendered icon fingerprints
in the same environment. It does not depend on historical `.so` files, old JSON
reports, the installed Breeze plugin, or a sibling checkout.

Keep this guide updated with behavioral changes: record their intent, current
implementation locations, and verification. Add meaningful regression coverage
for bugs that could return during an upgrade. Update the patch-history table once
the associated commit is available; use Git history for exact implementation
details instead of maintaining a duplicate patch file.
