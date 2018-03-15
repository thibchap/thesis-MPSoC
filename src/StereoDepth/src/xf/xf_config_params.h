#pragma once

/* config width and height */

//#define ZEDCAMERA_720P
#define ZEDCAMERA_VGA

#ifdef ZEDCAMERA_720P
#define XF_HEIGHT	720
#define XF_WIDTH 	1280
#elif defined ZEDCAMERA_VGA
#define XF_HEIGHT	376
#define XF_WIDTH 	672
#endif

/* NO_OF_DISPARITIES must be greater than '0' and less than the image width */
#define NO_OF_DISPARITIES	64 //48

/* NO_OF_DISPARITIES must not be lesser than PARALLEL_UNITS and NO_OF_DISPARITIES/PARALLEL_UNITS must be a non-fractional number */
#define PARALLEL_UNITS		16 //16

/* SAD window size must be an odd number and it must be less than minimum of image height and width and less than the tested size '21' */
#define SAD_WINDOW_SIZE		15//15

// Configure this based on the number of rows needed for Remap function
#define XF_REMAP_BUFSIZE	64	// 64	// ideal would be 72 - 78
