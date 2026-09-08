#!/usr/bin/env bash
set -euo pipefail

project_dir="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
build_root="$project_dir/build"
mkdir -p -- "$build_root"

# Keep KDE Builder's automatic Git URL aliases local to this build.
export GIT_CONFIG_GLOBAL="$build_root/kde-builder.gitconfig"

exec kde-builder \
  --rc-file "$project_dir/kde-builder.yaml" \
  --source-dir "$(dirname -- "$project_dir")" \
  --dest-dir "$(basename -- "$project_dir")" \
  --build-dir "$build_root" \
  --log-dir "$build_root/log" \
  --install-dir "$build_root/install" \
  --persistent-data-file "$build_root/kde-builder-state.json" \
  --no-metadata \
  --no-check-self-updates \
  "$@" \
  --build-only \
  breezesquared
