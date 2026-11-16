/* Host test of amiga/amiga_data.c: every file Alex 4's load_datafile()
 * asks for (the generated data_files.h mappings and the two scripts) is
 * read from the unpacked Data drawer, and each one's size and bytes are
 * printed as "name size fnv1a" for test_data.sh to compare with the same
 * members of upstream's data.zip. Then a screenshot PNG is written for the
 * script to decode.
 *   test_data <Data drawer> <png out>
 * Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4. */
#include <stdio.h>
#include <stdlib.h>
#define INSIDE_DATA
#include "data_files.h"
#include "miniz.h"

static int failed;

static void one(mz_zip_archive *z, const char *name)
{
	size_t size, i;
	unsigned char *p = mz_zip_reader_extract_file_to_heap(z, name, &size, 0);
	unsigned long h = 2166136261UL;

	if (!p) {
		printf("MISSING %s\n", name);
		failed = 1;
		return;
	}
	if (p[size] != 0) {
		printf("NOT ZERO-ENDED %s\n", name);
		failed = 1;
	}
	for (i = 0; i < size; i++)
		h = ((h ^ p[i]) * 16777619UL) & 0xffffffffUL;
	printf("%s %lu %08lx\n", name, (unsigned long)size, h);
	mz_free(p);
}

int main(int argc, char **argv)
{
	mz_zip_archive z;
	int i;
	size_t len;
	unsigned char img[7 * 5 * 4];
	void *png;
	FILE *fp;

	if (argc != 3)
		return 2;
	mz_zip_zero_struct(&z);
	if (mz_zip_reader_init_file(&z, "no/such/drawer", 0)) {
		printf("opened a drawer that isn't there\n");
		return 1;
	}
	if (!mz_zip_reader_init_file(&z, argv[1], 0)) {
		printf("can't open %s\n", argv[1]);
		return 1;
	}
	for (i = 0; i < I_MAX; i++) one(&z, _image_mono_mapping[i]);
	for (i = 0; i < I_MAX; i++) one(&z, _image_color_mapping[i]);
	for (i = 0; i < S_MAX; i++) one(&z, _sound_mapping[i]);
	for (i = 0; i < MAP_MAX; i++) one(&z, _map_mapping[i]);
	for (i = 0; i < F_MAX; i++) one(&z, _font_mapping[i]);
	for (i = 0; i < MSC_MAX; i++) one(&z, _music_mapping[i]);
	one(&z, "scripts/intro.txt");
	one(&z, "scripts/intro.map");
	one(&z, "scripts/outro.txt");
	one(&z, "scripts/outro.map");
	{
		size_t s;
		if (mz_zip_reader_extract_file_to_heap(&z, "maps/no-such.map", &s, 0)) {
			printf("read a file that isn't there\n");
			failed = 1;
		}
	}
	mz_zip_reader_end(&z);

	/* 7 x 5 RGBA, every byte different from its neighbours */
	for (i = 0; i < (int)sizeof(img); i++)
		img[i] = (unsigned char)(i * 37 + 11);
	png = tdefl_write_image_to_png_file_in_memory(img, 7, 5, 4, &len);
	if (!png || !(fp = fopen(argv[2], "wb")) || fwrite(png, 1, len, fp) != len) {
		printf("png write failed\n");
		return 1;
	}
	fclose(fp);
	mz_free(png);
	return failed;
}
