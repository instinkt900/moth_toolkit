#!/usr/bin/env python3
"""conan create every Moth toolkit module, in dependency order.

Usage:
    python3 tools/moth_create.py                       # all modules, Release
    python3 tools/moth_create.py -s build_type=Debug   # a debug cache
    python3 tools/moth_create.py core gfx ui           # just these (still ordered)
    python3 tools/moth_create.py -r conancenter        # restrict to one remote
    python3 tools/moth_create.py --list                # print the order and stop
    python3 tools/moth_create.py --with-variants       # also build the published variants

Refreshes the local Conan cache so other projects build against the current
sources. The common `conan create` flags are accepted directly; anything else
goes after a `--` separator:

    python3 tools/moth_create.py -- --lockfile=conan.lock

This module list is the single source of truth for build order: the CI
workflows read it with --list rather than keeping their own copy.
"""

import argparse
import re
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

# Non-default option sets that are published alongside each module's defaults,
# because a real consumer needs them and would otherwise build from source:
#
#   core    enable_platform=False  moth_noised links its own GLFW through Magnum;
#                                  without this it ends up with two copies.
#   gfx     enable_glslang=True    runtime GLSL compilation (examples/shader_demo).
#   packer  with_ui=True           the --layout/--layouts-dir collectors, which
#                                  tools/moth_packer requires.
#   profile with_imgui=False       headless consumers; also what moth_toolkit
#                                  selects when built with enable_gfx=False.
#
# Each entry is a list of variants; each variant is a space-separated set of
# Conan options. The release workflow reads this with --list-variants, so adding
# one here is all it takes to start publishing it.
VARIANTS: dict[str, list[str]] = {
    "core": ["enable_platform=False"],
    "gfx": ["enable_glslang=True"],
    "packer": ["with_ui=True"],
    "profile": ["with_imgui=False"],
}


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

    check_order()
    check_versions()


def check_order() -> None:
    """Fail if MODULES is not a topological sort of the recipes' moth_* requires.

    The release workflow filters this order rather than the detected module set,
    so a module listed before something it requires would only show up mid-publish
    as a confusing "not found in remotes" for a package that was about to be built
    two steps later. Catching it here fails fast instead, locally and in CI.

    Requires are read straight out of the recipes, so the order cannot drift from
    what Conan actually resolves. Conditional requires (moth_profile pulls
    moth_graphics only when with_imgui) count too: a published variant can enable
    them, so the order has to hold for every option set, not just the defaults.
    """
    dir_of = {pkg: name for name, pkg in MODULES}
    position = {name: index for index, (name, _) in enumerate(MODULES)}
    violations = []

    for name, _ in MODULES:
        source = (MODULES_DIR / name / "conanfile.py").read_text()
        for required in sorted(set(re.findall(r'self\.requires\(\s*"(moth_\w+)', source))):
            if required not in dir_of:
                sys.exit(f"error: modules/{name} requires '{required}', which is not a "
                         f"package in MODULES in tools/moth_create.py.")
            if position[dir_of[required]] > position[name]:
                violations.append(f"{name} requires {dir_of[required]}, "
                                  f"which MODULES lists after it")

    if violations:
        sys.exit("error: MODULES in tools/moth_create.py is not in dependency order:\n"
                 + "\n".join("       - " + v for v in violations)
                 + "\n       reorder it so every module comes after what it requires.")


def module_version(name: str) -> str:
    return (MODULES_DIR / name / "version.txt").read_text().strip()


def satisfies(spec: str, version: str) -> bool:
    """Does `version` satisfy the Conan requirement `spec`?

    Only the forms this repo actually uses are understood: an exact version and
    the tilde (patch) range. Anything else raises, so a spec the checker cannot
    reason about fails loudly instead of silently passing — a quiet pass here
    would recreate the very mistake this check exists to catch.
    """
    parts = tuple(int(p) for p in version.split("."))

    if not spec.startswith("["):
        return spec == version

    inner = spec[1:-1].strip()
    if not inner.startswith("~"):
        raise ValueError(spec)

    wanted = tuple(int(p) for p in inner[1:].split("."))
    # ~X.Y allows any patch of X.Y; ~X.Y.Z additionally requires >= X.Y.Z.
    return parts[:2] == wanted[:2] and parts >= wanted[:len(parts)]


