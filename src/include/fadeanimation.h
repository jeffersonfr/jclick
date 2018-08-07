#ifndef __FADEANIMATION_PHOTOBOOTH_H
#define __FADEANIMATION_PHOTOBOOTH_H

#include "animation.h"

#include "jgui/jimage.h"
#include "jgui/jcolor.h"

#include <vector>

class FadeAnimation : public Animation {

	private:
		std::vector<jgui::Image *> _frames;
		jgui::Color _color;
		int _state;
		int _index;
		int _progress;

	public:
		FadeAnimation(std::vector<std::string> images);

		virtual ~FadeAnimation();

		virtual bool Paint(jgui::Component *cmp, jgui::Graphics *g);

};

#endif
