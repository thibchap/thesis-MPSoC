#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ap_int.h"
#include "hls_stream.h"
#include "common/xf_common.h"
#include "common/xf_utility.h"
#include "common/xf_params.h"
#include "imgproc/xf_sobel.hpp"
#include "core/xf_magnitude.hpp"


#if __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#define __ARM_NEON__
#define __ARM_NEON
#else
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#endif


#define IM_TYPE 	XF_8UC1
#define GRAD_TYPE 	XF_16UC1
#define NPC 		XF_NPPC8
#define WIDTH 		4416
#define HEIGHT 		1242
#define FILTER_WIDTH	3

#define MAG_TYPE 	XF_L1NORM	// magnitude = abs(gx) + abs(gy)
//#define MAG_TYPE 	XF_L2NORM	// magnitude = sqrt(gx^2 + gy^2)

/*void sobel_accel(xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC1> &_src,
		xf::Mat<GRAD_TYPE, HEIGHT, WIDTH, NPC1> &_gx,
		xf::Mat<GRAD_TYPE, HEIGHT, WIDTH, NPC1> &_gy,
		xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC1> &_dst);
*/
void sobel_accel(xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &_src,
		xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &_gx,
		xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &_gy);
