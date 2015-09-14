#include "mainframe.h"
#include "menuframe.h"
#include "levelframe.h"
#include "slideanimation.h"
#include "gridanimation.h"
#include "fadeanimation.h"
#include "painter.h"
#include "config.h"
#include "jsystem.h"
#include "jmessagedialogbox.h"
#include "jlocalipcserver.h"
#include "jremoteipcserver.h"
#include "jstringutils.h"
#include "jplayermanager.h"
#include "jvideodevicecontrol.h"
#include "jdebug.h"

#include <sstream>

#define CAMERA_ENABLED

#define CONTROL_STEP 	2
#define GAPX	16
#define GAPY	16

#define CTRL_DN(id) SetControlValue(id, GetControlValue(id) - CONTROL_STEP)
#define CTRL_UP(id) SetControlValue(id, GetControlValue(id) + CONTROL_STEP)

class RepaintThread : public jthread::Thread {

	private:
		jgui::Component *_component;

	public:
		RepaintThread(jgui::Component *c = NULL)
		{
			_component = c;
		}

		virtual ~RepaintThread()
		{
		}

		virtual void SetComponent(jgui::Component *c)
		{
			_component = c;
		}

		virtual void Run()
		{
			_component->Repaint();
		}

};

RepaintThread _repaint_thread;

MainFrame::MainFrame():
	jgui::Frame(),
	jmedia::FrameGrabberListener(),
	jipc::RemoteCallListener()
{
	_current = NULL;
	_player = NULL;
	_animation = NULL;
	_grabber = NULL;
	_frame = NULL;
	_thumb = 0;
	_border_index = 0;
	_counter = -1;
	_fade = -1;
	_bw = false;
	_view_crop = false;
	_need_repaint;

	_fregion.x = 0;
	_fregion.y = 0;
	_fregion.width = GetWidth();
	_fregion.height = GetHeight();

	_wregion.x = 0;
	_wregion.y = 0;
	_wregion.width = GetWidth();
	_wregion.height = GetHeight();

	Painter::Initialize();

	jgui::jinsets_t insets = GetInsets();

	_theme = new jgui::Theme();

	_theme->SetComponentFont(Painter::GetFont(4));
	_theme->SetComponentBorderSize(1);
	_theme->SetComponentBackgroundColor(0x40, 0x40, 0x40, 0xd0);
	_theme->SetComponentScrollbarColor(0x40, 0x40, 0x40, 0xd0);
	_theme->SetWindowFont(Painter::GetFont(0));
	_theme->SetWindowBorderSize(0);
	_theme->SetWindowBackgroundColor(0x20, 0x20, 0x20, 0xd0);
	_theme->SetWindowInsets(insets.left, insets.top+32, insets.right, insets.bottom);

	jgui::ThemeManager::GetInstance()->SetTheme(_theme);

	LoadResources();

	// INFO:: load "loading" frames
	jgui::Image *loading = jgui::Image::CreateImage(__C->GetResourcesPath() + "/images/loading.png");

	if (loading != NULL) {
		jgui::jsize_t isize = loading->GetSize();
		int n = 10;

		isize.width = isize.width/n;

		for (int i=0; i<n; i++) {
			_loading_frames.push_back(loading->Crop(i*isize.width, 0, isize.width, isize.height));
		}

		delete loading;
	}

	_loading_index = 0;

	SetUndecorated(true);
	SetDefaultExitEnabled(false);
		
	_repaint_thread.SetComponent(this);

	// *((int *)(NULL)) = 0;
}

MainFrame::~MainFrame()
{
	delete _current;
	delete _theme;
	delete _player;
	delete _frame;
	delete _grabber;
	delete _animation;
}

