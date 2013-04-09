#ifndef RENDER_H_
#define RENDER_H_

#include "mcowl.h"
#include "bitmap.h"

int render_column_flat(bitmap_t *bitmap, block_t slice[16][16], int rendermode);

#endif /* RENDER_H_ */
