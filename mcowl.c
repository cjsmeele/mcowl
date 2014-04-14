/* Copyright (c) 2012, 2013, Chris Smeele
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

#include "mcowl.h"
#include "render.h"
#include "blocks.h"
#include "bitmap.h"

/* mcowl uses FliPPeh's NBT library, which can be found here:
 * https://github.com/FliPPeh/cNBT
 */
#include "nbt.h"

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

size_t get_filesize(FILE* file){
	if(!file){
		return 0;
	}
	fseek(file, 0, 2);
	size_t size = ftell(file);
	fseek(file, 0, 0);
	return size;
}


nbt_node* get_column_nbt(FILE *regionfile, const col_link_t *col_link){
	if(col_link->length_sectors == 0){
		return 0;
	}
	if(fseek(regionfile, col_link->offset*4096, SEEK_SET)){
		printf_d("Could not seek to column at offset %d (region corrupt?)", col_link->offset);
		return 0;
	}

	uint8_t col_header[5];
	size_t bytes_read = fread(col_header, 1, 5, regionfile);
	if(bytes_read != 5){
		printf_d("Could not read column header, aborting (%u/%d bytes read)", (uint32_t)bytes_read, 5);
		return 0;
	}

	uint32_t col_length = ((uint32_t)col_header[0]<<24 | (uint32_t)col_header[1]<<16 | (uint32_t)col_header[2]<<8 | (uint32_t)col_header[3]);

	if(col_length > 1024*1024)
		die("Column too large!");

	uint8_t *col_data = (uint8_t*)malloc(col_length);
	bytes_read = fread(col_data, 1, col_length, regionfile);
	if(bytes_read != col_length)
		die("Could not read column data, aborting");

	nbt_node *tree = nbt_parse_compressed(col_data, col_length);

	if(col_data) free(col_data);
	if(!tree){
		printf_d("NBT error: %s", nbt_error_to_string(errno));
		return 0;
	}
	return tree;
}


