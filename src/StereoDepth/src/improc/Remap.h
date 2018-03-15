#pragma once
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

class Remap
{
public:
	enum Resolution {VGA, HD720};

	Remap(Resolution res);
	~Remap();

	void remap(cv::Mat& input1, cv::Mat& input2, cv::Mat& output1, cv::Mat& output2);
	cv::Rect roi;
	int getWidth() const { return image_width; }
	int getHeight() const { return image_height; }
private:

	cv::Mat imgSrc;

	/* Inputs */
	cv::Mat cameraMatL, cameraMatR;	//< input camera matrix
	cv::Size sizeImage;
	cv::Mat R;	// rotation matrix
	cv::Mat T;	// translation vector

	cv::Point point1;
	cv::Point point2;


public:
	/* Inputs */
	cv::Mat distCoefL, distCoefR;		//< input distorsion coefficient vector
										/* Outputs */
	cv::Mat newCameraMatL, newCameraMatR;
	cv::Mat rotMatL, rotMatR;	// rotation matrices
	cv::Mat map_xL, map_yL;
	cv::Mat map_xR, map_yR;
private:
	/* Outputs */
	cv::Mat projMatL, projMatR;	// projection matrices
	cv::Mat Q_mat;		// disparity-to-depth mapping matrix (see reprojectImageTo3D ).
	cv::Size newImageSize;	// ImageSize after calibration
	cv::Rect validRoiL;		// valid pixels ROI for Left calibrated image
	cv::Rect validRoiR;		// valid pixels ROI for Right calibrated image

	// Image infos
	Resolution res;
	int image_width;
	int image_height;


	//--------------------------------
	//  HD 720p
	//--------------------------------
	// Camera matrices data m = | fx 0  cx |
	//							| 0  fy cy |
	//							| 0  0  1  |
	double dataCameraMatL[9] = { 700.056, 0.00000, 589.765,
								0.00000, 700.056, 369.459,
								0.00000, 0.00000, 1.00000 };

	double dataCameraMatR[9] = { 699.963, 0.00000, 633.551,
								0.00000, 699.963, 351.691,
								0.00000, 0.00000, 1.00000 };

	//--------------------------------
	//  VGA
	//--------------------------------
	double dataCameraMatL_VGA[9] = { 350.028, 0.00000, 310.383,
									0.00000, 350.028, 192.229,
									0.00000, 0.00000, 1.00000 };

	double dataCameraMatR_VGA[9] = { 349.981, 0.00000, 332.275,
									0.00000, 349.981, 183.346,
									0.00000, 0.00000, 1.00000 };


	// Distortion coefficient data, m = [ r1 r2 t1 t2 ]
	double dataDistCoefL[5] = { -0.173562, 0.0258997, 0.0, 0.0, 0.0 };
	double dataDistCoefR[5] = { -0.170861, 0.0211828, 0.0, 0.0, 0.0 };

	const double baseline = 0.12;	// 120 mm
	double dataRotVec[3] = { 0.00385411, 0.0, 0.000622115 };	// {rx, ry, rz}
};