void MainFrame::LoadResources()
{
	// load shutter sound
	if (_player != NULL) {
		delete _player;
		_player = NULL;
	}

	_player = jmedia::PlayerManager::CreatePlayer(__C->GetCameraShutterSound());

	// load frames borders
	std::vector<std::string> files;

	jio::File file(__C->GetFramesPath());

	file.ListFiles(&files);

	jthread::AutoLock lock(&_mutex);

	while (_borders.size() > 0) {
		jgui::Image *image = (*_borders.begin());

		_borders.erase(_borders.begin());

		delete image;
	}

	// CHANGE:: remove this to add a empty frame by default
	// _borders.push_back(NULL);

	for (std::vector<std::string>::iterator i=files.begin(); i!=files.end(); i++) {
		std::string image = __C->GetFramesPath() + "/" + (*i);

		jio::File f(image);

		if (f.GetType() != jio::JFT_DIRECTORY) {
			_borders.push_back(jgui::Image::CreateImage(image));
		}
	}

	camera_shutter_t shutter = __C->CameraSettings::GetCameraShutter();

	for (int i=0; i<_shutter_frames.size(); i++) {
		jgui::Image *image = _shutter_frames[i];

		delete image;
	}

	_shutter_frames.clear();

	if (shutter.type == "image") {
		jgui::Image *image = jgui::Image::CreateImage(shutter.image);

		if (image != NULL) {
			jgui::jsize_t isize = image->GetSize();
			int iw = isize.width/shutter.cols;
			int ih = isize.height/shutter.rows;

			for (int i=0; i<shutter.cols*shutter.rows; i++) {
				int x = fmod(i, isize.width/iw);
				int y = floor(i*iw/isize.width);

				_shutter_frames.push_back(image->Crop(x*iw, y*ih, iw, ih));
			}

			delete image;
		} else {
			JDEBUG(JERROR, "Camera Shutter:: image \"%s\" is corrupted or inexistent\n", shutter.image.c_str());
		}
	}
	
	camera_shutter_timeline_t timeline =  __C->GetCameraShutterTimeLine();

	for (int i=0; i<_timeline_frames.size(); i++) {
		jgui::Image *image = _timeline_frames[i];

		delete image;
	}

	_timeline_frames.clear();

	if (timeline.image.empty() == false) {
		jgui::Image *image = jgui::Image::CreateImage(timeline.image);

		if (image != NULL) {
			jgui::jsize_t isize = image->GetSize();

			isize.width = isize.width/3;

			for (int i=0; i<3; i++) {
				_timeline_frames.push_back(image->Crop(i*isize.width, 0, isize.width, isize.height));
			}

			delete image;
		} else {
			JDEBUG(JERROR, "Camera Shutter Timeline:: image \"%s\" is corrupted or inexistent\n", timeline.image.c_str());
		}
	}
}

jgui::jregion_t MainFrame::GetFrameBounds()
{
	return _fregion;
}

jgui::jregion_t MainFrame::GetViewportBounds()
{
	return _wregion;
}

void MainFrame::ReleaseAll()
{
	exit(0);
}

jipc::Response * MainFrame::ProcessCall(jipc::Method *method)
{
	jipc::Response *response = new jipc::Response();

	response->SetBooleanParam("self", true);

	std::string name = method->GetName();

	std::cout << "remote call:: " << method->what() << std::endl;

	if (name == "help") {
		response->SetTextParam("methods", 
				"help,quit,reset,startShutter,stopShutter,loadConfiguration <path>,nextBorder,previousBorder,toggleBW,{gamma,brightness,contrast,saturation} <value:up,dn>");
	} else if (name == "quit") {
		ReleaseAll();
	} else if (name == "reset") {
		_bw = false;

		ResetControlValues();
	} else if (name == "startShutter") {
		StartShutter();
	} else if (name == "stopShutter") {
		StopShutter();
	} else if (name == "loadConfiguration") {
		std::string path = method->GetTextParam("path");

		try {
			__C->LoadConfiguration(path);

			LoadResources();
		} catch (jcommon::RuntimeException &e) {
			JDEBUG(JERROR, "%s\n", e.what().c_str());
		}
	} else if (name == "nextBorder") {
		NextBorder();
	} else if (name == "previousBorder") {
		PreviousBorder();
	} else if (name == "toggleBW") {
		ToogleBlackAndWhite();
	} else if (name == "gamma") {
		std::string value = method->GetTextParam("value");

		if (value == "up") {
			CTRL_UP(jmedia::JVC_GAMMA);
		} else if (value == "dn") {
			CTRL_DN(jmedia::JVC_GAMMA);
		}
	} else if (name == "brightness") {
		std::string value = method->GetTextParam("value");

		if (value == "up") {
			CTRL_UP(jmedia::JVC_BRIGHTNESS);
		} else if (value == "dn") {
			CTRL_DN(jmedia::JVC_BRIGHTNESS);
		}
	} else if (name == "contrast") {
		std::string value = method->GetTextParam("value");

		if (value == "up") {
			CTRL_UP(jmedia::JVC_CONTRAST);
		} else if (value == "dn") {
			CTRL_DN(jmedia::JVC_CONTRAST);
		}
	} else if (name == "saturation") {
		std::string value = method->GetTextParam("value");

		if (value == "up") {
			CTRL_UP(jmedia::JVC_SATURATION);
		} else if (value == "dn") {
			CTRL_DN(jmedia::JVC_SATURATION);
		}
	} else {
		response->SetBooleanParam("self", false);
	}

	std::cout << "response:: " << response->what() << std::endl;

	return response;
}

