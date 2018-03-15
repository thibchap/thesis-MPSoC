/*
 * CaptureVideo.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: Thibault
 */

#include "CaptureVideo.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

using namespace std;
using namespace std::chrono;

void CaptureVideo::errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

int CaptureVideo::xioctl(int fh, int request, void *arg)
{
	int r;
	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);
	return r;
}

CaptureVideo::CaptureVideo(string dev_name, uint w, uint h, uint fps) :
										dev_name(dev_name), w(w), h(h), fps(fps)
{
	open_device();
	init_device();
}

CaptureVideo::~CaptureVideo() {
	if(capturing)
		stopCapture();

	// TODO free buffers
	close_device();
}

CaptureVideo& CaptureVideo::getInstance(string dev_name, uint w, uint h, uint fps) {
	static CaptureVideo instance(dev_name, w, h, fps);
	return instance;
}

CaptureVideo& CaptureVideo::getInstance(_ZED::Resolution _res, _ZED::Fps _fps){
	uint width, height, fps;
	switch(_res) {
	case _ZED::RESOLUTION_HD2K:
		width = 2 * 2208; height = 1242; break;
	case _ZED::RESOLUTION_HD1080:
		width = 2 * 1920; height = 1080; break;
	case _ZED::RESOLUTION_HD720:
		width = 2 * 1280; height = 720; break;
	case _ZED::RESOLUTION_VGA:
		width = 2 * 672; height = 376; break;
	default:
		width = 2 * 672; height = 376; break;
	}
	switch(_fps) {
	case _ZED::FPS_120:
		fps = 120; break;
	case _ZED::FPS_60:
		fps = 60; break;
	case _ZED::FPS_30:
		fps = 30; break;
	case _ZED::FPS_15:
		fps = 15; break;
	default:
		fps = 15; break;
	}
	return getInstance("/dev/video2", width, height, fps);
}

void CaptureVideo::open_device()
{
	struct stat st;

	if (-1 == stat(dev_name.c_str(), &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n",
				dev_name.c_str(), errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", dev_name.c_str());
		exit(EXIT_FAILURE);
	}

	fd = open(dev_name.c_str(), O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
				dev_name.c_str(), errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void CaptureVideo::init_device(void)
{
	unsigned int min;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\\n",
					dev_name.c_str());
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\\n",
				dev_name.c_str());
		exit(EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "%s does not support streaming i/o\\n",
				dev_name.c_str());
		exit(EXIT_FAILURE);
	}

	/* Select video input, video standard and tune here. */
	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = w;
	fmt.fmt.pix.height      = h;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_NONE;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
		errno_exit("VIDIOC_S_FMT");

	/* Note VIDIOC_S_FMT may change width and height. */
	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;
	buffer_size = fmt.fmt.pix.sizeimage;

	// Set fps
	v4l2_streamparm streamparm = v4l2_streamparm();
	streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	streamparm.parm.capture.timeperframe.numerator = 1;
	streamparm.parm.capture.timeperframe.denominator = fps;
	if(-1 == xioctl (fd, VIDIOC_S_PARM, &streamparm))
		errno_exit("VIDIOC_S_PARM");
	fps = streamparm.parm.capture.timeperframe.denominator;

	create_buffers();
}

void CaptureVideo::create_buffers()
{
	/*
	struct v4l2_requestbuffers req;
	CLEAR(req);
	req.count  = NBUF_DRIVER;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno)
			errno_exit("Device does not support user pointer");
		else
			errno_exit("VIDIOC_REQBUFS");
	}

	if (req.count != NBUF_DRIVER)
		errno_exit("Too many buffers requested");
	 */

	v4l2_create_buffers create;
	CLEAR(create);
	create.count = NBUF;// - NBUF_DRIVER;
	create.format = fmt;
	create.memory = V4L2_MEMORY_USERPTR;
	create.index = 0; // todo  check!
	if(-1 == xioctl(fd, VIDIOC_CREATE_BUFS, &create))
		errno_exit("VIDIOC_CREATE_BUFS");

	buffers = (buffer_t*) calloc(NBUF, sizeof(buffer_t));

	for (uint i = 0; i < NBUF; ++i) {
		buffer_t buf;
		buf.length = buffer_size;
		buf.start = malloc(buffer_size);
		if (!buf.start)
			errno_exit("Out of memory");
		buffers[i] = buf;
	}
}

