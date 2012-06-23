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

	return 0;
}

int render_world_bitmap(bitmap_t *bitmap, const block_t *world_map, uint32_t width, uint32_t height){
	assert(bitmap != NULL);
	assert(world_map != NULL);

	printf_d("Allocating %d bytes (%.1fMiB) for PNM pixel data", width*height*3, (float)(width*height*3)/1024/1024)
	if(pnm_new(bitmap, width, height)){
		printf_d("Could not allocate PNM buffer. Max size is set at %d bytes (%.1fMiB)", MAX_BITMAP_PIXELS*3, (float)(MAX_BITMAP_PIXELS*3)/1024/1024);
		return 1;
	}
	assert(bitmap->pixels != NULL);

	memset(bitmap->pixels, 0x00, width*height*3);

	for(uint32_t z=0;z<height;z++){
		for(uint32_t x=0;x<width;x++){
			uint16_t color[3];
			color[0] = world_map[z*width+x].type >= sizeof(color_legend)/3?0xff:color_legend[world_map[z*width+x].type][0];
			color[1] = world_map[z*width+x].type >= sizeof(color_legend)/3?0xff:color_legend[world_map[z*width+x].type][1];
			color[2] = world_map[z*width+x].type >= sizeof(color_legend)/3?0xff:color_legend[world_map[z*width+x].type][2];

			for(uint8_t color_i=0;color_i<3;color_i++){
				if(world_map[z*width+x].overlay_type){
					color[color_i] *= (double)((double)(tranparency[world_map[z*width+x].overlay_type])/256);
					color[color_i] += (double)color_legend[world_map[z*width+x].overlay_type][color_i]/256 *
							(256-tranparency[world_map[z*width+x].overlay_type]);
				}
				if(world_map[z*width+x].depth < -1 &&
						(world_map[z*width+x].overlay_type == 8 || world_map[z*width+x].overlay_type == 9))
					color[color_i] *= 1+ (double)world_map[z*width+x].depth/32;
				else
					color[color_i] *= 1+ (double)world_map[z*width+x].depth/128;

				if(color[color_i] > 255) color[color_i] = 255;
			}

			bitmap->pixels[(z*width+x)*3+0] = color[0];
			bitmap->pixels[(z*width+x)*3+1] = color[1];
			bitmap->pixels[(z*width+x)*3+2] = color[2];
		}
	}

	return 0;
}
