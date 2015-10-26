#ifndef __MENUFRAME_PHOTOBOOTH_H
#define __MENUFRAME_PHOTOBOOTH_H

#include "jpanel.h"
#include "jthread.h"
#include "jsemaphore.h"
#include "jruntimeexception.h"
#include "jparammapper.h"
#include "jstringtokenizer.h"
#include "jdebug.h"
#include "jproperties.h"
#include "jsystem.h"
#include "jipcserver.h"
#include "jdatalistener.h"

enum item_type_t {
	LABEL_ITEM,
	ACTION_ITEM,
	RANGE_ITEM,
	TIME_ITEM,
	LIST_ITEM
};

struct options_t {
	std::string name;
	item_type_t type;
	int value;
	int min;
	int max;
	std::vector<std::string> elements;
};

class MainFrame;

class MenuFrame : public jgui::Panel, public jcommon::DataListener {

	private:
		std::map<std::string, jgui::Image *> _images;
		jthread::Mutex _mutex;
		jgui::Window *_current;
		MainFrame *_frame;
		std::string _state;
		int _index,
				_camera_index,
				_image_index,
				_media_index,
				_system_index;

	public:
		MenuFrame(MainFrame *frame);

		virtual ~MenuFrame();

		virtual void OnAction(std::string state, std::string id, int options_index);
		virtual void OnSelection(std::string state, std::string id, int options_index);
		virtual void ProcessKeyDown(jgui::jkeyevent_symbol_t key, std::string state, std::string id, int &options_index);
		virtual void DrawOptions(jgui::Graphics *g, std::string title, std::string id, int options_index);
		virtual void PushItem(std::string id, std::string name, int value, int min, int max, item_type_t type);
		virtual void Initialize();
		virtual void DrawMenu(jgui::Graphics *g);
		virtual bool KeyPressed(jgui::KeyEvent *event);
		virtual void Paint(jgui::Graphics *g);
		virtual void DataChanged(jcommon::ParamMapper *params);

};

#endif

