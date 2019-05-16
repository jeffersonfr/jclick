#include "camerasettings.h"
#include "config.h"

#include "jgui/jbufferedimage.h"
#include "jcommon/jproperties.h"
#include "jcommon/jstringutils.h"
#include "jcommon/jstringtokenizer.h"
#include "jlogger/jloggerlib.h"
#include "jexception/jruntimeexception.h"

#include <stdlib.h>
#include <fcntl.h>

CameraSettings *CameraSettings::_instance = new CameraSettings();

CameraSettings::CameraSettings()
{
	try {
		LoadConfiguration(PREFERENCES_PATH("system", "conf"));
	} catch (jexception::RuntimeException &e) {
		JDEBUG(JERROR, "%s\n", e.What().c_str());
	}
	
	SetTextParam("config.directory", "/etc/" __LOCAL_MODULE_NAME__ "");

	SetIntegerParam("frame.timeout", 2);
}

CameraSettings::~CameraSettings()
{
}

CameraSettings * CameraSettings::GetInstance()
{
	return _instance;
}

std::map<std::string, std::string> CameraSettings::GetParams(std::string str)
{
	std::map<std::string, std::string> params;
	std::string key, value;
	bool is_key = true, is_string = false;

	for (int i=0; i<str.size(); i++) {
		char c = str[i];

		if (is_key == true) {
			// INFO:: new param
			if (c == ';') {
				key = "";
				value = "";
			} else if (c == ':') {
				is_key = false;
			} else if (isalnum(c) != 0){
				key = key + c;
			}
		} else {
			if (is_string == false) {
				if (c == ';') {
					params[key] = value;

					key = "";
					value = "";
					is_key = true;
					is_string = false;
				} else if (c == '"') {
					is_string = true;
				} else {
					value = value + c;
				}
			} else {
				if (c == '"') {
					params[key] = value;

					key = "";
					value = "";
					is_key = true;
					is_string = false;
				} else {
					value = value + c;
				}
			}
		}
	}

	if (key != "" && value != "") {
		params[key] = value;
	}

	return params;
}

std::string CameraSettings::GetFullPath(std::string path)
{
	if (path[0] == '@') {
		path = std::string(__LOCAL_MODULE_PREFIX__) + path.substr(1);
	}
	
	return path;
}

