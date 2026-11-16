/* What Alex 4 asks of the C library and libnix doesn't have.
 * Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4. */
#include <string.h>
#include "libgen.h"

/* load_level_files() (a Shell start with a level list named) takes the
 * drawer of the list with dirname(), joins it to each map's name with
 * "%s%s" and frees it. So, for AmigaDOS: the path is cut in place just
 * after its last '/' or ':' (the drawer, separator kept: "Work:Levels/",
 * "Work:"), or to "" for a bare name (the current drawer). The pointer
 * returned is always `path`, so free() is right. */
char *dirname(char *path)
{
	char *slash, *colon, *cut;

	if (!path)
		return NULL;
	slash = strrchr(path, '/');
	colon = strrchr(path, ':');
	cut = slash && (!colon || slash > colon) ? slash : colon;
	if (cut)
		cut[1] = '\0';
	else
		path[0] = '\0';
	return path;
}
