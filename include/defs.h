/* SPDX-License-Identifier: GPL */
#ifndef __DEFS_H__
#define __DEFS_H__

#define num_threads 8

#define CHUNK_DIM 20

#define CHUNK_WIDTH 400
#define CHUNK_HEIGHT 400

#define CHUNK_AMOUNT (CHUNK_WIDTH * CHUNK_HEIGHT)

#define WIDTH (CHUNK_WIDTH * CHUNK_DIM)
#define HEIGHT (CHUNK_HEIGHT * CHUNK_DIM)

#define COLORAMOUNT 6
#define SITEAMOUNT (10 * CHUNK_AMOUNT) // how about ~10 per chunk

extern const unsigned char COLORS[COLORAMOUNT][3];

extern unsigned int num_items;

#endif
