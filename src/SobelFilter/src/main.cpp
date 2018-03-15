#include <iostream>
#include <string>
#include "Graphics/Graphics2D.h"
#include "tools/ZEDCamera.h"
#include "tools/Chrono.h"
#include "hwSobel.h"

using namespace std;
using namespace cv;

int parseArgs(int argc, char** argv);
void usage(int argc, char** argv);

// Settings, modified in parseArgs() function
struct Settings {
	bool cam = false;
	bool gl = false;
	bool sw = false;
	bool hw = false;
	bool hw2 = false;
	bool gpu = false;
	int queueSize = 15;
	ZED::Resolution res = ZED::RESOLUTION_VGA;
	ZED::Fps fps = ZED::FPS_15;
} settings;

int main(int argc, char** argv)
{
	// 0. Parse Arguments
	if (parseArgs(argc, argv) < 0) {
		usage(argc, argv);
		return 1;
	}

	Graphics2D* graph = nullptr;
	ZEDCamera zed(2, settings.res, settings.fps);

	// Open an OpenGL (Graphics2D) window or a GTK window
	if(settings.gl or settings.gpu) { graph = new Graphics2D(); }
	else { namedWindow("window", WINDOW_AUTOSIZE); }

	// Start camera and capture frames
	if(settings.cam) { zed.startCapture(); }
	else { zed.fillQueue(settings.queueSize); }

	// Set a string to identify the parameters utilized
	String strParam("");
	if(settings.gpu) { strParam += "gpu"; }
	else if(settings.sw) { strParam += "sw"; }
	else if(settings.hw) { strParam += "hw"; }
	if(settings.cam) { strParam += " + cam"; }
	if(settings.gl) { strParam += " + gl"; }

	// Constants and variables
	const int rows = zed.getHeight();
	const int cols = zed.getWidth();
	const int fps = zed.getFps();
	int queue_idx = 0;
	Chrono chrono, c_imshow, c_cap, c_filter;

	// CV Matrices
	Mat frameBGR, frame, frameGL;
	Mat grad_x(rows, cols, CV_8UC1);
	Mat grad_y(rows, cols, CV_8UC1);
	Mat abs_grad_x(rows, cols, CV_8UC1);
	Mat abs_grad_y(rows, cols, CV_8UC1);
	Mat grad(rows, cols, CV_8UC1);

	// XF Matrices
	xf::Mat<IM_TYPE,HEIGHT,WIDTH,NPC> imgInput(rows, cols);
	//xf::Mat<GRAD_TYPE,HEIGHT,WIDTH,NPC> gx(rows, cols);
	//xf::Mat<GRAD_TYPE,HEIGHT,WIDTH,NPC> gy(rows, cols);
	//xf::Mat<IM_TYPE,HEIGHT,WIDTH,NPC> mag(rows, cols);
	xf::Mat<IM_TYPE,HEIGHT,WIDTH,NPC> gx(rows, cols);
	xf::Mat<IM_TYPE,HEIGHT,WIDTH,NPC> gy(rows, cols);

	Chrono C_CVTGRAY, C_SOBELXY, C_ABS, C_ADD, C_CVT565, C_DRAW;
	while(true) {
		chrono.start();

		// Get new frame from queue
		// ------------------------
		c_cap.start();
		if(settings.cam) {
			while(!zed.spsc_queue.pop(frameBGR)) {
				this_thread::sleep_for(chrono::milliseconds(1));
			}
		}
		else {
			frameBGR = zed.queue[++queue_idx % settings.queueSize];
		}
		c_cap.stop();

		// Apply image filter
		// ------------------
		c_filter.start();
		if(settings.gpu) {
			// GPU Sobel filter
			cvtColor(frameBGR, frameGL, CV_BGR2BGR565);
			graph->drawSobel(frameGL);
		}
		else if(settings.sw) {
			// Software Sobel filter
			C_CVTGRAY.start();
			cvtColor(frameBGR, frame, CV_BGR2GRAY);
			C_CVTGRAY.stop();

			C_SOBELXY.start();
			Sobel(frame, grad_x, CV_16S, 1, 0);
			Sobel(frame, grad_y, CV_16S, 0, 1);
			C_SOBELXY.stop();

			C_ABS.start();
			convertScaleAbs( grad_x, abs_grad_x );
			convertScaleAbs( grad_y, abs_grad_y );
			C_ABS.stop();

			C_ADD.start();
			add(abs_grad_x, abs_grad_y, grad);
			C_ADD.stop();
		}
		else if(settings.hw) {
			// Hardware Sobel filter
			C_CVTGRAY.start();
			cvtColor(frameBGR, frame, CV_BGR2GRAY);
			C_CVTGRAY.stop();

			C_SOBELXY.start();
			imgInput.copyTo(frame.data);		// memcpy xf::Mat::data <- cv::Mat::data
			sobel_accel(imgInput, gx, gy);	// hw accelerated Sobel
			grad_x.data = (uchar*) gx.copyFrom();
			grad_y.data = (uchar*) gy.copyFrom();
			C_SOBELXY.stop();

			C_ABS.start();
			convertScaleAbs( grad_x, abs_grad_x );
			convertScaleAbs( grad_y, abs_grad_y );
			C_ABS.stop();

			C_ADD.start();
			add(abs_grad_x, abs_grad_y, grad);
			C_ADD.stop();
		}
		else {
			// No filter
			grad = frameBGR;
		}
		c_filter.stop();

		// Display frame on screen
		// -----------------------
		c_imshow.start();
		if(!settings.gpu) {
			if(settings.gl) {
				if(settings.sw or settings.hw) {
					C_CVT565.start();
					cvtColor(grad, frameGL, CV_GRAY2BGR565);
					C_CVT565.stop();
				}
				else {
					cvtColor(frameBGR, frameGL, CV_BGR2BGR565);
				}
				C_DRAW.start();
				graph->drawImage(frameGL);
				C_DRAW.stop();
			}
			else {
				imshow("window", grad);
				waitKey(1);			// wait at least 1 ms
			}
		}
		c_imshow.stop();


		// Display chrono values
		// ---------------------
		chrono.stop();
		if(chrono.getTotalTime() > 2.0) {
			float t =  chrono.getAvgTime();
			cout << "resolution@framerate, cap, filter, display, frametime, FPS" << endl
					<< strParam << endl << cols << "x" << rows << "p" << fps << endl
					<< c_cap.getAvgTime() * 1000 << endl
					<< c_filter.getAvgTime() * 1000 << endl
					<< c_imshow.getAvgTime() * 1000 << endl
					<< t * 1000 << endl
					<< 1/t << endl
					<< endl;
			chrono.reset();
			c_cap.reset();
			c_imshow.reset();
			c_filter.reset();

			std::cout << "C_CVTGRAY, C_SOBELXY, C_ABS, C_ADD, C_CVT565, C_DRAW;" << endl
					<< C_CVTGRAY.getAvgTime() * 1000 << std::endl
					<< C_SOBELXY.getAvgTime() * 1000 << std::endl
					<< C_ABS.getAvgTime() * 1000 << std::endl
					<< C_ADD.getAvgTime() * 1000 << std::endl
					<< C_CVT565.getAvgTime() * 1000 << std::endl
					<< C_DRAW.getAvgTime() * 1000 << std::endl
					<< std::endl;
			C_CVTGRAY.reset(); C_SOBELXY.reset(); C_ABS.reset(); C_ADD.reset(); C_CVT565.reset(); C_DRAW.reset();
		}
	}

	return 0;
}


int parseArgs(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		String s(argv[i]);
		if(s == "--help" or s == "-h") return -1;
		else if(s == "--cam"){ settings.cam = true; }
		else if(s == "--gl") { settings.gl = true; }
		else if(s == "--sw") { settings.sw = true; }
		else if(s == "--hw") { settings.hw = true; }
		else if(s == "--gpu"){ settings.gpu = true; }
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
	cout 	<< "Usage: "<< argv[0] << " \"<options>\"\n"
			<< "Options :\n"
			<< "  --help    Show this help\n"
			<< "  --cam     Run camera continuously\n"
			<< "  --gl      Display in OpenGL ES\n"
			<< "  --sw      Enable Software Sobel\n"
			<< "  --hw      Enable Hardware Sobel (PL)\n"
			<< "  --gpu     Enable GPU Sobel\n"
			<< endl
			<< "  --res <N> Select camera resolution N:value\n"
			<< "            0:VGA, 1:HD720, 2:HD1080, 3:HD2K\n"
			<< "  --fps <N> Select camera frames per second N:value\n"
			<< "            0:15, 1:30, 2:60, 3:120\n"
			;//<< "NOTE: Use quotation marks (\"--opt1 --opt2 ...\") for more than 4 options\n";
}
