/*
 * render.c
 *
 *  Created on: Jun 21, 2012
 *      Author: chris
 */

#include "mcowl.h"
#include "legend.h"


int render_world_text(char **text_render, const block_t *world_map, uint32_t width, uint32_t height){
	assert(world_map != NULL);

	*text_render = (char*)malloc((width+1)*height+1);

	if(!*text_render){
		printf_d("Could not allocate %d bytes (%.1fKiB)", width*height+height+1, (double)(width*height+height+1) /1024);
		die("Could not allocate text render buffer");
	}

	memset(*text_render, 0, (width+1)*height+1);
	char *render = *text_render;

	for(uint32_t z=0;z<height;z++){
		for(uint32_t x=0;x<width;x++){
			render[z*(width+1)+x] = world_map[z*width+x].type >= sizeof(text_legend)?'?':text_legend[world_map[z*width+x].type];
		}
		render[z*(width+1)+width] = '\n';
	}

	render[width*height+height] = 0;

	//printf_d("len: %d", strlen(*text_render));

	return 0;
}

int render_world_bitmap(bitmap_t *bitmap, const block_t *world_map, uint32_t width, uint32_t height){
	assert(bitmap != NULL);
	assert(world_map != NULL);

	if(pnm_new(bitmap, width, height)){
		printf_d("Could not allocate PNM buffer of %d bytes (%.1fKiB)", width*height*3, (float)(width*height*3)/1024);
		return 1;
	}
	assert(bitmap->pixels != NULL);

	memset(bitmap->pixels, 0x00, width*height*3);

	for(uint32_t z=0;z<height;z++){
		for(uint32_t x=0;x<width;x++){
			bitmap->pixels[(z*width+x)*3+0] = world_map[z*width+x].type >= sizeof(color_legend)/3?0xff:color_legend[world_map[z*width+x].type][0];
			bitmap->pixels[(z*width+x)*3+1] = world_map[z*width+x].type >= sizeof(color_legend)/3?0xff:color_legend[world_map[z*width+x].type][1];
			bitmap->pixels[(z*width+x)*3+2] = world_map[z*width+x].type >= sizeof(color_legend)/3?0xff:color_legend[world_map[z*width+x].type][2];
		}
	}

	return 0;
}
