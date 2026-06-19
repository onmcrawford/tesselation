// SPDX-License-Identifier: GPL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "bar.h"
#include "defs.h"
#include "fast_tesselation.h"
#include "tpool.h"

unsigned int num_items;

void nImgs(unsigned int num_items)
{
	tpool *tm;
	struct argument *vals;
	size_t i;

	initialiseSites(); // ignore output, just to make threadsafe
	tm = tpool_create(num_threads);
	vals = calloc(num_items, sizeof(*vals));

	for (i = 0; i < num_items; i++) {
		vals[i].name = i;
		vals[i].power = 1.0 + i * (1.0 / (num_items - 1));
		// printf("%d, %.6f\n", vals[i].name, vals[i].power);
		tpool_add_work(tm, worker, vals + i);
	}
	tpool_wait(tm);

	free(vals);
	tpool_destroy(tm);
	flushBar("Progress: ");
}

void singleImage(void)
{
	initialiseSites(); // ignore output, just to make threadsafe
	// reserve space for image
	image imageRam;

	// Sites = getSites();
	imageRam.height = HEIGHT;
	imageRam.width = WIDTH;

	int h = imageRam.height;
	int w = imageRam.width;

	imageRam.bytemap = malloc((unsigned long)WIDTH * HEIGHT * 3 *
				  sizeof(unsigned char));

	if (!imageRam.bytemap) {
		printf("malloc failed\n");
		exit(1);
	}

	// generate worker for all chunks
	tpool *tm;
	struct argument *vals;

	// when we generate a single image, we need a bunch of args for all chunks, not for all imgs
	vals = calloc(CHUNK_AMOUNT, sizeof(*vals));
	if (vals == NULL) {
		printf("could not allocate memory for arguments\n");
		exit(1);
	}
	tm = tpool_create(num_threads);

	for (int i = 0; i < CHUNK_AMOUNT; i++) {
		vals[i] = (struct argument){
			.power = 2,
			.chunkX = i % CHUNK_WIDTH,
			.chunkY = i / CHUNK_WIDTH,
			.name = i,
			.img = imageRam,
		};
		tpool_add_work(tm, subWorker, &vals[i]);
	}

	// cleanup threadpool
	tpool_wait(tm);

	free(vals);
	tpool_destroy(tm);

	// write img out to file
	FILE *f = fopen("out/test0.ppm", "wb");

	if (!f) {
		char *cwd = getcwd(NULL, 0);

		printf("CWD: %s\n", cwd);
		free(cwd);
		printf("file open error %s\n", strerror(errno));
		exit(1);
	}

	fprintf(f, "P6\n%u %u\n255\n", w, h);

	// We write in small parts, around 1gb
	if (write_buffer(f, imageRam.bytemap,
			 sizeof(unsigned char) * w * h * 3) != 1) {
		fclose(f);
		printf("uhh file write error idk\n");
	} else {
		fclose(f);
	}

	flushBar("progress: ");
	free(imageRam.bytemap);
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		printf("Not enough arguments provided.\n");
		return 1;
	}

	num_items = atoi(argv[1]);

	// maybe make this a little more safe
	if (num_items == 1)
		singleImage();
	else
		nImgs(num_items);
}
