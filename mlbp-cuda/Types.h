#ifndef TYPES_H_
#define TYPES_H_

typedef unsigned char byte;

typedef struct {
	int x, y;
} int_pair;

typedef struct {
	int_pair gaps_pixel;
	int_pair grid_size;
	int_pair end_pixels;
} image_region;



#endif
