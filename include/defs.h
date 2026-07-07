/* SPDX-License-Identifier: GPL */
#ifndef __DEFS_H__
#define __DEFS_H__

#define num_threads 8

// For a background
#define CHUNK_DIM 40
#define CHUNK_WIDTH 40
#define CHUNK_HEIGHT 40

#define CHUNK_AMOUNT (CHUNK_WIDTH * CHUNK_HEIGHT)

#define WIDTH (CHUNK_WIDTH * CHUNK_DIM)
#define HEIGHT (CHUNK_HEIGHT * CHUNK_DIM)

#define COLORAMOUNT 6
#define SITEAMOUNT (10 * CHUNK_AMOUNT) // how about ~10 per chunk
// #define SITEAMOUNT 20

extern const unsigned char COLORS[COLORAMOUNT][3];

extern unsigned int num_items;

#endif
