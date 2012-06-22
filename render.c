/*
 * render.c
 *
 *  Created on: Jun 21, 2012
 *      Author: chris
 */

#include "mcowl.h"
#include "legend.h"


int render_world_text(char **text_render, const block_t *world_map, uint32_t width, uint32_t height){

	printf_d("Rendering world map, %d x %d", width, height);

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
