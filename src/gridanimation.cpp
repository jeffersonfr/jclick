#include "gridanimation.h"
#include "config.h"

#include "jcommon/jstringutils.h"
#include "jgui/jbufferedimage.h"

#include <stdio.h>

#define GRIDANIMATION_STEP	16

GridAnimation::GridAnimation(std::vector<std::string> images):
	Animation()
{
	jgui::jsize_t<int> screen; // TODO:: = jgui::GFXHandler::GetInstance()->GetScreenSize();
	jgui::jinsets_t crop = __C->GetSourceCrop();
	std::string temporary = __C->GetTempPath();
	int count = images.size(); // __C->GetThumbsCount();
	camera_greetings_t greetings = __C->GetCameraGreetings();
	int
		iw = 16, 
		ih = 16;
	int
		cols = 0,
		rows = 0;
	int gapx = 32;
	char tmp[1024];

	_frames = NULL;
	_state = 0;
	_progress = 0;

	_color = jgui::Color(jcommon::StringUtils::ToLower(greetings.bgcolor));

	screen.width = (screen.width - crop.left - crop.right) / 2;
	screen.height = (screen.height - crop.top - crop.bottom) / 2;

	double root = sqrt(count);

	if (root != (double)((int)root)) {
		cols = (double)(((int)root) + 1);
	}

	for (int i=1; i<=cols; i++) {
		rows = i;
		
		if (count <= i*cols) {
			break;
		}
	}

	/*
	for (int i=0; i<count; i++) {
		sprintf(tmp, "%s/%s", temporary.c_str(), images[i]);

		if ((void *)_frames == NULL) {
			_frames = jgui::Image::CreateImage(count*(screen.width+gapx)-gapx, screen.height, jgui::JPF_ARGB);
		}

		_frames->GetGraphics()->DrawImage(tmp, i*(screen.width+gapx), 0, screen.width, screen.height);
	}
	*/
}

GridAnimation::~GridAnimation()
{
	// delete _frames;
}

bool GridAnimation::Paint(jgui::Component *cmp, jgui::Graphics *g)
{
	camera_animation_t animation = __C->GetCameraAnimation();
	camera_greetings_t greetings = __C->GetCameraGreetings();
	int delay = animation.delay;
	int gap = 0; // (int)(cmp->GetHeight()*0.1);

	// TODO::

	usleep(delay*1000);

	return false;
}

