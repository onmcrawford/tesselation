/* SPDX-License-Identifier: GPL */
#ifndef __TESSELATION_H__
#define __TESSELATION_H__
#include "defs.h"

struct argument;
struct image_struct;
struct site_struct;
struct schunk_struct;

typedef struct image_struct image;
typedef struct site_struct sites;
typedef struct schunk_struct s_chunk;

// unsigned int width, height; unsigned char *bytemap;
struct image_struct {
	unsigned int width, height;
	unsigned char *bytemap;
};

// int x, y; unsigned char r, g, b;
struct site_struct {
	int x, y;
	unsigned char r, g, b; // color value at char
};

// sites **siteArr; int sitesHeld;
// special chunk, 3x3 chunks, with overlap
struct schunk_struct {
	sites **siteArr;
	int sitesHeld;
};

// float power; int name;
struct argument {
	float power;
	int name;
	unsigned char chunkX;
	unsigned char chunkY;
	struct image_struct img;
};

// allows for chunk based writing to files
int write_buffer(FILE *f, void *buf, size_t size);

// sets initial values for where sites are
void initialiseSites(void);

// function called to be placed in threadpool to create a single image
void worker(void *arg);

// function called to figure out only a single chunk, so that creating a single
//image is multithreaded too
void subWorker(void *arg);

// remove broken data
void cleansites(void);

#endif
