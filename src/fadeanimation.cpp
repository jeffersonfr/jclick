#include "fadeanimation.h"
#include "config.h"
#include "jimage.h"
#include "jstringutils.h"
#include "painter.h"

#include <stdio.h>

#define FADEANIMATION_STEP	16

FadeAnimation::FadeAnimation(std::vector<std::string> images):
	Animation()
{
	jgui::jsize_t screen = jgui::GFXHandler::GetInstance()->GetScreenSize();
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
		jgui::Image *frame = jgui::Image::CreateImage(temporary + "/" + images[i]);

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

		Painter::DrawBox(g, _color, 0, gap, cmp->GetWidth(), cmp->GetHeight()-2*gap);
	} else if (_state == 1) {
		_progress = _progress - FADEANIMATION_STEP/2;

		Painter::DrawBox(g, _color, 0, gap, cmp->GetWidth(), cmp->GetHeight()-2*gap);

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

		g->DrawImage(blend, (cmp->GetWidth()-img->GetWidth())/2, (cmp->GetHeight()-img->GetHeight())/2, img->GetWidth(), img->GetHeight());

		delete blend;

		if (_progress < 0x80) {
			if ((_index+1) < _frames.size()) {
				jgui::Image *img = _frames[_index+1];

				blend = img->Blend((0xff-_progress)/255.0);

				g->DrawImage(blend, (cmp->GetWidth()-img->GetWidth())/2, (cmp->GetHeight()-img->GetHeight())/2, img->GetWidth(), img->GetHeight());

				delete blend;
			}
		}
	} else if (_state == 2) {
		_state = 3;
		_progress = 0xff;
	
		Painter::DrawBox(g, _color, 0, gap, cmp->GetWidth(), cmp->GetHeight()-2*gap);

		jgui::Image *image = jgui::Image::CreateImage(greetings.background);
		
		g->DrawImage(image, 0, 0, cmp->GetWidth(), cmp->GetHeight()-2*gap);

		delete image;

		Painter::DrawString(g, 1, 1, jgui::Color(greetings.fgcolor), 0, 0, cmp->GetWidth(), cmp->GetHeight(), greetings.message);

		g->Flip();
		
		sleep(greetings.timeout);
	} else if (_state == 3) {
		_progress = _progress - FADEANIMATION_STEP;

		if (_progress <= 0) {
			return false;
		}

		_color.SetAlpha(_progress);

		Painter::DrawBox(g, _color, 0, gap, cmp->GetWidth(), cmp->GetHeight()-2*gap);
	}
	
	usleep(delay*1000);

	return true;
}

