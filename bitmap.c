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

#include "bitmap.h"

int bitmap_allocate(bitmap_t *bitmap, int width, int height){
	if(width <= 0 || height <= 0)
		return 1;
	bitmap->pixels = malloc(width * height * BITMAP_BYTES_PER_PIXEL);
	if(!bitmap->pixels)
		return 1;
	bitmap->width = width;
	bitmap->height = height;

	return 0;
}

bitmap_t *bitmap_new(int width, int height){
	if(width <= 0 || height <= 0)
		return NULL;
	bitmap_t *bitmap = (bitmap_t*)malloc(sizeof(bitmap_t));
	if(!bitmap)
		return NULL;
	if(!bitmap_allocate(bitmap, width, height)){
		return bitmap;
	}else{
		free(bitmap);
		return NULL;
	}
}

uint8_t *bitmap_at(bitmap_t *bitmap, int x, int y){
	if(x < 0 || x > bitmap->width || 
		y < 0 || y > bitmap->height)
		return NULL;
	return &bitmap->pixels[(y*bitmap->width + x) * BITMAP_BYTES_PER_PIXEL];
}

int bitmap_writepnm(const bitmap_t *bitmap, FILE *outfile){
	if(fprintf(outfile, "P6\n%d %d\n%d\n", bitmap->width, bitmap->height, 255) < 0)
		return 1;
	for(int i=0; i<bitmap->width*bitmap->height*BITMAP_BYTES_PER_PIXEL; i++){
		// Filter out alpha channel
		if(!((i+1) % 4))
			continue;
		if(fputc(bitmap->pixels[i], outfile) < 0)
			return 1;
	}
	return 0;
}

int bitmap_free(bitmap_t *bitmap){
	if(bitmap){
		if(bitmap->pixels)
			free(bitmap->pixels);
		free(bitmap);
	}
	return 0;
}
