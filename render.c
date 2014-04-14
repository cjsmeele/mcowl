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

#include "render.h"
#include "mcowl.h"
#include "blocks.h"

#include <string.h>

int render_column_flat(bitmap_t *bitmap, block_t slice[16][16], int rendermode){
	memset(bitmap->pixels, 0, bitmap->width * bitmap->height * BITMAP_BYTES_PER_PIXEL);

	for(int z=0; z<16; z++){
		for(int x=0; x<16; x++){
			int color[3];
			color[0] = blockdescs[slice[x][z].type].color[0];
			color[1] = blockdescs[slice[x][z].type].color[1];
			color[2] = blockdescs[slice[x][z].type].color[2];
			if(blockdescs[slice[x][z].type].get_color){
				blockdescs[slice[x][z].type].get_color(slice[x][z].type, slice[x][z].data, color);
			}
			for(int color_i=0; color_i<3; color_i++){
				if(rendermode == RENDER_DEPTH && slice[x][z].overlay_type){
					double alpha = (double)(blockdescs[slice[x][z].overlay_type].color[3]) / 255;
					color[color_i]  = alpha * blockdescs[slice[x][z].overlay_type].color[color_i];
					color[color_i] += (1 - alpha) * blockdescs[slice[x][z].type].color[color_i];
				}
				double multiplier = 1;
				int depth = slice[x][z].depth;
				if(depth > 30){
					multiplier = 1.5;
				}else if(depth >= 0){
					multiplier += (double)depth/30/2;
				}else if(depth >= -30){
					multiplier += (double)depth/30/3;
				}else{
					multiplier = 0.5;
				}

				if(rendermode == RENDER_DEPTH)
					color[color_i] *= multiplier;
				else if(rendermode == RENDER_HEIGHT)
					color[color_i] = slice[x][z].depth+64;

				if(color[color_i] > 255)
					color[color_i] = 255;
			}

			bitmap->pixels[(z*16+x)*BITMAP_BYTES_PER_PIXEL+0] = color[0];
			bitmap->pixels[(z*16+x)*BITMAP_BYTES_PER_PIXEL+1] = color[1];
			bitmap->pixels[(z*16+x)*BITMAP_BYTES_PER_PIXEL+2] = color[2];
			bitmap->pixels[(z*16+x)*BITMAP_BYTES_PER_PIXEL+3] = color[3];
		}
	}

	return 0;
}