void CameraSettings::LoadConfiguration(std::string file)
{
	_config_file = file;

	jcommon::Properties p(_config_file);

	p.Load();

	std::map<std::string, std::string> v = p.GetParameters();

	std::string dir = GetTextParam("config.directory");

	RemoveAll();

	SetTextParam("config.directory", dir);

	for (std::map<std::string, std::string>::iterator i=v.begin(); i!=v.end(); i++) {
		SetTextParam(i->first, i->second);
	}

	// load screen saver
	{
		SetTextParam("screen.saver", jcommon::StringUtils::Trim(GetTextParam("screen.saver"), "'\""));
	}

	// load system font name
	{
		SetTextParam("system.font", jcommon::StringUtils::Trim(GetTextParam("system.font"), "'\""));
	}

	// load system language
	{
		SetTextParam("system.language", jcommon::StringUtils::Trim(GetTextParam("system.language"), "'\""));
	}

	// load image format
	{
		SetTextParam("image.format", jcommon::StringUtils::Trim(GetTextParam("image.format"), "'\""));
	}

	// load camera input
	{
		std::map<std::string, std::string> params = GetParams(GetTextParam("camera.input"));

		_camera_input.is_local = true;
		_camera_input.id = "local";

		for (std::map<std::string, std::string>::iterator i=params.begin(); i!=params.end(); i++) {
			if (i->first == "local") {
				_camera_input.is_local = true;
				_camera_input.id = i->second;
			} else if (i->first == "remote") {
				_camera_input.is_local = false;
				_camera_input.id = i->second;
			}
		}
	}

	// load camera mode
	{
		std::map<std::string, std::string> params = GetParams(GetTextParam("camera.mode"));

		_camera_mode.width = -1;
		_camera_mode.height = -1;

		for (std::map<std::string, std::string>::iterator i=params.begin(); i!=params.end(); i++) {
			if (i->first == "size") {
				jcommon::StringTokenizer tokens(i->second, "x");
				
				if (tokens.GetSize() != 2) {
					throw jexception::RuntimeException("Parser failed [camera.mode]: " + GetTextParam("camera.mode"));
				}

				_camera_mode.width = atoi(tokens.GetToken(0).c_str());
				_camera_mode.height = atoi(tokens.GetToken(1).c_str());
			}
		}
	}

	// load camera viewport
	{
		jcommon::StringTokenizer tokens(GetTextParam("camera.viewport"), ",");

		if (tokens.GetSize() != 4) {
			throw jexception::RuntimeException("Parser failed [camera.viewport]: " + GetTextParam("camera.viewport"));
		}

		_camera_viewport.left = atoi(tokens.GetToken(0).c_str());
		_camera_viewport.top = atoi(tokens.GetToken(1).c_str());
		_camera_viewport.right = atoi(tokens.GetToken(2).c_str());
		_camera_viewport.bottom = atoi(tokens.GetToken(3).c_str());
	}

	// load camera viewport aspect
	{
		SetTextParam("camera.viewport.aspect", jcommon::StringUtils::Trim(GetTextParam("camera.viewport.aspect"), "'\""));
	}

	// load camera animation
	{
		std::map<std::string, std::string> params = GetParams(GetTextParam("camera.animation"));

		_camera_animation.type = "slide";
		_camera_animation.delay = 100;

		for (std::map<std::string, std::string>::iterator i=params.begin(); i!=params.end(); i++) {
			if (i->first == "type") {
				std::string type = i->second;

				if (type != "slide" && 
						type != "grid" && 
						type != "fade") {
					throw jexception::RuntimeException("Parser failed [camera.animation]: " + GetTextParam("camera.animation"));
				}

				_camera_animation.type = type;
			} else if (i->first == "delay") {
				_camera_animation.delay = atoi(i->second.c_str());
			}
		}
	}

	// load camera shutter
	{
		std::map<std::string, std::string> params = GetParams(GetTextParam("camera.shutter"));

		_camera_shutter.type = "fade";
		_camera_shutter.color = "white";
		_camera_shutter.file = "";
		_camera_shutter.sound = "";
		_camera_shutter.step = 1;
		_camera_shutter.delay= 0;
		_camera_shutter.range_min = 1;
		_camera_shutter.range_max = 1;

		for (std::map<std::string, std::string>::iterator i=params.begin(); i!=params.end(); i++) {
			if (i->first == "type") {
				_camera_shutter.type = i->second;
			} else if (i->first == "color") {
				_camera_shutter.color = i->second;
			} else if (i->first == "file") {
				_camera_shutter.file = GetFullPath(i->second);
			} else if (i->first == "sound") {
				_camera_shutter.sound = GetFullPath(i->second);
			} else if (i->first == "step") {
				_camera_shutter.step = atoi(i->second.c_str());
			} else if (i->first == "delay") {
				_camera_shutter.delay = atoi(i->second.c_str());
			} else if (i->first == "range") {
				// INFO:: range [1 .. n]
				jcommon::StringTokenizer tokens(i->second, ",");

				if (tokens.GetSize() == 1) {
					_camera_shutter.range_min = atoi(tokens.GetToken(0).c_str())-1;
					_camera_shutter.range_max = atoi(tokens.GetToken(0).c_str())-1;
				} else if (tokens.GetSize() == 2) {
					_camera_shutter.range_min = atoi(tokens.GetToken(0).c_str())-1;
					_camera_shutter.range_max = atoi(tokens.GetToken(1).c_str())-1;
				} else {
					throw jexception::RuntimeException("Parser failed [camera.shutter]: " + GetTextParam("camera.shutter"));
				}
			}
		}
	}

	// load camera timeline
	{
		std::map<std::string, std::string> params = GetParams(GetTextParam("camera.timeline"));

		_camera_timeline.color = "white";
		_camera_timeline.image = "";
		_camera_timeline.size.width = 64;
		_camera_timeline.size.height = 64;

		for (std::map<std::string, std::string>::iterator i=params.begin(); i!=params.end(); i++) {
			if (i->first == "color") {
				_camera_timeline.color = i->second;
			} else if (i->first == "image") {
				_camera_timeline.image = GetFullPath(i->second);
			} else if (i->first == "size") {
				jcommon::StringTokenizer tokens(i->second, "x");
				
				if (tokens.GetSize() != 2) {
					throw jexception::RuntimeException("Parser failed [camera.shutter.timeline]: " + GetTextParam("camera.shutter.timeline"));
				}

				_camera_timeline.size.width = atoi(tokens.GetToken(0).c_str());
				_camera_timeline.size.height = atoi(tokens.GetToken(1).c_str());
			}
		}
	}

	// load camera greetings
	{
		std::map<std::string, std::string> params = GetParams(GetTextParam("camera.greetings"));

		jgui::jinsets_t margin;

		margin.left = 0;
		margin.top = 0;
		margin.right = 0;
		margin.bottom = 0;

		_camera_greetings.margin = margin;
		_camera_greetings.bgcolor = "white";
		_camera_greetings.fgcolor = "black";
		_camera_greetings.loading = "";
		_camera_greetings.background = "";
		_camera_greetings.message = "";
		_camera_greetings.timeout = 32;

		for (std::map<std::string, std::string>::iterator i=params.begin(); i!=params.end(); i++) {
			if (i->first == "margin") {
				jcommon::StringTokenizer tokens(i->second, ",");

				if (tokens.GetSize() != 4) {
					throw jexception::RuntimeException("Parser failed [camera.greetings]:margin: " + GetTextParam("camera.greetings"));
				}

				margin.left = atoi(tokens.GetToken(0).c_str());
				margin.top = atoi(tokens.GetToken(1).c_str());
				margin.right = atoi(tokens.GetToken(2).c_str());
				margin.bottom = atoi(tokens.GetToken(3).c_str());
		
				_camera_greetings.margin = margin;
			} else if (i->first == "bgcolor") {
				_camera_greetings.bgcolor = i->second;
			} else if (i->first == "bgcolor") {
				_camera_greetings.fgcolor = i->second;
			} else if (i->first == "loading") {
				_camera_greetings.loading = GetFullPath(i->second);
			} else if (i->first == "background") {
				_camera_greetings.background = GetFullPath(i->second);
			} else if (i->first == "message") {
				_camera_greetings.message = i->second;
			} else if (i->first == "timeout") {
				_camera_greetings.timeout = atoi(i->second.c_str());
			} else if (i->first == "file") {
				std::string path = GetFullPath(i->second);

				int fd = open(path.c_str(), O_RDONLY);

				if (fd > 0) {
					char tmp[4096];
					int r;

					if ((r = read(fd, tmp, 4096)) > 0) {
						_camera_greetings.message = std::string(tmp, r-1);
					}
				}
			}
		}
	}

	// load image crop
	{
		jcommon::StringTokenizer tokens(GetTextParam("image.crop"), ",");

		if (tokens.GetSize() != 4) {
			throw jexception::RuntimeException("Parser failed [image.crop]: " + GetTextParam("image.crop"));
		}

		_image_crop.left = atoi(tokens.GetToken(0).c_str());
		_image_crop.top = atoi(tokens.GetToken(1).c_str());
		_image_crop.right = atoi(tokens.GetToken(2).c_str());
		_image_crop.bottom = atoi(tokens.GetToken(3).c_str());
	}

	// load destination regions
	{
		jcommon::StringTokenizer tokens(GetTextParam("image.destination"), ";");

		if (tokens.GetSize() == 0) {
			throw jexception::RuntimeException("Parser failed [image.destination]: " + GetTextParam("image.destination"));
		}

		_image_regions.clear();

		for (int i=0; i<tokens.GetSize(); i++) {
			jcommon::StringTokenizer params(tokens.GetToken(i), ",");

			if (params.GetSize() != 4 && params.GetSize() != 5) {
				throw jexception::RuntimeException("Parser failed:: " + tokens.GetToken(i));
			}

			camera_photo_t t;

			t.region.x = atoi(params.GetToken(0).c_str());
			t.region.y = atoi(params.GetToken(1).c_str());
			t.region.width = atoi(params.GetToken(2).c_str());
			t.region.height = atoi(params.GetToken(3).c_str());
			t.degrees = 0;

			if (params.GetSize() == 5) {
				t.degrees = atof(params.GetToken(4).c_str());
			}

			_image_regions.push_back(t);
		}
	}

	// load image composition
	{
		std::map<std::string, std::string> params = GetParams(GetTextParam("image.compose"));

		_image_compose.size.width = -1;
		_image_compose.size.height = -1;
		_image_compose.color = "white";
		_image_compose.image = "";
		_image_compose.over = false;
		_image_compose.alpha = 0xff;
		_image_compose.degrees = 0;
		_image_compose.aspect = false;

		for (std::map<std::string, std::string>::iterator i=params.begin(); i!=params.end(); i++) {
			if (i->first == "size") {
				jcommon::StringTokenizer size(i->second, "x");

				_image_compose.size.width = atoi(size.GetToken(0).c_str());
				_image_compose.size.height = atoi(size.GetToken(1).c_str());
			} else if (i->first == "color") {
				_image_compose.color = i->second;
			} else if (i->first == "image") {
				_image_compose.image = GetFullPath(i->second);
			} else if (i->first == "alpha") {
				_image_compose.alpha = atoi(i->second.c_str());
			} else if (i->first == "degrees") {
				_image_compose.degrees = atoi(i->second.c_str());
			} else if (i->first == "over") {
				_image_compose.over = true;
				
				if (i->second == "false") {
					_image_compose.over = false;
				}
			} else if (i->first == "aspect") {
				_image_compose.aspect = true;
				
				if (i->second == "false") {
					_image_compose.aspect = false;
				}
			}
		}

		int iw = 100;
		int ih = 100;

		if (_image_compose.size.width < 0 || _image_compose.size.width < 0) {
			if (_image_compose.image != "") {
        jgui::Image *image = new jgui::BufferedImage(_image_compose.image);
				jgui::jsize_t<int> size = image->GetSize();

				if (size.width > 0 && size.height > 0) {
					_image_compose.size.width = size.width;
					_image_compose.size.height = size.height;
				} else {
					_image_compose.size.width = iw;
					_image_compose.size.height = ih;
				}

        delete image;
			} else {
				_image_compose.size.width = iw;
				_image_compose.size.height = ih;
			}
		}
	}

	// load camera delay
	{
		_camera_delay = GetIntegerParam("camera.delay");

		if (_camera_delay < 0) {
			throw jexception::RuntimeException("Parser failed [camera.delay]: " + GetTextParam("camera.delay") + " cannot be less than zero");
		}
	}

	// load camera interval
	{
		_camera_interval = GetIntegerParam("camera.interval");

		if (_camera_interval < 0) {
			throw jexception::RuntimeException("Parser failed [camera.interval]: " + GetTextParam("camera.interval") + " cannot be less than zero");
		}
	}

	// load image thumbs
	{
		_image_count = GetIntegerParam("image.thumbs");

		if (_image_count <= 0) {
			throw jexception::RuntimeException("Parser failed [camera.thumbs]: " + GetTextParam("camera.thumbs") + " must be greater than zero");
		}
	}
}

