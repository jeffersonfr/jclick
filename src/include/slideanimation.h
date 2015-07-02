#ifndef __SLIDEANIMATION_PHOTOBOOTH_H
#define __SLIDEANIMATION_PHOTOBOOTH_H

#include "animation.h"
#include "jcolor.h"

class SlideAnimation : public Animation {

	private:
		jgui::Image *_frames;
		jgui::Image *_offscreen;
		jgui::Color _color;
		int _state;
		int _progress;

	public:
		SlideAnimation();

		virtual ~SlideAnimation();

		virtual bool Paint(jgui::Component *cmp, jgui::Graphics *g);

};

#endif
