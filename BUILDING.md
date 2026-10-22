# Building BreezeSquared

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
Installation, activation, and live KWin/settings validation remain pending.

## Installation

This setup only builds. The fork uses these identities:

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

The repository also retains upstream application styles and assets. A future
installation step must select the fork's decoration components rather than
installing the full upstream tree. The build script does not install or
activate either plugin.
