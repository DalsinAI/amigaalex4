Alex the Allegator 4 for GFOS
=============================

A port of Alex the Allegator 4, the little platform game Johan Peitz
(Free Lunch Design) made in 2003 in the style of the Nintendo GameBoy,
in carstene1ns's SDL 2 version from https://github.com/carstene1ns/alex4
at commit 5e3532c5f23b70d1c1c1bfcacc6d29165f97a67e (game v1.3), for GFOS:
AmigaOS 3.2 with GFOpenUp, on a 68040 or 68060 with an FPU, on
GFOpenUp's SDL 2.

Alex's girlfriend Lola has been caught by humans. Walk, jump, stomp and
eat your way through ten levels and two guardians, collecting cherries and
stars, to set her free. The music is a ProTracker module.

Status: built in Cradle's os32-gcc16-open6 stove; not yet run in a lab.


What you need
-------------

- GFOS: AmigaOS 3.2 with GFOpenUp 0.8.26 or later, which installs
  LIBS:GFOpenGPU/SDL2.module and SDL2_mixer.module.
- A 68040 or 68060 with an FPU (not an LC040 or EC040), an RTG screen
  with room for a 640 x 480 window (the game is 160 x 120, drawn four
  times as big; Options changes it), and about 6 MB of free memory.
- Nothing else: the pictures, maps, music and sounds are in the drawer.


How to play
-----------

Install it one of three ways:

- GFOpenUp: `GFOpenUpTool INSTALL Parts/Alex4` puts it in SYS:Games/Alex4.
- Run Install-Alex4 from the archive (it offers Work:Games).
- Or copy the Alex4 drawer where you want it.

Then double-click Alex4. The cursor keys walk, Alt jumps, Ctrl spits; the
rest is in the drawer's Controls file. A Shell start with a level list
(`Alex4 Work:MyLevels/levels.txt`) plays your own levels.

The settings, high scores, log and screenshots are kept in the drawer's
User drawer; Install-Alex4 and GFOpenUp keep it when they update the game.


Building
--------

    sh package/build-amiga.sh        (STOVE=/path/to/os32-gcc16-open6)

or, in Cradle, stove_build with the os32-gcc16-open6 stove and target
package. It fetches upstream at the commit package/alex4.lock names,
applies amiga/patches, builds with amiga/Makefile.amigaos and packs
Alex4-VERSION-68040.lha and the GFOpenUp part Parts/Alex4.

Host tests: `sh tests/host/test_all.sh` checks the patch applies and puts
every file the game writes in PROGDIR:User, the AmigaDOS dirname(), and
reads every one of the 253 data files through the Amiga data shim,
comparing each with the same member of upstream's data.zip; it also
decodes the screenshot PNGs the shim writes.


What was changed
----------------

- amiga/patches/0001-amigaos-drawer-paths.patch: the settings
  (config.ini), high scores, log and screenshots go to PROGDIR:User, the
  data are read from PROGDIR:Data, no working-directory change; the
  window opens at 640 x 480; screenshots read the pixels as RGBA bytes
  (upstream asked for ARGB words, which come out in a different byte
  order on a big-endian 68k).
- amiga/miniz.h, amiga/amiga_data.c: upstream packs its data into data.zip
  and reads it with a built-in copy of miniz. The port builds no miniz: it
  ships the data unpacked in Data/, and these read each file from there
  under the name upstream asks data.zip for. Screenshots are PNGs with
  stored deflate blocks, written by our own few lines.
- amiga/amiga_compat.c, amiga/libgen.h: dirname() for AmigaDOS paths,
  which libnix doesn't have.
- amiga/amiga_start.c: 256 KB of stack, the $VER string, PROGDIR:User
  made at start.
- amiga/: the Makefile, the GlowIcons (our own art), the Installer script
  and its spec. package/: the build, the release archive and the GFOpenUp
  part.

The AmiCradle Team.
