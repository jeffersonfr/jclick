#include "mainframe.h"
#include "menuframe.h"
#include "levelframe.h"
#include "slideanimation.h"
#include "gridanimation.h"
#include "fadeanimation.h"
#include "painter.h"
#include "config.h"

#include "jgui/jbufferedimage.h"
#include "jipc/jlocalipcserver.h"
#include "jipc/jremoteipcserver.h"
#include "jmedia/jplayermanager.h"
#include "jmedia/jvideodevicecontrol.h"
#include "jmedia/jvideosizecontrol.h"
#include "jlogger/jloggerlib.h"

#include <sstream>
#include <algorithm>

#include <stdarg.h>

#define CAMERA_ENABLED

#define CONTROL_STEP 	2
#define GAPX	16
#define GAPY	16

#define CTRL_DN(id) SetControlValue(id, GetControlValue(id) - CONTROL_STEP)
#define CTRL_UP(id) SetControlValue(id, GetControlValue(id) + CONTROL_STEP)

class RepaintThread {

	private:
		jgui::Component *_component;
    std::thread _thread;
    bool _is_running;

	public:
		RepaintThread(jgui::Component *c = NULL)
		{
			_component = c;
      _is_running = false;
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
      _is_running = true;

			_component->Repaint();
      
      _is_running = false;
		}

    virtual void Wait()
    {
      if (_is_running == true) {
        _thread.join();
      }
    }

    virtual void Start()
    {
      if (_is_running == true) {
        _thread.join();
      }

      _thread = std::thread(&RepaintThread::Run, this);
    }
};

RepaintThread _repaint_thread;

MainFrame::MainFrame():
	jgui::Window(0, 0, 1280, 720),
	jevent::FrameGrabberListener(),
	jipc::RemoteCallListener()
{
	_current = NULL;
	_audio_player = NULL;
	_grabber_player = NULL;
	_animation = NULL;
	_frame = NULL;
	_thumb = 0;
	_border_index = 0;
	_counter = -1;
	_fade = -1;
	_bw = false;
	_view_crop = false;
	_need_repaint;
	_screensaver = NULL;

  jgui::jsize_t size = GetSize();

	_fregion.x = 0;
	_fregion.y = 0;
	_fregion.width = size.width;
	_fregion.height = size.height;

	_wregion.x = 0;
	_wregion.y = 0;
	_wregion.width = size.width;
	_wregion.height = size.height;

	jgui::jinsets_t insets = GetInsets();

	LoadResources();

	_menu_frame = new MenuFrame(this);
	_level_frame = new LevelFrame(this);

	Add(_menu_frame);
	Add(_level_frame);

	SetBackgroundVisible(false);

	_repaint_thread.SetComponent(this);

	// *((int *)(NULL)) = 0;
}

MainFrame::~MainFrame()
{
	std::string temporary = __C->GetTempPath();

	Command("rm -r \"%s\"", temporary.c_str());

	StopGrabber();
  SetTheme(NULL);

	delete _screensaver;
	delete _menu_frame;
	delete _level_frame;
	delete _current;
	delete _frame;
	delete _audio_player;
	delete _animation;
}

static bool ascending_sort(const std::string &a, const std::string &b)
{
	if (a > b) {
		return false;
	}

	return true;
}

void MainFrame::StartGrabber()
{
#ifdef CAMERA_ENABLED
	if (_grabber_player != NULL) {
		return;
	}

	_grabber_player = jmedia::PlayerManager::CreatePlayer(std::string("v4l2:") + __C->GetTextParam("camera.device")); 

	// TODO:: _grabber_player->Configure(size.width, size.height);
	
	jgui::jsize_t size = __C->GetCameraMode();

	jmedia::VideoSizeControl *control = (jmedia::VideoSizeControl *)_grabber_player->GetControl("video.size");
	
	if (control != NULL) {
		control->SetSize(size.width, size.height);
	}

	_grabber_player->RegisterFrameGrabberListener(this);
	_grabber_player->Play();
	
	ResetControlValues();
#endif
}

void MainFrame::StopGrabber()
{
#ifdef CAMERA_ENABLED
	if (_grabber_player == NULL) {
		return;
	}

	_grabber_player->RemoveFrameGrabberListener(this);

	_grabber_player->Stop();
	delete _grabber_player;
	_grabber_player = NULL;
#endif
}