int populate_coltable(FILE *regionfile, col_link_t *cat,
		uint32_t *min_x, uint32_t *min_z, uint32_t *max_x, uint32_t *max_z){

	uint32_t region_header[32*32];
	int32_t column_count = 0;
	
	fseek(regionfile, 0, SEEK_SET);
	size_t bytes_read = fread(region_header, sizeof(uint32_t), 32*32, regionfile);
	if(bytes_read != 32*32){
		printf_d("Could not read header from region file, got %u/%d bytes", (uint32_t)bytes_read, 32*32);
		return -1;
	}

	uint32_t x, z;
	x = z = 0;

	if(min_x && min_z && max_x && max_z)
		*min_x = *min_z = *max_x = *max_z = 9001;

	for(uint32_t i=0; i<32*32; i++){
		x = i/32;
		z = i%32;

		swap_bytes(&region_header[i], sizeof(uint32_t));
		cat[i].offset = region_header[i]>>8;
		cat[i].length_sectors = region_header[i] & 0xff;

		if(cat[i].length_sectors){
			column_count++;

			if(min_x && min_z && max_x && max_z){
				nbt_node *column = get_column_nbt(regionfile, &cat[i]);
				if(!column){
					printf_d("Warning: Could not read column %u (%u,%u)!", i, x, z);
				}else{
					nbt_free(column);

					if(*min_x == 9001 || x < *min_x) *min_x = x;
					if(*max_x == 9001 || x > *max_x) *max_x = x;
					if(*min_z == 9001 || z < *min_z) *min_z = z;
					if(*max_z == 9001 || z > *max_z) *max_z = z;
				}
			}
		}
	}

	return column_count;
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
		print_d("Warning: Could not find 'Sections' NBT tag");
		return 1;
	}

	nbt_node *chunk_nodes[16];
	memset(chunk_nodes, 0, 16*sizeof(nbt_node*));

	struct list_head* pos;

	column->allocated_b = 0;

	list_for_each(pos, &sections_list->entry){
		nbt_node *section_node = list_entry(pos, struct nbt_list, entry)->data;
		struct nbt_list* section_datalist = section_node->payload.tag_compound;
		struct list_head* spos;

		list_for_each(spos, &section_datalist->entry){
			struct nbt_node* ydatanode = list_entry(spos, struct nbt_list, entry)->data;
			if(ydatanode->name == NULL)
				continue;
			if(strcmp(ydatanode->name, "Y") == 0){
				if(ydatanode->payload.tag_byte >= 16 || ydatanode->payload.tag_byte < 0){
					die("Chunk Y out of range");
				}
				chunk_nodes[ydatanode->payload.tag_byte] = section_node;
				column->allocated_b |= 1 << ydatanode->payload.tag_byte;
			}
		}
	}

	for(uint8_t i=0; i<16; i++){
		if(!chunk_nodes[i])
			continue;
		nbt_node *blocks = nbt_find_by_name(chunk_nodes[i], "Blocks");
		if(!blocks)
			die("Could not find Blocks tag");
		if(blocks->type == TAG_BYTE_ARRAY){
			struct nbt_byte_array *blocks_b = (struct nbt_byte_array*)&blocks->payload;

			if(blocks_b->length != 16*16*16)
				die("Incorrect chunk size");

			for(uint32_t y=0; y<16; y++){
				for(uint32_t x=0; x<16; x++){
					for(uint32_t z=0; z<16; z++){
						column->chunks[i].blocks[y][x][z].type = blocks_b->data[(y*16*16)+(x*16)+z];
					}
				}
			}
		}else{
			die("Blocks tag is not a byte array");
		}
		nbt_node *datas = nbt_find_by_name(chunk_nodes[i], "Data");
		if(!datas)
			die("Could not find Data tag");
		if(datas->type == TAG_BYTE_ARRAY){
			struct nbt_byte_array *datas_b = (struct nbt_byte_array*)&datas->payload;

			if(datas_b->length != 16*16*16/2)
				die("Incorrect chunk size");

			for(uint32_t y=0; y<16; y++){
				for(uint32_t x=0; x<16; x++){
					for(uint32_t z=0; z<16; z++){
						if(z&1)
							column->chunks[i].blocks[y][x][z].data = datas_b->data[((y*16*16)+(x*16)+z)/2] >> 4;
						else
							column->chunks[i].blocks[y][x][z].data = datas_b->data[((y*16*16)+(x*16)+z)/2] & 0x0f;
					}
				}
			}
		}else{
			die("Data tag is not a byte array");
		}
	}
	
	return 0;
}

int get_filename(char *buffer, size_t buffer_len, const char *pathname){
	if(strlen(pathname) < 1 || buffer_len < 1 || pathname[strlen(pathname)-1] == '/' ||
			pathname[strlen(pathname)-1] == '\\'){
		return -1;
	}
	for(int64_t i=strlen(pathname)-1; i>=0; i--){
		if(pathname[i] == '/' || pathname[i] == '\\'){
			if(strlen(&pathname[i+1]) > buffer_len-1)
				return -1;
			strcpy(buffer, &pathname[i+1]);
			break;
		}
		if(i == 0){
			if(strlen(&pathname[i]) > buffer_len-1)
				return -1;
			strcpy(buffer, &pathname[i]);
			break;
		}
	}
	return 0;
}

