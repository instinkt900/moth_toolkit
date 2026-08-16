#!/usr/bin/env python3
"""Generate a new Moth game project from the bundled template.

Usage:
    python3 tools/moth_new.py my_game [--dir PATH]

Creates PATH/my_game/ containing main.cpp, CMakeLists.txt, and conanfile.py that
build against the packaged moth_graphics module (the Game facade + renderer),
then prints the build instructions.
"""

import argparse
import shutil
import sys
from pathlib import Path

TEMPLATE_DIR = Path(__file__).resolve().parent / "template"


def generate(name: str, output_dir: Path) -> Path:
    if not name or any(c.isspace() or c in "/\\" for c in name):
        sys.exit(f"error: '{name}' is not a valid project name")

    project_dir = output_dir / name
    if project_dir.exists():
        sys.exit(f"error: {project_dir} already exists")

    shutil.copytree(TEMPLATE_DIR, project_dir)

    for path in project_dir.rglob("*"):
        if path.is_file():
            text = path.read_text(encoding="utf-8")
            updated = text.replace("@PROJECT_NAME@", name)
            if updated != text:
                path.write_text(updated, encoding="utf-8")

    return project_dir


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("name", help="Project name (used as the CMake/Conan target name).")
    parser.add_argument("--dir", type=Path, default=Path("."),
                        help="Output directory (default: current directory).")
    args = parser.parse_args()

    project_dir = generate(args.name, args.dir)
    print(f"Created {project_dir}/")
    print()
    print("Next steps:")
    print(f"  cd {project_dir}")
    print("  conan install . --build=missing -s build_type=Release")
    print("  cmake --preset conan-release")
    print("  cmake --build --preset conan-release")
    print(f"  ./build/Release/{args.name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
