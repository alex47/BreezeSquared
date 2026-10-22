# Building and installing BreezeSquared

This working copy is based on Breeze `v6.7.5`. The separate `breeze` checkout is
the preserved upstream reference; use this checkout for development.

## Prerequisites

Install KDE Builder and its standard distribution dependencies following
[KDE's setup guide](https://develop.kde.org/docs/getting-started/building/kde-builder-setup/).
This configuration uses system development packages instead of building KDE
Frameworks or KDecoration from source. Breeze requires Qt 6.10 or newer,
KDE Frameworks 6.26 or newer, and KDecoration3. The 6.7.5 upgrade was built and
tested with Qt 6.11.2, Frameworks 6.29.0, and KDecoration3 6.7.5.

On the Ubuntu development machine, the additional packages are:

```sh
sudo apt install --no-install-recommends \
  extra-cmake-modules libkdecorations3-dev \
  libkf6coreaddons-dev libkf6colorscheme-dev libkf6config-dev \
  libkf6guiaddons-dev libkf6i18n-dev libkf6iconthemes-dev \
  libkf6windowsystem-dev libkf6kcmutils-dev
```

## Build

From this checkout, preview the build and then compile:

```sh
./build.sh --pretend
./build.sh
```

The script can also be invoked by its absolute path from another directory.
It uses `kde-builder.yaml` and the existing KDE Builder metadata installed
during setup. It skips metadata updates, source updates, and installation.
The source branch stays on the current working branch.

Only the `breezesquareddecoration` and `kcm_breezesquareddecoration` targets are compiled,
using Qt 6 and an optimized build with debugging information. The upstream
CMake configuration still checks dependencies for other Breeze components.

All build output, logs, persistent build state, and KDE Builder's Git URL
aliases are stored in the ignored `build` directory. For this checkout name,
the plugins are produced under:

- `build/breezesquared/bin/org.kde.breezesquared.so`
- `build/breezesquared/bin/org.kde.kdecoration3.kcm/kcm_breezesquareddecoration.so`

An optional `KF6FrameworkIntegration` warning concerns the application style;
it does not prevent these decoration targets from building.

Re-run `./build.sh` for incremental builds. Use `./build.sh --reconfigure`
after changing configuration options.

## Regression tests

The tracked regression suite can build from a clean checkout without KDE Builder:

```sh
python3 tests/run_tests.py
```

It needs the development packages above, Git, CMake, Ninja, Python 3.12 or newer,
and DejaVu Sans (`fonts-dejavu-core` on Ubuntu). It builds the fork and its recorded
upstream reference from local source, then checks geometry, input, rendering, and
compatibility with the upstream title-bar size and icons. It does not install or
activate plugins. See [tests/README.md](tests/README.md) for results, faster checks,
and limitations; see [FORK.md](FORK.md) for upstream porting.

The 6.7.5 upgrade has passed the local build and isolated regression suite.
On 2026-10-22, the installed decoration was also confirmed loaded in the running
KWin process using the checks below. Full manual desktop and settings validation
remains pending.

## Installation

On Alex's Ubuntu machine, build and replace the two BreezeSquared plugins with:

```bash
cd /run/media/alex/SSD/Projects/BreezeSquared/breezesquared &&
./build.sh &&
sudo install -m 644 \
  build/breezesquared/bin/org.kde.breezesquared.so \
  /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3/org.kde.breezesquared.so.new &&
sudo mv -fT \
  /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3/org.kde.breezesquared.so.new \
  /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3/org.kde.breezesquared.so &&
sudo install -m 644 \
  build/breezesquared/bin/org.kde.kdecoration3.kcm/kcm_breezesquareddecoration.so \
  /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3.kcm/kcm_breezesquareddecoration.so.new &&
sudo mv -fT \
  /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3.kcm/kcm_breezesquareddecoration.so.new \
  /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3.kcm/kcm_breezesquareddecoration.so
```

This installs only BreezeSquared's decoration and settings module; it does not
update KDE or stock Breeze. Writing to `.new` and then renaming it avoids
overwriting a library that a running process still has mapped. The repository
also contains upstream styles and assets, so do not use a whole-tree install
for this workflow. `./build.sh` itself only builds.

These paths are specific to this machine and assume the plugin directories
already exist. On another system, find the Qt plugin root with
`qtpaths6 --query QT_INSTALL_PLUGINS` and adjust the paths.

If BreezeSquared is not already selected, choose it in System Settings → Window
Decorations and apply. Close and reopen System Settings to load an updated
settings module.

## Reload the decoration without logging out

Installing the file does not replace the copy already loaded by KWin. Changing
decoration settings or switching decorations is not a reliable binary reload.

Save your work first: replacing the Wayland compositor can close applications
that do not support compositor restarts. In a terminal in the current Wayland
session, run this separate, optional step:

```bash
kwin_wayland --replace & disown
```

This worked on Alex's session on 2026-10-22. It is the KWin replacement method
[recommended by a KWin developer](https://discuss.kde.org/t/how-to-restart-plasmashell-wayland-from-a-tty/25319/5).
If you prefer to avoid replacing KWin now, the new plugin can load at the next
normal login.

## Confirm the new decoration is loaded

From the checkout, first check that the built and installed files have identical
SHA-256 hashes:

```sh
sha256sum \
  build/breezesquared/bin/org.kde.breezesquared.so \
  /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3/org.kde.breezesquared.so
```

Then compare the installed file's inode with KWin's mapped library:

```sh
stat -c 'Installed inode: %i' \
  /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3/org.kde.breezesquared.so
sudo grep 'org.kde.breezesquared.so' /proc/$(pgrep -u "$USER" -x kwin_wayland)/maps
```

- Several mapping lines for the same library are normal.
- A path ending in `(deleted)` means KWin still holds a replaced file in memory.
  The installation can have succeeded while that old copy remains loaded.
- Matching hashes, the installed inode in the mapping's fifth column, and no
  `(deleted)` suffix confirm that KWin has mapped the current installed build.
- No matching lines means that process has not loaded BreezeSquared.

The inode changes when the file is replaced; compare the current values rather
than reusing a number from an earlier install. These checks confirm which binary
is loaded; the manual behavior checks are listed in [tests/README.md](tests/README.md).

## Plugin identities and settings

The fork uses these identities:

| Item | Identity |
| --- | --- |
| Decoration name | BreezeSquared |
| Decoration plugin | `org.kde.breezesquared` |
| Settings plugin | `kcm_breezesquareddecoration` |
| Settings file | `breezesquaredrc` in the user's KDE configuration directory |
| CMake package | `BreezeSquared` |

Settings are independent of Breeze's `breezerc`; no existing settings are
copied or migrated automatically. Standard KWin settings, such as the
title-bar button layout, remain managed by KWin.