bool CameraSettings::IsOptimized()
{
	return GetBooleanParam("camera.optimize");
}

bool CameraSettings::GetCameraAutoExposure()
{
	return GetBooleanParam("camera.auto_exposure");
}

int CameraSettings::GetCameraDelay()
{
	return _camera_delay;
}

int CameraSettings::GetCameraInterval()
{
	return _camera_interval;
}

int CameraSettings::GetThumbsCount()
{
	return _image_count;
}

std::string CameraSettings::GetFramesPath()
{
	return GetFullPath(GetTextParam("frames.path"));
}

std::string CameraSettings::GetPhotosPath()
{
	return GetFullPath(GetTextParam("photos.path"));
}

std::string CameraSettings::GetTempPath()
{
	return GetTextParam("temp.path");
}

std::string CameraSettings::GetResourcesPath()
{
	return GetFullPath(GetTextParam("resources.path"));
}

std::string CameraSettings::GetFrameSelection()
{
	return GetTextParam("frame.selection");
}

std::string CameraSettings::GetImageFormat()
{
	return GetTextParam("image.format");
}

std::string CameraSettings::GetScreenSaver()
{
	return GetFullPath(GetTextParam("screen.saver"));
}

std::string CameraSettings::GetSystemLanguage()
{
	return GetTextParam("system.language");
}

