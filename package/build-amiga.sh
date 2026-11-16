#!/bin/sh
# The release archive and the GFOpenUp part, for Cradle's stove_build
# (target package):
#  1. upstream Alex 4, fetched at the commit package/alex4.lock names (it
#     is never kept in this repository), with amiga/patches applied;
#  2. the game built by amiga/Makefile.amigaos;
#  3. the drawer, its icon and the Installer script beside it packed as
#     NAME-VERSION-68040.lha (tools/pack_dist.py), with its .sha256 and a
#     short .readme;
#  4. OUT/Parts/Alex4, a GFOpenUp part: its Part file and the drawer's
#     files for SYS:Games/Alex4, installed with
#       GFOpenUpTool INSTALL Parts/Alex4
#     It needs the GFOpenGPU part, whose SDL2.module draws the game.
# Nothing is published from here.
# Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4.
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/.." && pwd)
. "$HERE/package.env"
. "$HERE/alex4.lock"
OUT=${OUT:-"$ROOT/build-amiga-package"}
mkdir -p "$OUT"
OUT=$(CDPATH= cd -- "$OUT" && pwd)
STOVE=${STOVE:-"$HOME/AmiCradle/stoves/os32-gcc16-open6"}
# outside the checkout: Cradle cleans it (git clean -ffdx) before every job
WORK=${WORK:-"$HOME/.cache/amigaalex4"}
cd "$ROOT"

# 1. upstream at the locked commit, patched afresh every time
A4="$WORK/alex4"
if [ "$(git -C "$A4" rev-parse HEAD 2>/dev/null)" != "$ALEX4_COMMIT" ]; then
    rm -rf "$A4"; mkdir -p "$A4"
    git -C "$A4" init -q
    git -C "$A4" fetch -q --depth 1 "$ALEX4_URL" "$ALEX4_COMMIT"
    git -C "$A4" checkout -q FETCH_HEAD
fi
git -C "$A4" checkout -q -- . && git -C "$A4" clean -qfdx
for p in "$ROOT"/amiga/patches/*.patch; do
    git -C "$A4" apply "$p"
done

# 2. the game (a stove's compiler is in its prefix/, or STOVE is the prefix)
if [ -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ]; then PREFIX="$STOVE/prefix"; else PREFIX="$STOVE"; fi
make -f amiga/Makefile.amigaos A4="$A4" STOVE="$STOVE" PREFIX="$PREFIX" -j4 $TARGET > "$OUT/build-output.txt" 2>&1 || { tail -n 60 "$OUT/build-output.txt"; exit 1; }

# 3. the archive
python3 "$ROOT/tools/pack_dist.py" --dist "$DIST" --name "$NAME" --out "$OUT"

# 4. the GFOpenUp part
sh "$ROOT/package/write-part.sh" "$DIST/$NAME" "$OUT/Parts/$NAME" "$NAME" "$PORT_VERSION" \
    "Alex the Allegator 4: the GameBoy-style platform game, 12 stages"
echo "part: $(grep -c '^copy' "$OUT/Parts/$NAME/Part") files, $(du -sk "$OUT/Parts/$NAME" | cut -f1) KB"
ls -l "$DIST/$NAME/$NAME"
