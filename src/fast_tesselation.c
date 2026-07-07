// SPDX-License-Identifier: GPL
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "fast_tesselation.h"
#include "defs.h"
#include "bar.h"

static sites *completeSites; // array containing all sites in img
static s_chunk *sChunkedSites; // special chunks, formed by 3x3 chunks

int write_buffer(FILE *f, void *buf, size_t size)
{
	size_t offset = 0;
	size_t remaining = size;
	size_t written = 0;
	size_t to_write = 0;
	const size_t CHUNK_SIZE = (4 * 1024 * 1024); // 4 MB

	while (offset < size) {
		// written = min(CHUNK_SIZE, remaining)
		to_write = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;
		written = fwrite((char *)buf + offset, 1, to_write, f);
		if (written != to_write) {
			fprintf(stderr, "uh oh write error\n");
			return -1;
		}

		// printf("wrote %lu chunk, with %lu remaining", written,
		//    remaining);

		offset += written;
		remaining = size - offset;
	}
	return 1;
}

static inline int randMax(int max)
{
	return rand() % (max);
}

// grows the 'site array' in 's_chunk workingChunk'
// at given pointer by one and adds argument to it
void append(s_chunk *workingChunk, sites *item)
{
	size_t arrSizeOld = (*workingChunk).sitesHeld;

	// reallocate the array to the size of itself
	// very ineffictient, change this?
	(*workingChunk).siteArr = realloc((*workingChunk).siteArr,
					  (arrSizeOld + 1) * sizeof(item));
	if ((*workingChunk).siteArr == NULL) {
		printf("Well, fuck. The adding of one element to the array complains for some reason");
		printf("diagnose maybe idk\n");
		exit(1);
	}

	// now also increment the arrSize
	(*workingChunk).sitesHeld = arrSizeOld + 1;
	// since it is 0 indexed, using the old size as index yields the last element
	(*workingChunk).siteArr[arrSizeOld] = item;
	// printf("%p\n", (*workingChunk).siteArr[arrSizeOld]);
}

void cleanSites(void)
{
	free(completeSites);
	for (int i = 0; i < SITEAMOUNT; i++)
		free((*sChunkedSites).siteArr);

	free(sChunkedSites);
}

void initialiseSites(void)
{
	// initialise the random function
	// srand(time(NULL));
	srand(8008135);

	static sites mutableInternal[SITEAMOUNT];
	static s_chunk chunkedSitesTmp[CHUNK_AMOUNT];
	int colorChoice;
	int tX, tY; // x and y of the generated point [why t?]
	int cX, cY; // x and y in the chunk grid, so chunk coordinates
	char tR, tG, tB; // RGB coordinates of point

	// clear any garbage pointers and values
	for (int i = 0; i < CHUNK_AMOUNT; i++) {
		chunkedSitesTmp[i].siteArr = NULL;
		chunkedSitesTmp[i].sitesHeld = 0;
	}

	for (int i = 0; i < SITEAMOUNT; i++) {
		tX = randMax(WIDTH);
		tY = randMax(HEIGHT);

		colorChoice = randMax(COLORAMOUNT);
		tR = COLORS[colorChoice][0];
		tG = COLORS[colorChoice][1];
		tB = COLORS[colorChoice][2];

		mutableInternal[i] = (sites){
			.x = tX,
			.y = tY,
			.r = tR,
			.g = tG,
			.b = tB,
		};

		// j is for Y, k is for X
		for (int j = -1; j <= 1; j++) {
			// if the current Y falls outside of chunk boundaries (< 0, > max), skip
			cY = (tY / CHUNK_DIM) + j;
			if (cY < 0 || cY >= CHUNK_HEIGHT)
				continue;

			for (int k = -1; k <= 1; k++) {
				// if the current X falls outside of chunk boundaries (< 0, > max), skip
				cX = (tX / CHUNK_DIM) + k;
				if (cX < 0 || cX >= CHUNK_WIDTH)
					continue;

				// printf("I AM GOING TO ADD %p\n", &mutableInternal[i]);
				append(&chunkedSitesTmp[(cY * CHUNK_WIDTH) + cX],
				       &mutableInternal[i]);
			}
		}
	}
	completeSites = mutableInternal;
	sChunkedSites = chunkedSitesTmp;
}

