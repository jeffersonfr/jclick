#ifndef __ANIMATION_PHOTOBOOTH_H
#define __ANIMATION_PHOTOBOOTH_H

#include "jcomponent.h"
#include "jgraphics.h"

class Animation {

	private:

	public:
		Animation();

		virtual ~Animation();

		virtual bool Paint(jgui::Component *cmp, jgui::Graphics *g);

};

#endif