camera_animation_t & CameraSettings::GetCameraAnimation()
{
	return _camera_animation;
}

camera_shutter_t & CameraSettings::GetCameraShutter()
{
	return _camera_shutter;
}

camera_shutter_timeline_t & CameraSettings::GetCameraShutterTimeLine()
{
	return _camera_timeline;
}

camera_greetings_t & CameraSettings::GetCameraGreetings()
{
	return _camera_greetings;
}

camera_input_t & CameraSettings::GetCameraInput()
{
	return _camera_input;
}

jgui::jsize_t<int> & CameraSettings::GetCameraMode()
{
	return _camera_mode;
}

jgui::jinsets_t & CameraSettings::GetCameraViewport()
{
	return _camera_viewport;
}

camera_viewport_aspect CameraSettings::GetCameraViewportAspect()
{
	std::string aspect = GetTextParam("camera.viewport.aspect");

	if (aspect == "full") {
		return CVA_FULL;
	}

	return CVA_KEEP;
}

bool CameraSettings::GetCameraViewportFlip()
{
	return GetBooleanParam("camera.viewport.flip");
}

jgui::jinsets_t & CameraSettings::GetSourceCrop()
{
	return _image_crop;
}

std::vector<camera_photo_t> & CameraSettings::GetDestinationRegions()
{
	return _image_regions;
}

