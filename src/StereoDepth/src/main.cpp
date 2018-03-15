#include <iostream>
#include <string>
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

#include "Graphics/Graphics2D.h"
#include "Graphics/Graphics3D.h"
#include "tools/ZEDCamera.h"
#include "tools/Chrono.h"
#include "tools/CaptureVideo.h"
#include "improc/Remap.h"
#include "improc/stereoBM.h"
#include "xf/xf_stereo_pipeline_config.h"
#include "xf/cameraParameters.h"

// Options
#define USE_V4L2
#define USE_GPU_GRAY16

using namespace std;

// Program Settings
struct Settings {
	bool cam = false;
	bool gl = false;
	bool sw = false;
	bool hw = false;
	bool gl3d = false;
	int queueSize = 15;
	ZED::Resolution res = ZED::RESOLUTION_VGA;
	ZED::Fps fps = ZED::FPS_15;
};

int parseArgs(int argc, char** argv, Settings& settings);	// Command-line argument parsing
void usage(int argc, char** argv);							// Print command-line usage

int main(int argc, char** argv)
{
	cv::setUseOptimized(false);
	// 0. Parse Arguments
	Settings _s;
	if (parseArgs(argc, argv, _s) < 0) {
		usage(argc, argv);
		return 1;
	}
	const Settings settings = _s; // const copy

	// Set a string to identify the parameters utilized
	std::string strParam("");
	if(settings.sw) { strParam += "sw"; }
	else if(settings.hw) { strParam += "hw"; }
	if(settings.cam) { strParam += "+cam"; }
	if(settings.gl3d) { strParam += "+gl3D"; }
	else if(settings.gl) { strParam += "+gl2D"; }


	Graphics2D* graph = nullptr;
	Graphics3D* graph3D = nullptr;
#ifdef USE_V4L2
	CaptureVideo& zed = CaptureVideo::getInstance((_ZED::Resolution)settings.res, (_ZED::Fps)settings.fps);
#else
	ZEDCamera zed(2, settings.res, settings.fps);
#endif

	// Open an OpenGL (Graphics2D) window or a GTK window
	if(settings.gl) { graph = new Graphics2D(); }
	else if (settings.gl3d) { graph3D = new Graphics3D(); }
	else { cv::namedWindow("window", cv::WINDOW_AUTOSIZE); }

	// Start camera and capture frames
#ifdef USE_V4L2
	zed.startCapture();
#else
	if(settings.cam) { zed.startCapture(); }
	else { zed.fillQueue(settings.queueSize); }
	int queue_idx = 0;
#endif

	// Constants and variables
	int rows = zed.getHeight();
	int cols = zed.getWidth() / 2;
	int fps = zed.getFps();
	Chrono chrono, c_imshow, c_cap, c_filter;

	// roi
	cv::Rect roiL(0, 0, cols, rows);
	cv::Rect roiR(cols, 0, cols, rows);

	// CV Matrices
	cv::Mat frameYUYV(rows, cols * 2, CV_8UC2);
	cv::Mat frameBGR(rows, cols * 2, CV_8UC3);
	cv::Mat frameGray(rows, cols * 2, CV_8UC1);
	cv::Mat frameGL(rows, cols * 2, CV_8UC2);
	cv::Mat imLeft(rows, cols, CV_8UC3);
	cv::Mat imRight(rows, cols, CV_8UC3);
	cv::Mat imGrayL(rows, cols, CV_8UC1);
	cv::Mat imGrayR(rows, cols, CV_8UC1);
	cv::Mat imRemappedL(rows, cols, CV_8UC1);
	cv::Mat imRemappedR(rows, cols, CV_8UC1);

	// Stereo Block Matching
	cv::Mat disp16S(rows, cols, CV_16S);
	cv::Mat disp(rows, cols, CV_8UC1);
	cv::Ptr<cv::StereoBM> sbm;
	initStereoBM(sbm);
	const float COEF_DISP_S16_TO_U8 = 256.f /(float)sbm->getNumDisparities() / 16.f;

	Remap remapper((Remap::Resolution)settings.res);

	Chrono C_ROI, C_REMAP, C_CVTGRAY, C_SBM, C_CVTUC1, C_CVT565, C_DRAW, C_COPYTO, C_COPYFROM;

	// XF Matrices
	rows = XF_HEIGHT;
	cols = XF_WIDTH;
	xf::Mat<XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> leftMat(rows,cols);
	xf::Mat<XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> rightMat(rows,cols);

	xf::Mat<XF_32FC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mapxLMat(rows,cols);
	xf::Mat<XF_32FC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mapyLMat(rows,cols);
	xf::Mat<XF_32FC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mapxRMat(rows,cols);
	xf::Mat<XF_32FC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mapyRMat(rows,cols);

	xf::Mat<XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> leftRemappedMat(rows,cols);
	xf::Mat<XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> rightRemappedMat(rows,cols);

	xf::Mat<XF_16UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> dispMat(rows,cols);

	// Set stereo BM state (config)
	xf::xFSBMState<SAD_WINDOW_SIZE,NO_OF_DISPARITIES,PARALLEL_UNITS> bm_state;
	bm_state.preFilterCap = 31;//61;
	bm_state.uniquenessRatio = 15; //10;
	bm_state.textureThreshold = 20; //200;
	bm_state.minDisparity = 0;

	// camera parameters for rectification
#if __SDSCC__
	ap_fixed<32,12> *cameraMatL = (ap_fixed<32,12>*)sds_alloc_non_cacheable(XF_CAMERA_MATRIX_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *cameraMatR = (ap_fixed<32,12>*)sds_alloc_non_cacheable(XF_CAMERA_MATRIX_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *rotMatL = (ap_fixed<32,12>*)sds_alloc_non_cacheable(XF_CAMERA_MATRIX_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *rotMatR = (ap_fixed<32,12>*)sds_alloc_non_cacheable(XF_CAMERA_MATRIX_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *distCoefL = (ap_fixed<32,12>*)sds_alloc_non_cacheable(XF_DIST_COEFF_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *distCoefR = (ap_fixed<32,12>*)sds_alloc_non_cacheable(XF_DIST_COEFF_SIZE*sizeof(ap_fixed<32,12>));
#else
	ap_fixed<32,12> *cameraMatL = (ap_fixed<32,12>*)malloc(XF_CAMERA_MATRIX_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *cameraMatR = (ap_fixed<32,12>*)malloc(XF_CAMERA_MATRIX_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *rotMatL = (ap_fixed<32,12>*)malloc(XF_CAMERA_MATRIX_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *rotMatR = (ap_fixed<32,12>*)malloc(XF_CAMERA_MATRIX_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *distCoefL = (ap_fixed<32,12>*)malloc(XF_DIST_COEFF_SIZE*sizeof(ap_fixed<32,12>));
	ap_fixed<32,12> *distCoefR = (ap_fixed<32,12>*)malloc(XF_DIST_COEFF_SIZE*sizeof(ap_fixed<32,12>));
#endif
	// copy camera params
	for(int i=0; i< XF_CAMERA_MATRIX_SIZE; i++) {
		cameraMatL[i] = (ap_fixed<32,12>)cameraMA_l[i];
		cameraMatR[i] = (ap_fixed<32,12>)cameraMA_r[i];
		rotMatL[i] = (ap_fixed<32,12>)irA_l[i];
		rotMatR[i] = (ap_fixed<32,12>)irA_r[i];
	}
	// copy distortion coefficients
	for(int i=0; i < XF_DIST_COEFF_SIZE; i++) {
		distCoefL[i] = (ap_fixed<32,12>)distC_l[i];
		distCoefR[i] = (ap_fixed<32,12>)distC_r[i];
	}


	/*------------------------------------------------------*\
	 * 					Main Loop							*
	\*------------------------------------------------------*/
	while(true) {
		chrono.start();

		// Get new frame from queue
		// ------------------------
		c_cap.start();
#ifdef USE_V4L2
		void* ptr_data = nullptr;
		size_t sizeData = 0;
		while(!zed.getFrame(&ptr_data, &sizeData)) {
			this_thread::sleep_for(chrono::milliseconds(1));
		}
		if(sizeData == 0)
			continue;
		frameYUYV.data = (uchar*)ptr_data;
#else
		if(settings.cam) {
			while(!zed.spsc_queue.pop(frameBGR)) {
				this_thread::sleep_for(chrono::milliseconds(1));
			}
		}
		else {
			frameBGR = zed.queue[(++queue_idx % settings.queueSize)];
		}
#endif
		c_cap.stop();

		// Apply image filter
		// ------------------
		c_filter.start();
		if(settings.sw) {
			if (settings.gl3d){
				C_ROI.start();
				cv::cvtColor(frameYUYV, frameBGR, cv::COLOR_YUV2BGR_YUY2);
				cout << "frameBGR " << frameBGR.cols << " " << frameBGR.rows << endl;
				imLeft = frameBGR(roiL);
				imRight = frameBGR(roiR);
				C_ROI.stop();

				// Remapping
				C_REMAP.start();
				remapper.remap(imLeft, imRight, imRemappedL, imRemappedR);
				C_REMAP.stop();

				C_CVTGRAY.start();
				cv::cvtColor(imRemappedL, imGrayL, cv::COLOR_BGR2GRAY);
				cv::cvtColor(imRemappedR, imGrayR, cv::COLOR_BGR2GRAY);
				C_CVTGRAY.stop();

				//Stereo Block Matching
				C_SBM.start();
				sbm->compute(imGrayL, imGrayR, disp16S);
				C_SBM.stop();
			}
			else {
				C_CVTGRAY.start();
				cv::cvtColor(frameYUYV, frameGray, cv::COLOR_YUV2GRAY_YUY2);
				C_CVTGRAY.stop();

				C_ROI.start();
				imGrayL = frameGray(roiL);
				imGrayR = frameGray(roiR);
				C_ROI.stop();

				// Remapping
				C_REMAP.start();
				remapper.remap(imGrayL, imGrayR, imRemappedL, imRemappedR);
				C_REMAP.stop();

				//Stereo Block Matching
				C_SBM.start();
				sbm->compute(imRemappedL, imRemappedR, disp16S);
				C_SBM.stop();
			}
#ifndef USE_GPU_GRAY16
			C_CVTUC1.start();
			disp16S.convertTo(disp, CV_8UC1, (256. / sbm->getNumDisparities() / 16.));
			C_CVTUC1.stop();
#endif
		}
		else if(settings.hw) {
			// Hardware filter
			if (settings.gl3d){
				C_CVTGRAY.start();
				cv::cvtColor(frameYUYV, frameGray, cv::COLOR_YUV2GRAY_YUY2);
				C_CVTGRAY.stop();

				C_ROI.start();
				imGrayL = frameGray(roiL).clone();
				imGrayR = frameGray(roiR).clone();
				imLeft = frameYUYV(roiL);
				C_ROI.stop();

				C_CVT565.start();
				cv::cvtColor(imLeft, imLeft, cv::COLOR_YUV2BGR_YUY2); // TODO bypass and remap+display texture as YUYV
				cv::cvtColor(imLeft, imLeft, cv::COLOR_BGR2BGR565);
				C_CVT565.stop();

				C_REMAP.start();
				cv::remap(imLeft, frameGL, remapper.map_xL, remapper.map_yL, cv::INTER_NEAREST);
				C_REMAP.stop();

				C_COPYTO.start();
				leftMat.copyTo(imGrayL.data);
				rightMat.copyTo(imGrayR.data);
				C_COPYTO.stop();

				C_SBM.start();
				stereopipeline_accel(leftMat, rightMat, dispMat,
						mapxLMat, mapyLMat,	mapxRMat, mapyRMat, leftRemappedMat, rightRemappedMat,
						bm_state, cameraMatL, cameraMatR, distCoefL,
						distCoefR, rotMatL, rotMatR, XF_CAMERA_MATRIX_SIZE, XF_DIST_COEFF_SIZE);
				C_SBM.stop();

				C_COPYFROM.start();
				disp16S.data = (uchar*) dispMat.copyFrom();
				C_COPYFROM.stop();
			}
			else {
				C_CVTGRAY.start();
				cv::cvtColor(frameYUYV, frameGray, cv::COLOR_YUV2GRAY_YUY2); // TODO : move this to PL
				C_CVTGRAY.stop();

				C_ROI.start();
				imGrayL = frameGray(roiL).clone();	// TODO : move this to PL
				imGrayR = frameGray(roiR).clone();
				C_ROI.stop();

				C_COPYTO.start();
				leftMat.copyTo(imGrayL.data);
				rightMat.copyTo(imGrayR.data);
				C_COPYTO.stop();

				C_SBM.start();
				stereopipeline_accel(leftMat, rightMat, dispMat,
						mapxLMat, mapyLMat,	mapxRMat, mapyRMat, leftRemappedMat, rightRemappedMat,
						bm_state, cameraMatL, cameraMatR, distCoefL, distCoefR,
						rotMatL, rotMatR, XF_CAMERA_MATRIX_SIZE, XF_DIST_COEFF_SIZE);
				C_SBM.stop();

				C_COPYFROM.start();
				disp16S.data = (uchar*) dispMat.copyFrom();
				C_COPYFROM.stop();
			}
#ifndef USE_GPU_GRAY16
			C_CVTUC1.start();
			//-- Convert it as a CV_8UC1 image
			disp16S.convertTo(disp, CV_8UC1, (256.0/NO_OF_DISPARITIES)/(16.));
			C_CVTUC1.stop();
#endif

		}
		else {
			// No filter
			if(!settings.gl)
				cv::cvtColor(frameYUYV, frameBGR, cv::COLOR_YUV2BGR_YUY2);
			disp = frameBGR;
			disp16S = frameBGR;
		}
		c_filter.stop();

		// Display frame on screen
		// -----------------------
		c_imshow.start();
		if(settings.gl) {
			if(settings.sw or settings.hw) {
#ifdef USE_GPU_GRAY16
				C_DRAW.start();
				graph->drawImageGray16(disp16S, COEF_DISP_S16_TO_U8);
				C_DRAW.stop();
#else
				//			C_CVT565.start();
				//			cvtColor(disp, frameGL, cv::COLOR_GRAY2BGR565);
				//			//cvtColor(imRemappedL, frameGL, CV_BGR2BGR565); // debug
				//			C_CVT565.stop();
				C_DRAW.start();
				//graph->drawImageBGR565(frameGL);
				graph->drawImageGray8(disp);
				C_DRAW.stop();
#endif
			}
			else {
				graph->drawImageYUYV(frameYUYV);
			}
		}
		else if (settings.gl3d){
			C_DRAW.start();
			graph3D->drawPoints(disp16S, frameGL, COEF_DISP_S16_TO_U8);
			C_DRAW.stop();
		}
		else {
			cv::imshow("window", disp);
			cv::waitKey(1);			// wait at least 1 ms
		}
		c_imshow.stop();

		chrono.stop();
		if(chrono.getTotalTime() > 2.0) {
			float t =  chrono.getAvgTime();
			cout << "resolution@framerate\t cap\t filter\t display\t frametime\t FPS" << endl
					<< strParam << endl << cols*2 << "x" << rows << "p" << fps << endl
					<< c_cap.getAvgTime() * 1000 << endl
					<< c_filter.getAvgTime() * 1000 << endl
					<< c_imshow.getAvgTime() * 1000 << endl
					<< t * 1000 << endl	// frametime
					<< 1/t << endl		// FPS
					<< endl;
			chrono.reset();
			c_cap.reset();
			c_imshow.reset();
			c_filter.reset();

			std::cout << "C_ROI, C_REMAP, C_CVTGRAY, C_SBM, C_CVTUC1, C_CVT565, C_DRAW, C_COPYTO, C_COPYFROM" << endl
					<< C_ROI.getAvgTime() * 1000 << std::endl
					<< C_REMAP.getAvgTime() * 1000 << std::endl
					<< C_CVTGRAY.getAvgTime() * 1000 << std::endl
					<< C_SBM.getAvgTime() * 1000 << std::endl
					<< C_CVTUC1.getAvgTime() * 1000 << std::endl
					<< C_CVT565.getAvgTime() * 1000 << std::endl
					<< C_DRAW.getAvgTime() * 1000 << std::endl
					<< C_COPYTO.getAvgTime() * 1000 << std::endl
					<< C_COPYFROM.getAvgTime() * 1000 << std::endl
					<< std::endl;
			C_ROI.reset(); C_REMAP.reset(); C_CVTGRAY.reset(); C_SBM.reset(); C_CVTUC1.reset(); C_CVT565.reset(); C_DRAW.reset();
		}
	}

	return 0;
}


int parseArgs(int argc, char** argv, Settings& settings) {
	for (int i = 1; i < argc; i++) {
		std::string s(argv[i]);
		if(s == "--help" or s == "-h") return -1;
		else if(s == "--cam"){ settings.cam = true; }
		else if(s == "--gl") { settings.gl = true; }
		else if(s == "--3d") { settings.gl3d = true; }
		else if(s == "--sw") { settings.sw = true; }
		else if(s == "--hw") { settings.hw = true; }
		else if(s == "--res") {
			if(++i < argc) { settings.res = (ZED::Resolution) (uint) atoi(argv[i]); }
			else return -1;
		}
		else if(s == "--fps") {
			if(++i < argc) { settings.fps = (ZED::Fps) (uint) atoi(argv[i]); }
			else return -1;
		}
		else return -1;
	}
	return 0;
}

void usage(int argc, char** argv) {
	std::cout 	<< "Usage: "<< argv[0] << " \"<options>\"\n"
			<< "Options :\n"
			<< "  --help    Show this help\n"
			<< "  --cam     Run camera continuously\n"
			<< "  --gl      Display in OpenGL ES\n"
			<< "  --3d      Display cloud points (OpenGL ES, 3D)\n"
			<< "  --sw      Enable Software Stereo Depth\n"
			<< "  --hw      Enable Hardware Stereo Depth (PL)\n"
			<< std::endl
			<< "  --res <N> Select camera resolution N:value\n"
			<< "            0:VGA, 1:HD720, 2:HD1080, 3:HD2K\n"
			<< "  --fps <N> Select camera frames per second N:value\n"
			<< "            0:15, 1:30, 2:60, 3:120\n"
			<< "NOTE: Use quotation marks (\"--opt1 --opt2 ...\") for more than 4 options\n";
}
