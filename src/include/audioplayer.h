#ifndef __AUDIOPLAYER_PHOTOBOOTH_H
#define __AUDIOPLAYER_PHOTOBOOTH_H

#include "jmutex.h"

#include <directfb.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

class AudioPlayer {

	private:
		IDirectFBVideoProvider *_provider;
		IDirectFBSurface *_surface;
		jthread::Mutex _mutex;
		std::string _file;
		bool _is_closed;

	public:
		AudioPlayer(std::string file);

		virtual ~AudioPlayer();

		virtual void Play();

};

#endif
