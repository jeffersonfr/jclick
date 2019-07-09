#include "slideanimation.h"
#include "config.h"
#include "painter.h"

#include "jcommon/jstringutils.h"
#include "jgui/jbufferedimage.h"
#include "jexception/joutofmemoryexception.h"

#include <stdio.h>

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#define SLIDEANIMATION_STEP	32

SlideAnimation::SlideAnimation(std::vector<std::string> images):
	Animation()
{
	jgui::jsize_t<int> screen; // TODO:: = jgui::GFXHandler::GetInstance()->GetScreenSize();
	jgui::jinsets_t crop = __C->GetSourceCrop();
	std::string temporary = __C->GetTempPath();
	camera_greetings_t greetings = __C->GetCameraGreetings();
	int count = __C->GetThumbsCount();
	int iw = 16;
	int ih = 16;

	_frames = NULL;
	_state = 0;
	_progress = 0;

	_color = jgui::jcolor_t(jcommon::StringUtils::ToLower(greetings.bgcolor));

	jgui::jsize_t<int> scale = screen;

	_offscreen = new jgui::BufferedImage(jgui::JPF_ARGB, scale);

	screen.width = (screen.width - crop.left - crop.right) / 2;
	screen.height = (screen.height - crop.top - crop.bottom) / 2;

	int fw = count*(screen.width+SLIDEANIMATION_STEP);
	int fh = screen.height;

	_frames = new jgui::BufferedImage(jgui::JPF_ARGB, {fw, fh});

	if (_frames == NULL) {
		throw jexception::OutOfMemoryException(
				jcommon::StringUtils::Format("Cannot allocate the suface %dx%d", fw, fh));
	}

	jgui::Graphics *g = _frames->GetGraphics();
  jgui::jsize_t<int> fsize = _frames->GetSize();
	
	g->SetColor(_color);
	g->FillRectangle({0, 0, fsize.width, fsize.height});

	for (int i=0; i<count; i++) {
		jgui::Image *image = new jgui::BufferedImage(temporary + "/" + images[i]);
		jgui::jsize_t<int> size = image->GetSize();
	
		int iw = size.width;
		int ih = size.height;

		if (iw > screen.width) {
			iw = screen.width;
			ih = (size.height * screen.width)/size.width;
		}

		if (ih > screen.height) {
			ih = screen.height;
			iw = (size.width * screen.height)/size.height;
		}

		g->DrawImage(image, {i*(screen.width+SLIDEANIMATION_STEP)+(screen.width-iw)/2, (screen.height-ih)/2, iw, ih});

		delete image;
	}
}

SlideAnimation::~SlideAnimation()
{
	delete _frames;
	delete _offscreen;
}

bool SlideAnimation::Paint(jgui::Component *cmp, jgui::Graphics *g)
{
	camera_animation_t animation = __C->GetCameraAnimation();
	camera_greetings_t greetings = __C->GetCameraGreetings();
  jgui::jsize_t<int> size = cmp->GetSize();
	int delay = animation.delay;
	int gap = 0; // (int)(cmp->GetHeight()*0.1);

	if (_state == 0) {
		_progress = _progress + SLIDEANIMATION_STEP;

		if (_progress > 0xff) {
			_state = 1;
			_progress = size.width;
			
			_color(3, 0xff);
		}

		_color(3, _progress);

		Painter::DrawBox(g, _color, 0, gap, size.width, size.height-2*gap);
	} else if (_state == 1) {
    jgui::jsize_t<int> fsize = _frames->GetSize();

		_progress = _progress - SLIDEANIMATION_STEP;

		if (_progress < -fsize.width) {
			_state = 2;
			_progress = greetings.timeout;
		}
		
		if (_state == 1) {
			_offscreen->GetGraphics()->SetColor(_color);
			_offscreen->GetGraphics()->Clear();
			_offscreen->GetGraphics()->DrawImage(_frames, jgui::jpoint_t<int>{_progress, 0});

			g->DrawImage(_offscreen, jgui::jpoint_t<int>{0, (size.height-fsize.height)/2});
		}
	} else if (_state == 2) {
		_state = 3;
		_progress = 0xff;
	
		Painter::DrawBox(g, _color, 0, gap, size.width, size.height-2*gap);

		jgui::Image *image = new jgui::BufferedImage(greetings.background);

		g->DrawImage(image, {0, 0, size.width, size.height-2*gap});

		delete image;

		/*
		greetings.margin.left = CLAMP(greetings.margin.left, 0, 48);
		greetings.margin.top = CLAMP(greetings.margin.top, 0, 48);
		greetings.margin.right = CLAMP(greetings.margin.right, 0, 48);
		greetings.margin.bottom = CLAMP(greetings.margin.bottom, 0, 48);
		*/

		int x = (size.width*greetings.margin.left)/100;
		int y = (size.height*greetings.margin.top)/100;
		int w = size.width-(size.width*greetings.margin.right)/100-x;
		int h = size.height-(size.height*greetings.margin.bottom)/100-y;

		Painter::DrawString(g, 1, 1, jgui::jcolor_t<float>(greetings.fgcolor), x, y, w, h, greetings.message);

		// TODO:: g->Flip();
		
		sleep(greetings.timeout);
	} else if (_state == 3) {
		_progress = _progress - SLIDEANIMATION_STEP;

		if (_progress <= 0) {
			return false;
		}

		Painter::DrawBox(g, _color, 0, gap, size.width, size.height-2*gap);
	}

	usleep(delay*1000);

	return true;
}

