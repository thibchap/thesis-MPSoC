/*
 * ZEDCamera.h
 *
 *  Created on: Dec 18, 2017
 *      Author: Thibault
 */
#pragma once

//#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <boost/lockfree/spsc_queue.hpp>
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

namespace ZED
{
enum Resolution { RESOLUTION_VGA, RESOLUTION_HD720, RESOLUTION_HD1080, RESOLUTION_HD2K };
enum Fps { FPS_15, FPS_30, FPS_60, FPS_120 };
}

class ZEDCamera {
	//----------------------------------------------//
	//					Public						//
	//----------------------------------------------//
public:
	//--------------------------------------//
	//				Constructors			//
	//--------------------------------------//
	ZEDCamera();
	ZEDCamera(int cam_id, ZED::Resolution res, ZED::Fps fps);
	virtual ~ZEDCamera();

	//--------------------------------------//
	//				Methods					//
	//--------------------------------------//
	void startCapture();		// run capture loop
	void stopCapture();		// run capture loop
	void fillQueue(const int n_frame);

	int getFps() const { return fps; }
	int getWidth() const {	return width; }
	int getHeight() const { return height; }


	//--------------------------------------//
	//				Attributes				//
	//--------------------------------------//
	std::vector<cv::Mat> queue;
	// Single Producer - Single Consumer Wait Free Queue
	boost::lockfree::spsc_queue<cv::Mat, boost::lockfree::capacity<8>> spsc_queue; // only for reading (pop)

	//----------------------------------------------//
	//					Private						//
	//----------------------------------------------//
private:
	void captureLoop();
	void displayProperties() const;

	//--------------------------------------//
	//				Attributes				//
	//--------------------------------------//
	cv::VideoCapture cap;

	cv::Mat frame;
	cv::Mat frameGray;
	cv::Mat frameRGB;
	cv::Mat imgLeft;
	cv::Mat imgRight;
	bool opened;

	int width;
	int height;
	int fps;

	std::atomic<bool> stop;
	std::thread threadCapture;
};

void cameraTest();