void MainFrame::RandomBorder()
{
	_border_index = random() % (int)_borders.size();
}

void MainFrame::NextBorder()
{
	_border_index++;

	if (_border_index >= (int)_borders.size()) {
		_border_index = 0;
	}
}

void MainFrame::PreviousBorder()
{
	_border_index--;

	if (_border_index < 0) {
		_border_index = _borders.size()-1;
	}
}

void MainFrame::InitializeRegions()
{
	jgui::jinsets_t t = __C->GetCameraViewport();
	int tx = (t.left*_wregion.width)/100;
	int ty = (t.top*_wregion.height)/100;
	int tw = _wregion.width-2*tx;
	int th = _wregion.height-2*ty;

	if (__C->GetCameraViewportAspect() == CVA_FULL) {
		_fregion.height = th;
		_fregion.width = tw;
	} else { // CVA_KEEP
		_fregion.height = th;
		_fregion.width = (_fregion.height*_cregion.width)/_cregion.height;

		if (_fregion.width > tw) {
			_fregion.width = tw;
			_fregion.height = (_fregion.width*_cregion.height)/_cregion.width;
		}
	}

	_fregion.y = ty+(_wregion.height-2*ty-_fregion.height)/2;
	_fregion.x = tx+(_wregion.width-2*tx-_fregion.width)/2;
	
	_need_repaint = true;

	Repaint();
}
	
void MainFrame::FrameGrabbed(jmedia::FrameGrabberEvent *event)
{
	if (__C->IsOptimized() == true && (void *)_animation != NULL) {
		Repaint();

		return;
	}

	jgui::jsize_t t = event->GetFrame()->GetSize();

	if (_frame == NULL) {
		_cregion.x = 0;
		_cregion.y = 0;
		_cregion.width = t.width;
		_cregion.height = t.height;

		InitializeRegions();
	} else {
		delete _frame;
		_frame = NULL;
	}

	_mutex.Lock();

	if (__C->GetCameraViewportFlip() == false) {
		_frame = jgui::Image::CreateImage(event->GetFrame());
	} else {
		_frame = event->GetFrame()->Flip(jgui::JFF_HORIZONTAL);
	}

	_mutex.Unlock();

	if (_bw == true) {
		int count = t.width*t.height;
		uint32_t *ptr = new uint32_t[count];

		_frame->GetGraphics()->GetRGBArray(&ptr, 0, 0, t.width, t.height);

		for (int i=0; i<count; i++) {
			uint32_t pixel = ptr[i];
			int r = (pixel >> 16) & 0xff;
			int g = (pixel >> 8) & 0xff;
			int b = (pixel >> 0) & 0xff;

			int y = CLAMP(0.299*r + 0.587*g + 0.114*b, 0, 255);

			ptr[i] = 0xff000000 | (y << 16) | (y << 8) | (y << 0);
		}

		_frame->GetGraphics()->SetRGBArray(ptr, 0, 0, t.width, t.height);

		delete [] ptr;
	}

	if (_repaint_thread.IsRunning() == true) {
		_repaint_thread.WaitThread();
	}

	Repaint();
}

void MainFrame::ReleaseFrame()
{
	jthread::AutoLock lock(&_mutex);

	delete _frame;
	_frame = NULL;

	_need_repaint = true;
}

void MainFrame::Initialize()
{
	_screen = jgui::GFXHandler::GetInstance()->GetScreenSize();

#ifdef CAMERA_ENABLED
	_grabber = jmedia::PlayerManager::CreatePlayer(std::string("v4l2:") + __C->GetTextParam("camera.device")); 

	jgui::jsize_t size = __C->GetCameraMode();

	_grabber->RegisterFrameGrabberListener(this);
	// TODO:: _grabber->Configure(size.width, size.height);
	_grabber->Play();
	
	ResetControlValues();
#endif

	Show();

	camera_input_t t = __C->GetCameraInput();
	jipc::IPCServer *input = NULL;

	if (t.is_local == true) {
		input = new jipc::LocalIPCServer(t.id);
	} else {
		input = new jipc::RemoteIPCServer(atoi(t.id.c_str()));
	}
	
	while (true) {
		try {
			input->WaitCall(this);
		} catch (jcommon::Exception &e) {
			JDEBUG(JWARN, "Connection interrupted: %s\n", e.what().c_str());
		}
	}
}

