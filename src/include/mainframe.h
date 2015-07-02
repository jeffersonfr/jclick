#ifndef __MAINFRAME_PHOTOBOOTH_H
#define __MAINFRAME_PHOTOBOOTH_H

#include "jframe.h"
#include "jthread.h"
#include "jsemaphore.h"
#include "jruntimeexception.h"
#include "jparammapper.h"
#include "jstringtokenizer.h"
#include "jdebug.h"
#include "jproperties.h"
#include "jsystem.h"
#include "jipcserver.h"
#include "jthememanager.h"

#include "framelistener.h"
#include "audioplayer.h"
#include "camerasettings.h"
#include "videograbber.h"
#include "animation.h"
#include "videograbber.h"

#include <directfb.h>

class MainFrame : public jgui::Frame, public jthread::Thread, public FrameListener, public jipc::RemoteCallListener {

	private:
		std::vector<jgui::Image *> _borders;
		std::vector<jgui::Image *> _shutter_frames;
		std::vector<jgui::Image *> _timeline_frames;
		std::vector<jgui::Image *> _loading_frames;
		jthread::Mutex _mutex;
		jthread::Semaphore _sem_lock;
		jthread::Semaphore _sem_release;
		jgui::Window *_current;
		VideoGrabber *_grabber;
		AudioPlayer *_player;
		Animation *_animation;
		jgui::Image *_frame;
		jgui::Theme *_theme;
		jgui::jsize_t _screen;
		jgui::jregion_t _wregion;
		jgui::jregion_t _fregion;
		jgui::jregion_t _cregion;
		uint32_t *_rgb32;
		int _border_index;
		int _loading_index;
		int _thumb;
		int _counter;
		int _fade;
		bool _bw;
		bool _view_crop;
		bool _running;
		bool _lock_menu;
		bool _need_repaint;

	public:
		MainFrame();

		virtual ~MainFrame();

		virtual jipc::Response * ProcessCall(jipc::Method *method);

		virtual int Command(const char *fmt, ...);

		virtual void Initialize();
		virtual void ReleaseAll();

		virtual void InitializeRegions();

		virtual void ProcessFrame(const uint8_t *buffer, int width, int height, pixelformat_t format);
		virtual void ReleaseFrame();

		virtual void LoadResources();

		virtual jgui::jregion_t GetFrameBounds();
		virtual jgui::jregion_t GetViewportBounds();

		virtual void RandomBorder();
		virtual void NextBorder();
		virtual void PreviousBorder();
		virtual void StartShutter();
		virtual void StopShutter();
		virtual void ToogleBlackAndWhite();

		virtual void ShowControlStatus(video_control_t id);
		virtual int GetControlValue(video_control_t id);
		virtual void SetControlValue(video_control_t id, int value);
		virtual void ResetControlValues();
		
		virtual void Paint(jgui::Graphics *g);

		virtual bool KeyPressed(jgui::KeyEvent *event);
		virtual bool MousePressed(jgui::MouseEvent *event);
		virtual bool MouseWheel(jgui::MouseEvent *event);

		virtual void Run();

};

#endif
