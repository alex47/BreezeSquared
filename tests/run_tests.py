#!/usr/bin/env python3
"""Build the fork and its recorded upstream reference, then compare them."""
# SPDX-License-Identifier: BSD-3-Clause

import argparse
import json
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile


ROOT = Path(__file__).resolve().parents[1]
SIZES = ("ButtonTiny", "ButtonSmall", "ButtonDefault", "ButtonLarge", "ButtonVeryLarge")
BORDER_VARIANTS = {
    f"border-{titlebar}-{corners}-{outline}": [
        "--border-state",
        *(["--hide-titlebar"] if titlebar == "hidden" else []),
        *(["--no-rounded-corners"] if corners == "square" else []),
        *(["--outline"] if outline == "outline" else []),
    ]
    for titlebar in ("shown", "hidden")
    for corners in ("rounded", "square")
    for outline in ("outline", "no-outline")
}
VARIANTS = (*SIZES, "no-alpha", "border-on-maximized", "gradient", "alternate-layout", "inactive", *BORDER_VARIANTS)


def run(command, *, capture=False):
    result = subprocess.run(command, cwd=ROOT, text=True, capture_output=capture)
    if result.returncode:
        if capture:
            print(result.stdout, end="", file=sys.stderr)
            print(result.stderr, end="", file=sys.stderr)
        raise RuntimeError(f"Command failed ({result.returncode}): {command}")
    return result.stdout if capture else None


def configure(source, build, *, testing):
    run([
        "cmake", "-S", str(source), "-B", str(build), "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=RelWithDebInfo", "-DBUILD_QT5=OFF", "-DBUILD_QT6=ON",
        "-DWITH_DECORATIONS=ON", "-DWITH_WALLPAPERS=OFF",
        f"-DBUILD_TESTING={'ON' if testing else 'OFF'}",
    ])


def build_targets(build, jobs, *targets):
    run(["cmake", "--build", str(build), "--parallel", str(jobs), "--target", *targets])


def upstream_source(work, commit):
    source = work / "upstream" / commit / "source"
    marker = source / ".breezesquared-test-source"
    if source.exists():
        if not marker.is_file() or marker.read_text().strip() != commit:
            raise RuntimeError(f"Unrecognized reference source directory: {source}. Use a fresh --build-dir.")
        return source
    source.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="extract-", dir=source.parent) as temporary:
        temporary = Path(temporary)
        archive = temporary / "source.tar"
        run(["git", "archive", "--format=tar", f"--output={archive}", commit])
        extracted = temporary / "source"
        extracted.mkdir()
        with tarfile.open(archive) as stream:
            stream.extractall(extracted, filter="data")
        (extracted / marker.name).write_text(commit + "\n")
        extracted.rename(source)
    return source


def compare_reports(expected, actual, label):
    if len(expected) != len(actual):
        raise RuntimeError(f"{label}: reference and fork have different case counts")
    for old, new in zip(expected, actual):
        if old != new:
            fields = [key for key in old.keys() | new.keys() if old.get(key) != new.get(key)]
            raise RuntimeError(
                f"{label}: upstream compatibility changed in {new.get('case')}: {', '.join(fields)}. "
                "Inspect the reference and current JSON reports before accepting this change."
            )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build" / "regression")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--upstream-ref", help="Compare against this local Git ref while preparing an upgrade")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if sys.version_info < (3, 12):
        parser.error("Python 3.12 or later is required for safe archive extraction")

    work = args.build_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    # A failed run must not leave a previous success summary behind.
    summary_path = work / "summary.json"
    summary_path.unlink(missing_ok=True)

    manifest = json.loads((ROOT / "upstream-base.json").read_text())
    ref = args.upstream_ref or manifest["tag"]
    commit = run(["git", "rev-parse", "--verify", "--end-of-options", f"{ref}^{{commit}}"], capture=True).strip()
    if not args.upstream_ref and commit != manifest["commit"]:
        raise RuntimeError("Upstream tag does not match upstream-base.json; verify the reference before testing")

    source = upstream_source(work, commit)
    current_build = work / "current"
    reference_build = source.parent / "build"

    print("Building BreezeSquared and the isolated regression harness", flush=True)
    configure(ROOT, current_build, testing=True)
    build_targets(current_build, args.jobs, "breezesquareddecoration", "kcm_breezesquareddecoration", "breezesquared_geometry_test")
    reports = current_build / "tests" / "reports"
    for name in VARIANTS:
        (reports / f"{name}.json").unlink(missing_ok=True)
    run(["ctest", "--test-dir", str(current_build), "--output-on-failure", "--parallel", str(args.jobs), "-R", "^breezesquared\\."])
    missing = [name for name in VARIANTS if not (reports / f"{name}.json").is_file()]
    if missing:
        raise RuntimeError(f"CTest did not produce all expected reports: {', '.join(missing)}")

    print(f"Building upstream {ref} ({commit[:12]}) for comparison", flush=True)
    configure(source, reference_build, testing=False)
    build_targets(reference_build, args.jobs, manifest["decoration_target"])
    reference_plugin = reference_build / manifest["decoration_file"]
    if not reference_plugin.is_file():
        raise RuntimeError(f"Reference plugin not found: {reference_plugin}; check upstream build layout")
    paths = json.loads((current_build / "tests" / "test-paths.json").read_text())
    comparisons = []
    reference_cases = [(size, size, []) for size in SIZES]
    reference_cases += [(name, "ButtonDefault", options) for name, options in BORDER_VARIANTS.items()]
    for name, size, options in reference_cases:
        reference_report = work / f"upstream-{name}.json"
        run([paths["harness"], str(reference_plugin), size, "--baseline", *options, "--report", str(reference_report)])
        expected = json.loads(reference_report.read_text())
        actual = json.loads((reports / f"{name}.json").read_text())
        compare_reports(expected, actual, name)
        comparisons.append({"variant": name, "button_size": size, "configurations": len(actual)})
        print(f"{name}: matches upstream in {len(actual)} configurations", flush=True)

    counts = {name: len(json.loads((reports / f"{name}.json").read_text())) for name in VARIANTS}
    summary = {
        "upstream_ref": ref, "upstream_commit": commit,
        "fork_revision": run(["git", "rev-parse", "HEAD"], capture=True).strip(),
        "fork_worktree_dirty": bool(run(["git", "status", "--porcelain"], capture=True).strip()),
        "configurations": sum(counts.values()), "variants": counts,
        "upstream_comparisons": comparisons, "result": "passed",
    }
    summary_path.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"All {summary['configurations']} configurations passed. Report: {summary_path}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, tarfile.TarError, RuntimeError) as error:
        print(f"Test run failed: {error}", file=sys.stderr)
        sys.exit(1)
