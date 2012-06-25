/*
 * pnm2png.c
 *
 *  Created on: Jun 23, 2012
 *      Author: chris
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <png.h>
#include <pnm.h>

#define die(text) do{fprintf(stderr, "[ERROR] %-14s %3d: %s\n", __FILE__, __LINE__, text); \
		exit(1);}while(0)

#define MAX_PNM_FILESIZE (120*1024*1024)

size_t get_filesize(FILE* file){
	if(!file){
		return 0;
	}
	fseek(file, 0, 2);
	size_t size = ftell(file);
	fseek(file, 0, 0);
	return size;
}


int write_png(bitmap_t *bitmap, const char *filename, char *title){
	FILE *pngfile = fopen(filename, "wb");
	if(!pngfile)
		die("Could not open PNG file for writing");

	png_structp png_p = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if(!png_p)
		die("Could not create PNG struct");
	png_infop info_p = png_create_info_struct(png_p);
	if(!info_p)
		die("Could not create PNG info struct");

	png_bytep row = 0;

	if(setjmp(png_jmpbuf(png_p))){
		if(png_p)  free(png_p);
		if(info_p) free(info_p);
		if(row)    free(row);
		fclose(pngfile);
		die("Could not create PNG (libpng jumped on me)");
	}
	png_init_io(png_p, pngfile);
	png_set_IHDR(png_p, info_p, bitmap->width, bitmap->height, 8,
			PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	if(title){
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = "Title";
		title_text.text = title;
		png_set_text(png_p, info_p, &title_text, 1);
	}
	png_write_info(png_p, info_p);

	row = malloc(bitmap->width * 3 * sizeof(png_byte));

	for(uint32_t y=0;y<bitmap->height;y++){
		for(uint32_t x=0;x<bitmap->width;x++) {
			row[x*3+0] = bitmap->pixels[y*bitmap->width*3+x*3+0];
			row[x*3+1] = bitmap->pixels[y*bitmap->width*3+x*3+1];
			row[x*3+2] = bitmap->pixels[y*bitmap->width*3+x*3+2];
		}
		png_write_row(png_p, row);
	}

	png_write_end(png_p, NULL);

	if(png_p)  free(png_p);
	if(info_p) free(info_p);
	if(row)    free(row);
	fclose(pngfile);

	return 0;
}

int get_pnm(bitmap_t *bitmap, const char *filename){
	bitmap->pixels = 0;

	FILE *pnmfile;
	pnmfile = fopen(filename, "rb");
	if(!pnmfile){
		die("Could not open PNM file");
	}

	size_t filesize = get_filesize(pnmfile);

	if(filesize > MAX_PNM_FILESIZE){
		fclose(pnmfile);
		die("PNM file too large");
	}

	uint8_t *buffer = malloc(filesize);
	if(!buffer){
		die("Could not allocate PNM buffer");
	}
	size_t bytes_read = fread(buffer, 1, filesize, pnmfile);

	if(bytes_read != filesize){
		fclose(pnmfile);
		die("Could not read PNM file.");
	}

	fclose(pnmfile);

	if(pnm_parse(bitmap, filesize, buffer))
		return 1;

	return 0;
}

int main(int argc, char **argv){
	if(argc < 2 || !strlen(argv[1])){
		printf("Usage: pnm2png [worldname]\n");
		return 0;
	}

	const char *worldname = argv[1];

	if(strlen(worldname) > 64-strlen("mcowl-.png")){
		die("World name too long");
	}

	for(uint16_t i=0;i<strlen(worldname);i++){
		if(
				(worldname[i] < '0' || worldname[i] > '9') &&
				(worldname[i] < 'A' || worldname[i] > 'Z') &&
				(worldname[i] < 'a' || worldname[i] > 'z') &&
				(worldname[i] != '-' && worldname[i] != '_')){
			//printf("\n'%s'\n\n", worldname);
			die("Invalid characters in world name");
		}
	}

	char pngfilename[256];
	sprintf(pngfilename, "mcowl-%s.png", worldname);
	char pngtitle[256];
	sprintf(pngtitle, "Minecraft MCowl World render: %s", worldname);

	bitmap_t bitmap;

	get_pnm(&bitmap, "world.pnm");
	write_png(&bitmap, pngfilename, pngtitle);

	return 0;
}
