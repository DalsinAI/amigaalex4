#!/bin/sh
# Host test of the Amiga data drawer and screenshot writer (amiga/miniz.h,
# amiga/amiga_data.c): fetches upstream Alex 4 at the locked commit (or
# takes A4=), lays out Data/ as amiga/Makefile.amigaos does, reads every
# file the game loads through the shim and compares it with the same
# member of the data.zip upstream builds; then decodes the shim's PNG and
# checks its pixels. A 300 x 200 PNG checks the 65535-byte block split.
#   sh tests/host/test_data.sh
# Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/../.." && pwd)
. "$ROOT/package/alex4.lock"
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
if [ -n "${A4:-}" ]; then
    A=$A4
else
    git -C "$T" init -q a4
    git -C "$T/a4" fetch -q --depth 1 "$ALEX4_URL" "$ALEX4_COMMIT"
    git -C "$T/a4" checkout -q FETCH_HEAD
    A=$T/a4
fi
# the drawer, as the Makefile copies it
mkdir -p "$T/Data"
for d in $ALEX4_DATA_DIRS; do cp -R "$A/data/$d" "$T/Data/$d"; done
# upstream's data.zip: the same files, zipped from data/
(cd "$A/data" && python3 -I -c '
import sys, zipfile, os
with zipfile.ZipFile(sys.argv[1], "w", zipfile.ZIP_DEFLATED) as z:
    for d in sys.argv[2:]:
        for f in sorted(os.listdir(d)):
            z.write(os.path.join(d, f), d + "/" + f)
' "$T/data.zip" $ALEX4_DATA_DIRS)
${CC:-cc} -std=gnu99 -Wall -Wextra -I"$ROOT/amiga" -I"$A/data" \
    -o "$T/test_data" "$HERE/test_data.c" "$ROOT/amiga/amiga_data.c"
"$T/test_data" "$T/Data" "$T/shot.png" > "$T/shim.txt"
python3 -I - "$T/data.zip" "$T/shim.txt" "$T/shot.png" <<'PY'
import sys, zipfile, struct, zlib
zf = zipfile.ZipFile(sys.argv[1])
n = 0
for line in open(sys.argv[2]):
    name, size, h = line.split()
    data = zf.read(name)
    f = 2166136261
    for b in data:
        f = ((f ^ b) * 16777619) & 0xffffffff
    assert int(size) == len(data) and int(h, 16) == f, name
    n += 1
print("data: %d files read through the shim match data.zip" % n)
png = open(sys.argv[3], "rb").read()
assert png[:8] == b"\x89PNG\r\n\x1a\n"
p, idat, ihdr = 8, b"", None
while p < len(png):
    ln, = struct.unpack(">I", png[p:p+4]); typ = png[p+4:p+8]; body = png[p+8:p+8+ln]
    crc, = struct.unpack(">I", png[p+8+ln:p+12+ln])
    assert crc == zlib.crc32(typ + body) & 0xffffffff, typ
    if typ == b"IHDR": ihdr = struct.unpack(">IIBBBBB", body)
    if typ == b"IDAT": idat += body
    p += 12 + ln
assert ihdr == (7, 5, 8, 6, 0, 0, 0), ihdr
raw = zlib.decompress(idat)
want = bytes((i * 37 + 11) & 255 for i in range(7 * 5 * 4))
rows = [raw[r * 29:(r + 1) * 29] for r in range(5)]
assert all(r[0] == 0 for r in rows) and b"".join(r[1:] for r in rows) == want
print("png: 7 x 5 RGBA decodes to the pixels written")
PY
# a picture bigger than one stored block (300 x 200 x 4 + rows > 65535)
cat > "$T/big.c" <<'C'
#include <stdio.h>
#include <stdlib.h>
#include "miniz.h"
int main(void) {
	static unsigned char img[300 * 200 * 4]; size_t len, i;
	for (i = 0; i < sizeof img; i++) img[i] = (unsigned char)(i * 7 + i / 300);
	void *p = tdefl_write_image_to_png_file_in_memory(img, 300, 200, 4, &len);
	fwrite(p, 1, len, stdout); mz_free(p); return 0;
}
C
${CC:-cc} -std=gnu99 -I"$ROOT/amiga" -o "$T/big" "$T/big.c" "$ROOT/amiga/amiga_data.c"
"$T/big" > "$T/big.png"
python3 -I - "$T/big.png" <<'PY'
import sys, zlib, struct
png = open(sys.argv[1], "rb").read(); p = 8; idat = b""
while p < len(png):
    ln, = struct.unpack(">I", png[p:p+4])
    if png[p+4:p+8] == b"IDAT": idat += png[p+8:p+8+ln]
    p += 12 + ln
raw = zlib.decompress(idat)
want = bytes((i * 7 + i // 300) & 255 for i in range(300 * 200 * 4))
assert b"".join(raw[r * 1201 + 1:(r + 1) * 1201] for r in range(200)) == want
print("png: 300 x 200 RGBA over %d stored blocks decodes" % ((len(raw) + 65534) // 65535))
PY
echo "PASS"