void MainFrame::LoadResources()
{
	// load frames borders
	std::vector<std::string> files;

	jio::File *file = jio::File::OpenDirectory(__C->GetFramesPath());

	if (file != NULL) {
		file->ListFiles(&files);

		delete file;
		file = NULL;
	}

  _mutex.lock();

	while (_borders.size() > 0) {
		jgui::Image *image = (*_borders.begin());

		_borders.erase(_borders.begin());

		delete image;
	}

	// CHANGE:: remove this to add a empty frame by default
	// _borders.push_back(NULL);

	for (std::vector<std::string>::iterator i=files.begin(); i!=files.end(); i++) {
		std::string image = __C->GetFramesPath() + "/" + (*i);

		file = jio::File::OpenFile(image);

		if (file != NULL) {
			_borders.push_back(new jgui::BufferedImage(image));
		}
	}

	camera_shutter_t shutter = __C->CameraSettings::GetCameraShutter();

	if (_audio_player != NULL) {
		delete _audio_player;
	}

	_audio_player = jmedia::PlayerManager::CreatePlayer(__C->GetCameraShutter().sound);

	for (int i=0; i<_shutter_frames.size(); i++) {
		jgui::Image *image = _shutter_frames[i];

		delete image;
	}

	_shutter_frames.clear();

	if (shutter.type == "image") {
		jio::File *dir = jio::File::OpenDirectory(shutter.file);

		if (dir == NULL) {
			jgui::Image *image = new jgui::BufferedImage(shutter.file);

			if (image != NULL) {
				_shutter_frames.push_back(image);
			} else {
				JDEBUG(JERROR, "Camera Shutter:: image \"%s\" is corrupted or inexistent\n", shutter.file.c_str());
			}
		} else {
			std::vector<std::string> files;

			dir->ListFiles(&files);

			std::sort(files.begin(), files.end(), ascending_sort);

			for (std::vector<std::string>::iterator i=files.begin(); i!=files.end(); i++) {
				jgui::Image *image = new jgui::BufferedImage(shutter.file + "/" + (*i));

				if (image != NULL) {
					_shutter_frames.push_back(image);
				}
			}

			if (_shutter_frames.size() == 0) {
				JDEBUG(JERROR, "Camera Shutter:: image directory \"%s\" does not have any image\n", shutter.file.c_str());
			}

			delete dir;
		}
	}
	
	camera_shutter_timeline_t timeline =  __C->GetCameraShutterTimeLine();

	for (int i=0; i<_timeline_frames.size(); i++) {
		jgui::Image *image = _timeline_frames[i];

		delete image;
	}

	_timeline_frames.clear();

	if (timeline.image.empty() == false) {
		jgui::Image *image = new jgui::BufferedImage(timeline.image);

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
	
	// INFO:: load "loading" frames
	camera_greetings_t greetings = __C->GetCameraGreetings();

	for (int i=0; i<_loading_frames.size(); i++) {
		jgui::Image *image = _loading_frames[i];

		delete image;
	}

	_loading_frames.clear();

	jio::File *dir = jio::File::OpenDirectory(greetings.loading);

	if (dir == NULL) {
		jgui::Image *image = new jgui::BufferedImage(greetings.loading);

		if (image != NULL) {
			_loading_frames.push_back(image);
		} else {
			JDEBUG(JERROR, "Camera Shutter:: image \"%s\" is corrupted or inexistent\n", greetings.loading.c_str());
		}
	} else {
		std::vector<std::string> files;

		dir->ListFiles(&files);

		std::sort(files.begin(), files.end(), ascending_sort);

		for (std::vector<std::string>::iterator i=files.begin(); i!=files.end(); i++) {
			jgui::Image *image = new jgui::BufferedImage(greetings.loading + "/" + (*i));

			if (image != NULL) {
				_loading_frames.push_back(image);
			}
		}

		if (_loading_frames.size() == 0) {
			JDEBUG(JERROR, "Camera Shutter:: image directory \"%s\" does not have any image\n", greetings.loading.c_str());
		}

		delete dir;
	}

	if (_screensaver != NULL) {
		delete _screensaver;
		_screensaver = NULL;
	}

	_screensaver = new jgui::BufferedImage(__C->GetScreenSaver());

	_loading_index = 0;
	
	std::string temporary = __C->GetTempPath();
	camera_compose_t compose = __C->GetComposition();

	// create temporary dir
	Command("mkdir -p \"%s\"", temporary.c_str());
	Command("rm -rf \"%s/camera*\"", temporary.c_str());

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

  _mutex.unlock();
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

	std::cout << "remote call:: " << method->What() << std::endl;

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
		} catch (jexception::RuntimeException &e) {
			JDEBUG(JERROR, "%s\n", e.What().c_str());
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

	std::cout << "response:: " << response->What() << std::endl;

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
	
void MainFrame::FrameGrabbed(jevent::FrameGrabberEvent *event)
{
	if (__C->IsOptimized() == true && (void *)_animation != NULL) {
		Repaint();

		return;
	}

  jgui::Image *frame = reinterpret_cast<jgui::Image *>(event->GetSource());
	jgui::jsize_t t = frame->GetSize();

	_mutex.lock();

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

	if (__C->GetCameraViewportFlip() == false) {
		_frame = dynamic_cast<jgui::Image *>(frame->Clone());
	} else {
		_frame = frame->Flip(jgui::JFF_HORIZONTAL);
	}

	_mutex.unlock();

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

	_repaint_thread.Wait();

	Repaint();
}

void MainFrame::ReleaseFrame()
{
  _mutex.lock();

	delete _frame;
	_frame = NULL;

	_need_repaint = true;
  
  _mutex.unlock();
}

void MainFrame::Initialize()
{
	// TODO:: _screen = jgui::GFXHandler::GetInstance()->GetScreenSize();

	StartGrabber();

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
		} catch (jexception::Exception &e) {
			JDEBUG(JWARN, "Connection interrupted: %s\n", e.What().c_str());
		}
	}
}

