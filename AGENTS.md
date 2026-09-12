# Working on BreezeSquared

Read [FORK.md](FORK.md) before changing decoration behavior or updating the
upstream base. Read [BUILDING.md](BUILDING.md) for builds and
[tests/README.md](tests/README.md) for verification.

## Scope and behavior

- This repository is the working fork. If a sibling `../breeze` checkout exists,
  preserve it as the untouched upstream reference. Tests obtain their reference
  from this repository's Git history and do not need that sibling.
- Preserve the behavior contract in `FORK.md`, including the existing title-bar
  height, upstream icon sizes, matched paint/input boundaries, and the remaining
  left margin. Clarify a requested design change if its intent is unclear.
- Keep the decoration and settings identities separate from Breeze. Retain
  upstream namespaces, filenames, translations, and unrelated components where
  a change is not needed for the fork.

## Verification and desktop access

- Build with `./build.sh`. Run `python3 tests/run_tests.py` for a full regression
  run and comparison with the recorded upstream base. These commands do not
  install or activate a plugin.
- Keep generated sources, binaries, logs, and test reports under ignored `build/`
  directories. Keep reusable test code and instructions tracked in Git.
- Use the isolated, windowless tests for automated rendering and input checks.
  Never take desktop screenshots or inspect unrelated user files or settings.
- Do not start or stop GUI applications, restart KWin, or change the desktop
  session without explicit user approval. Treat installation and activation as
  separate actions from building and testing. Report missing system dependencies;
  the user manages privileged installation.

## Maintenance and Git

- Update the relevant `FORK.md` entries and regression checks with behavioral
  changes. Record the purpose, affected functions, and any API assumptions.
- For an upstream upgrade, use a separate upgrade branch and the procedure in
  `FORK.md`. Inspect upstream changes even when Git reports no conflicts.
- Keep `upstream-base.json` and `FORK.md` consistent. Update the recorded base
  when the upgrade has been verified. Do not change the reference just to make a
  failed compatibility test pass.
- Commit focused, verified milestones. Inspect working and staged changes, and
  stage explicit paths. Do not include unrelated work or build artifacts.
- Do not amend, squash, rebase, reset, or push unless explicitly requested.
  `origin` initially points to KDE upstream; check remotes before any publishing.