static inline double distance(int x1, int x2, int y1, int y2, float p)
{
	double dx = x1 - x2;
	double dy = y1 - y2;

	return pow(fabs(dx), p) + pow(fabs(dy), p);
}

static inline sites *closest(unsigned int x, unsigned int y, float p)
{
	int cX = x / CHUNK_DIM;
	int cY = y / CHUNK_DIM;
	int sitesInChunk = sChunkedSites[cY * CHUNK_WIDTH + cX].sitesHeld;

	if (sitesInChunk == 0) {
		static sites zeroSite = {
			.x = 0,
			.y = 0,
			.r = 0,
			.g = 0,
			.b = 0,
		};
		return &zeroSite;
	}

	// now that we now this adress is populated:
	sites **siteOpts = sChunkedSites[cY * CHUNK_WIDTH + cX].siteArr;

	double d;
	int closestIndex = 0;

	double minD = distance(x, siteOpts[0]->x, y, siteOpts[0]->y, p);

	for (int i = 1; i < sitesInChunk; i++) {
		d = distance(x, siteOpts[i]->x, y, siteOpts[i]->y, p);
		if (d < minD) {
			minD = d;
			closestIndex = i;
		}
	}
	return siteOpts[closestIndex];
}

// alr for this function we just gotta go and generate one tesselation
void worker(void *arg)
{
	struct argument *val = arg;
	image imageRam;
	sites *closestSite; // just pointer to single one

	// Sites = getSites();
	imageRam.bytemap = malloc((unsigned long)WIDTH * HEIGHT * 3 *
				  sizeof(unsigned char));

	// maybe change out this for some more sennsible stuff later
	imageRam.width = WIDTH;
	imageRam.height = HEIGHT;

	unsigned int w = imageRam.width;
	unsigned int h = imageRam.height;

	if (!imageRam.bytemap)
		perror("malloc failed");

	for (unsigned long i = 0; i < h; i++) {
		for (unsigned long j = 0; j < w; j++) {
			closestSite = closest(j, i, (*val).power); // => 2
			imageRam.bytemap[i * w * 3 + j * 3 + 0] =
				closestSite->r;
			imageRam.bytemap[i * w * 3 + j * 3 + 1] =
				closestSite->g;
			imageRam.bytemap[i * w * 3 + j * 3 + 2] =
				closestSite->b;
		}
	}
	char fileName[20];

	sprintf(fileName, "out/test%d.ppm", val->name);

	FILE *f = fopen(fileName, "wb");

	if (!f) {
		printf("ehh file open error idk what to do\n");
		exit(1);
	}

	fprintf(f, "P6\n%u %u\n255\n", w, h);

	if (fwrite(imageRam.bytemap, sizeof(unsigned char), w * h * 3, f) !=
	    (unsigned long)w * h * 3) {
		fclose(f);
		printf("uhh file write error idk\n");
	} else {
		fclose(f);
	}

	printBar("Progress: ", (val->name * 100) / num_items);
	free(imageRam.bytemap);
}

// subWorker
void subWorker(void *arg)
{
	struct argument *val = arg;
	image imageRam = val->img;
	sites *closestSite;

	// iterate only over the area of our particular chunk
	unsigned int startX = val->chunkX * CHUNK_DIM;
	unsigned int startY = val->chunkY * CHUNK_DIM;

	unsigned int endX = startX + CHUNK_DIM;
	unsigned int endY = startY + CHUNK_DIM;

	unsigned int current_pixel = 0;

	unsigned int w = imageRam.width;
	// unsigned int h = imageRam.height;

	for (unsigned int i = startY; i < endY; i++) {
		for (unsigned int j = startX; j < endX; j++) {
			closestSite = closest(j, i, (*val).power); // => 2
			current_pixel = 3 * (i * w) + // row
					3 * j; // column selector
			imageRam.bytemap[current_pixel + 0] = closestSite->r;
			imageRam.bytemap[current_pixel + 1] = closestSite->g;
			imageRam.bytemap[current_pixel + 2] = closestSite->b;
		}
	}

	// printf("x");
	// fflush(stdout);

	incrementBar("Progress: ", CHUNK_AMOUNT);
}
