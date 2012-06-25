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


int render_column_iso(){
	print_d("Isometric mapping is not yet implemented");
	return 2;
}

int render_column_flat(bitmap_t *bitmap, block_t slice[16][16]){
	assert(bitmap != NULL);
	assert(bitmap->pixels != NULL);
	memset(bitmap->pixels, 0, bitmap->width*bitmap->height*3);

	for(uint32_t z=0;z<16;z++){
		for(uint32_t x=0;x<16;x++){
			uint16_t color[3];
			color[0] = slice[x][z].type >= sizeof(color_legend)/3?0xff:color_legend[slice[x][z].type][0];
			color[1] = slice[x][z].type >= sizeof(color_legend)/3?0xff:color_legend[slice[x][z].type][1];
			color[2] = slice[x][z].type >= sizeof(color_legend)/3?0xff:color_legend[slice[x][z].type][2];

			for(uint8_t color_i=0;color_i<3;color_i++){
				if(slice[x][z].overlay_type){
					color[color_i] *= (double)((double)(tranparency[slice[x][z].overlay_type])/256);
					color[color_i] += (double)color_legend[slice[x][z].overlay_type][color_i]/256 *
							(256-tranparency[slice[x][z].overlay_type]);
				}
				double multiplier = 1;
				int16_t depth = slice[x][z].depth;
				if(depth > 30){
					multiplier = 1.5;
				}else if(depth >= 0){
					multiplier += (double)depth/30/2;
				}else if(depth >= -30){
					multiplier += (double)depth/30/2;
				}else{
					multiplier = 0.5;
				}

				color[color_i] *= multiplier;

				if(color[color_i] > 255) color[color_i] = 255;
			}

			bitmap->pixels[(z*16+x)*3+0] = color[0];
			bitmap->pixels[(z*16+x)*3+1] = color[1];
			bitmap->pixels[(z*16+x)*3+2] = color[2];
		}
	}

	return 0;
}
