#ifndef __VIDEOGRABBER_PHOTOBOOTH_H
#define __VIDEOGRABBER_PHOTOBOOTH_H

#include "framelistener.h"
#include "videocontrol.h"
#include "jthread.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

enum jcapture_method_t {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

struct buffer {
	void *start;
	size_t length;
};

class VideoGrabber : public jthread::Thread {

	private:
		VideoControl *_video_control;
		FrameListener *_listener;
		std::string _device;
		jcapture_method_t _method;
		int _handler;
		struct buffer *buffers;
		unsigned int n_buffers;
		int out_buf;
		int xres;
		int yres;
		bool _running;
		pixelformat_t _pixelformat;

	private:
		void InitBuffer(unsigned int buffer_size);

		void InitSharedMemory();

		void InitUserPtr(unsigned int buffer_size);

		void ReleaseDevice();

		int GetFrame();

	public:
		VideoGrabber(FrameListener *listener, std::string device);

		virtual ~VideoGrabber();

		virtual void ExceptionHandler(std::string msg);

		virtual void Open();
		
		virtual void Configure(int width, int height);

		virtual void Start();

		virtual void Stop();

		virtual VideoControl * GetVideoControl();

		virtual void ProcessFrame(const uint8_t *buffer, int size, pixelformat_t format);

		virtual void Run();

};

#endif
