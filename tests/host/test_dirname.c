/* Host test of amiga/amiga_compat.c's AmigaDOS dirname(): the drawer of a
 * level list, joined to a map's name the way load_level_files() does.
 * Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libgen.h"

static int bad;

static void t(const char *in, const char *want)
{
	char *p = strdup(in), *d = dirname(p), joined[256];

	snprintf(joined, sizeof(joined), "%s%s", d, "level01.map");
	if (d != p || strcmp(joined, want)) {
		printf("FAIL %s -> %s (want %s)\n", in, joined, want);
		bad = 1;
	}
	free(d);	/* as upstream does: must be the strdup'd pointer */
}

int main(void)
{
	t("Work:Levels/my.txt", "Work:Levels/level01.map");
	t("Work:my.txt", "Work:level01.map");
	t("my.txt", "level01.map");
	t("Levels/sub/my.txt", "Levels/sub/level01.map");
	t("PROGDIR:Data/maps/list.txt", "PROGDIR:Data/maps/level01.map");
	t("Work:Le:vels", "Work:Le:level01.map");
	t("", "level01.map");
	if (!bad)
		printf("dirname: 7 AmigaDOS paths PASS\n");
	return bad;
}
