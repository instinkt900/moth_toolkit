#!/usr/bin/env python3
"""List the modules whose current version is missing binaries on a Conan remote.

Usage:
    python3 tools/moth_unpublished.py URL --os Linux --os Windows

URL is the Artifactory Conan API URL, e.g.
https://example.com/artifactory/api/conan/conan-local. It is read anonymously.

Prints the module directory names, in dependency order and space-separated,
whose modules/<name>/version.txt has no binary on the remote for at least one
of the given operating systems. A module published for Linux only still counts
as unpublished for Windows, so a release that failed part way is retried.

Exits non-zero when the remote cannot be queried, so an outage is never read as
"nothing is published" and does not trigger a rebuild of every module.
"""

import argparse
import json
import sys
import time
import urllib.error
import urllib.request

from moth_create import MODULES, module_version


def published_os(url: str, pkg: str, version: str) -> set[str]:
    """The operating systems with a binary for pkg/version on the remote."""
    query = f"{url.rstrip('/')}/v2/conans/{pkg}/{version}/_/_/search"
    for attempt in range(3):
        try:
            with urllib.request.urlopen(query, timeout=30) as response:
                packages = json.load(response)
            return {p.get("settings", {}).get("os", "") for p in packages.values()}
        except urllib.error.HTTPError as e:
            if e.code == 404:  # the recipe itself is not on the remote
                return set()
            error = e
        except (urllib.error.URLError, TimeoutError) as e:
            error = e
        time.sleep(2 ** attempt)
    sys.exit(f"error: cannot query {query}: {error}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("url", help="Artifactory Conan API URL of the remote.")
    parser.add_argument("--os", action="append", required=True, dest="systems",
                        help="An OS that must have a binary, e.g. Linux. Repeatable.")
    args = parser.parse_args()

    missing = []
    for name, pkg in MODULES:
        version = module_version(name)
        have = published_os(args.url, pkg, version)
        lacking = [s for s in args.systems if s not in have]
        state = f"missing {', '.join(lacking)}" if lacking else "published"
        print(f"{pkg}/{version}: {state}", file=sys.stderr)
        if lacking:
            missing.append(name)

    print(" ".join(missing))
    return 0


if __name__ == "__main__":
    sys.exit(main())