int check_allocated_regions(const char *region_allocation_map[64][64], const char *region_files[],
		uint32_t region_count,
		int32_t *column_min_x, int32_t *column_min_z, int32_t *column_max_x, int32_t *column_max_z){

	memset(region_allocation_map, 0, 64*64*sizeof(char*));
	int32_t found_regions = 0;
	char filename_buffer[32];

	int32_t region_min_x, region_min_z, region_max_x, region_max_z;

	region_min_x = region_min_z = region_max_x = region_max_z = 9001;
	*column_min_x = *column_min_z = *column_max_x = *column_max_z = 9001;

	for(uint32_t i=0; i<region_count; i++){
		if(strlen(region_files[i]) < 9){
			printf_d("Skipping region \"%s\": Filename too short", region_files[i]);
			continue;
		}
		if(get_filename(filename_buffer, 32, region_files[i])){
			printf_d("Skipping region \"%s\": Could not get filename out of path", region_files[i]);
			continue;
		}

		int32_t region_x, region_z;
		int32_t matched = sscanf(filename_buffer, "r.%d.%d.mca", &region_x, &region_z);

		if(matched != 2){
			if(matched == -1){
				printf_d("errno: %d", errno);
			}
			printf_d("Skipping region \"%s\": Could not parse filename", filename_buffer);
		}else{
			if(region_x < -32 || region_x > 31 || region_z < -32 || region_z > 31){
				printf_d("Skipping region \"%s\": Coordinates out of range: %d,%d", filename_buffer, region_x, region_z);
				continue;
			}

			FILE *regionfile = fopen(region_files[i], "rb");
			if(!regionfile){
				printf_d("Skipping region \"%s\": Could not open region file", filename_buffer);
				continue;
			}

			col_link_t cat[32*32];
			uint32_t r_column_min_x, r_column_min_z, r_column_max_x, r_column_max_z;
			r_column_min_x = r_column_min_z = r_column_max_x = r_column_max_z = 9001;
			int32_t col_count = populate_coltable(regionfile, cat,
					&r_column_min_x, &r_column_min_z, &r_column_max_x, &r_column_max_z);

			fclose(regionfile);

			if(col_count == -1){
				printf_d("Skipping region \"%s\": Could not read column allocation table", filename_buffer);
				continue;
			}

			if(col_count == 0){
				printf_d("Skipping region \"%s\": Region has no allocated columns", filename_buffer);
				continue;
			}

			printf_d("Region (%d,%d) from \"%s\" has %d columns", region_x, region_z, filename_buffer, col_count);
			region_allocation_map[region_x+32][region_z+32] = region_files[i];
			found_regions++;

			if(region_min_x == 9001 || region_x < region_min_x) region_min_x = region_x;
			if(region_max_x == 9001 || region_x > region_max_x) region_max_x = region_x;
			if(region_min_z == 9001 || region_z < region_min_z) region_min_z = region_z;
			if(region_max_z == 9001 || region_z > region_max_z) region_max_z = region_z;

			if(*column_min_x == 9001 || ((int32_t)r_column_min_x+32*region_x) < *column_min_x) *column_min_x = ((int32_t)r_column_min_x+32*region_x);
			if(*column_min_z == 9001 || ((int32_t)r_column_min_z+32*region_z) < *column_min_z) *column_min_z = ((int32_t)r_column_min_z+32*region_z);
			if(*column_max_x == 9001 || ((int32_t)r_column_max_x+32*region_x) > *column_max_x) *column_max_x = ((int32_t)r_column_max_x+32*region_x);
			if(*column_max_z == 9001 || ((int32_t)r_column_max_z+32*region_z) > *column_max_z) *column_max_z = ((int32_t)r_column_max_z+32*region_z);
		}
	}

	*column_min_x = region_min_x*32;
	*column_min_z = region_min_z*32;
	*column_max_x = region_max_x*32+32-1;
	*column_max_z = region_max_z*32+32-1;

	return found_regions;
}