void CaptureVideo::startCapture() {
	enum v4l2_buf_type type;
	// Enqueue all buffers at start
	qbuf_num = 0;
	for (uint i = 0; i < NBUF; ++i) {

		buffer_t& b = buffers[i];
		//cout << "buffer start " << b.start << " length " << b.length << endl;

		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.index = i;	// index is transmitted here
		buf.m.userptr = (ulong) b.start;
		buf.length = b.length;

//		cout << "...Enqueuing " << buf.index
//				<< ", start addr " << (void*)buf.m.userptr
//				<< ", length " << buf.length
//				<< endl;

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
		qbuf_num++;
	}
	// Start streaming
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
		errno_exit("VIDIOC_STREAMON");

	capturing = true;

	// Start main loop in a separate thread
	threadCapture = std::thread(&CaptureVideo::mainloop, this);
}

void CaptureVideo::stopCapture()
{
	enum v4l2_buf_type type;

	// Stop streaming
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
		errno_exit("VIDIOC_STREAMOFF");

	capturing = false;
	threadCapture.join();
}

void CaptureVideo::mainloop() {
	uint count = 0;
	cout << "Starting main loop (CaptureVideo)" << endl;

	struct v4l2_buffer v4l2_buf;
	CLEAR(v4l2_buf);
	v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf.memory = V4L2_MEMORY_USERPTR;

	buffer_t buf;

	while (capturing) {
		if (fifo_in.pop(buf)) {					// A buffer has been returned in the queue
			v4l2_buf.m.userptr = (ulong) buf.start;
			v4l2_buf.length = buffer_size;
			if (-1 == xioctl(fd, VIDIOC_QBUF, &v4l2_buf))	// pass one buffer to the driver
				errno_exit("VIDIOC_QBUF");
			qbuf_num++;
		}
		if (qbuf_num == 0)
			this_thread::sleep_for(milliseconds(10));
		while (qbuf_num > 0) {
			if (-1 == xioctl(fd, VIDIOC_DQBUF, &v4l2_buf))	// dequeue one buffer (blocking)
				errno_exit("VIDIOC_DQBUF");
			qbuf_num--;
			buf.start = (void*) v4l2_buf.m.userptr;
			buf.length = v4l2_buf.bytesused;
			//cout << "fifo_out push, buf " << buf.start << ", " << buf.length << endl;
			fifo_out.push(buf);
			printFramerate();
		}
		count++;
	}

}

void CaptureVideo::close_device(void)
{
	if (-1 == close(fd))
		errno_exit("close");
	fd = -1;
}

bool CaptureVideo::getFrame(void** ptrData, size_t* size) {
	static buffer_t buf { nullptr, 0 };
	buffer_t latest[2];
	int i = 0;

	if (buf.start != nullptr) {
		fifo_in.push(buf);
		buf.start = nullptr;
	}

	if (fifo_out.pop(latest[i])) {
		i = !i;
		while(fifo_out.pop(latest[i])) {
			i = !i;
			fifo_in.push(latest[i]);	// pass all the accumulated buffers to fifo_in
		}
		buf = latest[!i];
		//cout << "fifo_out pop, buf " << buf.start << ", " << buf.length << endl;
		*ptrData = buf.start;
		*size = buf.length;
		return true;
	}

	return false;
}


void CaptureVideo::printFramerate() {
	static std::chrono::time_point<std::chrono::high_resolution_clock> t0 = std::chrono::high_resolution_clock::now();
	static uint frame = 0;
	frame++;
	double dt = std::chrono::duration_cast<std::chrono::duration<double>> (std::chrono::high_resolution_clock::now() - t0).count();
	if (dt > 2.0) {
		printf("[CaptureVideo] %d frames in %f seconds... FPS: %f\n", frame, dt, frame/dt);
		frame = 0;
		t0 = std::chrono::high_resolution_clock::now();
	}
}
