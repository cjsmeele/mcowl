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
	uint8_t type;
	uint8_t data;
} block_t;

typedef struct{
	bool allocated;
	block_t blocks[16][16][16];
} chunk_t;

typedef struct{
	chunk_t chunks[16];
} column_t;

typedef struct{
	column_t columns[32][32];
} region_t;

typedef struct{
	region_t *regions;
} world_t;


typedef struct{
	int32_t offset;
	uint8_t length_sectors;
} col_link_t;

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

int populate_coltable(FILE *regionfile, col_link_t *cat){

	uint32_t region_header[32*32];
	
	fseek(regionfile, 0, SEEK_SET);
	size_t bytes_read = fread(region_header, sizeof(uint32_t), 32*32, regionfile);
	if(bytes_read != 32*32)
		die("Could not read header from region file, aborting");

	for(uint32_t i=0;i<32*32;i++){
		swap_bytes(&region_header[i], sizeof(uint32_t));
		cat[i].offset = region_header[i]>>8;
		cat[i].length_sectors = region_header[i] & 0xff;
	}
	
	return 0;
}

nbt_node* get_col_nbt(FILE *regionfile, const col_link_t *col_link){

	if(col_link->length_sectors == 0){
		return 0;
	}
	
	if(fseek(regionfile, col_link->offset*4096, SEEK_SET))
		die("Could not seek to col (region corrupt?)");

	FILE *tempfile = tmpfile();
	if(!tempfile) die("Could not create temporary file");

	uint8_t col_header[5];
	size_t bytes_read = fread(col_header, 1, 5, regionfile);
	if(bytes_read != 5)
		die("Could not read col header, aborting");

	int32_t col_length = (int32_t)((uint32_t)col_header[0]<<24 | (uint32_t)col_header[1]<<16 | (uint32_t)col_header[2]<<8 | (uint32_t)col_header[3]);

	col_length = (int32_t)((uint32_t)col_header[0]<<24 | (uint32_t)col_header[1]<<16 | (uint32_t)col_header[2]<<8 | (uint32_t)col_header[3]);

	if(col_length > 1024*1024)
		die("Column too large!");

	uint8_t *col_data = (uint8_t*)malloc(col_length);
	bytes_read = fread(col_data, 1, col_length, regionfile);
	if(bytes_read != col_length)
		die("Could not read column data, aborting");

	size_t bytes_written = fwrite(col_data, 1, col_length, tempfile);
	if(bytes_written != col_length)
		die("Could not write column data to temporary file");
	fseek(tempfile, 0, SEEK_SET);

	nbt_node *tree = nbt_parse_file(tempfile);
	if(tree == NULL) die_with_err(errno);

	fclose(tempfile);
	
	return tree;
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

int get_column(column_t *column, nbt_node *coltree){

	memset(column, 0, sizeof(column_t));
	
	nbt_node *leveltree = nbt_find_by_name(coltree, "Level");

	if((leveltree == NULL) || (strcmp(leveltree->name, "Level") != 0)
			|| (leveltree->type != TAG_COMPOUND)){
		return 1;
	}
	
	nbt_node *sections = nbt_find_by_name(leveltree, "Sections");
	struct nbt_list *sections_list = sections->payload.tag_compound;

	if((sections == NULL) || (strcmp(sections->name, "Sections") != 0)
			|| (sections->type != TAG_LIST)){
		printf("SEC= %d: %s\n", sections->type, sections->name);
		return 1;
	}

	nbt_node *chunk_nodes[16];
	for(uint32_t i=0;i<16;i++)
		chunk_nodes[i] = NULL;

	struct list_head* pos;

	uint8_t highest_chunk = 0;

	list_for_each(pos, &sections_list->entry){
		nbt_node *section_node = list_entry(pos, struct nbt_list, entry)->data;
		struct nbt_list* section_datalist = section_node->payload.tag_compound;
		struct list_head* spos;
		
		//printf("Ysection! %d: %s\n", section_node->type, section_node->name);

		list_for_each(spos, &section_datalist->entry){
			//printf("Subsection! ");
			struct nbt_node* ydatanode = list_entry(spos, struct nbt_list, entry)->data;
			if(ydatanode->name == NULL)
				continue;
			//printf("GOT %d: %s\n", ydatanode->type, ydatanode->name);
			if(strcmp(ydatanode->name, "Y") == 0){
				//printf("GOT Y=%d!\n", ydatanode->payload.tag_byte);
				if(ydatanode->payload.tag_byte >= 16 || ydatanode->payload.tag_byte < 0){
					die("Chunk Y out of range");
				}
				chunk_nodes[ydatanode->payload.tag_byte] = section_node;
				if(ydatanode->payload.tag_byte > highest_chunk)
					highest_chunk = ydatanode->payload.tag_byte;
			}
		}
	}

	for(uint8_t i=0;i<16;i++){
		if(!chunk_nodes[i])
			continue;
		//printf("Filling chunk %d\n", i);
		nbt_node *blocks = nbt_find_by_name(chunk_nodes[i], "Blocks");
		if(!blocks){
			die("Could not find Blocks tag");
		}
		if(blocks->type == TAG_BYTE_ARRAY){
			struct nbt_byte_array *blocks_b = (struct nbt_byte_array*)&blocks->payload;

			if (blocks_b->length != 16*16*16)
				die("Incorrect chunk size");

			for(uint32_t y=0;y<16;y++){
				for(uint32_t x=0;x<16;x++){
					for(uint32_t z=0;z<16;z++){
						column->chunks[i].blocks[y][x][z].type = blocks_b->data[(y*16*16)+(x*16)+z];
					}
				}
			}
		}else{
			die("Blocks tag is not a byte array");
		}
	}
	
	return 0;
}

int main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: mcowl [regionfile]\n");
		return 1;
	}

	FILE *regionfile = fopen(argv[1], "rb");
	if(!regionfile)
		die("Could not open region file");


	col_link_t cat[32*32];
	populate_coltable(regionfile, cat);

	nbt_node *coltree = get_col_nbt(regionfile, &cat[coords_to_index(-255, 320)]);
	//nbt_node *coltree = get_col_nbt(regionfile, &cat[coords_to_index(-240, 370)]);

	column_t column;
	if(get_column(&column, coltree))
		die("Auw");

	char watermap[16][16];

	for(uint8_t z=0;z<16;z++){
		for(uint8_t x=0;x<16;x++){
			//watermap[x][z] = (column.chunks[3].blocks[14][x][z].type == 8 || column.chunks[3].blocks[14][x][z].type == 9)?'#':'-';
			if(column.chunks[3].blocks[14][x][z].type == 8 || column.chunks[3].blocks[14][x][z].type == 9){
				watermap[x][z] = '~';
			}else if(column.chunks[3].blocks[14][x][z].type == 1){
				watermap[x][z] = 'S';
			}else if(column.chunks[3].blocks[14][x][z].type == 3 || column.chunks[3].blocks[14][x][z].type == 2){
				watermap[x][z] = '#';
			}else if(column.chunks[3].blocks[14][x][z].type == 12){
				watermap[x][z] = 's';
			}else if(column.chunks[3].blocks[14][x][z].type == 98){
				watermap[x][z] = 'B';
			}else{
				watermap[x][z] = '.';
				printf(";%d;", column.chunks[3].blocks[14][x][z].type);
			}
			//watermap[x][z] = (column.chunks[5].blocks[15][x][z].type != 0)?'#':'-';
			//watermap[x][z] = column.chunks[2].blocks[15][x][z].type;
		}
	}

	printf("Surface water at x[%d-%d], z[%d-%d]:\n", -255%16 * (-255),0,0,0);

	for(uint8_t z=0;z<16;z++){
		for(uint8_t x=0;x<16;x++){
			putchar(watermap[x][15-z]);
			//printf("%02x.",watermap[x][z]);
		}
		putchar('\n');
	}

	//char *tree_ascii = nbt_dump_ascii(coltree);
	//puts(tree_ascii);

	nbt_free(coltree);

	return 0;
}
