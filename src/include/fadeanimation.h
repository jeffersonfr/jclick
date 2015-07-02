#ifndef __FADEANIMATION_PHOTOBOOTH_H
#define __FADEANIMATION_PHOTOBOOTH_H

#include "animation.h"
#include "jimage.h"
#include "jcolor.h"

#include <vector>

class FadeAnimation : public Animation {

	private:
		std::vector<jgui::Image *> _frames;
		jgui::Color _color;
		int _state;
		int _index;
		int _progress;

	public:
		FadeAnimation();

		virtual ~FadeAnimation();

		virtual bool Paint(jgui::Component *cmp, jgui::Graphics *g);

};

#endif