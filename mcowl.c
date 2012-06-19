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

#include "nbt.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct{
	
} block_t;

typedef struct{
	bool allocated;
	block_t blocks[16][16][16];
} chunk_t;

typedef struct{
	chunk_t chunks[1*16*1];
} column_t;

typedef struct{
	column_t columns[32*1*32];
} region_t;

typedef struct{
	region_t *regions;
} world_t;


typedef struct{
	int32_t offset;
	uint8_t length_sectors;
} chunk_link_t;

static void die(const char* message){
	fprintf(stderr, "%s\n", message);
	exit(1);
}

static void die_with_err(int err){
	fprintf(stderr, "Error %i: %s\n", err, nbt_error_to_string(err));
	exit(1);
}


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

int populate_chunktable(FILE *regionfile, chunk_link_t *cat){

	uint32_t region_header[32*32];
	
	fseek(regionfile, 0, SEEK_SET);
	size_t bytes_read = fread(region_header, sizeof(uint32_t), 32*32, regionfile);
	if(bytes_read != 32*32)
		die("Could not read header from region file, aborting");

	for(uint32_t i=0;i<32*32;i++){
		swap_bytes(&region_header[i], sizeof(uint32_t));
		cat[i].offset = region_header[i]>>8;
		cat[i].length_sectors = region_header[i] & 0xff;

		//printf(" hex: %08x , offset: 0x%08x , sectors: % 3u\n", region_header[i], cat[i].offset, cat[i].length_sectors);
		//get_chunk_nbt(regionfile, &cat[i]);
	}
	
	return 0;
}

nbt_node* get_chunk_nbt(FILE *regionfile, chunk_link_t *chunk_link){

	if(chunk_link->length_sectors == 0){
		//printf("(caught empty chunk)\n");
		return 0;
	}
	
	if(fseek(regionfile, chunk_link->offset*4096, SEEK_SET))
		die("Could not seek to chunk (region corrupt?)");

	FILE *tempfile = tmpfile();
	if(!tempfile) die("Could not create temporary file");

	uint8_t chunk_header[5];
	size_t bytes_read = fread(chunk_header, 1, 5, regionfile);
	if(bytes_read != 5)
		die("Could not read chunk header, aborting");

	int32_t chunk_length = (int32_t)((uint32_t)chunk_header[0]<<24 | (uint32_t)chunk_header[1]<<16 | (uint32_t)chunk_header[2]<<8 | (uint32_t)chunk_header[3]);
	//swap_bytes(chunk_header, 4);
	chunk_length = (int32_t)((uint32_t)chunk_header[0]<<24 | (uint32_t)chunk_header[1]<<16 | (uint32_t)chunk_header[2]<<8 | (uint32_t)chunk_header[3]);
	uint8_t compression = chunk_header[4];

	//printf("  \\- hex: 0x%02x%02x%02x%02x%02x , ", chunk_header[0], chunk_header[1], chunk_header[2], chunk_header[3], chunk_header[4]);
	//printf("length: 0x%08x , compression: 0x%02x\n", chunk_length, compression);

	if(chunk_length > 1024*1024)
		die("Chunk too large!");

	uint8_t *chunk_data = (uint8_t*)malloc(chunk_length);
	bytes_read = fread(chunk_data, 1, chunk_length, regionfile);
	if(bytes_read != chunk_length)
		die("Could not read chunk data, aborting");

	fwrite(chunk_data, 1, chunk_length, tempfile);
	fseek(tempfile, 0, SEEK_SET);

	nbt_node *tree = nbt_parse_file(tempfile);
	if(tree == NULL) die_with_err(errno);

	fclose(tempfile);
	
	return tree;
}

inline uint32_t chunk_coords_to_index(int32_t x, int32_t z){
	return z*32 + x;
}

inline uint32_t coords_to_index(int32_t x, int32_t z){
	int32_t intra_x = (x<0?(x/16-1):(x/16))%32;
	int32_t intra_z = (z<0?(z/16-1):(z/16))%32;
	if(intra_x < 0)
		intra_x = 32 + intra_x;
	if(intra_z < 0)
		intra_z = 32 + intra_z;
	return chunk_coords_to_index(intra_x, intra_z);
}


int main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: mcowl [regionfile]\n");
		return 1;
	}

	FILE *regionfile = fopen(argv[1], "rb");
	if(!regionfile)
		die("Could not open region file");


	chunk_link_t cat[32*32];
	populate_chunktable(regionfile, cat);

	nbt_node *tree = get_chunk_nbt(regionfile, &cat[coords_to_index(-255, 320)]);

	char *tree_ascii = nbt_dump_ascii(tree);
	puts(tree_ascii);


	return 0;
}