int map_column(bitmap_t *bitmap, FILE *regionfile, col_link_t *col_link, int rendermode){
	memset(bitmap->pixels, 0, 16*16*BITMAP_BYTES_PER_PIXEL);

	block_t column_map[16][16];
	memset(column_map, 0, 16*16*sizeof(block_t));

	column_t column;

	nbt_node *col_tree = get_column_nbt(regionfile, col_link);
	if(!col_tree){
		printf_d("Could not map column: Could not get column NBT data from region file (offset: %d)", col_link->offset);
		return 1;
	}
	get_column(&column, col_tree);
	nbt_free(col_tree);

	uint16_t blocks_mapped = 0;

	for(int8_t i=15; i>=0; i--){
		if(!(column.allocated_b & (1<<i)))
			continue;
		for(int8_t y=15; y>=0; y--){
			for(uint8_t x=0; x<16; x++){
				for(uint8_t z=0; z<16; z++){
					if(!column_map[z][x].type && column.chunks[i].blocks[y][z][x].type){
						if((rendermode != RENDER_DEPTH) ||
								blockdescs[column.chunks[i].blocks[y][z][x].type].color[3] == 255){
							column_map[z][x].type = column.chunks[i].blocks[y][z][x].type;
							column_map[z][x].data = column.chunks[i].blocks[y][z][x].data;
							column_map[z][x].depth = i*16+y-64;
							blocks_mapped++;
						}else{
							if(blockdescs[column.chunks[i].blocks[y][z][x].type].color[3] &&
									!column_map[z][x].overlay_type)
								column_map[z][x].overlay_type = column.chunks[i].blocks[y][z][x].type;
						}
						if(blocks_mapped >= 16*16)
							break;
					}
				}
			}
		}
	}
	render_column_flat(bitmap, column_map, rendermode);
	return 0;
}

int map_region(bitmap_t *bitmap, bitmap_t *column_bitmap, FILE *regionfile, int rendermode){
	col_link_t cat[32*32];
	populate_coltable(regionfile, cat, 0,0,0,0);

	memset(bitmap->pixels, 0,  32*16*32*16*BITMAP_BYTES_PER_PIXEL);
	memset(column_bitmap->pixels, 0, 16*16*BITMAP_BYTES_PER_PIXEL);

	for(uint32_t x=0; x<32; x++){
		for(uint32_t z=0; z<32; z++){
			block_t column_map[16][16];
			memset(column_map, 0, 16*16*sizeof(block_t));
			if(cat[x*32+z].length_sectors){
				memset(column_bitmap->pixels, 0, 16*16*BITMAP_BYTES_PER_PIXEL);
				map_column(column_bitmap, regionfile, &cat[x*32+z], rendermode);
				for(uint8_t cx=0; cx<16; cx++){
					for(uint8_t cz=0; cz<16; cz++){
						memcpy(&bitmap->pixels[(z*32*16*16+x*16+cz*32*16+cx)*BITMAP_BYTES_PER_PIXEL],
								&column_bitmap->pixels[(cz*16+cx)*BITMAP_BYTES_PER_PIXEL], BITMAP_BYTES_PER_PIXEL);
					}
				}
			}
		}
	}

	return 0;
}

