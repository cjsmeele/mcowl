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

#include "mcowl.h"
#include "legend.h"


bitmap_t block_bitmaps[256];
bitmap_t data_bitmaps[16][16];

size_t get_filesize(FILE* file){
	if(!file){
		return 0;
	}
	fseek(file, 0, 2);
	size_t size = ftell(file);
	fseek(file, 0, 0);
	return size;
}

int get_bitmaps_blocks(const char *filename){
	FILE *bmp_blocks_file = fopen(filename, "rb");
	if(!bmp_blocks_file){
		printf_d("Could not open blocks bitmap table '%s'", filename);
		return 1;
	}

	size_t filesize = get_filesize(bmp_blocks_file);

	uint8_t *bmp_blocks = malloc(filesize);
	if(!bmp_blocks) die("Could not allocate blocks bitmap file buffer");

	size_t bytes_read = fread(bmp_blocks, 1, filesize, bmp_blocks_file);
	if(bytes_read != filesize){
		printf_d("Could not read blocks bitmap file (%u/%u bytes read)", (uint32_t)bytes_read, (uint32_t)filesize);
		fclose(bmp_blocks_file);
		return 1;
	}
	fclose(bmp_blocks_file);

	if(pnm_parse_bulk(block_bitmaps, ISOMETRIC_BLOCK_SIZE, ISOMETRIC_BLOCK_SIZE, bmp_blocks, filesize, false)){
		print_d("Could not parse block bitmaps");
		return 1;
	}

	return 0;
}
int get_bitmaps_data(const char *filename){
	return 0;
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

	for(uint32_t i=0;i<32*32;i++){
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

	for(uint8_t i=0;i<16;i++){
		if(!chunk_nodes[i])
			continue;
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

int get_filename(char *buffer, size_t buffer_len, const char *pathname){
	if(strlen(pathname) < 1 || buffer_len < 1 || pathname[strlen(pathname)-1] == '/' ||
			pathname[strlen(pathname)-1] == '\\'){
		return -1;
	}
	for(int64_t i=strlen(pathname)-1;i>=0;i--){
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

	for(uint32_t i=0; i<region_count;i++){
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
	if(rendermode == RENDER_FLAT){
		memset(bitmap->pixels, 0, 16*16*3);

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

		for(int8_t i=15;i>=0;i--){
			if(!(column.allocated_b & (1<<i)))
				continue;
			for(int8_t y=15;y>=0;y--){
				for(uint8_t x=0;x<16;x++){
					for(uint8_t z=0;z<16;z++){
						if(!column_map[z][x].type && column.chunks[i].blocks[y][z][x].type){
							if(!tranparency[column.chunks[i].blocks[y][z][x].type]){
								column_map[z][x].type = column.chunks[i].blocks[y][z][x].type;
								column_map[z][x].depth = i*16+y-64;
								blocks_mapped++;
							}else{
								if(tranparency[column.chunks[i].blocks[y][z][x].type] != 255 &&
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
		render_column_flat(bitmap, column_map);
	}else{
		int32_t s = ISOMETRIC_BLOCK_SIZE;
		memset(bitmap->pixels, 0, (15*(s+2)+s) * ((s+2)*16+(s/2+1-2)*256));

		column_t column;

		nbt_node *col_tree = get_column_nbt(regionfile, col_link);
		if(!col_tree){
			printf_d("Could not map column: Could not get column NBT data from region file (offset: %d)", col_link->offset);
			return 1;
		}
		get_column(&column, col_tree);
		nbt_free(col_tree);

		int32_t i_start_z = 0;
		int32_t i_start_x = 0;
		int32_t i_end_z = 0;
		int32_t i_end_x = 0;

		for(uint32_t ia=0;ia<32;ia++){
			if(ia < 16){
				i_start_z = ia;
				i_start_x = 0;
				i_end_z = 0;
				i_end_x = ia;
			}else{
				i_start_z = 15;
				i_start_x = ia%16+1;
				i_end_z = ia%16-1;
				i_end_x = 15;
			}
			int32_t z, x;

			for(z=i_start_z, x=i_start_x;
					z >= i_end_z && x <= i_end_x;
					z--, x++){
				int32_t real_z = ia*((s+2)/2/2);
				int32_t real_x = x*((s+2)/2) + (15-z)*((s+2)/2);

				for(int16_t chunk_y=0;chunk_y<16;chunk_y++){
					for(int16_t block_y=0;block_y<16;block_y++){
						int32_t block_y_total = chunk_y*16+block_y;
						real_z = ia*((s+2)/2/2) + ((s/2+1-2)*(255-block_y_total));
						//bitmap->pixels[(real_z*(15*16+s)+real_x)*3+1] = 0xff;
						uint8_t b = column.chunks[chunk_y].blocks[block_y][x][z].type;
						for(int32_t pz=0;pz<s;pz++){
							for(int32_t px=0;px<s;px++){
								if(!(block_bitmaps[b].pixels[(pz*s+px)*3+0] == 255 &&
										block_bitmaps[b].pixels[(pz*s+px)*3+1] == 0 &&
										block_bitmaps[b].pixels[(pz*s+px)*3+2] == 255)){
									memcpy(&bitmap->pixels[((real_z+pz)*(15*(s+2)+s)+real_x+px)*3],
											&block_bitmaps[b].pixels[(pz*s+px)*3], 3);
								}
							}
						}
					}
				}
				//printf_d("ia: %d  %d,%d  =>  %d,%d", ia, z,x, real_z,real_x);
			}
		}
	}

	return 0;
}

int map_region(bitmap_t *bitmap, bitmap_t *column_bitmap, FILE *regionfile, int rendermode){
	assert(rendermode == RENDER_FLAT);
	col_link_t cat[32*32];
	populate_coltable(regionfile, cat, 0,0,0,0);

	assert(bitmap->width == 32*16);
	assert(bitmap->height == 32*16);
	assert(bitmap->pixels != NULL);
	memset(bitmap->pixels, 0, 32*16*32*16*3);

	assert(column_bitmap->width == 16);
	assert(column_bitmap->height == 16);
	assert(column_bitmap->pixels != NULL);
	memset(column_bitmap->pixels, 0, 16*16*3);

	for(uint32_t x=0;x<32;x++){
		for(uint32_t z=0;z<32;z++){
			block_t column_map[16][16];
			memset(column_map, 0, 16*16*sizeof(block_t));
			if(cat[x*32+z].length_sectors){
				memset(column_bitmap->pixels, 0, 16*16*3);
				map_column(column_bitmap, regionfile, &cat[x*32+z], rendermode);
				for(uint8_t cx=0;cx<16;cx++){
					for(uint8_t cz=0;cz<16;cz++){
						memcpy(&bitmap->pixels[(z*32*16*16+x*16+cz*32*16+cx)*3], &column_bitmap->pixels[(cz*16+cx)*3], 3);
					}
				}
			}
		}
	}

	return 0;
}

int map_world(bitmap_t *bitmap, const char *worldname, uint16_t region_count, const char *region_files[], int rendermode){
	printf_d("Mapping world \"%s\"", worldname);

	if(rendermode == RENDER_FLAT){
		print_d("Render mode set to FLAT");
	}else if(rendermode == RENDER_ISOMETRIC){
		print_d("Render mode set to ISOMETRIC");
	}else{
		die("");
	}

	const char *region_allocation_map[64][64];
	int32_t min_x, min_z, max_x, max_z;
	region_count = check_allocated_regions(region_allocation_map, region_files, region_count,
			&min_x, &min_z, &max_x, &max_z);

	printf_d("World has %u allocated region(s), from column (%d,%d) to (%d,%d)", region_count,
			min_x, min_z, max_x, max_z);

	if(region_count == 0)
		die("No regions to be mapped!");

	uint32_t width_columns = max_x-min_x+1;
	uint32_t height_columns = max_z-min_z+1;
	uint32_t width = width_columns*16;
	uint32_t height = height_columns*16;

	uint32_t bitmap_width = rendermode==RENDER_ISOMETRIC?width*ISOMETRIC_BLOCK_SIZE:width;
	uint32_t bitmap_height = rendermode==RENDER_ISOMETRIC?height*ISOMETRIC_BLOCK_SIZE:height;

	printf_d("Allocating world pixel buffer (%ux%u)", bitmap_width, bitmap_height);
	if(pnm_new(bitmap, bitmap_width, bitmap_height))
		die("Could not create pixel buffer");
	memset(bitmap->pixels, 0, bitmap_width*bitmap_height*3);

	bitmap_t region_bitmap;
	bitmap_t column_bitmap;

	if(rendermode == RENDER_ISOMETRIC){
		printf_d("Allocating region pixel buffer (%ux%u)", 32*16*ISOMETRIC_BLOCK_SIZE, 32*16*ISOMETRIC_BLOCK_SIZE);
		if(pnm_new(&region_bitmap, 32*16*ISOMETRIC_BLOCK_SIZE, 32*16*ISOMETRIC_BLOCK_SIZE))
			die("Could not create pixel buffer");
		memset(region_bitmap.pixels, 0, (32*16*ISOMETRIC_BLOCK_SIZE*32*16*ISOMETRIC_BLOCK_SIZE)*3);
		printf_d("Allocating column pixel buffer (%ux%u)", 16*ISOMETRIC_BLOCK_SIZE, 16*ISOMETRIC_BLOCK_SIZE);
		if(pnm_new(&column_bitmap, 16, 16))
			die("Could not create pixel buffer");
		memset(column_bitmap.pixels, 0, (16*16)*3);
	}else{
		printf_d("Allocating region pixel buffer (%ux%u)", 32*16, 32*16);
		if(pnm_new(&region_bitmap, 32*16, 32*16))
			die("Could not create pixel buffer");
		memset(region_bitmap.pixels, 0, (32*16*32*16)*3);
		printf_d("Allocating column pixel buffer (%ux%u)", 16, 16);
		if(pnm_new(&column_bitmap, 16, 16))
			die("Could not create pixel buffer");
		memset(column_bitmap.pixels, 0, (16*16)*3);
	}

	int32_t x_start = 9001;
	int32_t z_start = 9001;
	int32_t x_end = 9001;
	int32_t z_end = 9001;

	for(int32_t x=-32;x<32;x++){
		for(int32_t z=-32;z<32;z++){
			if(region_allocation_map[x+32][z+32] && x_start == 9001)
				x_start = x;
			if(region_allocation_map[x+32][z+32])
				x_end = x;
		}
	}
	for(int32_t z=-32;z<32;z++){
		for(int32_t x=-32;x<32;x++){
			if(region_allocation_map[x+32][z+32] && z_start == 9001)
				z_start = z;
			if(region_allocation_map[x+32][z+32])
				z_end = z;
		}
	}
	int32_t regions_x = x_end - x_start + 1;

	if(rendermode == RENDER_FLAT){
		for(int32_t x=-32;x<32;x++){
			for(int32_t z=-32;z<32;z++){
				if(region_allocation_map[x+32][z+32]){

					printf_d("Mapping region %d,%d", x, z);

					FILE *regionfile = fopen(region_allocation_map[x+32][z+32], "rb");
					if(!regionfile){
						printf_d("Could not open region file for (%d,%d) \"%s\"",
								x, z, region_allocation_map[x+32][z+32]);
						continue;
					}
					map_region(&region_bitmap, &column_bitmap, regionfile, rendermode);
					fclose(regionfile);

					for(uint32_t col_x=0;col_x<32;col_x++){
						if((int32_t)(x*32+col_x) < min_x || (int32_t)(x*32+col_x) > max_x)
							continue;

						for(uint32_t col_z=0;col_z<32;col_z++){
							if((int32_t)(z*32+col_z) < min_z || (int32_t)(z*32+col_z) > max_z)
								continue;

							for(uint32_t block_x=0;block_x<16;block_x++){
								for(uint32_t block_z=0;block_z<16;block_z++){

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
									memcpy(&bitmap->pixels[world_index*3], &region_bitmap.pixels[region_index*3], 3);
								}
							}
						}
					}
				}
			}
		}
	}else if(rendermode == RENDER_ISOMETRIC){
		int32_t i_start_z = 0;
		int32_t i_end_x = 0;

		for(uint32_t ia=0;(ia<64||ia<64);ia++){
			i_start_z = ia;
			i_end_x = ia;
			int32_t z, x;

			for(z=i_start_z, x=0;
					z >= 0 && x <= i_end_x;
					z--, x++){

				if(region_allocation_map[x][z]){
					printf_d("Mapping region %d,%d", x-32, z-32);

					FILE *regionfile = fopen(region_allocation_map[x][z], "rb");
					if(!regionfile){
						printf_d("Could not open region file for (%d,%d) \"%s\"",
								x-32, z-32, region_allocation_map[x][z]);
						continue;
					}
					map_region(&region_bitmap, &column_bitmap, regionfile, rendermode);
					fclose(regionfile);

					for(uint32_t col_x=0;col_x<32;col_x++){
						if((int32_t)(x*32+col_x) < min_x || (int32_t)(x*32+col_x) > max_x)
							continue;

						for(uint32_t col_z=0;col_z<32;col_z++){
							if((int32_t)(z*32+col_z) < min_z || (int32_t)(z*32+col_z) > max_z)
								continue;

							for(uint32_t block_x=0;block_x<16;block_x++){
								for(uint32_t block_z=0;block_z<16;block_z++){

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
									memcpy(&bitmap->pixels[world_index*3], &region_bitmap.pixels[region_index*3], 3);
								}
							}
						}
					}
				}

			}
		}

	}else{
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("Usage: mcowl [worldname] [regionfiles]\n");
		return 0;
	}

	fprintf(stderr, "mcowl 0.1, compiled on %s for %s\n",
			__DATE__ " " __TIME__, OSNAME);
	fprintf(stderr, "Copyright (c) 2012, Chris Smeele\n"
			"All rights reserved.\n\n");

	if(strlen(argv[1]) > 63 || !strlen(argv[1]))
		die("World name too long");

	char worldname[64];
	strcpy(worldname, argv[1]);

	for(uint16_t i=0;i<strlen(worldname);i++){
		if(
				(worldname[i] < '0' || worldname[i] > '9') &&
				(worldname[i] < 'A' || worldname[i] > 'Z') &&
				(worldname[i] < 'a' || worldname[i] > 'z') &&
				(worldname[i] != '-' && worldname[i] != '_')){
			die("Invalid characters in world name");
		}
	}

	int rendermode = RENDER_ISOMETRIC;

	if(get_bitmaps_blocks("blocks6.pnm") || get_bitmaps_data("data6.pnm")){
		print_d("Warning: Could not parse blocks.pnm or data.pnm, needed for isometric rendering");
		if(rendermode == RENDER_ISOMETRIC){
			die("Can not render without block bitmaps");
		}
	}

	bitmap_t bitmap_render;
	memset(&bitmap_render, 0, sizeof(bitmap_t));


	/*FILE *rf = fopen("testregions/loca/Midpoint/region/r.-1.0.mca", "rb");
	pnm_new(&bitmap_render, 15*(ISOMETRIC_BLOCK_SIZE+2)+ISOMETRIC_BLOCK_SIZE,
			(ISOMETRIC_BLOCK_SIZE+2)*16+(ISOMETRIC_BLOCK_SIZE/2+1-2)*256);
	col_link_t cat[32*32];
		populate_coltable(rf, cat, 0,0,0,0);
	map_column(&bitmap_render, rf, &cat[19*32+7], RENDER_ISOMETRIC);*/
	map_world(&bitmap_render, argv[1], argc-2, (const char**)argv+2, RENDER_FLAT);

	if(!bitmap_render.pixels)
		die("Render empty, no bitmap to write out!");

	FILE *pnmfile = fopen("world.pnm", "wb");
	if(!pnmfile)
		die("Could not open pnm file for writing");
	pnm_write(&bitmap_render, pnmfile);
	fclose(pnmfile);
	if(bitmap_render.pixels)
		free(bitmap_render.pixels);

	print_d("Temporary image saved as 'world.pnm'");

	FILE *pnm2png_check = 0;

	char pngfilename[256];
	sprintf(pngfilename, "mcowl-%s.png", worldname);

	switch(OSTYPE){
	case OS_LINUX:
	case OS_MACOS:
		pnm2png_check = fopen("pnm2png", "rb");
		break;
	case OS_WIN32:
		pnm2png_check = fopen("pnm2png.exe", "rb");
		break;
	case OS_OTHER:
	default:
		printf("\n  Warning: Your OS type is specified as OTHER, or '%s'.\n"
				"  I may not be able to find or call the pnm2png executable.\n\n", OSNAME);
		pnm2png_check = fopen("pnm2png", "rb");
		break;
	}

	int converter_exit = 7;

	if(pnm2png_check){
		fclose(pnm2png_check);
		print_d("Trying to convert 'world.pnm' to a PNG file");
		char command[256];

		switch(OSTYPE){
		case OS_OTHER:
		case OS_LINUX:
		case OS_MACOS:
			sprintf(command, "./pnm2png %s", worldname);
			break;
		case OS_WIN32:
			sprintf(command, "pnm2png.exe %s", worldname);
			break;
		default:
			die("whooooooops");
			break;
		}
		converter_exit = system(command);
		if(converter_exit){
			printf_d("pnm2png returned %d", converter_exit);
			printf("\n  Either I could not execute pnm2png, or the program itself failed.\n"
					"  If you're unhappy with the PNM file format, please convert it yourself.\n\n");
		}else{
			FILE *png_check = fopen(pngfilename, "rb");
			if(png_check){
				print_d("File successfully converted");
				printf("\n  PNG file saved as '%s'!\n\n", pngfilename);
				fclose(png_check);
			}else{
				print_d("pnm2png reports success, but I can't find the PNG file!");
				printf("\n  It seems something went wrong while converting.\n"
						"  See if you can find a PNG file in my directory.\n"
						"  If the PNG file does not exist you can use a tool like The GIMP to convert\n"
						"  the world.pnm file yourself. Sorry :)\n\n");
			}
		}
	}else{
		printf("\n  I could not find the pnm2png executable.\n"
				"  If you're unhappy with the PNM file format, please convert it yourself.\n"
				"  (The GIMP can handle PNM files)\n\n");
	}

	return 0;
}
