#include "Remap.h"
#include <iostream>
using namespace std;

Remap::Remap(Resolution res): res(res)
{
	switch (res) {
	case VGA:
		image_height = 376;
		image_width = 1344; break;
	case HD720:
		image_height = 720;
		image_width = 2560; break;
	default:
		image_height = 720;
		image_width = 2560; break;
	}
	/* Voir stereo_zedConfig.cpp comme example */
	/* Init cameraMatrix LEFT */
	if (res == HD720) {
		cameraMatL = cv::Mat(3, 3, CV_64FC1, dataCameraMatL);
		cameraMatR = cv::Mat(3, 3, CV_64FC1, dataCameraMatR);
	}
	else {
		cameraMatL = cv::Mat(3, 3, CV_64FC1, dataCameraMatL_VGA);
		cameraMatR = cv::Mat(3, 3, CV_64FC1, dataCameraMatR_VGA);
	}

	/* Init distCoeffs */
	distCoefL = cv::Mat(5, 1, CV_64FC1, dataDistCoefL);
	distCoefR = cv::Mat(5, 1, CV_64FC1, dataDistCoefR);

	newCameraMatL = cameraMatL;
	newCameraMatR = cameraMatR;

	sizeImage = cv::Size(image_width / 2, image_height);

	/* Rotation matix */
	R = cv::Mat::eye(3, 3, CV_64FC1);
	cv::Mat rotVec = cv::Mat(3, 1, CV_64FC1, dataRotVec);
	cv::Rodrigues(rotVec, R); // Converts rotation vector to rotation matrix
	/* Translation vector */
	double dataT[] = { -baseline, 0.0, 0.0 };
	T = cv::Mat(3, 1, CV_64FC1, dataT);

	rotMatL = cv::Mat::eye(3, 3, CV_64FC1);	// will be modified by stereoRectify
	rotMatR = cv::Mat::eye(3, 3, CV_64FC1);

	cv::stereoRectify(cameraMatL, distCoefL, cameraMatR, distCoefR,
			sizeImage, R, T, rotMatL, rotMatL, newCameraMatL, newCameraMatR, Q_mat,
			cv::CALIB_ZERO_DISPARITY, -1.0, newImageSize, &validRoiL, &validRoiR);

	/* Compute undistort and rectify transformation maps */
	cv::initUndistortRectifyMap(cameraMatL, distCoefL, rotMatL,
			newCameraMatL, sizeImage, CV_32FC1, map_xL, map_yL);

	cv::initUndistortRectifyMap(cameraMatR, distCoefR, rotMatR,
			newCameraMatR, sizeImage, CV_32FC1, map_xR, map_yR);

	// top left corner
	point1.x = std::max(validRoiL.x, validRoiR.x);
	point1.y = std::max(validRoiL.y, validRoiR.y);
	// botom right corner
	point2.x = std::min(validRoiL.width + validRoiL.x, validRoiR.width + validRoiR.x);
	point2.y = std::min(validRoiL.height + validRoiL.y, validRoiR.height + validRoiL.y);
	roi = cv::Rect(point1, point2);
}

Remap::~Remap()
{
}

void Remap::remap(cv::Mat& input1, cv::Mat& input2, cv::Mat& output1, cv::Mat& output2) {
	cv::remap(input1, output1, map_xL, map_yL, cv::INTER_NEAREST); //INTER_NEAREST, INTER_LINEAR, INTER_CUBIC
	cv::remap(input2, output2, map_xR, map_yR, cv::INTER_NEAREST);
}
