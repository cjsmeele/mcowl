/* Copyright (c) 2012, Chris Johan Smeele
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *   * Neither the name of cjsx9g nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MCOWL_H_
#define MCOWL_H_

#define DEBUG (1)

/* mcowl uses FliPPeh's NBT library, which can be found here:
 * https://github.com/FliPPeh/cNBT
 */
#include "nbt.h"
#include "pnm.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


#define die(text) do{fprintf(stderr, "[ERROR] %-14s %3d: %s\n", __FILE__, __LINE__, text); \
		exit(1);}while(0)

#define printf_d(format, ...) if(DEBUG){ \
			fprintf(stderr, "[debug] %-14s %3d: ", __FILE__, __LINE__); \
			fprintf(stderr, format "\n", __VA_ARGS__); \
		}


typedef struct{
	uint8_t type;
	//uint8_t light;
	int16_t depth;
	uint8_t overlay_type;
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

int render_world_text(char **text_render, const block_t *world_map, uint32_t width, uint32_t height);
int render_world_bitmap(bitmap_t *bitmap, const block_t *world_map, uint32_t width, uint32_t height);

#endif /* MCOWL_H_ */
