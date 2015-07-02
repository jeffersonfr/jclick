#ifndef __NETWORKFRAME_PHOTOBOOTH_H
#define __NETWORKFRAME_PHOTOBOOTH_H

#include "jframe.h"
#include "jlabel.h"
#include "jcombobox.h"
#include "jdebug.h"
#include "jgridlayout.h"

class MainFrame;

class NetworkFrame : public jgui::Frame {

	private:
		jgui::GridLayout *_info_layout;
		jgui::Label *_address_label;
		jgui::ComboBox *_address;

	public:
		NetworkFrame(MainFrame *frame);

		virtual ~NetworkFrame();

};

#endif
