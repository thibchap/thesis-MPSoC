/*
 * ZEDCamera.cpp
 *
 *  Created on: Dec 18, 2017
 *      Author: Thibault
 */
#include <chrono>
#include "ZEDCamera.h"
#include "Chrono.h"

using namespace std;
//using namespace cv;
using namespace ZED;

//--------------------------------------//
//				Constructors			//
//--------------------------------------//
ZEDCamera::ZEDCamera() : opened(false) {
}

ZEDCamera::ZEDCamera(int cam_id, Resolution _res, Fps _fps) : cap(cam_id), opened(true), stop(true) {
	switch(_res) {
	case RESOLUTION_HD2K:
		width = 2 * 2208; height = 1242; break;
	case RESOLUTION_HD1080:
		width = 2 * 1920; height = 1080; break;
	case RESOLUTION_HD720:
		width = 2 * 1280; height = 720; break;
	case RESOLUTION_VGA:
		width = 2 * 672; height = 376; break;
	default:
		width = 2 * 672; height = 376; break;
	}
	switch(_fps) {
	case FPS_120:
		fps = 120; break;
	case FPS_60:
		fps = 60; break;
	case FPS_30:
		fps = 30; break;
	case FPS_15:
		fps = 15; break;
	default:
		fps = 15; break;
	}
	if (!cap.isOpened()) { // check if we succeeded
		cout << "[ZEDCamera] Error opening VideoCapture device/file " << cam_id << endl;
		opened = false;
	}

	cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	cap.set(CV_CAP_PROP_FPS, fps);
	//cap.set(CAP_PROP_CONVERT_RGB, true);	// Not supported by V4L2/ZEDCamera
	//cap.set(CV_CAP_PROP_BUFFERSIZE, 3);	// Not supported by V4L2

	frame = cv::Mat(height, width, CV_8UC3);

	displayProperties();
}


ZEDCamera::~ZEDCamera() {
	if(!stop) {
		stopCapture();
	}
}


//--------------------------------------//
//				Methods					//
//--------------------------------------//
void ZEDCamera::startCapture(){
	if (!opened) {
		cerr << "[ZEDCamera] Error camera not opened" << endl;
		return;
	}
	stop = false;
	threadCapture = thread(&ZEDCamera::captureLoop, this);
	this_thread::sleep_for(chrono::milliseconds(10)); // wait for the other thread to start
}


void ZEDCamera::stopCapture(){
	stop = true;
	threadCapture.join();
}

void ZEDCamera::captureLoop() {
	cv::Mat _frame;
	while(!stop) {
		cap >> _frame;
		spsc_queue.push(_frame);
	}
}

void ZEDCamera::fillQueue(const int n_frame) {
	if (!opened) {
		cerr << "[ZEDCamera] Error camera not opened" << endl;
		return;
	}
	if (!stop) {
		cerr << "[ZEDCamera] Error camera already started" << endl;
		return;
	}

	cv::Mat _frame;
	for(int i = 0; i < 5; i++) {	// grab some frames
			cap >> _frame;
	}

	for(int i = 0; i < n_frame; i++) {	// fill queue
		cap >> _frame;
		queue.push_back(_frame);
	}
	opened = false;
	cap.release();
}

void ZEDCamera::displayProperties() const {
	cout << endl << "CAMERA PROPERTIES" << endl << "--------------------" << endl;
	cout << "Frame width : " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
	cout << "Frame height : " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
	cout << "Fps : " << cap.get(CV_CAP_PROP_FPS) << endl;
	//cout << "Brightness : " << cap.get(CV_CAP_PROP_BRIGHTNESS) << endl;
	//cout << "Contrast : " << cap.get(CV_CAP_PROP_CONTRAST) << endl;
	//cout << "Saturation : " << cap.get(CV_CAP_PROP_SATURATION) << endl;
	//cout << "Hue : " << cap.get(CV_CAP_PROP_HUE) << endl;
	//cout << "Gain : " << cap.get(CV_CAP_PROP_GAIN) << endl;
	//cout << "Buffersize " << cap.get(CV_CAP_PROP_BUFFERSIZE) << endl; // NOT Supported with V4L2
	cout << "frame " << frame.cols << "x" << frame.rows << endl;
	//cout << "frameRGB " << frameRGB.cols << "x" << frameRGB.rows << endl;
	//cout << "imgLeft " << imgLeft.cols << "x" << imgLeft.rows << endl;
	//cout << "imgRight " << imgRight.cols << "x" << imgRight.rows << endl;
	cout << endl;
}


//--------------------------------------//
//				Other					//
//--------------------------------------//
void cameraTest() {
	ZEDCamera zed(2, ZED::RESOLUTION_VGA, ZED::FPS_15);
	Chrono c, wc;
	cv::namedWindow("win", cv::WINDOW_AUTOSIZE);

	zed.startCapture();
	cv::Mat frame;
	while(1){
		c.start();

		while(zed.spsc_queue.pop(frame)) {
			this_thread::sleep_for(chrono::milliseconds(1));
		}
		cv::imshow("win", frame);

		wc.start();
		cv::waitKey(1); 	// wait AT LEAST 1 ms
		wc.stop();
		c.stop();

		if(c.getTotalTime() > 2.0) {
			cout << "Frame time = " << c.getAvgTime() * 1000.0 << " ms, FPS = " << 1.0 / c.getAvgTime() << endl;
			cout << "waitKey time = " << wc.getAvgTime() * 1000.0 << " ms" << endl;
			c.reset();
			wc.reset();
		}
	}
}