camera_compose_t & CameraSettings::GetComposition()
{
	return _image_compose;
}

void CameraSettings::SetCameraViewport(jgui::jinsets_t insets)
{
	jcommon::Properties p(_config_file);
	char tmp[255];

	_camera_viewport = insets;

	sprintf(tmp, "%d,%d,%d,%d", insets.left, insets.top, insets.right, insets.bottom);

	try {
		p.Load();
		p.SetTextParam("camera.viewport", tmp);
		p.Save();
	} catch (jexception::RuntimeException &e) {
	}
}

void CameraSettings::SetSourceCrop(jgui::jinsets_t insets)
{
	jcommon::Properties p(_config_file);
	char tmp[255];

	_image_crop = insets;

	sprintf(tmp, "%d,%d,%d,%d", insets.left, insets.top, insets.right, insets.bottom);

	try {
		p.Load();
		p.SetTextParam("image.crop", tmp);
		p.Save();
	} catch (jexception::RuntimeException &e) {
	}
}

void CameraSettings::SetSystemLanguage(std::string language)
{
	jcommon::Properties p(_config_file);

	try {
		p.Load();
		p.SetTextParam("system.language", std::string("\"") + language + "\"");
		p.Save();
	} catch (jexception::RuntimeException &e) {
	}
	
	SetTextParam("system.language", language);
}

void CameraSettings::SetCameraViewportAspect(std::string aspect)
{
	jcommon::Properties p(_config_file);

	try {
		p.Load();
		p.SetTextParam("camera.viewport.aspect", std::string("\"") + aspect + "\"");
		p.Save();
	} catch (jexception::RuntimeException &e) {
	}
	
	SetTextParam("camera.viewport.aspect", aspect);
}

void CameraSettings::SetImageFormat(std::string format)
{
	jcommon::Properties p(_config_file);

	try {
		p.Load();
		p.SetTextParam("image.format", std::string("\"") + format + "\"");
		p.Save();
	} catch (jexception::RuntimeException &e) {
	}
	
	SetTextParam("image.format", format);
}

