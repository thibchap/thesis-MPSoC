/*
 * CaptureVideo.h
 *
 *  Created on: March 2, 2018
 *      Author: Thibault
 */
#pragma once
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
#include <chrono>
#include <atomic>
#include <thread>
#include <string>
#include <iostream>
#include <cassert>
#include <cstring>
#include <boost/lockfree/spsc_queue.hpp>

namespace _ZED
{
enum Resolution { RESOLUTION_VGA, RESOLUTION_HD720, RESOLUTION_HD1080, RESOLUTION_HD2K };
enum Fps { FPS_15, FPS_30, FPS_60, FPS_120 };
}

class CaptureVideo {
public:

	struct buffer_t {		// C-like structure for V4L2 inter-operation
		void   *start;				// ptr to the buffer data
		size_t  length;				// data size in bytes
	};

	//------------------//
	// Constructors		//
	//------------------//
private:
	CaptureVideo(std::string dev_name, uint w, uint h, uint fps);
	virtual ~CaptureVideo();

	//------------------//
	// Public Methods	//
	//------------------//
public:
	static CaptureVideo& getInstance(std::string dev_name = "/dev/video2", uint w = 2560, uint h = 720, uint fps = 15);	// Singleton pattern
	static CaptureVideo& getInstance(_ZED::Resolution res, _ZED::Fps fps);
	void startCapture();
	void stopCapture();

	/*
	 * 'ptrData' will point to the new frame data of size 'size'.
	 * return true if a frame was available and ptrData and size are set correctly
	 * Warning : previous frame data will be overwritten
	 *
	 */
	bool getFrame(void** ptrData, size_t* size);

	// getters
	uint getWidth () const { return w; }
	uint getHeight () const { return h; }
	uint getFps () const { return fps; }

private:
	//------------------//
	// Private methods	//
	//------------------//
	void open_device();
	void init_device();
	void uninit_device();
	void close_device();
	void create_buffers();

	void mainloop();
	void printFramerate();

	// tools
	int xioctl(int fh, int request, void *arg);
	void errno_exit(const char *s);
private:
	// constants
#define CAPTURE_VIDEO_N_BUF 8
	const uint NBUF = CAPTURE_VIDEO_N_BUF;	// number of buffer allocated in user-space

	// inputs
	std::string dev_name;
	uint w, h, fps;					// camera params
	int fd;							// camera file descriptor

	// state
	std::atomic_bool capturing;

	// buffer management
	buffer_t* buffers;
	size_t buffer_size;				// in bytes
	int qbuf_num;	// number of buffer passed to the driver (queued)

	// inter-thread queues
	boost::lockfree::spsc_queue< buffer_t, boost::lockfree::capacity<CAPTURE_VIDEO_N_BUF> > fifo_in;
	boost::lockfree::spsc_queue< buffer_t, boost::lockfree::capacity<CAPTURE_VIDEO_N_BUF> > fifo_out;

	std::thread threadCapture;

	// V4L2 settings
	v4l2_capability cap;
	v4l2_format fmt;
};
