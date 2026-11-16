/* The few miniz calls Alex 4 makes, for AmigaOS, without miniz.
 *
 * Upstream packs its data into data.zip and reads it with miniz. The Amiga
 * port ships the same files unpacked in the drawer (PROGDIR:Data), so the
 * "archive" here is that drawer and "extracting" a member reads the file
 * of the same name. Screenshots are written as PNGs with stored (not
 * deflated) blocks: valid PNGs that need no compressor.
 *
 * Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4. */
#ifndef ALEX4_AMIGA_MINIZ_H
#define ALEX4_AMIGA_MINIZ_H

#include <stddef.h>
#include <string.h>

typedef int mz_bool;

typedef struct mz_zip_archive {
	char root[512];	/* the data drawer, without a trailing '/' */
	int open;
} mz_zip_archive;

#define mz_zip_zero_struct(z) memset((z), 0, sizeof(*(z)))

/* "Opens" the data drawer `path` (a directory that must exist). */
mz_bool mz_zip_reader_init_file(mz_zip_archive *z, const char *path, unsigned flags);
/* Reads root/name whole into memory; NULL if it can't. */
void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *z, const char *name,
	size_t *size, unsigned flags);
mz_bool mz_zip_reader_end(mz_zip_archive *z);
void mz_free(void *p);

/* A w x h PNG of `chans` (3 or 4) bytes per pixel, R G B (A) in memory,
 * as miniz's call of the same name; free it with mz_free. */
void *tdefl_write_image_to_png_file_in_memory(const void *image, int w, int h,
	int chans, size_t *len);

#endif