bool MainFrame::MousePressed(jgui::MouseEvent *event)
{
	if (jgui::Frame::MousePressed(event) == true) {
		return true;
	}

	if (event->GetButton() == jgui::JMB_BUTTON1) {
		StartShutter();
	} else if (event->GetButton() == jgui::JMB_BUTTON2) {
		RandomBorder();
	}

	return true;
}

bool MainFrame::MouseWheel(jgui::MouseEvent *event)
{
	if (jgui::Frame::MouseWheel(event) == true) {
		return true;
	}

	if (event->GetClickCount() > 0) {
		NextBorder();
	} else {
		PreviousBorder();
	}

	return true;
}

bool MainFrame::KeyPressed(jgui::KeyEvent *event)
{
	if (jgui::Frame::KeyPressed(event) == true) {
		return true;
	}

	if ((void *)_animation != NULL) {
		return false;
	}

	if (_current != NULL) {
		_lock_menu = false;

		delete _current;
		_current = NULL;
	}

	if (event->GetSymbol() == jgui::JKS_ENTER) {
		StartShutter();
	} else if (event->GetSymbol() == jgui::JKS_ESCAPE) {
		StopShutter();

		_need_repaint = true;
	} else if (event->GetSymbol() == jgui::JKS_CURSOR_LEFT) {
		PreviousBorder();
	} else if (event->GetSymbol() == jgui::JKS_CURSOR_RIGHT) {
		NextBorder();
	} else if (event->GetSymbol() == jgui::JKS_m || event->GetSymbol() == jgui::JKS_M) {
		_current = new MenuFrame(this);
	} else if (event->GetSymbol() == jgui::JKS_q || event->GetSymbol() == jgui::JKS_Q) {
		ReleaseAll();
	} else if (event->GetSymbol() == jgui::JKS_r || event->GetSymbol() == jgui::JKS_R) {
		_bw = false;

		ResetControlValues();

		LevelFrame::GetInstance()->Hide();
	} else if (event->GetSymbol() == jgui::JKS_t || event->GetSymbol() == jgui::JKS_T) {
		ToogleBlackAndWhite();
	} else if (event->GetSymbol() == jgui::JKS_v || event->GetSymbol() == jgui::JKS_V) {
		if (_view_crop == false) {
			_view_crop = true;
		} else {
			_view_crop = false;
		}
			
		_need_repaint = true;
	} else if (event->GetSymbol() == jgui::JKS_b) {
		CTRL_DN(jmedia::JVC_BRIGHTNESS);
	} else if (event->GetSymbol() == jgui::JKS_B) {
		CTRL_UP(jmedia::JVC_BRIGHTNESS);
	} else if (event->GetSymbol() == jgui::JKS_g) {
		CTRL_DN(jmedia::JVC_GAMMA);
	} else if (event->GetSymbol() == jgui::JKS_G) {
		CTRL_UP(jmedia::JVC_GAMMA);
	} else if (event->GetSymbol() == jgui::JKS_c) {
		CTRL_DN(jmedia::JVC_CONTRAST);
	} else if (event->GetSymbol() == jgui::JKS_C) {
		CTRL_UP(jmedia::JVC_CONTRAST);
	} else if (event->GetSymbol() == jgui::JKS_s) {
		CTRL_DN(jmedia::JVC_SATURATION);
	} else if (event->GetSymbol() == jgui::JKS_S) {
		CTRL_UP(jmedia::JVC_SATURATION);
	}

#ifndef CAMERA_ENABLED
	Repaint();
#endif

	if (_current != NULL) {
		_lock_menu = true;

		_current->Show();
	}

	return true;
}

void MainFrame::StartShutter()
{
		if (IsRunning() == false) {
			_running = true;

			Start();
		}
}

void MainFrame::StopShutter()
{
		_running = false;

		WaitThread();
}

void MainFrame::ToogleBlackAndWhite()
{
		if (_bw == false) {
			_bw = true;
		} else {
			_bw = false;
		}
}

