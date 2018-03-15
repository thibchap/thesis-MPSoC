#pragma once
//#include "opencv2/opencv.hpp"
#if __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#define __ARM_NEON__
#define __ARM_NEON
#else
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#endif

#include "xf/xf_config_params.h"

bool initStereoBM(cv::Ptr<cv::StereoBM>& sbm) {
	int numDisparities = NO_OF_DISPARITIES; //112;		//< Mult of 16
	int blockSize = SAD_WINDOW_SIZE; //15;			// odd > 4
	int preFilterCap = 31; //63;
	//int preFilterSize = 5;
	int textureThreshold = 20;//20;
	int uniquenessRatio = 15;//13;
	//int speckleRange = 8;
	//int disp12MaxDiff = -1;

	sbm = cv::StereoBM::create(numDisparities, blockSize);
	sbm->setPreFilterCap(preFilterCap);
	//sbm->setPreFilterSize(preFilterSize);
	sbm->setMinDisparity(0);
	sbm->setTextureThreshold(textureThreshold);
	sbm->setUniquenessRatio(uniquenessRatio);
	//sbm->setSpeckleRange(speckleRange);			// by default 0
	//sbm->setDisp12MaxDiff(disp12MaxDiff);		// by default -1
}
