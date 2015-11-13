#ifndef __CAMERASETTINGS_PHOTOBOOTH_H
#define __CAMERASETTINGS_PHOTOBOOTH_H

#include "jruntimeexception.h"
#include "jparammapper.h"
#include "jstringtokenizer.h"
#include "jproperties.h"
#include "jparammapper.h"
#include "jgraphics.h"

struct camera_input_t {
	std::string id;
	bool is_local;
};

struct camera_animation_t {
	std::string type;
	int delay;
};

struct camera_photo_t {
	jgui::jregion_t region;
	int degrees;
};

struct camera_compose_t {
	jgui::jsize_t size;
	std::string color;
	std::string image;
	int alpha;
	int degrees;
	bool over;
	bool aspect;
};

struct camera_shutter_t {
	std::string type;
	std::string color;
	std::string file;
	std::string sound;
	int step;
	int delay;
	int range_min;
	int range_max;
};

struct camera_shutter_timeline_t {
	std::string image;
	std::string color;
	jgui::jsize_t size;
};

struct camera_greetings_t {
	jgui::jinsets_t margin;
	std::string bgcolor;
	std::string fgcolor;
	std::string loading;
	std::string background;
	std::string message;
	int timeout;
};

enum camera_viewport_aspect {
	CVA_KEEP,
	CVA_FULL
};

class CameraSettings : public jcommon::ParamMapper {

	private:
		static CameraSettings *_instance;

		std::vector<camera_photo_t> _image_regions;
		camera_input_t _camera_input;
		camera_animation_t _camera_animation;
		camera_compose_t _image_compose;
		camera_shutter_t _camera_shutter;
		camera_shutter_timeline_t _camera_timeline;
		camera_greetings_t _camera_greetings;
		jgui::jinsets_t _image_crop;
		jgui::jinsets_t _camera_viewport;
		jgui::jsize_t _camera_mode;
		std::string _config_file;
		std::string _image_format;
		int _camera_delay;
		int _camera_interval;
		int _image_count;

	private:
		CameraSettings();

		std::string GetFullPath(std::string path);

		std::map<std::string, std::string> GetParams(std::string str);

	public:
		virtual ~CameraSettings();

		static CameraSettings * GetInstance();

		void LoadConfiguration(std::string file);
		std::string GetFramesPath();
		std::string GetPhotosPath();
		std::string GetTempPath();
		std::string GetResourcesPath();
		std::string GetFrameSelection();
		std::string GetImageFormat();
		std::string GetScreenSaver();
		std::string GetSystemLanguage();
		bool IsOptimized();
		int GetCameraDelay();
		int GetCameraInterval();
		int GetThumbsCount();
		bool GetCameraAutoExposure();
		camera_animation_t & GetCameraAnimation();
		camera_shutter_t & GetCameraShutter();
		camera_shutter_timeline_t & GetCameraShutterTimeLine();
		camera_greetings_t & GetCameraGreetings();
		camera_input_t & GetCameraInput();
		jgui::jsize_t & GetCameraMode();
		jgui::jinsets_t & GetCameraViewport();
		camera_viewport_aspect GetCameraViewportAspect();
		bool GetCameraViewportFlip();
		jgui::jinsets_t & GetSourceCrop();
		std::vector<camera_photo_t> & GetDestinationRegions();
		camera_compose_t & GetComposition();

		void SetSystemLanguage(std::string language);
		void SetCameraViewport(jgui::jinsets_t insets);
		void SetCameraViewportAspect(std::string aspect);
		void SetSourceCrop(jgui::jinsets_t insets);
		void SetImageFormat(std::string format);

};

jgui::jinsets_t ScreenToVirtual(jgui::jsize_t screen, jgui::jsize_t scale, jgui::jinsets_t t);
jgui::jregion_t ScreenToVirtual(jgui::jsize_t screen, jgui::jsize_t scale, jgui::jregion_t t);

jgui::jinsets_t VirtualToScreen(jgui::jsize_t screen, jgui::jsize_t scale, jgui::jinsets_t t);
jgui::jregion_t VirtualToScreen(jgui::jsize_t screen, jgui::jsize_t scale, jgui::jregion_t t);

#endif