void MainFrame::ShowControlStatus(jmedia::jvideo_control_t id)
{
	if (_lock_menu == true) {
		return;
	}

	std::string str;

	if (id == jmedia::JVC_BRIGHTNESS) {
		str = __L->GetParam("brightness_label");
	} else if (id == jmedia::JVC_CONTRAST) {
		str = __L->GetParam("contrast_label");
	} else if (id == jmedia::JVC_SATURATION) {
		str = __L->GetParam("saturation_label");
	} else if (id == jmedia::JVC_HUE) {
		str = __L->GetParam("hue_label");
	} else if (id == jmedia::JVC_GAMMA) {
		str = __L->GetParam("gamma_label");
	}

	if (str.empty() == false) {
		LevelFrame::GetInstance()->Show(str, GetControlValue(id));
	}
}

int MainFrame::GetControlValue(jmedia::jvideo_control_t id)
{
	jmedia::VideoDeviceControl *control = (jmedia::VideoDeviceControl *)_grabber->GetControl("video.device");
	
	if (control != NULL) {
		return control->GetValue(id);
	}

	return 0;
}

void MainFrame::SetControlValue(jmedia::jvideo_control_t id, int value)
{
	jmedia::VideoDeviceControl *control = (jmedia::VideoDeviceControl *)_grabber->GetControl("video.device");
	
	if (control != NULL) {
		control->SetValue(id, value);
	}

	ShowControlStatus(id);
}

void MainFrame::ResetControlValues()
{
	jmedia::VideoDeviceControl *control = (jmedia::VideoDeviceControl *)_grabber->GetControl("video.device");
	
	if (control != NULL) {
		std::vector<jmedia::jvideo_control_t> controls = control->GetControls();

		for (std::vector<jmedia::jvideo_control_t>::iterator i=controls.begin(); i!=controls.end(); i++) {
			control->Reset(*i);
		}
	}
}

int MainFrame::Command(const char *fmt, ...) 
{
	int tmp_size = 4096;
	char tmp[tmp_size];
	va_list va;

	va_start(va, fmt);
	vsnprintf(tmp, tmp_size-1, fmt, va); tmp[tmp_size] = 0;
	va_end(va);

	return system(tmp);
}

