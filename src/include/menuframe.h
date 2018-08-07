#ifndef __MENUFRAME_PHOTOBOOTH_H
#define __MENUFRAME_PHOTOBOOTH_H

#include "photoframe.h"

#include "jcommon/jparammapper.h"
#include "jevent/jdatalistener.h"
#include "jgui/jcontainer.h"

#include <string>

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

class MenuFrame : public jgui::Container, public jevent::DataListener {

	private:
		std::map<std::string, jgui::Image *> _images;
    std::mutex _mutex;
		jgui::Container *_current;
		MainFrame *_frame;
		PhotoFrame *_photo_frame;
		std::string _state;
		int _index;
		int _camera_index;
		int _image_index;
		int _media_index;
		int _system_index;

	public:
		MenuFrame(MainFrame *frame);

		virtual ~MenuFrame();

		virtual void OnAction(std::string state, std::string id, int options_index);
		virtual void OnSelection(std::string state, std::string id, int options_index);
		virtual void ProcessKeyDown(jevent::jkeyevent_symbol_t key, std::string state, std::string id, int &options_index);
		virtual void DrawOptions(jgui::Graphics *g, std::string title, std::string id, int options_index);
		virtual void PushItem(std::string id, std::string name, int value, int min, int max, item_type_t type);
		virtual void Initialize();
		virtual void DrawMenu(jgui::Graphics *g);
		virtual bool KeyPressed(jevent::KeyEvent *event);
		virtual void Paint(jgui::Graphics *g);
		virtual void DataChanged(jcommon::ParamMapper *params);

};

#endif