def check_versions() -> None:
    """Fail if a recipe requires a moth_* version the repo no longer provides.

    Conan resolves a range against what it can see, so a module whose version was
    bumped without its dependents' ranges being updated does not fail the build:
    the dependents quietly resolve to the last version published to the remote and
    are built against stale headers. Nothing in the release pipeline can notice,
    because from Conan's point of view the graph is valid. The invariant that
    catches it is that a monorepo built from one commit should never reach past
    itself — every moth_* requirement here must be satisfied by the version that
    is in the tree right now.
    """
    dir_of = {pkg: name for name, pkg in MODULES}
    stale = []

    for recipe in sorted(REPO_ROOT.rglob("conanfile.py")):
        if ".git" in recipe.parts:
            continue
        for pkg, spec in re.findall(r'self\.requires\(\s*"(moth_\w+)/([^"]+)"',
                                    recipe.read_text()):
            if pkg not in dir_of:
                continue  # check_order already reports an unknown moth_* package.
            current = module_version(dir_of[pkg])
            try:
                ok = satisfies(spec, current)
            except ValueError:
                sys.exit(f"error: {recipe.relative_to(REPO_ROOT)} requires "
                         f"'{pkg}/{spec}', a form tools/moth_create.py cannot check.\n"
                         f"       extend satisfies() so the version guard keeps working.")
            if not ok:
                rel = recipe.relative_to(REPO_ROOT)
                stale.append(f"{rel}: requires {pkg}/{spec}, "
                             f"but modules/{dir_of[pkg]}/version.txt is {current}")

    if stale:
        sys.exit("error: recipe(s) require a moth_* version this repo no longer provides:\n"
                 + "\n".join("       - " + v for v in stale)
                 + "\n       these resolve to an older published package instead of the "
                   "current source,\n       which succeeds silently. Update the ranges to "
                   "match the bumped module(s).")


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
    parser.add_argument("--list-variants", metavar="MODULE",
                        help="Print the extra option sets published for MODULE, one per "
                             "line, and exit. Prints nothing if it has no variants.")
    parser.add_argument("--with-variants", action="store_true",
                        help="Also create each module's published variants (see VARIANTS).")
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

    if args.list_variants:
        known = {name for name, _ in MODULES}
        if args.list_variants not in known:
            sys.exit(f"error: unknown module: {args.list_variants}")
        for variant in VARIANTS.get(args.list_variants, []):
            print(variant)
        return 0

    selected = select(args.modules)

    if args.list:
        for name, pkg in selected:
            print(f"{name} {pkg}" if args.with_packages else name)
        return 0

    extra = conan_args(args, passthrough)
    failed: list[str] = []
    started = time.monotonic()

    for index, (name, pkg) in enumerate(selected, start=1):
        # The default build first, then any published variants. Each is a
        # separate binary under the same reference.
        builds = [(pkg, [])]
        if args.with_variants:
            for variant in VARIANTS.get(name, []):
                opts = []
                for option in variant.split():
                    opts += ["-o", option]
                builds.append((f"{pkg} [{variant}]", opts))

        for label, opts in builds:
            cmd = [args.conan, "create", str(MODULES_DIR / name)] + extra + opts
            print(f"\n[{index}/{len(selected)}] {label}\n  $ {shlex.join(cmd)}", flush=True)

            if args.dry_run:
                continue

            try:
                result = subprocess.run(cmd, cwd=REPO_ROOT)
            except FileNotFoundError:
                sys.exit(f"error: '{args.conan}' not found — is Conan on your PATH? "
                         f"Pass --conan /path/to/conan if it lives in a virtualenv.")

            if result.returncode != 0:
                failed.append(label)
                if not args.keep_going:
                    print(f"\nFailed creating {label}. Later modules depend on it, so "
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
