#ifndef __CAMERASETTINGS_PHOTOBOOTH_H
#define __CAMERASETTINGS_PHOTOBOOTH_H

#include "jcommon/jparammapper.h"
#include "jgui/jgraphics.h"

struct camera_input_t {
	std::string id;
	bool is_local;
};

struct camera_animation_t {
	std::string type;
	int delay;
};

struct camera_photo_t {
	jgui::jrect_t<int> region;
	int degrees;
};

struct camera_compose_t {
	jgui::jsize_t<int> size;
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
	jgui::jsize_t<int> size;
};

struct camera_greetings_t {
	jgui::jinsets_t<int> margin;
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
		jgui::jinsets_t<int> _image_crop;
		jgui::jinsets_t<int> _camera_viewport;
		jgui::jsize_t<int> _camera_mode;
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
		jgui::jsize_t<int> & GetCameraMode();
		jgui::jinsets_t<int> & GetCameraViewport();
		camera_viewport_aspect GetCameraViewportAspect();
		bool GetCameraViewportFlip();
		jgui::jinsets_t<int> & GetSourceCrop();
		std::vector<camera_photo_t> & GetDestinationRegions();
		camera_compose_t & GetComposition();

		void SetSystemLanguage(std::string language);
		void SetCameraViewport(jgui::jinsets_t<int> insets);
		void SetCameraViewportAspect(std::string aspect);
		void SetSourceCrop(jgui::jinsets_t<int> insets);
		void SetImageFormat(std::string format);

};

jgui::jinsets_t<int> ScreenToVirtual(jgui::jsize_t<int> screen, jgui::jsize_t<int> scale, jgui::jinsets_t<int> t);
jgui::jrect_t<int> ScreenToVirtual(jgui::jsize_t<int> screen, jgui::jsize_t<int> scale, jgui::jrect_t<int> t);

jgui::jinsets_t<int> VirtualToScreen(jgui::jsize_t<int> screen, jgui::jsize_t<int> scale, jgui::jinsets_t<int> t);
jgui::jrect_t<int> VirtualToScreen(jgui::jsize_t<int> screen, jgui::jsize_t<int> scale, jgui::jrect_t<int> t);

#endif
