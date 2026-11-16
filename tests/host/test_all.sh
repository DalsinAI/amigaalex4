#!/bin/sh
# Every host test of the port: the data drawer and screenshot writer
# (test_data.sh), the AmigaDOS dirname(), and that amiga/patches apply to
# upstream at the locked commit.
#   sh tests/host/test_all.sh        (A4=/path/to/alex4 to skip the fetch)
# Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/../.." && pwd)
. "$ROOT/package/alex4.lock"
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
if [ -z "${A4:-}" ]; then
    git -C "$T" init -q a4
    git -C "$T/a4" fetch -q --depth 1 "$ALEX4_URL" "$ALEX4_COMMIT"
    git -C "$T/a4" checkout -q FETCH_HEAD
    A4=$T/a4
fi
export A4
mkdir -p "$T/p" && cp -R "$A4/src" "$T/p/"
for p in "$ROOT"/amiga/patches/*.patch; do (cd "$T/p" && patch -s -p1 --dry-run < "$p" && patch -s -p1 < "$p"); done
echo "patches: apply to $ALEX4_COMMIT"
# the patched sources keep nothing of the player's in the current drawer
# (only the lines an AmigaOS build compiles: tests/host/amiga_lines.py)
if for f in main.c misc.c; do python3 -I "$HERE/amiga_lines.py" "$T/p/src/$f"; done |
        grep '"config.ini"\|"highscore\|"log.txt"\|"a4_%03d\|get_homedir\|chdir' | grep -v ALEX4_USER; then
    echo "FAIL: a file outside PROGDIR:User"; exit 1
fi
echo "patches: config, high scores, log and screenshots go to ALEX4_USER"
${CC:-cc} -std=gnu99 -Wall -Wextra -I"$ROOT/amiga" -o "$T/test_dirname" \
    "$HERE/test_dirname.c" "$ROOT/amiga/amiga_compat.c"
"$T/test_dirname"
sh "$HERE/test_data.sh"