void MainFrame::Paint(jgui::Graphics *g)
{
	jthread::AutoLock lock(&_mutex);

	camera_shutter_t shutter = __C->GetCameraShutter();

	if (_need_repaint == true || __C->IsOptimized() == false || (__C->IsOptimized() == true && _animation == NULL)) {
		if (_need_repaint == true || (__C->IsOptimized() == false && _animation != NULL)) {
			_need_repaint = false;

			g->Clear();
		}

		g->DrawImage(_frame, _fregion.x, _fregion.y, _fregion.width, _fregion.height);

		if (_borders.size() > 0) {
			g->DrawImage(_borders[_border_index], _fregion.x, _fregion.y, _fregion.width, _fregion.height);
		}

		if (_view_crop == true) {
			jgui::jinsets_t insets = __C->GetSourceCrop();
			jgui::jregion_t region = GetFrameBounds();
			int tx = (insets.left*_fregion.width)/100;
			int ty = (insets.top*_fregion.height)/100;
			jgui::jpen_t pen = g->GetPen();

			pen.width = 4;

			g->SetPen(pen);
			g->SetColor(0xf0f00000);
			g->DrawRectangle(
				tx+region.x, 
				ty+region.y, 
				region.width-2*tx, 
				region.height-2*ty
			);
		}
	}

	if (_animation != NULL) {
		if (_animation->Paint(this, g) == false) {
			_need_repaint = true;

			delete _animation;
			_animation = NULL;

			_sem_lock.Notify();

			return;
		}
	}

	if (_fade >= 0) {
		if (shutter.type == "fade") {
			jgui::Color color(shutter.color);

			color.SetAlpha(0xff-_fade);

			g->SetColor(color);
			g->FillRectangle(_fregion.x, _fregion.y, _fregion.width, _fregion.height);

			_fade = _fade + shutter.step;

			if (_fade >= shutter.range_max) {
				_fade = -1;
			}

			usleep(shutter.delay*1000);
		} else if (shutter.type == "image") {
			g->DrawImage(_shutter_frames[_fade], _fregion.x, _fregion.y, _fregion.width, _fregion.height);

			_fade = _fade + shutter.step;

			if (_fade > shutter.range_max) {
				_fade = -1;
			}

			usleep(shutter.delay*1000);
		} 

		if (_fade < 0) {
			_sem_lock.Notify();
		}
	} else {
		if (_counter >= 0) {
			// INFO:: draw photo counter
			camera_shutter_timeline_t t = __C->GetCameraShutterTimeLine();
			int dy = 128;
			int wy = 10;

			g->SetColor(jgui::Color("black"));
			g->FillRectangle(_fregion.x, _fregion.height-dy, _fregion.width, wy);
			g->SetColor(jgui::Color(t.color));
			g->FillRectangle(_fregion.x, _fregion.height-dy+2, _fregion.width, 2);
			g->FillRectangle(_fregion.x, _fregion.height-dy+6, _fregion.width, 2);

			int k = _fregion.width/__C->GetThumbsCount()/2;

			for (int i=0; i<__C->GetThumbsCount(); i++) {
				if (_timeline_frames.size() == 0) {
					g->SetColor(jgui::Color("black"));
					g->FillEllipse(_fregion.x+k, _fregion.height-dy+wy/2, t.size.width/2, t.size.height/2);

					if (i < _thumb) {
						g->SetColor(jgui::Color("green"));
					} else if (i == _thumb) {
						g->SetColor(jgui::Color("red"));
					} else {
						g->SetColor(jgui::Color("black"));
					}

					g->FillEllipse(_fregion.x+k, _fregion.height-dy+wy/2, t.size.width/2, t.size.height/2);
				} else {
					int index = 0;

					if (i < _thumb) {
						index = 2;
					} else if (i == _thumb) {
						index = 1;
					}

					g->DrawImage(_timeline_frames[index], _fregion.x+k-t.size.width/2, _fregion.height-dy+wy/2-t.size.height/2, t.size.width, t.size.height);
				}

				k = k+_fregion.width/__C->GetThumbsCount();
			}

			if (_counter > 0) {
				Painter::DrawString(g, 3, 4, 0xfff0f0f0, 0, 0, _wregion.width, _wregion.height, jgui::JHA_CENTER, jgui::JVA_CENTER, "%d", _counter);
			} else {
				if (_thumb == __C->GetThumbsCount()) {
					jgui::jsize_t t = GetSize();
					int iw = t.width/4;
					int ih = t.height/22;

					// Painter::DrawString(g, 0, 4, 0xfff0f0f0, 0, 0, _wregion.width, _wregion.height, __L->GetParam("mainframe.waiting").c_str());
					g->DrawImage(_loading_frames[_loading_index], (t.width-iw)/2, (t.height-ih)/2, iw, ih);

					_loading_index = (_loading_index+1)%_loading_frames.size();;
				}
			}
		}
	}
}

