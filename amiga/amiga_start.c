/* The Amiga port's start: the stack, the version string and PROGDIR:User
 * (the settings, the high scores, the log and screenshots), made before
 * main() so the first file the game writes has its drawer.
 * Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4. */
#include <sys/stat.h>

/* A Shell gives 4 KB of stack; SDL 2 wants more. libnix's startup sets
 * this much up. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) =
    "$VER: Alex4 1.3 (16.11.2026)";

static void __attribute__((constructor)) amiga_user_drawer(void)
{
	mkdir("PROGDIR:User", 0755);
}
