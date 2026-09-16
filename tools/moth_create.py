#!/usr/bin/env python3
"""conan create every Moth toolkit module, in dependency order.

Usage:
    python3 tools/moth_create.py                       # all modules, Release
    python3 tools/moth_create.py -s build_type=Debug   # a debug cache
    python3 tools/moth_create.py core gfx ui           # just these (still ordered)
    python3 tools/moth_create.py -r conancenter        # restrict to one remote
    python3 tools/moth_create.py --list                # print the order and stop

Refreshes the local Conan cache so other projects build against the current
sources. The common `conan create` flags are accepted directly; anything else
goes after a `--` separator:

    python3 tools/moth_create.py -- --lockfile=conan.lock

This module list is the single source of truth for build order: the CI
workflows read it with --list rather than keeping their own copy.
"""

import argparse
import shlex
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
MODULES_DIR = REPO_ROOT / "modules"

# Dependency order: a module is created only once everything it requires is in
# the cache. gfx precedes anim/tilemap, ui precedes packer, and the toolkit
# aggregate comes last because it depends on all of them.
MODULES = [
    ("core", "moth_core"),
    ("gfx", "moth_graphics"),
    ("anim", "moth_anim"),
    ("ui", "moth_ui"),
    ("packer", "moth_packer"),
    ("ecs", "moth_ecs"),
    ("physics", "moth_physics"),
    ("tilemap", "moth_tilemap"),
    ("audio", "moth_audio"),
    ("assets", "moth_assets"),
    ("net", "moth_net"),
    ("noise", "moth_noise"),
    ("profile", "moth_profile"),
    ("bridge", "moth_bridge"),
    ("toolkit", "moth_toolkit"),
]


def check_registry() -> None:
    """Fail if modules/ and MODULES have drifted apart."""
    on_disk = {p.name for p in MODULES_DIR.iterdir()
               if p.is_dir() and (p / "conanfile.py").is_file()}
    listed = {name for name, _ in MODULES}

    missing = sorted(on_disk - listed)
    if missing:
        sys.exit("error: module(s) not listed in MODULES in tools/moth_create.py: "
                 + ", ".join(missing)
                 + "\n       add them in dependency order so they are built and published.")

    stale = sorted(listed - on_disk)
    if stale:
        sys.exit("error: MODULES in tools/moth_create.py lists module(s) that no "
                 "longer exist: " + ", ".join(stale))


def select(requested: list[str]) -> list[tuple[str, str]]:
    if not requested:
        return MODULES

    known = {name for name, _ in MODULES}
    unknown = [r for r in requested if r not in known]
    if unknown:
        sys.exit(f"error: unknown module(s): {', '.join(unknown)}\n"
                 f"       known: {', '.join(name for name, _ in MODULES)}\n"
                 f"       (conan flags this script does not define go after a '--' separator)")

    # Keep dependency order regardless of the order given on the command line.
    return [(name, pkg) for name, pkg in MODULES if name in set(requested)]


def conan_args(args: argparse.Namespace, passthrough: list[str]) -> list[str]:
    out = []
    for build in args.build or ["missing"]:
        out += ["--build", build]
    # Default to Release: the modules are normally consumed as release builds.
    for setting in args.settings or ["build_type=Release"]:
        out += ["-s", setting]
    for option in args.options or []:
        out += ["-o", option]
    if args.profile:
        out += ["-pr", args.profile]
    if args.remote:
        out += ["-r", args.remote]
    if args.no_remote:
        out += ["-nr"]
    return out + passthrough


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("modules", nargs="*",
                        help="Module directory names to create (default: all).")
    parser.add_argument("--list", action="store_true",
                        help="Print the modules in dependency order and exit.")
    parser.add_argument("--with-packages", action="store_true",
                        help="With --list, print '<dir> <package>' pairs.")
    parser.add_argument("--dry-run", action="store_true",
                        help="Print the conan commands without running them.")
    parser.add_argument("--keep-going", action="store_true",
                        help="Carry on after a failure instead of stopping at the first.")
    parser.add_argument("--conan", default="conan",
                        help="Path to the conan executable (default: conan).")

    conan = parser.add_argument_group("conan create options")
    conan.add_argument("-s", "--settings", action="append", metavar="KEY=VALUE",
                       help="Conan setting (default: build_type=Release). Repeatable.")
    conan.add_argument("-o", "--options", action="append", metavar="KEY=VALUE",
                       help="Conan option, e.g. 'moth_packer/*:with_ui=True'. Repeatable.")
    conan.add_argument("-pr", "--profile", metavar="NAME", help="Conan profile.")
    conan.add_argument("-r", "--remote", metavar="NAME",
                       help="Restrict resolution to this remote.")
    conan.add_argument("-nr", "--no-remote", action="store_true",
                       help="Resolve from the local cache only.")
    conan.add_argument("--build", action="append", metavar="MODE",
                       help="Conan --build mode (default: missing). Repeatable.")

    # Everything after '--' is handed to conan create untouched.
    argv = sys.argv[1:]
    passthrough: list[str] = []
    if "--" in argv:
        split = argv.index("--")
        argv, passthrough = argv[:split], argv[split + 1:]
    args = parser.parse_args(argv)

    check_registry()
    selected = select(args.modules)

    if args.list:
        for name, pkg in selected:
            print(f"{name} {pkg}" if args.with_packages else name)
        return 0

    extra = conan_args(args, passthrough)
    failed: list[str] = []
    started = time.monotonic()

    for index, (name, pkg) in enumerate(selected, start=1):
        cmd = [args.conan, "create", str(MODULES_DIR / name)] + extra
        print(f"\n[{index}/{len(selected)}] {pkg}\n  $ {shlex.join(cmd)}", flush=True)

        if args.dry_run:
            continue

        try:
            result = subprocess.run(cmd, cwd=REPO_ROOT)
        except FileNotFoundError:
            sys.exit(f"error: '{args.conan}' not found — is Conan on your PATH? "
                     f"Pass --conan /path/to/conan if it lives in a virtualenv.")

        if result.returncode != 0:
            failed.append(pkg)
            if not args.keep_going:
                print(f"\nFailed creating {pkg}. Later modules depend on it, so "
                      f"stopping here (use --keep-going to continue).", file=sys.stderr)
                return result.returncode

    if args.dry_run:
        return 0

    elapsed = time.monotonic() - started

    if failed:
        print(f"\n{len(failed)} of {len(selected)} failed: {', '.join(failed)}",
              file=sys.stderr)
        return 1

    print(f"\nCreated {len(selected)} module(s) in {elapsed:.0f}s. "
          f"Local Conan cache is up to date.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