void MainFrame::Run()
{
	std::vector<camera_photo_t> dst = __C->GetDestinationRegions();
	std::string repository = __C->GetPhotosPath();
	std::string temporary = __C->GetTempPath();
	camera_shutter_t shutter = __C->GetCameraShutter();
	camera_animation_t animation = __C->GetCameraAnimation();
	camera_compose_t compose = __C->GetComposition();
	jgui::jinsets_t crop = __C->GetSourceCrop();

	// create temporary dir
	Command("rm -rf \"%s\"", temporary.c_str());
	Command("mkdir -p \"%s\"", temporary.c_str());
	Command("mkdir -p \"%s\"", repository.c_str());

	// compose process
	if (compose.image != "") {
		if (compose.over == true) {
			Command("convert -size %dx%d xc:none \"%s\" -geometry %dx%d! -composite \"%s/compose.png\"", 
					compose.size.width, compose.size.height, compose.image.c_str(), compose.size.width, compose.size.height, temporary.c_str());
			Command("convert -size %dx%d xc:%s -resize %dx%d! \"%s/background.png\"", 
				compose.size.width, compose.size.height, compose.color.c_str(), compose.size.width, compose.size.height, temporary.c_str());
		} else {
			Command("convert -size %dx%d xc:%s \"%s\" -geometry %dx%d! -composite \"%s/background.png\"", 
					compose.size.width, compose.size.height, compose.color.c_str(), compose.image.c_str(), compose.size.width, compose.size.height, temporary.c_str());
		}
	} else {
		Command("convert -size %dx%d xc:%s -resize %dx%d! \"%s/background.png\"", 
			compose.size.width, compose.size.height, compose.color.c_str(), compose.size.width, compose.size.height, temporary.c_str());
	}

	for (int i=0; i<__C->GetThumbsCount(); i++) {
		_thumb = i;

		// counter
		for (int j=0; j<__C->GetCameraDelay(); j++) {
			_counter = __C->GetCameraDelay()-j;

			sleep(1);

			_need_repaint = true;

			if (_running == false) {
				goto _run_cleanup;
			}
		}

		_thumb = i+1;

		// INFO:: create a image to dump 
		jgui::Image *clone = jgui::Image::CreateImage(_frame);

		if (_borders.size() > 0) {
			clone->GetGraphics()->DrawImage(_borders[_border_index], 0, 0, _frame->GetWidth(), _frame->GetHeight());
		}
		
		jgui::Image *image = clone;

		if (__C->GetCameraViewportFlip() == true) {
			image = clone->Flip(jgui::JFF_HORIZONTAL);

			delete clone;
		}

		image->GetGraphics()->Dump(temporary, "camera");

		delete image;

		_fade = shutter.range_min;
		
		_player->Play();

		_sem_lock.Wait();

		_counter = 0;

		if (__C->GetFrameSelection() == "auto" && i < (__C->GetThumbsCount()-1)) {
			RandomBorder();
		}

		// process images
		int tx = (crop.left*_frame->GetWidth())/100;
		int ty = (crop.top*_frame->GetHeight())/100;

		Command("convert \"%s/camera_%04d.ppm\" -crop %dx%d+%d+%d \"%s/camera_%04d.png\"", 
				temporary.c_str(), i, _cregion.width-2*tx, _cregion.height-2*ty, tx, ty, temporary.c_str(), i);

		if (_running == false) {
			goto _run_cleanup;
		}
		
		if (compose.aspect == true) {
			if (i < dst.size()) {
				camera_photo_t t = dst[i];

				Command("convert -size %dx%d xc:%s \"%s/camera_%04d.png\" -geometry %dx%d -gravity center -composite \"%s/camera_%04d.png\"", 
						t.region.width, t.region.height, compose.color.c_str(), temporary.c_str(), i, t.region.width, t.region.height, temporary.c_str(), i);
			}
		}

		if (_running == false) {
			goto _run_cleanup;
		}
		
		if (i < dst.size()) {
			camera_photo_t t = dst[i];

			Command("convert \"%s/background.png\" -draw \"Translate %d,%d Rotate %d Image Over 0,0 %d,%d '%s/camera_%04d.png'\" \"%s/background.png\"", 
					temporary.c_str(), t.region.x, t.region.y, t.degrees, t.region.width, t.region.height, temporary.c_str(), i, temporary.c_str());
		}

		sleep(__C->GetCameraInterval());

		if (_running == false) {
			goto _run_cleanup;
		}
	}

	_counter = -1;

	if (_running == false) {
		goto _run_cleanup;
	}

	if (animation.type == "slide") {
		_animation = new SlideAnimation();
	} else if (animation.type == "grid") {
		_animation = new GridAnimation();
	} else if (animation.type == "fade") {
		_animation = new FadeAnimation();
	}

	if (compose.over == true) {
		Command("convert \"%s/background.png\" -draw \"Image Over 0,0 %d,%d '%s/compose.png'\" \"%s/background.png\"", temporary.c_str(), compose.size.width, compose.size.height, temporary.c_str(), temporary.c_str());
	}
	
	// apply rotation and alpha
	Command("convert \"%s/background.png\" -rotate %d \"%s/compose.png\"", temporary.c_str(), compose.degrees, temporary.c_str());
	// CHANGE:: -alpha set not working
	// Command("convert \"%s/compose.png\" -rotate %d -alpha set -channel a -evaluate set %d\% \"%s/compose.png\"", temporary.c_str(), compose.degrees, compose.alpha, temporary.c_str());

	Command("convert \"%s/compose.png\" \"%s/compose-`date +\"%%s\"`.%s\"", temporary.c_str(), repository.c_str(), __C->GetImageFormat().c_str());
	// Command("cp \"%s/compose.png\" \"%s/compose-`date +\"%%s\"`.%s\"", temporary.c_str(), repository.c_str(), __C->GetImageFormat().c_str());

	_sem_lock.Wait();

_run_cleanup:

	// delete temporary
	Command("rm -r \"%s\"", temporary.c_str());

	_counter = -1;
}

