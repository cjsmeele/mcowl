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

#define OS_OTHER 0
#define OS_LINUX 1
#define OS_WIN32 2
#define OS_MACOS 3

#ifndef OSTYPE
	#if defined __linux__
		#define OSTYPE OS_LINUX
		#define OSNAME "linux"
	#elif defined _WIN32
		#define OSTYPE OS_WIN32
		#define OSNAME "win32"
	#elif defined __APPLE__
		#define OSTYPE OS_MACOS
		#define OSNAME "mac-os"
	#else
		#define OSTYPE OS_OTHER
		#define OSNAME "unknown_target"
	#endif
#endif

#if OSTYPE > OS_MACOS
	#define OSTYPE OS_OTHER
#endif

#if OSTYPE < 0
	#define OSTYPE OS_OTHER
#endif

#ifndef OSNAME
	#define OSNAME "unknown_target"
#endif

#define ISOMETRIC_BLOCK_SIZE ((2+2-1)*2)
#define ISOMETRIC_COLUMN_WIDTH (15*(ISOMETRIC_BLOCK_SIZE+2)+ISOMETRIC_BLOCK_SIZE)
#define ISOMETRIC_COLUMN_HEIGHT ((ISOMETRIC_BLOCK_SIZE+2)/2*16 + (ISOMETRIC_BLOCK_SIZE/2+1-2)*256)

#define die(text) do{fprintf(stderr, "[ERROR] %-14s %3d: %s\n", __FILE__, __LINE__, text); \
		exit(1);}while(0)

#define print_d(text) if(DEBUG){ \
			fprintf(stderr, "[debug] %-14s %3d: ", __FILE__, __LINE__); \
			fprintf(stderr, text "\n"); \
		}

#define printf_d(format, ...) if(DEBUG){ \
			fprintf(stderr, "[debug] %-14s %3d: ", __FILE__, __LINE__); \
			fprintf(stderr, format "\n", __VA_ARGS__); \
		}

enum RenderMode{
	RENDER_FLAT=1,
	RENDER_DEPTH,
	RENDER_HEIGHT,
	RENDER_LIGHT,
	RENDER_HIGHLIGHT,
	RENDER_ISOMETRIC
};

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

int render_column_flat(bitmap_t *bitmap, block_t slice[16][16], int rendermode);

#endif /* MCOWL_H_ */
