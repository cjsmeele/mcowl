/*
 * mcowl.h
 *
 *  Created on: Jun 20, 2012
 *      Author: chris
 */

#ifndef MCOWL_H_
#define MCOWL_H_

#define DEBUG (1)

#include "nbt.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define die(text) do{fprintf(stderr, "[ERROR] %-12s %3d: %s\n", __FILE__, __LINE__, text); \
		exit(1);}while(0)

#define printf_d(format, ...) if(DEBUG){ \
			fprintf(stderr, "[debug] %-12s %3d: ", __FILE__, __LINE__); \
			fprintf(stderr, format "\n", __VA_ARGS__); \
		}


typedef struct{
	uint8_t type;
	//uint8_t light;
	//uint8_t depth;
} block_t;

typedef struct{
	bool allocated;
	block_t blocks[16][16][16];
} chunk_t;

typedef struct{
	chunk_t chunks[16];
	uint16_t allocated_b;
} column_t;

typedef struct{
	int32_t offset;
	uint8_t length_sectors;
} col_link_t;


static inline void* swap_bytes(void* s, size_t len){
	for(char *b=s, *e=b+len-1;
			b<e;
			b++,e--){
		char t = *b;

		*b = *e;
		*e = t;
	}

	return s;
}

inline uint32_t col_coords_to_index(int32_t x, int32_t z){
	return z*32 + x;
}

inline uint32_t coords_to_index(int32_t x, int32_t z){
	int32_t intra_x = (x<0?(x/16-1):(x/16))%32;
	int32_t intra_z = (z<0?(z/16-1):(z/16))%32;
	if(intra_x < 0)
		intra_x = 32 + intra_x;
	if(intra_z < 0)
		intra_z = 32 + intra_z;
	return col_coords_to_index(intra_x, intra_z);
}


//int render_chunk_text(block_t chunk_map[16][16], char text_map[16][16]);
int render_world_text(char **text_render, const block_t *world_map, uint32_t width, uint32_t height);

#endif /* MCOWL_H_ */
