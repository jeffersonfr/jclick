#include "fadeanimation.h"
#include "config.h"
#include "painter.h"

#include "jgui/jbufferedimage.h"
#include "jcommon/jstringutils.h"

#include <stdio.h>

#define FADEANIMATION_STEP	16

FadeAnimation::FadeAnimation(std::vector<std::string> images):
	Animation()
{
	jgui::jsize_t screen; // TODO:: = jgui::GFXHandler::GetInstance()->GetScreenSize();
	jgui::jinsets_t crop = __C->GetSourceCrop();
	std::string temporary = __C->GetTempPath();
	camera_greetings_t greetings = __C->GetCameraGreetings();
	int count = images.size(); // __C->GetThumbsCount();
	int 
		iw = 16, 
		ih = 16;

	_state = 0;
	_progress = 0; 
	_index = 0;

	_color = jgui::Color(jcommon::StringUtils::ToLower(greetings.bgcolor));

	screen.width = (screen.width - crop.left - crop.right) / 2;
	screen.height = (screen.height - crop.top - crop.bottom) / 2;

	for (int i=0; i<count; i++) {
		jgui::Image *frame = new jgui::BufferedImage(temporary + "/" + images[i]);

		_frames.push_back(frame);
	}
}

FadeAnimation::~FadeAnimation()
{
	while (_frames.size() > 0) {
		jgui::Image *image = (*_frames.begin());

		_frames.erase(_frames.begin());

		delete image;
	}
}

bool FadeAnimation::Paint(jgui::Component *cmp, jgui::Graphics *g)
{
	std::string temporary = __C->GetTempPath();
	camera_animation_t animation = __C->GetCameraAnimation();
	camera_greetings_t greetings = __C->GetCameraGreetings();
	int delay = animation.delay;
	int count = __C->GetThumbsCount();
	int gap = 0; // (int)(cmp->GetHeight()*0.1);

	if (_state == 0) {
		_progress = _progress + FADEANIMATION_STEP;

		if (_progress > 0xff) {
			_state = 1;
			_progress = 0xff;
			
			_color.SetAlpha(0xff);
		}

		_color.SetAlpha(_progress);

    jgui::jsize_t size = cmp->GetSize();

		Painter::DrawBox(g, _color, 0, gap, size.width, size.height-2*gap);
	} else if (_state == 1) {
    jgui::jsize_t size = cmp->GetSize();

		_progress = _progress - FADEANIMATION_STEP/2;

		Painter::DrawBox(g, _color, 0, gap, size.width, size.height-2*gap);

		if (_progress < FADEANIMATION_STEP) {
			_progress = 0xff;
			_index = _index + 1;

			if (_index >= _frames.size()) {
				_state = 2;

				return true;
			}
		}

		jgui::Image *img = _frames[_index];
		jgui::Image *blend = img->Blend(_progress/255.0);
    jgui::jsize_t isize = img->GetSize();
    jgui::jsize_t csize = cmp->GetSize();

		g->DrawImage(blend, (csize.width-isize.width)/2, (csize.height-isize.height)/2, isize.width, isize.height);

		delete blend;

		if (_progress < 0x80) {
			if ((_index+1) < _frames.size()) {
				jgui::Image *img = _frames[_index+1];
        jgui::jsize_t isize = img->GetSize();
        jgui::jsize_t csize = cmp->GetSize();

				blend = img->Blend((0xff-_progress)/255.0);

				g->DrawImage(blend, (csize.width-isize.width)/2, (csize.height-isize.height)/2, isize.width, isize.height);

				delete blend;
			}
		}
	} else if (_state == 2) {
    jgui::jsize_t size = cmp->GetSize();

		_state = 3;
		_progress = 0xff;
	
		Painter::DrawBox(g, _color, 0, gap, size.width, size.height-2*gap);

		jgui::Image *image = new jgui::BufferedImage(greetings.background);
		
		g->DrawImage(image, 0, 0, size.width, size.height-2*gap);

		delete image;

		Painter::DrawString(g, 1, 1, jgui::Color(greetings.fgcolor), 0, 0, size.width, size.height, greetings.message);

		// TODO:: mudar a forma de trabalho
    // g->Flip();
		
		sleep(greetings.timeout);
	} else if (_state == 3) {
		_progress = _progress - FADEANIMATION_STEP;

		if (_progress <= 0) {
			return false;
		}

		_color.SetAlpha(_progress);

    jgui::jsize_t size = cmp->GetSize();

		Painter::DrawBox(g, _color, 0, gap, size.width, size.height-2*gap);
	}
	
	usleep(delay*1000);

	return true;
}

