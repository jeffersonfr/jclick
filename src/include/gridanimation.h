#ifndef __GRIDANIMATION_PHOTOBOOTH_H
#define __GRIDANIMATION_PHOTOBOOTH_H

#include "animation.h"
#include "jcolor.h"

class GridAnimation : public Animation {

	private:
		jgui::Image *_frames;
		jgui::Color _color;
		int _state;
		int _progress;

	public:
		GridAnimation();

		virtual ~GridAnimation();

		virtual bool Paint(jgui::Component *cmp, jgui::Graphics *g);

};

#endif