bitmap_t *map_world(int region_count, const char *region_files[], int rendermode){
	print_d("Mapping world");

	const char *region_allocation_map[64][64];
	int32_t min_x, min_z, max_x, max_z;
	region_count = check_allocated_regions(region_allocation_map, region_files, region_count,
			&min_x, &min_z, &max_x, &max_z);

	printf_d("World has %u allocated region(s), from column (%d,%d) to (%d,%d)", region_count,
			min_x, min_z, max_x, max_z);

	if(region_count == 0)
		die("No regions to be mapped!");

	uint32_t width_columns  = max_x-min_x+1;
	uint32_t height_columns = max_z-min_z+1;
	uint32_t width  = width_columns*16;
	uint32_t height = height_columns*16;

	uint32_t bitmap_width  = width;
	uint32_t bitmap_height = height;

	printf_d("Allocating world pixel buffer (%ux%u)", bitmap_width, bitmap_height);
	bitmap_t *bitmap;
	if(!(bitmap = bitmap_new(bitmap_width, bitmap_height)))
		die("Could not create pixel buffer");
	memset(bitmap->pixels, 0, bitmap_width*bitmap_height*BITMAP_BYTES_PER_PIXEL);

	bitmap_t *region_bitmap;
	bitmap_t *column_bitmap;

	printf_d("Allocating region pixel buffer (%ux%u)", 32*16, 32*16);
	if(!(region_bitmap = bitmap_new(32*16, 32*16)))
		die("Could not create pixel buffer");
	memset(region_bitmap->pixels, 0, (32*16*32*16)*3);
	printf_d("Allocating column pixel buffer (%ux%u)", 16, 16);
	if(!(column_bitmap = bitmap_new(16, 16)))
		die("Could not create pixel buffer");
	memset(column_bitmap->pixels, 0, (16*16)*3);

	int32_t x_start = 9001;
	int32_t z_start = 9001;
	int32_t x_end = 9001;
	//int32_t z_end = 9001;

	for(int32_t x=-32; x<32; x++){
		for(int32_t z=-32; z<32; z++){
			if(region_allocation_map[x+32][z+32] && x_start == 9001)
				x_start = x;
			if(region_allocation_map[x+32][z+32])
				x_end = x;
		}
	}
	for(int32_t z=-32; z<32; z++){
		for(int32_t x=-32; x<32; x++){
			if(region_allocation_map[x+32][z+32] && z_start == 9001)
				z_start = z;
			//if(region_allocation_map[x+32][z+32])
			//	z_end = z;
		}
	}
	int32_t regions_x = x_end - x_start + 1;

	for(int32_t x=-32; x<32; x++){
		for(int32_t z=-32; z<32; z++){
			if(region_allocation_map[x+32][z+32]){

				printf_d("Mapping region %d,%d", x, z);

				FILE *regionfile = fopen(region_allocation_map[x+32][z+32], "rb");
				if(!regionfile){
					printf_d("Could not open region file for (%d,%d) \"%s\"",
							x, z, region_allocation_map[x+32][z+32]);
					continue;
				}
				map_region(region_bitmap, column_bitmap, regionfile, rendermode);
				fclose(regionfile);

				for(uint32_t col_x=0; col_x<32; col_x++){
					if((int32_t)(x*32+col_x) < min_x || (int32_t)(x*32+col_x) > max_x)
						continue;

					for(uint32_t col_z=0; col_z<32; col_z++){
						if((int32_t)(z*32+col_z) < min_z || (int32_t)(z*32+col_z) > max_z)
							continue;

						for(uint32_t block_x=0; block_x<16; block_x++){
							for(uint32_t block_z=0; block_z<16; block_z++){

								uint32_t world_index = 0;

								world_index += (z-z_start) * regions_x * 32*32 * 16*16;
								world_index += (x-x_start) * 32*16;
								world_index += col_z * regions_x * 32 * 16*16;
								world_index += col_x * 16;
								world_index += block_z * regions_x * 32 * 16;
								world_index += block_x;

								uint32_t region_index = 0;

								region_index += col_x * 32 * 16*16;
								region_index += col_z * 16;
								region_index += block_x * 32 * 16;
								region_index += block_z;

								if(world_index > width*height){
									printf_d("Column: %u,%u  Block: %u,%u  World: %ux%u = %u  i: %u",
											col_x, col_z, block_x, block_z, width, height, width*height, world_index);
									die("Block index went out of range!");
								}
								memcpy(&bitmap->pixels[world_index*BITMAP_BYTES_PER_PIXEL], &region_bitmap->pixels[region_index*BITMAP_BYTES_PER_PIXEL], 4);
							}
						}
					}
				}
			}
		}
	}

	bitmap_free(region_bitmap);
	bitmap_free(column_bitmap);

	return bitmap;
}

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("Usage: mcowl [regionfiles]\n");
		return 0;
	}

	fprintf(stderr, "mcowl 0.1, compiled on %s\n",
			__DATE__ " " __TIME__);
	fprintf(stderr, "Copyright (c) 2012, Chris Smeele\n"
			"All rights reserved.\n\n");

	int rendermode = RENDER_DEPTH;

	bitmap_t *bitmap_render = map_world(argc-1, (const char**)argv+1, rendermode);

	if(!bitmap_render->pixels)
		die("Render empty, no bitmap to write out!");

	FILE *pnmfile = fopen("world.pnm", "wb");
	if(!pnmfile)
		die("Could not open pnm file for writing");
	bitmap_writepnm(bitmap_render, pnmfile);
	fclose(pnmfile);
	bitmap_free(bitmap_render);

	print_d("World saved as 'world.pnm'");

	return 0;
}
