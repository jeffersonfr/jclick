#ifndef __MAINFRAME_PHOTOBOOTH_H
#define __MAINFRAME_PHOTOBOOTH_H

#include "camerasettings.h"
#include "animation.h"
#include "levelframe.h"
#include "menuframe.h"

#include "jgui/japplication.h"
#include "jgui/jwindow.h"
#include "jevent/jframegrabberlistener.h"
#include "jipc/jremotecalllistener.h"
#include "jmedia/jplayer.h"
#include "jmedia/jvideodevicecontrol.h"

#include <thread>
#include <mutex>
#include <condition_variable>

class MainFrame : public jgui::Window, public jevent::FrameGrabberListener, public jipc::RemoteCallListener {

	private:
		std::vector<jgui::Image *> _borders;
		std::vector<jgui::Image *> _shutter_frames;
		std::vector<jgui::Image *> _timeline_frames;
		std::vector<jgui::Image *> _loading_frames;
    std::thread _thread;
		std::mutex _mutex;
    std::condition_variable _sem_lock;
    std::condition_variable _sem_release;
		jgui::Container *_current;
		MenuFrame *_menu_frame;
		LevelFrame *_level_frame;
		jmedia::Player *_audio_player;
		jmedia::Player *_grabber_player;
		Animation *_animation;
		jgui::Image *_frame;
		jgui::Image *_screensaver;
		jgui::jsize_t _screen;
		jgui::jregion_t _wregion;
		jgui::jregion_t _fregion;
		jgui::jregion_t _cregion;
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

		virtual void FrameGrabbed(jevent::FrameGrabberEvent *event);

		virtual void ReleaseFrame();

		virtual void LoadResources();

		virtual void StartGrabber();
		virtual void StopGrabber();

		virtual jgui::jregion_t GetFrameBounds();
		virtual jgui::jregion_t GetViewportBounds();

		virtual void RandomBorder();
		virtual void NextBorder();
		virtual void PreviousBorder();
		virtual void StartShutter();
		virtual void StopShutter();
		virtual void ToogleBlackAndWhite();

		virtual void ShowControlStatus(jmedia::jvideo_control_t id);
		virtual int GetControlValue(jmedia::jvideo_control_t id);
		virtual void SetControlValue(jmedia::jvideo_control_t id, int value);
		virtual void ResetControlValues();
		
		virtual void Paint(jgui::Graphics *g);

		virtual bool KeyPressed(jevent::KeyEvent *event);
		virtual bool MousePressed(jevent::MouseEvent *event);
		virtual bool MouseWheel(jevent::MouseEvent *event);

		virtual void Run();

};

#endif
