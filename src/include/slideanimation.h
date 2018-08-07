#ifndef __SLIDEANIMATION_PHOTOBOOTH_H
#define __SLIDEANIMATION_PHOTOBOOTH_H

#include "animation.h"

#include "jgui/jcolor.h"

class SlideAnimation : public Animation {

	private:
		jgui::Image *_frames;
		jgui::Image *_offscreen;
		jgui::Color _color;
		int _state;
		int _progress;

	public:
		SlideAnimation(std::vector<std::string> images);

		virtual ~SlideAnimation();

		virtual bool Paint(jgui::Component *cmp, jgui::Graphics *g);

};

#endif
