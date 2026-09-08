# Building BreezeSquared

This working copy starts at Breeze `v6.6.5`. The separate `breeze` checkout is
the preserved upstream reference; use this checkout for development.

## Prerequisites

Install KDE Builder and its standard distribution dependencies following
[KDE's setup guide](https://develop.kde.org/docs/getting-started/building/kde-builder-setup/).
This configuration uses system development packages instead of building KDE
Frameworks or KDecoration from source. Breeze requires Qt 6.10 or newer,
KDE Frameworks 6.22 or newer, and KDecoration3.

On the Ubuntu development machine, the additional packages are:

```sh
sudo apt install --no-install-recommends \
  extra-cmake-modules libkdecorations3-dev \
  libkf6coreaddons-dev libkf6colorscheme-dev libkf6config-dev \
  libkf6guiaddons-dev libkf6i18n-dev libkf6iconthemes-dev \
  libkf6windowsystem-dev libkf6kcmutils-dev libkirigami-dev
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

Only the `breezedecoration` and `kcm_breezedecoration` targets are compiled,
using Qt 6 and an optimized build with debugging information. The upstream
CMake configuration still checks dependencies for other Breeze components.

All build output, logs, persistent build state, and KDE Builder's Git URL
aliases are stored in the ignored `build` directory. For this checkout name,
the plugins are produced under:

- `build/breezesquared/bin/org.kde.breeze.so`
- `build/breezesquared/bin/org.kde.kdecoration3.kcm/kcm_breezedecoration.so`

An optional `KF6FrameworkIntegration` warning concerns the application style;
it does not prevent these decoration targets from building.

Re-run `./build.sh` for incremental builds. Use `./build.sh --reconfigure`
after changing configuration options.

## Installation

This setup only builds. The plugins still have their upstream Breeze
identities. Give the fork separate plugin and settings identities before
installing it alongside the system Breeze decoration.
