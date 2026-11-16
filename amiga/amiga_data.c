/* The data drawer and the stored-block PNG writer behind amiga/miniz.h.
 * Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "miniz.h"

mz_bool mz_zip_reader_init_file(mz_zip_archive *z, const char *path, unsigned flags)
{
	struct stat st;
	size_t n = strlen(path);

	(void)flags;
	if (n == 0 || n >= sizeof(z->root) || stat(path, &st) != 0 || !S_ISDIR(st.st_mode))
		return 0;
	memcpy(z->root, path, n + 1);
	while (n > 1 && z->root[n - 1] == '/' && z->root[n - 2] != ':')
		z->root[--n] = '\0';
	z->open = 1;
	return 1;
}

void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *z, const char *name,
	size_t *size, unsigned flags)
{
	char path[1024];
	FILE *fp;
	long len;
	char *buf;
	size_t n = strlen(z->root);

	(void)flags;
	*size = 0;
	if (!z->open || !name || !*name)
		return NULL;
	/* PROGDIR:Data + "/" + "maps/level01.map"; a bare volume "X:" takes no '/' */
	if (snprintf(path, sizeof(path), "%s%s%s", z->root,
			(n && z->root[n - 1] == ':') ? "" : "/", name) >= (int)sizeof(path))
		return NULL;
	fp = fopen(path, "rb");
	if (!fp)
		return NULL;
	if (fseek(fp, 0, SEEK_END) != 0 || (len = ftell(fp)) < 0 || fseek(fp, 0, SEEK_SET) != 0) {
		fclose(fp);
		return NULL;
	}
	/* one byte more, zeroed: the scripts are read as text */
	buf = malloc((size_t)len + 1);
	if (buf && fread(buf, 1, (size_t)len, fp) != (size_t)len) {
		free(buf);
		buf = NULL;
	}
	fclose(fp);
	if (!buf)
		return NULL;
	buf[len] = '\0';
	*size = (size_t)len;
	return buf;
}

mz_bool mz_zip_reader_end(mz_zip_archive *z)
{
	z->open = 0;
	return 1;
}

void mz_free(void *p)
{
	free(p);
}

/* PNG */

static unsigned long crc_table[256];

static void make_crc_table(void)
{
	unsigned long c;
	int n, k;

	for (n = 0; n < 256; n++) {
		c = (unsigned long)n;
		for (k = 0; k < 8; k++)
			c = (c & 1) ? 0xedb88320UL ^ (c >> 1) : c >> 1;
		crc_table[n] = c;
	}
}

static unsigned long crc32_update(unsigned long crc, const unsigned char *p, size_t n)
{
	crc ^= 0xffffffffUL;
	while (n--)
		crc = crc_table[(crc ^ *p++) & 0xff] ^ (crc >> 8);
	return crc ^ 0xffffffffUL;
}

static unsigned char *put32(unsigned char *p, unsigned long v)
{
	p[0] = (unsigned char)(v >> 24);
	p[1] = (unsigned char)(v >> 16);
	p[2] = (unsigned char)(v >> 8);
	p[3] = (unsigned char)v;
	return p + 4;
}

/* writes a chunk whose data is already at p + 8; returns the end */
static unsigned char *chunk(unsigned char *p, const char *type, size_t len)
{
	put32(p, (unsigned long)len);
	memcpy(p + 4, type, 4);
	return put32(p + 8 + len, crc32_update(0, p + 4, len + 4));
}

void *tdefl_write_image_to_png_file_in_memory(const void *image, int w, int h,
	int chans, size_t *len)
{
	static const unsigned char sig[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n' };
	const unsigned char *src = image;
	size_t row, raw, blocks, zlen, total;
	unsigned long a = 1, b = 0;
	unsigned char *png, *p, *z;
	size_t left, i;
	int y;

	*len = 0;
	if (!image || w <= 0 || h <= 0 || (chans != 3 && chans != 4))
		return NULL;
	if (!crc_table[1])
		make_crc_table();
	row = (size_t)w * (size_t)chans;
	raw = (row + 1) * (size_t)h;		/* a filter byte (0) before each row */
	blocks = (raw + 65534) / 65535;		/* stored blocks hold 65535 bytes */
	zlen = 2 + raw + 5 * blocks + 4;
	total = 8 + (12 + 13) + (12 + zlen) + 12;
	png = malloc(total);
	if (!png)
		return NULL;

	memcpy(png, sig, 8);
	p = png + 8;
	put32(p + 8, (unsigned long)w);
	put32(p + 12, (unsigned long)h);
	p[16] = 8;				/* bit depth */
	p[17] = chans == 4 ? 6 : 2;		/* RGBA or RGB */
	p[18] = p[19] = p[20] = 0;		/* deflate, adaptive filters, no interlace */
	p = chunk(p, "IHDR", 13);

	z = p + 8;
	*z++ = 0x78;				/* zlib header: deflate, 32 KB window */
	*z++ = 0x01;
	left = raw;
	y = 0;
	i = 0;					/* the next byte's place in its row, 0 = filter byte */
	while (left) {
		size_t n = left > 65535 ? 65535 : left;
		left -= n;
		*z++ = left ? 0 : 1;		/* BFINAL on the last block, BTYPE 00 */
		*z++ = (unsigned char)n;
		*z++ = (unsigned char)(n >> 8);
		*z++ = (unsigned char)~n;
		*z++ = (unsigned char)(~n >> 8);
		while (n--) {
			unsigned char c = i ? src[(size_t)y * row + i - 1] : 0;
			*z++ = c;
			a = (a + c) % 65521;
			b = (b + a) % 65521;
			if (++i == row + 1) {
				i = 0;
				y++;
			}
		}
	}
	z = put32(z, (b << 16) | a);
	p = chunk(p, "IDAT", zlen);
	p = chunk(p, "IEND", 0);
	*len = (size_t)(p - png);
	return png;
}