bool MainFrame::MousePressed(jevent::MouseEvent *event)
{
	if (jgui::Window::MousePressed(event) == true) {
		return true;
	}

	if (event->GetButton() == jevent::JMB_BUTTON1) {
		StartShutter();
	} else if (event->GetButton() == jevent::JMB_BUTTON2) {
		RandomBorder();
	}

	return true;
}

bool MainFrame::MouseWheel(jevent::MouseEvent *event)
{
	if (jgui::Window::MouseWheel(event) == true) {
		return true;
	}

	if (event->GetClickCount() > 0) {
		NextBorder();
	} else {
		PreviousBorder();
	}

	return true;
}

bool MainFrame::KeyPressed(jevent::KeyEvent *event)
{
	if (_menu_frame->IsVisible() == true && _menu_frame->KeyPressed(event) == true) {
		return true;
	}

	if (_level_frame->IsVisible() == true && _level_frame->KeyPressed(event) == true) {
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

	bool exit = (event->GetSymbol() == jevent::JKS_ESCAPE || event->GetSymbol() == jevent::JKS_EXIT) || event->GetSymbol() == jevent::JKS_BACKSPACE;

	if (event->GetSymbol() == jevent::JKS_ENTER) {
		StartShutter();
	} else if (exit == true) {
		if (_menu_frame->IsVisible() == true) {
			_menu_frame->SetVisible(false);
		} else {
			StopShutter();

			_need_repaint = true;
		}
	} else if (event->GetSymbol() == jevent::JKS_CURSOR_LEFT) {
		PreviousBorder();
	} else if (event->GetSymbol() == jevent::JKS_CURSOR_RIGHT) {
		NextBorder();
	} else if (event->GetSymbol() == jevent::JKS_m || event->GetSymbol() == jevent::JKS_M) {
		_menu_frame->SetVisible(true);
	} else if (event->GetSymbol() == jevent::JKS_q || event->GetSymbol() == jevent::JKS_Q) {
		ReleaseAll();
	} else if (event->GetSymbol() == jevent::JKS_r || event->GetSymbol() == jevent::JKS_R) {
		_bw = false;

		_level_frame->Hide();

		ResetControlValues();
	} else if (event->GetSymbol() == jevent::JKS_t || event->GetSymbol() == jevent::JKS_T) {
		ToogleBlackAndWhite();
	} else if (event->GetSymbol() == jevent::JKS_v || event->GetSymbol() == jevent::JKS_V) {
		if (_view_crop == false) {
			_view_crop = true;
		} else {
			_view_crop = false;
		}
			
		_need_repaint = true;
	} else if (event->GetSymbol() == jevent::JKS_b) {
		CTRL_DN(jmedia::JVC_BRIGHTNESS);
	} else if (event->GetSymbol() == jevent::JKS_B) {
		CTRL_UP(jmedia::JVC_BRIGHTNESS);
	} else if (event->GetSymbol() == jevent::JKS_g) {
		CTRL_DN(jmedia::JVC_GAMMA);
	} else if (event->GetSymbol() == jevent::JKS_G) {
		CTRL_UP(jmedia::JVC_GAMMA);
	} else if (event->GetSymbol() == jevent::JKS_c) {
		CTRL_DN(jmedia::JVC_CONTRAST);
	} else if (event->GetSymbol() == jevent::JKS_C) {
		CTRL_UP(jmedia::JVC_CONTRAST);
	} else if (event->GetSymbol() == jevent::JKS_s) {
		CTRL_DN(jmedia::JVC_SATURATION);
	} else if (event->GetSymbol() == jevent::JKS_S) {
		CTRL_UP(jmedia::JVC_SATURATION);
	} else {
		return false;
	}

#ifndef CAMERA_ENABLED
	Repaint();
#endif

	if (_current != NULL) {
		_lock_menu = true;

		// TODO:: _current->Show();
	}

	return true;
}

void MainFrame::StartShutter()
{
	_level_frame->Hide();

	if (_running == false) {
		_running = true;

		_thread = std::thread(&MainFrame::Run, this);
	}
}

void MainFrame::StopShutter()
{
  if (_running == false) {
    return;
  }

	_running = false;

  _thread.join();
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

	if (str.empty() == false && _menu_frame->IsVisible() == false) {
		_level_frame->Show(str, GetControlValue(id));
	}
}

int MainFrame::GetControlValue(jmedia::jvideo_control_t id)
{
	if (_grabber_player == NULL) {
		return 0;
	}

	jmedia::VideoDeviceControl *control = (jmedia::VideoDeviceControl *)_grabber_player->GetControl("video.device");
	
	if (control != NULL) {
		return control->GetValue(id);
	}

	return 0;
}

void MainFrame::SetControlValue(jmedia::jvideo_control_t id, int value)
{
	jmedia::VideoDeviceControl *control = (jmedia::VideoDeviceControl *)_grabber_player->GetControl("video.device");
	
	if (control != NULL) {
		control->SetValue(id, value);
	}

	ShowControlStatus(id);
}

void MainFrame::ResetControlValues()
{
	jmedia::VideoDeviceControl *control = (jmedia::VideoDeviceControl *)_grabber_player->GetControl("video.device");
	
	if (control != NULL) {
		std::vector<jmedia::jvideo_control_t> controls = control->GetControls();

		for (std::vector<jmedia::jvideo_control_t>::iterator i=controls.begin(); i!=controls.end(); i++) {
			control->Reset(*i);
		}
		
		control->SetValue(jmedia::JVC_AUTO_EXPOSURE, (__C->GetCameraAutoExposure() == false)?0:100);
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
  _mutex.lock();

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

			_sem_lock.notify_all();
		}
		
    _mutex.unlock();

		return;
	}

	if (_fade >= 0) {
		if (shutter.type == "fade") {
			jgui::Color color(shutter.color);

			color.SetAlpha(0xff-_fade);

			g->SetColor(color);
			g->FillRectangle(_fregion.x, _fregion.y, _fregion.width, _fregion.height);
		} else if (shutter.type == "image") {
			shutter.range_max = _shutter_frames.size();

			g->DrawImage(_shutter_frames[_fade], _fregion.x, _fregion.y, _fregion.width, _fregion.height);
		} 

		usleep(shutter.delay*1000);

		_fade = _fade + shutter.step;

		if (_fade >= shutter.range_max) {
			_fade = -1;
		}

		if (_fade < 0) {
			_sem_lock.notify_all();
		}
	} else {
		if (_counter < 0) {
			if (_screensaver != NULL) {
				g->DrawImage(_screensaver, _fregion.x, _fregion.y, _fregion.width, _fregion.height);
			}
	
			jgui::Container::Paint(g);
		} else {
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
    
  _mutex.unlock();
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
	Command("mkdir -p \"%s\"", temporary.c_str());
	Command("mkdir -p \"%s\"", repository.c_str());
	Command("rm -rf \"%s/camera*\"", temporary.c_str());

	std::vector<std::string> images;

	Command("cp \"%s/background.png\" \"%s/image-background.png\"", temporary.c_str(), temporary.c_str());

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

    std::unique_lock<std::mutex> lock(_mutex);

		// INFO:: create a image to dump 
		jgui::Image *clone = dynamic_cast<jgui::Image *>(_frame->Clone());
		jgui::jsize_t size = _frame->GetSize();

		if (_borders.size() > 0) {
			jgui::Graphics *g = clone->GetGraphics();

			g->SetCompositeFlags(jgui::JCF_SRC_OVER);
			g->DrawImage(_borders[_border_index], 0, 0, size.width, size.height);
		}
		
		jgui::Image *image = clone;

		if (__C->GetCameraViewportFlip() == true) {
			image = clone->Flip(jgui::JFF_HORIZONTAL);

			delete clone;
		}

		std::string temp = image->GetGraphics()->Dump(temporary, "camera");

		delete image;

		images.push_back(temp);

		_fade = shutter.range_min;
		
		if (_audio_player != NULL) {
			_audio_player->Play();
		}

		_sem_lock.wait(lock);

		_counter = 0;

		if (__C->GetFrameSelection() == "auto" && i < (__C->GetThumbsCount()-1)) {
			RandomBorder();
		}

		// process images
		int tx = (crop.left*size.width)/100;
		int ty = (crop.top*size.height)/100;

		Command("mogrify -crop %dx%d+%d+%d \"%s/%s\"", 
				_cregion.width-2*tx, _cregion.height-2*ty, tx, ty, temporary.c_str(), temp.c_str());

		if (_running == false) {
			goto _run_cleanup;
		}
		
		if (compose.aspect == true) {
			if (i < dst.size()) {
				camera_photo_t t = dst[i];

				Command("convert -size %dx%d xc:%s \"%s/camera_%04d.png\" -geometry %dx%d -gravity center -composite \"%s/%s\"", 
						t.region.width, t.region.height, compose.color.c_str(), temporary.c_str(), i, t.region.width, t.region.height, temporary.c_str(), temp.c_str());
			}
		}

		if (_running == false) {
			goto _run_cleanup;
		}
		
		if (i < dst.size()) {
			camera_photo_t t = dst[i];

			// INFO:: zindex stacks the frames over each other
			Command("convert \"%s/image-background.png\" -draw \"Translate %d,%d Rotate %d Image Over 0,0 %d,%d '%s/%s'\" \"%s/image-background.png\"", 
					temporary.c_str(), t.region.x, t.region.y, t.degrees, t.region.width, t.region.height, temporary.c_str(), temp.c_str(), temporary.c_str());
		}

		sleep(__C->GetCameraInterval());

		if (_running == false) {
			goto _run_cleanup;
		}
	}

	if (_running == false) {
		goto _run_cleanup;
	}

	if (animation.type == "slide") {
		_animation = new SlideAnimation(images);
	} else if (animation.type == "grid") {
		_animation = new GridAnimation(images);
	} else if (animation.type == "fade") {
		_animation = new FadeAnimation(images);
	}

	_counter = -1;

	if (compose.over == true) {
		Command("convert \"%s/image-background.png\" -draw \"Image Over 0,0 %d,%d '%s/compose.png'\" \"%s/image-background.png\"", temporary.c_str(), compose.size.width, compose.size.height, temporary.c_str(), temporary.c_str());
	}
	
	// apply rotation and alpha
	Command("convert \"%s/image-background.png\" -rotate %d \"%s/image-compose.png\"", temporary.c_str(), compose.degrees, temporary.c_str());
	// CHANGE:: -alpha set not working
	// Command("convert \"%s/compose.png\" -rotate %d -alpha set -channel a -evaluate set %d\% \"%s/image-compose.png\"", temporary.c_str(), compose.degrees, compose.alpha, temporary.c_str());

	Command("convert \"%s/image-compose.png\" \"%s/compose-`date +\"%%s\"`.%s\"", temporary.c_str(), repository.c_str(), __C->GetImageFormat().c_str());
	// Command("cp \"%s/image-compose.png\" \"%s/compose-`date +\"%%s\"`.%s\"", temporary.c_str(), repository.c_str(), __C->GetImageFormat().c_str());

_run_cleanup:

  std::unique_lock<std::mutex> lock(_mutex);

	_sem_lock.wait(lock);

	// delete temporary
	for (int i=0; i<images.size(); i++) {
		// Command("rm -rf \"%s/%s\"", temporary.c_str(), images[i]);
		// printf("rm -rf \"%s/%s\"\n", temporary.c_str(), images[i].c_str());
	}

	Command("rm -rf \"%s/camera\"* \"%s/image-compose.png\" \"%s/image-background.png\"", temporary.c_str(), temporary.c_str(), temporary.c_str());
	// printf("rm -rf \"%s/camera\"* \"%s/image-compose.png\" \"%s/image-background.png\"\n", temporary.c_str(), temporary.c_str(), temporary.c_str());

	_counter = -1;
}

